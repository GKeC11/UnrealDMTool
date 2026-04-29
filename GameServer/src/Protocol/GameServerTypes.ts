export type GameServerProtocolMessage<TPayload = Record<string, unknown>> = {
    type: number;
    payload: TPayload;
};

export type GameServerResponsePayload<TData = unknown> = {
    success?: boolean;
    ok?: boolean;
    data?: TData;
    errorCode?: number;
    message?: string;
};

export type GameServerPublicAccount = {
    accountId?: string;
    playerName?: string;
};

export type GameServerAuthResponsePayload = GameServerResponsePayload & {
    token?: string;
    account?: GameServerPublicAccount;
    isRegistered?: boolean;
};

export type GameServerAuthRequestPayload = {
    accountId?: string;
    playerName?: string;
};

export type GameServerTokenRefreshPayload = {
    token?: string;
};

export type GameServerTokenVerifyPayload = {
    token?: string;
};

export type GameServerCurrentRoomPayload = GameServerResponsePayload<{
    roomId?: string;
}>;

export type GameServerRoomStatus = "waiting" | "playing";

export type GameServerRoomMember = {
    PlayerId?: string;
    PlayerName?: string;
    bIsReady?: boolean;
    bIsHost?: boolean;
};

export type GameServerRoom = {
    RoomId?: string;
    RoomName?: string;
    MaxPlayers?: number;
    Members?: GameServerRoomMember[];
    ServerAddress?: string;
    Status?: GameServerRoomStatus;
    bIsSynchronized?: boolean;
};

export type GameServerLobbyRoomListPayload = GameServerResponsePayload<GameServerRoom[]> & {
    onlineAccounts?: GameServerPublicAccount[];
    lobby?: {
        onlineAccounts?: GameServerPublicAccount[];
    };
};

export type GameServerRoomPayload = GameServerResponsePayload<GameServerRoom>;

export type GameServerCreateRoomPayload = {
    hostAccountId?: string;
    roomName?: string;
    maxPlayers?: number;
};

export type GameServerRoomIdPayload = {
    roomId?: string;
    accountId?: string;
};

export type GameServerStartGamePayload = GameServerResponsePayload<{
    serverAddress?: string;
    token?: string;
    room?: GameServerRoom;
}>;

export function parseGameServerPayload<TPayload>(responseBody: string): TPayload | undefined {
    const parsed = JSON.parse(responseBody) as GameServerProtocolMessage<TPayload> | TPayload;
    if (parsed && typeof parsed === "object" && "payload" in parsed) {
        return (parsed as GameServerProtocolMessage<TPayload>).payload;
    }

    return parsed as TPayload;
}
