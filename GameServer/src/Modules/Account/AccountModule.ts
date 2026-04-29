import { ConnectionContext } from "../../ConnectionContext";
import { MessageRouter } from "../../MessageRouter";
import { ProtocolAccount } from "../../Protocol/AccountDefine";
import { Logger } from "../../Utils/Logger";
import { AuthService, LoginResult } from "./AuthService";

type SendToClient = (ctx: ConnectionContext, type: number, payload?: Record<string, unknown>) => void;
type GetConnections = () => ConnectionContext[];

type AuthRequestPayload = {
    accountId?: string;
    playerName?: string;
};

type TokenRefreshPayload = {
    token?: string;
};

type TokenVerifyPayload = {
    token?: string;
};

export class AccountModule {
    private readonly logger = new Logger("AccountModule");
    private readonly authService: AuthService;
    private readonly sendToClient: SendToClient;
    private readonly getConnections: GetConnections;
    private readonly currentRoomIds = new Map<string, string>();

    public constructor(router: MessageRouter, sendToClient: SendToClient, authService = new AuthService(), getConnections: GetConnections = () => []) {
        this.authService = authService;
        this.sendToClient = sendToClient;
        this.getConnections = getConnections;

        router.register(ProtocolAccount.AUTH_REQUEST, (ctx, payload) => this.handleAuthRequest(ctx, payload));
        router.register(ProtocolAccount.TOKEN_REFRESH, (ctx, payload) => this.handleTokenRefresh(ctx, payload));
        router.register(ProtocolAccount.CURRENT_ROOM_REQUEST, (ctx) => this.handleCurrentRoomRequest(ctx));
        router.register(ProtocolAccount.TOKEN_VERIFY_REQUEST, (ctx, payload) => this.handleTokenVerify(ctx, payload));
    }

    public setCurrentRoomId(accountId: string, roomId: string): void {
        const resolvedAccountId = normalizeString(accountId);
        const resolvedRoomId = normalizeString(roomId);
        if (!resolvedAccountId || !resolvedRoomId) {
            return;
        }

        this.currentRoomIds.set(resolvedAccountId, resolvedRoomId);
        this.logger.info(`Account current room set. accountId=${resolvedAccountId}, roomId=${resolvedRoomId}`);
    }

    public clearCurrentRoomId(accountId: string, roomId?: string): void {
        const resolvedAccountId = normalizeString(accountId);
        const resolvedRoomId = normalizeString(roomId);
        if (!resolvedAccountId) {
            return;
        }

        const currentRoomId = this.currentRoomIds.get(resolvedAccountId) || "";
        if (!currentRoomId || (resolvedRoomId && currentRoomId !== resolvedRoomId)) {
            return;
        }

        this.currentRoomIds.delete(resolvedAccountId);
        this.logger.info(`Account current room cleared. accountId=${resolvedAccountId}, roomId=${currentRoomId}`);
    }

    public getCurrentRoomId(accountId: string): string {
        const resolvedAccountId = normalizeString(accountId);
        return resolvedAccountId ? this.currentRoomIds.get(resolvedAccountId) || "" : "";
    }

    private handleAuthRequest(ctx: ConnectionContext, payload: unknown): void {
        const request = asRecord<AuthRequestPayload>(payload);
        try {
            if (ctx.isAuthenticated) {
                throw new Error("connection is already authenticated");
            }

            const accountId = normalizeString(request.accountId);
            this.assertAccountNotLoggedIn(accountId);
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
            const tokenPayload = this.authService.verifyToken(request.token);
            this.assertAccountNotLoggedIn(tokenPayload.accountId, ctx);
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

    private handleTokenVerify(ctx: ConnectionContext, payload: unknown): void {
        const request = asRecord<TokenVerifyPayload>(payload);
        try {
            const tokenPayload = this.authService.verifyToken(request.token);
            const account = this.authService.getAccount(tokenPayload.accountId);
            if (!account) {
                throw new Error("account does not exist");
            }

            this.sendToClient(ctx, ProtocolAccount.TOKEN_VERIFY_RESPONSE, {
                success: true,
                ok: true,
                account: this.authService.toPublicAccount(account),
            });
            this.logger.debug(`Account token verified. connection=${ctx.id}, accountId=${account.accountId}`);
        } catch (error) {
            const message = error instanceof Error ? error.message : "Token verify failed";
            this.logger.warn(`Account token verify failed. connection=${ctx.id}, reason=${message}`);
            this.sendToClient(ctx, ProtocolAccount.TOKEN_VERIFY_RESPONSE, {
                success: false,
                errorCode: 401,
                message,
            });
        }
    }

    private handleCurrentRoomRequest(ctx: ConnectionContext): void {
        this.assertAuthenticated(ctx);
        const roomId = this.getCurrentRoomId(ctx.userId || "");
        this.sendToClient(ctx, ProtocolAccount.CURRENT_ROOM_RESPONSE, {
            success: true,
            ok: true,
            data: {
                roomId,
            },
        });
        this.logger.debug(`Account current room sent. connection=${ctx.id}, accountId=${ctx.userId}, roomId=${roomId}`);
    }

    private assertAuthenticated(ctx: ConnectionContext): void {
        if (!ctx.isAuthenticated || !ctx.userId) {
            throw new Error("account is not authenticated");
        }
    }

    private assertAccountNotLoggedIn(accountId: string, allowedCtx?: ConnectionContext): void {
        if (!accountId) {
            return;
        }

        const loggedInConnection = this.getConnections().find((ctx) =>
            ctx.isAuthenticated &&
            ctx.userId === accountId &&
            ctx.id !== allowedCtx?.id
        );
        if (loggedInConnection) {
            throw new Error("account is already authenticated");
        }
    }

    private applyLoginContext(ctx: ConnectionContext, result: LoginResult): void {
        ctx.userId = result.account.accountId;
        ctx.isAuthenticated = true;
        // Keep the verified account data and token available for room-to-DS handoff.
        ctx.sessionData.account = this.authService.toPublicAccount(result.account);
        ctx.sessionData.authToken = result.token;
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

function normalizeString(value: unknown): string {
    return typeof value === "string" ? value.trim() : "";
}
