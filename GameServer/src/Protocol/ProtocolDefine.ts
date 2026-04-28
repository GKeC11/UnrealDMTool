// Base protocol ids: 1-99.
export enum ProtocolBase {
    PING = 1,
    PONG = 2,
    ERROR = 99,
}

export const ProtocolRanges = {
    base: { min: 1, max: 99 },
    account: { min: 100, max: 199 },
    lobby: { min: 200, max: 299 },
    room: { min: 300, max: 399 },
} as const;
