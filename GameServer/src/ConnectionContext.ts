export type WebSocketLike = {
    destroyed?: boolean;
    remoteAddress?: string;
    write(data: unknown): void;
    end?(): void;
    destroy?(): void;
    on(eventName: string, listener: (...args: any[]) => void): void;
};

export type ConnectionContext = {
    id: string;
    ws: WebSocketLike;
    userId?: string;
    isAuthenticated: boolean;
    lastHeartbeat: number;
    sessionData: Record<string, unknown>;
};

export function createConnectionContext(ws: WebSocketLike, id: string): ConnectionContext {
    return {
        id,
        ws,
        isAuthenticated: false,
        lastHeartbeat: Date.now(),
        sessionData: {},
    };
}
