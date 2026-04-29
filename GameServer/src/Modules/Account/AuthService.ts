import crypto = require("node:crypto");
import fs = require("node:fs");
import path = require("node:path");
import { Logger } from "../../Utils/Logger";

export type AccountRecord = {
    accountId: string;
    playerName: string;
    sessionToken: string;
    createdAt: string;
    updatedAt: string;
    lastLoginAt: string;
};

export type PublicAccount = Omit<AccountRecord, "sessionToken">;

type ServerState = {
    version: number;
    accounts: Record<string, AccountRecord>;
    lobby: {
        onlineAccountIds: string[];
    };
    rooms: Record<string, unknown>;
};

export type LoginResult = {
    account: AccountRecord;
    token: string;
    isRegistered: boolean;
};

export type TokenPayload = {
    accountId: string;
    playerName: string;
    issuedAt: number;
    expiresAt: number;
};

const DEFAULT_TOKEN_TTL_MS = 24 * 60 * 60 * 1000;

export class AuthService {
    private readonly logger = new Logger("AuthService");
    private readonly dataFile: string;
    private readonly tokenSecret: string;
    private readonly tokenTtlMs: number;
    private readonly state: ServerState;

    public constructor(dataFile = resolveDefaultDataFile(), tokenSecret = resolveTokenSecret(), tokenTtlMs = DEFAULT_TOKEN_TTL_MS) {
        this.dataFile = dataFile;
        this.tokenSecret = tokenSecret;
        this.tokenTtlMs = tokenTtlMs;
        this.state = this.loadState();
    }

    public login(accountId: unknown, playerName: unknown): LoginResult {
        const resolvedAccountId = normalizeString(accountId);
        if (!resolvedAccountId) {
            throw new Error("accountId is required");
        }

        const resolvedPlayerName = normalizeString(playerName) || resolvedAccountId;
        const existing = this.state.accounts[resolvedAccountId];
        const now = new Date().toISOString();
        const token = this.createToken({
            accountId: resolvedAccountId,
            playerName: resolvedPlayerName,
            issuedAt: Date.now(),
            expiresAt: Date.now() + this.tokenTtlMs,
        });

        if (existing) {
            existing.playerName = resolvedPlayerName;
            existing.sessionToken = token;
            existing.lastLoginAt = now;
            existing.updatedAt = now;
            this.enterLobby(existing.accountId);
            this.saveState();
            this.logger.info(`Account login succeeded. accountId=${existing.accountId}, playerName=${existing.playerName}`);
            return {
                account: existing,
                token,
                isRegistered: false,
            };
        }

        const account: AccountRecord = {
            accountId: resolvedAccountId,
            playerName: resolvedPlayerName,
            sessionToken: token,
            createdAt: now,
            updatedAt: now,
            lastLoginAt: now,
        };

        this.state.accounts[account.accountId] = account;
        this.enterLobby(account.accountId);
        this.saveState();
        this.logger.info(`Account registered. accountId=${account.accountId}, playerName=${account.playerName}`);
        return {
            account,
            token,
            isRegistered: true,
        };
    }

    public refreshToken(token: unknown): LoginResult {
        const payload = this.verifyToken(token);
        const account = this.state.accounts[payload.accountId];
        if (!account) {
            throw new Error("account does not exist");
        }

        return this.login(account.accountId, account.playerName);
    }

    public getAccount(accountId: string): AccountRecord | undefined {
        return this.state.accounts[accountId];
    }

    public verifyToken(token: unknown): TokenPayload {
        const rawToken = normalizeString(token);
        const parts = rawToken.split(".");
        if (parts.length !== 3) {
            throw new Error("token format is invalid");
        }

        const [encodedHeader, encodedPayload, signature] = parts;
        const expectedSignature = this.sign(`${encodedHeader}.${encodedPayload}`);
        if (!timingSafeEqual(signature, expectedSignature)) {
            throw new Error("token signature is invalid");
        }

        const payload = JSON.parse(Buffer.from(encodedPayload, "base64url").toString("utf8")) as TokenPayload;
        if (!payload.accountId || !payload.expiresAt) {
            throw new Error("token payload is invalid");
        }

        if (payload.expiresAt <= Date.now()) {
            throw new Error("token is expired");
        }

        return payload;
    }

    public toPublicAccount(account: AccountRecord): PublicAccount {
        return {
            accountId: account.accountId,
            playerName: account.playerName,
            createdAt: account.createdAt,
            updatedAt: account.updatedAt,
            lastLoginAt: account.lastLoginAt,
        };
    }

    private createToken(payload: TokenPayload): string {
        const header = {
            alg: "HS256",
            typ: "JWT",
        };
        const encodedHeader = Buffer.from(JSON.stringify(header), "utf8").toString("base64url");
        const encodedPayload = Buffer.from(JSON.stringify(payload), "utf8").toString("base64url");
        const unsignedToken = `${encodedHeader}.${encodedPayload}`;
        return `${unsignedToken}.${this.sign(unsignedToken)}`;
    }

    private sign(value: string): string {
        return crypto
            .createHmac("sha256", this.tokenSecret)
            .update(value)
            .digest("base64url");
    }

    private enterLobby(accountId: string): void {
        if (!this.state.lobby.onlineAccountIds.includes(accountId)) {
            this.state.lobby.onlineAccountIds.push(accountId);
        }
    }

    private loadState(): ServerState {
        if (!fs.existsSync(this.dataFile)) {
            return createEmptyState();
        }

        const rawState = String(fs.readFileSync(this.dataFile, "utf8"));
        if (!rawState.trim()) {
            return createEmptyState();
        }

        return {
            ...createEmptyState(),
            ...JSON.parse(rawState),
        };
    }

    private saveState(): void {
        fs.mkdirSync(path.dirname(this.dataFile), {
            recursive: true,
        });
        fs.writeFileSync(this.dataFile, `${JSON.stringify(this.state, null, 2)}\n`, "utf8");
        this.logger.debug(`State saved. accounts=${Object.keys(this.state.accounts).length}, online=${this.state.lobby.onlineAccountIds.length}`);
    }
}

function createEmptyState(): ServerState {
    return {
        version: 1,
        accounts: {},
        lobby: {
            onlineAccountIds: [],
        },
        rooms: {},
    };
}

function resolveDefaultDataFile(): string {
    return process.env.GAME_SERVER_DATA
        ? path.resolve(String(process.env.GAME_SERVER_DATA))
        : path.resolve(__dirname, "..", "..", "..", "data", "state.json");
}

function resolveTokenSecret(): string {
    return String(process.env.JWT_SECRET || process.env.TOKEN_SECRET || "NoOutsiders-GameServer-Dev-Secret");
}

function normalizeString(value: unknown): string {
    return typeof value === "string" ? value.trim() : "";
}

function timingSafeEqual(left: string, right: string): boolean {
    const leftBuffer = Buffer.from(left);
    const rightBuffer = Buffer.from(right);
    return leftBuffer.length === rightBuffer.length && crypto.timingSafeEqual(leftBuffer, rightBuffer);
}
