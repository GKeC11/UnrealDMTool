import { ProtocolBase } from "../Protocol/ProtocolDefine";

export type GameMessage = {
    type: number;
    payload: Record<string, unknown>;
};

export class ResponseHelper {
    public static build(type: number, payload: Record<string, unknown> = {}): string {
        return JSON.stringify({
            type,
            payload,
        });
    }

    public static buildError(errorCode: number, message: string): string {
        return ResponseHelper.build(ProtocolBase.ERROR, {
            errorCode,
            message,
        });
    }
}
