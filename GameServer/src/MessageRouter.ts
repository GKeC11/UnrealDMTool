import { ConnectionContext } from "./ConnectionContext";
import { ProtocolBase } from "./Protocol/ProtocolDefine";
import { Logger } from "./Utils/Logger";
import { ResponseHelper } from "./Utils/ResponseHelper";

export type MessageHandler = (
    ctx: ConnectionContext,
    payload: unknown
) => void | Promise<void>;

export class MessageRouter {
    private readonly logger = new Logger("MessageRouter");
    private readonly handlers = new Map<number, MessageHandler>();

    public register(protocolId: number, handler: MessageHandler): void {
        if (this.handlers.has(protocolId)) {
            this.logger.warn(`Protocol handler overwritten. type=${protocolId}`);
        }

        this.handlers.set(protocolId, handler);
        this.logger.info(`Protocol handler registered. type=${protocolId}`);
    }

    public async dispatch(protocolId: number, ctx: ConnectionContext, payload: unknown): Promise<void> {
        const handler = this.handlers.get(protocolId);
        if (!handler) {
            this.logger.warn(`Protocol handler not found. type=${protocolId}, connection=${ctx.id}`);
            ctx.ws.write(ResponseHelper.build(ProtocolBase.ERROR, {
                errorCode: 404,
                message: `Protocol handler not found: ${protocolId}`,
            }));
            return;
        }

        try {
            await handler(ctx, payload);
        } catch (error) {
            const message = error instanceof Error ? error.message : "Unknown router error";
            this.logger.error(`Protocol handler failed. type=${protocolId}, connection=${ctx.id}`, message);
            ctx.ws.write(ResponseHelper.buildError(500, message));
        }
    }
}
