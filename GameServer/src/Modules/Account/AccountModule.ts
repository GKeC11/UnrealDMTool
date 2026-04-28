import { ConnectionContext } from "../../ConnectionContext";
import { MessageRouter } from "../../MessageRouter";
import { ProtocolAccount } from "../../Protocol/AccountDefine";
import { Logger } from "../../Utils/Logger";
import { AuthService, LoginResult } from "./AuthService";

type SendToClient = (ctx: ConnectionContext, type: number, payload?: Record<string, unknown>) => void;

type AuthRequestPayload = {
    accountId?: string;
    playerName?: string;
};

type TokenRefreshPayload = {
    token?: string;
};

export class AccountModule {
    private readonly logger = new Logger("AccountModule");
    private readonly authService: AuthService;
    private readonly sendToClient: SendToClient;

    public constructor(router: MessageRouter, sendToClient: SendToClient, authService = new AuthService()) {
        this.authService = authService;
        this.sendToClient = sendToClient;

        router.register(ProtocolAccount.AUTH_REQUEST, (ctx, payload) => this.handleAuthRequest(ctx, payload));
        router.register(ProtocolAccount.TOKEN_REFRESH, (ctx, payload) => this.handleTokenRefresh(ctx, payload));
    }

    private handleAuthRequest(ctx: ConnectionContext, payload: unknown): void {
        const request = asRecord<AuthRequestPayload>(payload);
        try {
            const result = this.authService.login(request.accountId, request.playerName);
            this.applyLoginContext(ctx, result);
            this.logger.info(`Account authenticated. connection=${ctx.id}, userId=${ctx.userId}, registered=${result.isRegistered}`);
            this.sendAuthResponse(ctx, result);
        } catch (error) {
            const message = error instanceof Error ? error.message : "Auth request failed";
            ctx.isAuthenticated = false;
            ctx.userId = undefined;
            this.logger.warn(`Account auth failed. connection=${ctx.id}, reason=${message}`);
            this.sendToClient(ctx, ProtocolAccount.AUTH_RESPONSE, {
                success: false,
                errorCode: 401,
                message,
            });
        }
    }

    private handleTokenRefresh(ctx: ConnectionContext, payload: unknown): void {
        const request = asRecord<TokenRefreshPayload>(payload);
        try {
            const result = this.authService.refreshToken(request.token);
            this.applyLoginContext(ctx, result);
            this.logger.info(`Account token refreshed. connection=${ctx.id}, userId=${ctx.userId}`);
            this.sendAuthResponse(ctx, result);
        } catch (error) {
            const message = error instanceof Error ? error.message : "Token refresh failed";
            this.logger.warn(`Account token refresh failed. connection=${ctx.id}, reason=${message}`);
            this.sendToClient(ctx, ProtocolAccount.AUTH_RESPONSE, {
                success: false,
                errorCode: 401,
                message,
            });
        }
    }

    private applyLoginContext(ctx: ConnectionContext, result: LoginResult): void {
        ctx.userId = result.account.accountId;
        ctx.isAuthenticated = true;
        // 后续房间和大厅模块可以从连接上下文读取账号公开信息，避免重复解析 Token。
        ctx.sessionData.account = this.authService.toPublicAccount(result.account);
    }

    private sendAuthResponse(ctx: ConnectionContext, result: LoginResult): void {
        this.sendToClient(ctx, ProtocolAccount.AUTH_RESPONSE, {
            success: true,
            token: result.token,
            account: this.authService.toPublicAccount(result.account),
            isRegistered: result.isRegistered,
        });
    }
}

function asRecord<T extends Record<string, unknown>>(value: unknown): T {
    return value && typeof value === "object" ? value as T : {} as T;
}
