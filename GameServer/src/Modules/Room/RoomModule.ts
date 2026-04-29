import { ConnectionContext } from "../../ConnectionContext";
import { MessageRouter } from "../../MessageRouter";
import { ProtocolLobby } from "../../Protocol/LobbyDefine";
import { ProtocolRoom } from "../../Protocol/RoomDefine";
import { Logger } from "../../Utils/Logger";
import { DedicatedServerManager, DedicatedServerSession } from "./DedicatedServerManager";

type SendToClient = (ctx: ConnectionContext, type: number, payload?: Record<string, unknown>) => void;
type GetConnections = () => ConnectionContext[];
type AccountRoomState = {
    setCurrentRoomId(accountId: string, roomId: string): void;
    clearCurrentRoomId(accountId: string, roomId?: string): void;
};

type RoomStatus = "waiting" | "playing";

type PublicAccount = {
    accountId?: string;
    playerName?: string;
};

type RoomMember = {
    PlayerId: string;
    PlayerName: string;
    bIsReady: boolean;
    bIsHost: boolean;
};

type Room = {
    RoomId: string;
    RoomName: string;
    MaxPlayers: number;
    Members: RoomMember[];
    Status: RoomStatus;
    ServerAddress: string;
    CreatedAt: string;
    UpdatedAt: string;
};

type CreateRoomPayload = {
    hostAccountId?: string;
    roomName?: string;
    maxPlayers?: number;
};

type RoomIdPayload = {
    roomId?: string;
    accountId?: string;
};

export class RoomModule {
    private readonly logger = new Logger("RoomModule");
    private readonly rooms = new Map<string, Room>();
    private readonly sendToClient: SendToClient;
    private readonly getConnections: GetConnections;
    private readonly dedicatedServerManager: DedicatedServerManager;
    private nextRoomIndex = 1;

    public constructor(
        router: MessageRouter,
        sendToClient: SendToClient,
        getConnections: GetConnections,
        private readonly accountRoomState: AccountRoomState,
        dedicatedServerManager = new DedicatedServerManager()
    ) {
        this.sendToClient = sendToClient;
        this.getConnections = getConnections;
        this.dedicatedServerManager = dedicatedServerManager;

        router.register(ProtocolLobby.ROOM_LIST_REQUEST, (ctx) => this.handleRoomListRequest(ctx));
        router.register(ProtocolRoom.CREATE_ROOM, (ctx, payload) => this.handleCreateRoom(ctx, payload));
        router.register(ProtocolRoom.JOIN_ROOM, (ctx, payload) => this.handleJoinRoom(ctx, payload));
        router.register(ProtocolRoom.ROOM_UPDATE, (ctx, payload) => this.handleLeaveRoom(ctx, payload));
        router.register(ProtocolRoom.START_GAME, (ctx, payload) => this.handleStartGame(ctx, payload));
    }

    public handleConnectionClosed(ctx: ConnectionContext): void {
        if (!ctx.userId) {
            return;
        }

        const room = this.findRoomByMember(ctx.userId);
        if (!room) {
            return;
        }

        this.removeMember(room, ctx.userId);
        this.logger.info(`Connection left room on close. connection=${ctx.id}, accountId=${ctx.userId}, roomId=${room.RoomId}`);
    }

    private handleRoomListRequest(ctx: ConnectionContext): void {
        const rooms = this.getPublicRooms();
        this.sendToClient(ctx, ProtocolLobby.ROOM_LIST_RESPONSE, {
            success: true,
            ok: true,
            data: rooms,
            onlineAccounts: this.getOnlineAccounts(),
            lobby: {
                onlineAccounts: this.getOnlineAccounts(),
            },
        });
        this.logger.debug(`Room list sent. connection=${ctx.id}, rooms=${rooms.length}`);
    }

    private handleCreateRoom(ctx: ConnectionContext, payload: unknown): void {
        this.assertAuthenticated(ctx);
        const request = asRecord<CreateRoomPayload>(payload);
        const hostAccountId = normalizeString(request.hostAccountId) || ctx.userId || "";
        if (hostAccountId !== ctx.userId) {
            throw new Error("hostAccountId must match current account");
        }

        const existingRoom = this.findRoomByMember(hostAccountId);
        if (existingRoom) {
            this.accountRoomState.setCurrentRoomId(hostAccountId, existingRoom.RoomId);
            this.sendRoomResponse(ctx, ProtocolRoom.ROOM_CREATED, existingRoom);
            return;
        }

        const roomName = normalizeString(request.roomName) || "Lobby";
        const joinableRoom = this.findJoinableRoomByName(roomName);
        if (joinableRoom) {
            joinableRoom.Members.push(this.createMember(hostAccountId, false));
            joinableRoom.UpdatedAt = new Date().toISOString();
            this.accountRoomState.setCurrentRoomId(hostAccountId, joinableRoom.RoomId);
            this.logger.info(`Room entered by name. roomId=${joinableRoom.RoomId}, roomName=${roomName}, accountId=${hostAccountId}, members=${joinableRoom.Members.length}`);
            this.sendRoomResponse(ctx, ProtocolRoom.ROOM_CREATED, joinableRoom);
            this.broadcastRoomUpdate(joinableRoom, ctx);
            return;
        }

        const room = this.createRoom(hostAccountId, request);
        this.rooms.set(room.RoomId, room);
        this.accountRoomState.setCurrentRoomId(hostAccountId, room.RoomId);
        this.logger.info(`Room created. roomId=${room.RoomId}, host=${hostAccountId}, maxPlayers=${room.MaxPlayers}`);
        this.sendRoomResponse(ctx, ProtocolRoom.ROOM_CREATED, room);
    }

    private handleJoinRoom(ctx: ConnectionContext, payload: unknown): void {
        this.assertAuthenticated(ctx);
        const request = asRecord<RoomIdPayload>(payload);
        const accountId = normalizeString(request.accountId) || ctx.userId || "";
        if (accountId !== ctx.userId) {
            throw new Error("accountId must match current account");
        }

        const room = this.getRoomOrThrow(request.roomId);
        if (room.Status !== "waiting") {
            throw new Error("room already started");
        }

        if (!room.Members.some((member) => member.PlayerId === accountId)) {
            if (room.Members.length >= room.MaxPlayers) {
                throw new Error("room is full");
            }

            room.Members.push(this.createMember(accountId, false));
            room.UpdatedAt = new Date().toISOString();
        }

        this.accountRoomState.setCurrentRoomId(accountId, room.RoomId);
        this.logger.info(`Room joined. roomId=${room.RoomId}, accountId=${accountId}, members=${room.Members.length}`);
        this.broadcastRoomUpdate(room);
    }

    private handleLeaveRoom(ctx: ConnectionContext, payload: unknown): void {
        this.assertAuthenticated(ctx);
        const request = asRecord<RoomIdPayload>(payload);
        const accountId = normalizeString(request.accountId) || ctx.userId || "";
        if (accountId !== ctx.userId) {
            throw new Error("accountId must match current account");
        }

        const room = this.getRoomOrThrow(request.roomId);
        this.removeMember(room, accountId);
        this.sendToClient(ctx, ProtocolRoom.ROOM_UPDATE, {
            success: true,
            ok: true,
            data: this.rooms.has(room.RoomId) ? this.toPublicRoom(room) : undefined,
            roomDeleted: !this.rooms.has(room.RoomId),
        });
    }

    private handleStartGame(ctx: ConnectionContext, payload: unknown): void {
        this.assertAuthenticated(ctx);
        const request = asRecord<RoomIdPayload>(payload);
        const room = this.getRoomOrThrow(request.roomId);
        const host = room.Members.find((member) => member.bIsHost);
        if (!host || host.PlayerId !== ctx.userId) {
            throw new Error("only room host can start game");
        }

        const session = this.dedicatedServerManager.startDS(room.RoomId);
        room.Status = "playing";
        room.ServerAddress = session.address;
        room.UpdatedAt = new Date().toISOString();

        this.logger.info(`Room game starting. roomId=${room.RoomId}, host=${ctx.userId}, address=${session.address}`);
        this.broadcastGameStarting(room, session);
    }

    private createRoom(hostAccountId: string, request: CreateRoomPayload): Room {
        const now = new Date().toISOString();
        return {
            RoomId: this.createRoomId(),
            RoomName: normalizeString(request.roomName) || "Lobby",
            MaxPlayers: clampInteger(request.maxPlayers, 2, 16, 8),
            Members: [this.createMember(hostAccountId, true)],
            Status: "waiting",
            ServerAddress: "",
            CreatedAt: now,
            UpdatedAt: now,
        };
    }

    private createMember(accountId: string, isHost: boolean): RoomMember {
        const account = this.findOnlineAccount(accountId);
        return {
            PlayerId: accountId,
            PlayerName: normalizeString(account?.playerName) || accountId,
            bIsReady: false,
            bIsHost: isHost,
        };
    }

    private removeMember(room: Room, accountId: string): void {
        const beforeCount = room.Members.length;
        room.Members = room.Members.filter((member) => member.PlayerId !== accountId);
        if (room.Members.length === beforeCount) {
            return;
        }

        this.accountRoomState.clearCurrentRoomId(accountId, room.RoomId);

        if (room.Members.length === 0) {
            this.rooms.delete(room.RoomId);
            this.dedicatedServerManager.stopDS(room.RoomId);
            this.logger.info(`Room deleted after last member left. roomId=${room.RoomId}`);
            return;
        }

        if (!room.Members.some((member) => member.bIsHost)) {
            room.Members[0].bIsHost = true;
        }

        room.UpdatedAt = new Date().toISOString();
        this.logger.info(`Room member left. roomId=${room.RoomId}, accountId=${accountId}, members=${room.Members.length}`);
        this.broadcastRoomUpdate(room);
    }

    private broadcastRoomUpdate(room: Room, exceptCtx?: ConnectionContext): void {
        const payload = {
            success: true,
            ok: true,
            data: this.toPublicRoom(room),
        };

        for (const ctx of this.getRoomConnections(room)) {
            if (exceptCtx && ctx.id === exceptCtx.id) {
                continue;
            }

            this.sendToClient(ctx, ProtocolRoom.ROOM_UPDATE, payload);
        }
    }

    private broadcastGameStarting(room: Room, session: DedicatedServerSession): void {
        for (const ctx of this.getRoomConnections(room)) {
            const payload = {
                success: true,
                ok: true,
                data: {
                    serverAddress: session.address,
                    token: normalizeString(ctx.sessionData.authToken),
                    room: this.toPublicRoom(room),
                },
            };

            this.sendToClient(ctx, ProtocolRoom.GAME_STARTING, payload);
        }
    }

    private sendRoomResponse(ctx: ConnectionContext, type: number, room: Room): void {
        this.sendToClient(ctx, type, {
            success: true,
            ok: true,
            data: this.toPublicRoom(room),
        });
    }

    private getRoomConnections(room: Room): ConnectionContext[] {
        const memberIds = new Set(room.Members.map((member) => member.PlayerId));
        return this.getConnections().filter((ctx) => ctx.userId && memberIds.has(ctx.userId));
    }

    private getPublicRooms(): Array<Record<string, unknown>> {
        return Array.from(this.rooms.values()).map((room) => this.toPublicRoom(room));
    }

    private toPublicRoom(room: Room): Record<string, unknown> {
        return {
            RoomId: room.RoomId,
            RoomName: room.RoomName,
            MaxPlayers: room.MaxPlayers,
            Members: room.Members,
            ServerAddress: room.ServerAddress,
            Status: room.Status,
            bIsSynchronized: true,
        };
    }

    private getRoomOrThrow(roomId: unknown): Room {
        const resolvedRoomId = normalizeString(roomId);
        const room = this.rooms.get(resolvedRoomId);
        if (!room) {
            throw new Error("room does not exist");
        }

        return room;
    }

    private findRoomByMember(accountId: string): Room | undefined {
        return Array.from(this.rooms.values()).find((room) => room.Members.some((member) => member.PlayerId === accountId));
    }

    private findJoinableRoomByName(roomName: string): Room | undefined {
        return Array.from(this.rooms.values()).find((room) =>
            room.RoomName === roomName &&
            room.Status === "waiting" &&
            room.Members.length < room.MaxPlayers
        );
    }

    private getOnlineAccounts(): PublicAccount[] {
        const accounts = this.getConnections()
            .map((ctx) => ctx.sessionData.account as PublicAccount | undefined)
            .filter((account): account is PublicAccount => !!account?.accountId);
        return uniqueBy(accounts, (account) => account.accountId || "");
    }

    private findOnlineAccount(accountId: string): PublicAccount | undefined {
        return this.getOnlineAccounts().find((account) => account.accountId === accountId);
    }

    private assertAuthenticated(ctx: ConnectionContext): void {
        if (!ctx.isAuthenticated || !ctx.userId) {
            throw new Error("account is not authenticated");
        }
    }

    private createRoomId(): string {
        const roomId = `room_${Date.now()}_${this.nextRoomIndex}`;
        this.nextRoomIndex += 1;
        return roomId;
    }
}

function asRecord<T extends Record<string, unknown>>(value: unknown): T {
    return value && typeof value === "object" ? value as T : {} as T;
}

function normalizeString(value: unknown): string {
    return typeof value === "string" ? value.trim() : "";
}

function clampInteger(value: unknown, min: number, max: number, fallback: number): number {
    const numberValue = Number(value);
    if (!Number.isInteger(numberValue)) {
        return fallback;
    }

    return Math.max(min, Math.min(max, numberValue));
}

function uniqueBy<T>(items: T[], getKey: (item: T) => string): T[] {
    const seen = new Set<string>();
    const result: T[] = [];
    for (const item of items) {
        const key = getKey(item);
        if (!key || seen.has(key)) {
            continue;
        }

        seen.add(key);
        result.push(item);
    }

    return result;
}
