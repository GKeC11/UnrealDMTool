import crypto = require("node:crypto");
import http = require("node:http");
import { ConnectionContext, createConnectionContext, WebSocketLike } from "./ConnectionContext";
import { ConnectionManager } from "./ConnectionManager";
import { MessageRouter } from "./MessageRouter";
import { AccountModule } from "./Modules/Account/AccountModule";
import { RoomModule } from "./Modules/Room/RoomModule";
import { ProtocolBase } from "./Protocol/ProtocolDefine";
import { Logger } from "./Utils/Logger";
import { GameMessage, ResponseHelper } from "./Utils/ResponseHelper";

export type GameServerOptions = {
    port?: number;
    path?: string;
    server?: any;
    heartbeatTimeoutMs?: number;
    cleanupIntervalMs?: number;
};

export class GameServer {
    public readonly router = new MessageRouter();
    public readonly connections: ConnectionManager;

    private readonly logger = new Logger("GameServer");
    private readonly accountModule: AccountModule;
    private readonly roomModule: RoomModule;
    private readonly port: number;
    private readonly path: string;
    private readonly server: any;
    private nextConnectionId = 1;

    public constructor(options: GameServerOptions = {}) {
        this.port = options.port ?? 8080;
        this.path = options.path ?? "/game-ws";
        this.server = options.server ?? http.createServer();
        this.connections = new ConnectionManager({
            heartbeatTimeoutMs: options.heartbeatTimeoutMs,
            cleanupIntervalMs: options.cleanupIntervalMs,
            onConnectionClosed: (ctx) => {
                this.roomModule?.handleConnectionClosed(ctx);
                this.logger.info(`Connection cleanup callback. id=${ctx.id}, userId=${ctx.userId || ""}`);
            },
        });
        this.accountModule = new AccountModule(this.router, (ctx, type, payload) => this.sendToClient(ctx, type, payload));
        this.roomModule = new RoomModule(
            this.router,
            (ctx, type, payload) => this.sendToClient(ctx, type, payload),
            () => this.connections.getAll()
        );
    }

    public start(): void {
        this.server.on("connection", (socket: any) => {
            const remoteAddress = String(socket?.remoteAddress || "");
            const remotePort = String(socket?.remotePort || "");
            this.logger.info(`TCP connection accepted. remote=${remoteAddress}:${remotePort}`);
        });
        this.server.on("clientError", (error: Error, socket: WebSocketLike) => {
            this.logger.warn(`HTTP client error before upgrade. message=${error.message}`);
            socket.destroy?.();
        });
        this.server.on("error", (error: Error) => {
            this.logger.error("HTTP server error.", error.message);
        });
        this.server.on("upgrade", (request: any, socket: WebSocketLike) => this.handleUpgrade(request, socket));
        this.connections.startHeartbeatScan();

        if (!this.server.listening) {
            this.server.listen(this.port, () => {
                this.logger.info(`Listening on ws://127.0.0.1:${this.port}${this.path}`);
            });
            return;
        }

        this.logger.info(`Attached to existing HTTP server. path=${this.path}`);
    }

    public stop(): void {
        this.connections.stopHeartbeatScan();
        for (const ctx of this.connections.getAll()) {
            ctx.ws.end?.();
            this.connections.remove(ctx.id);
        }
    }

    public sendToClient(ctx: ConnectionContext, type: number, payload: Record<string, unknown> = {}): void {
        if (ctx.ws.destroyed) {
            this.logger.warn(`Skip send to destroyed connection. id=${ctx.id}, type=${type}`);
            return;
        }

        ctx.ws.write(ResponseHelper.build(type, payload));
        this.logger.debug(`Message sent. id=${ctx.id}, type=${type}`);
    }

    public broadcast(type: number, payload: Record<string, unknown> = {}, filter?: (ctx: ConnectionContext) => boolean): void {
        for (const ctx of this.connections.getAll()) {
            if (filter && !filter(ctx)) {
                continue;
            }

            this.sendToClient(ctx, type, payload);
        }

        this.logger.info(`Broadcast finished. type=${type}, total=${this.connections.getAll().length}`);
    }

    private handleUpgrade(request: any, socket: WebSocketLike): void {
        const requestUrl = String(request.url || "");
        const remoteAddress = String(request.socket?.remoteAddress || socket.remoteAddress || "");
        this.logger.info(`Upgrade requested. url=${requestUrl}, expectedPath=${this.path}, remote=${remoteAddress}`);
        if (!requestUrl.startsWith(this.path)) {
            this.logger.warn(`Upgrade rejected: path mismatch. url=${requestUrl}, expectedPath=${this.path}, remote=${remoteAddress}`);
            socket.destroy?.();
            return;
        }

        const clientKey = String(request.headers?.["sec-websocket-key"] || "");
        if (!clientKey) {
            this.logger.warn(`Upgrade rejected: missing sec-websocket-key. url=${requestUrl}, remote=${remoteAddress}`);
            socket.destroy?.();
            return;
        }

        const upgradeHeader = String(request.headers?.upgrade || "");
        const connectionHeader = String(request.headers?.connection || "");
        const versionHeader = String(request.headers?.["sec-websocket-version"] || "");
        this.logger.debug(`Upgrade headers. upgrade=${upgradeHeader}, connection=${connectionHeader}, version=${versionHeader}, remote=${remoteAddress}`);

        socket.write([
            "HTTP/1.1 101 Switching Protocols",
            "Upgrade: websocket",
            "Connection: Upgrade",
            `Sec-WebSocket-Accept: ${createWebSocketAcceptKey(clientKey)}`,
            "",
            "",
        ].join("\r\n"));

        const ctx = createConnectionContext(createFramedWebSocket(socket), this.createConnectionId());
        this.connections.add(ctx);
        this.logger.info(`WebSocket handshake accepted. id=${ctx.id}, path=${this.path}, remote=${remoteAddress}`);

        socket.on("data", (chunk: unknown) => this.handleSocketData(ctx, chunk));
        socket.on("close", () => {
            this.logger.info(`WebSocket closed. id=${ctx.id}, userId=${ctx.userId || ""}`);
            this.connections.remove(ctx.id);
        });
        socket.on("error", (error: Error) => {
            this.logger.error(`WebSocket error. id=${ctx.id}`, error.message);
            this.connections.remove(ctx.id);
        });
    }

    private handleSocketData(ctx: ConnectionContext, chunk: unknown): void {
        const chunkLength = Buffer.isBuffer(chunk) ? (chunk as any).length : String(chunk ?? "").length;
        this.logger.debug(`Socket data received. id=${ctx.id}, bytes=${chunkLength}`);
        const text = decodeWebSocketTextFrame(chunk);
        if (!text) {
            this.logger.debug(`Socket data ignored: empty or non-text frame. id=${ctx.id}, bytes=${chunkLength}`);
            return;
        }

        let message: GameMessage;
        try {
            message = JSON.parse(text) as GameMessage;
        } catch {
            this.logger.warn(`Message parse failed. id=${ctx.id}, text=${text}`);
            this.sendToClient(ctx, ProtocolBase.ERROR, {
                errorCode: 400,
                message: "Message must be valid JSON",
            });
            return;
        }

        if (message.type === ProtocolBase.PING) {
            this.logger.debug(`Ping received. id=${ctx.id}`);
            this.connections.updateHeartbeat(ctx);
            this.sendToClient(ctx, ProtocolBase.PONG, {
                time: Date.now(),
            });
            return;
        }

        this.logger.debug(`Message received. id=${ctx.id}, type=${message.type}`);
        this.router.dispatch(message.type, ctx, message.payload ?? {}).catch((error: Error) => {
            this.logger.error(`Message dispatch failed. id=${ctx.id}, type=${message.type}`, error.message);
        });
    }

    private createConnectionId(): string {
        const id = `conn_${Date.now()}_${this.nextConnectionId}`;
        this.nextConnectionId += 1;
        return id;
    }
}

function createFramedWebSocket(socket: WebSocketLike): WebSocketLike {
    return {
        get destroyed() {
            return Boolean(socket.destroyed);
        },
        write(data: unknown): void {
            socket.write(encodeWebSocketTextFrame(String(data)));
        },
        end(): void {
            socket.end?.();
        },
        destroy(): void {
            socket.destroy?.();
        },
        on(eventName: string, listener: (...args: any[]) => void): void {
            socket.on(eventName, listener);
        },
    };
}

function createWebSocketAcceptKey(clientKey: string): string {
    return crypto
        .createHash("sha1")
        .update(`${clientKey}258EAFA5-E914-47DA-95CA-C5AB0DC85B11`)
        .digest("base64");
}

function encodeWebSocketTextFrame(text: string): any {
    const payload = Buffer.from(text, "utf8");
    const payloadLength = payload.length;

    if (payloadLength < 126) {
        const frame = Buffer.alloc(2 + payloadLength);
        frame[0] = 0x81;
        frame[1] = payloadLength;
        payload.copy(frame, 2);
        return frame;
    }

    const frame = Buffer.alloc(4 + payloadLength);
    frame[0] = 0x81;
    frame[1] = 126;
    frame.writeUInt16BE(payloadLength, 2);
    payload.copy(frame, 4);
    return frame;
}

function decodeWebSocketTextFrame(chunk: unknown): string | undefined {
    const buffer = Buffer.isBuffer(chunk) ? chunk : Buffer.from(chunk as any);
    if (buffer.length < 2) {
        return undefined;
    }

    const opcode = buffer[0] & 0x0f;
    if (opcode === 0x8) {
        return undefined;
    }

    let offset = 2;
    let payloadLength = buffer[1] & 0x7f;
    const masked = (buffer[1] & 0x80) !== 0;

    if (payloadLength === 126) {
        payloadLength = buffer.readUInt16BE(offset);
        offset += 2;
    } else if (payloadLength === 127) {
        const high = buffer.readUInt32BE(offset);
        const low = buffer.readUInt32BE(offset + 4);
        offset += 8;
        payloadLength = high * 0x100000000 + low;
    }

    let mask: any;
    if (masked) {
        mask = buffer.slice(offset, offset + 4);
        offset += 4;
    }

    const payload = buffer.slice(offset, offset + payloadLength);
    if (masked && mask) {
        for (let index = 0; index < payload.length; index += 1) {
            payload[index] ^= mask[index % 4];
        }
    }

    return payload.toString("utf8");
}
