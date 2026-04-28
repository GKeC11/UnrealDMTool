import { ConnectionContext } from "./ConnectionContext";
import { Logger } from "./Utils/Logger";

export type ConnectionClosedHandler = (ctx: ConnectionContext) => void;

export type ConnectionManagerOptions = {
    heartbeatTimeoutMs?: number;
    cleanupIntervalMs?: number;
    onConnectionClosed?: ConnectionClosedHandler;
};

export class ConnectionManager {
    private readonly logger = new Logger("ConnectionManager");
    private readonly connections = new Map<string, ConnectionContext>();
    private readonly heartbeatTimeoutMs: number;
    private readonly cleanupIntervalMs: number;
    private readonly onConnectionClosed?: ConnectionClosedHandler;
    private cleanupTimer?: any;

    public constructor(options: ConnectionManagerOptions = {}) {
        this.heartbeatTimeoutMs = options.heartbeatTimeoutMs ?? 90000;
        this.cleanupIntervalMs = options.cleanupIntervalMs ?? 10000;
        this.onConnectionClosed = options.onConnectionClosed;
    }

    public add(ctx: ConnectionContext): void {
        this.connections.set(ctx.id, ctx);
        this.logger.info(`Connection added. id=${ctx.id}, total=${this.connections.size}`);
    }

    public get(id: string): ConnectionContext | undefined {
        return this.connections.get(id);
    }

    public remove(id: string): void {
        const ctx = this.connections.get(id);
        if (!ctx) {
            return;
        }

        this.connections.delete(id);
        this.logger.info(`Connection removed. id=${id}, total=${this.connections.size}`);
        this.onConnectionClosed?.(ctx);
    }

    public getAll(): ConnectionContext[] {
        return Array.from(this.connections.values());
    }

    public updateHeartbeat(ctx: ConnectionContext): void {
        ctx.lastHeartbeat = Date.now();
        this.logger.debug(`Heartbeat updated. id=${ctx.id}`);
    }

    public startHeartbeatScan(): void {
        if (this.cleanupTimer) {
            return;
        }

        this.cleanupTimer = setInterval(() => this.closeTimeoutConnections(), this.cleanupIntervalMs);
        this.logger.info(`Heartbeat scan started. intervalMs=${this.cleanupIntervalMs}, timeoutMs=${this.heartbeatTimeoutMs}`);
    }

    public stopHeartbeatScan(): void {
        if (!this.cleanupTimer) {
            return;
        }

        clearInterval(this.cleanupTimer);
        this.cleanupTimer = undefined;
        this.logger.info("Heartbeat scan stopped.");
    }

    private closeTimeoutConnections(): void {
        const now = Date.now();
        for (const ctx of this.getAll()) {
            if (now - ctx.lastHeartbeat <= this.heartbeatTimeoutMs) {
                continue;
            }

            this.logger.warn(`Connection heartbeat timeout. id=${ctx.id}`);
            ctx.ws.destroy?.();
            this.remove(ctx.id);
        }
    }
}
