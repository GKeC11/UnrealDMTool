import crypto = require("node:crypto");
import childProcess = require("node:child_process");
import fs = require("node:fs");
import http = require("node:http");
import os = require("node:os");
import path = require("node:path");
import { URL } from "node:url";

type AccountId = string;
type RoomId = string;

type Account = {
    accountId: AccountId;
    playerName: string;
    sessionToken: string;
    createdAt: string;
    updatedAt: string;
    lastLoginAt: string;
};

type PublicAccount = Omit<Account, "sessionToken"> & {
    isOnline: boolean;
};

type RoomMemberData = {
    PlayerId: AccountId;
    PlayerName: string;
    bIsReady: boolean;
    bIsHost: boolean;
};

type RoomData = {
    RoomId: RoomId;
    RoomName: string;
    MaxPlayers: number;
    bIsSynchronized: true;
    Members: RoomMemberData[];
    CreatedAt: string;
    UpdatedAt: string;
    ServerAddress?: string;
};

type StoredRoomData = Omit<RoomData, "bIsSynchronized">;

type LobbyState = {
    onlineAccountIds: AccountId[];
};

type ServerState = {
    version: number;
    accounts: Record<AccountId, Account>;
    lobby: LobbyState;
    rooms: Record<RoomId, StoredRoomData>;
};

type RouteResult<T> = {
    value?: T;
    error?: string;
    statusCode?: number;
};

type LoginResult = {
    account: Account;
    bRegistered: boolean;
};

type LoginBody = {
    accountId?: string;
    playerName?: string;
};

type CreateRoomBody = {
    hostAccountId?: string;
    roomName?: string;
    maxPlayers?: number;
};

type AccountBody = {
    accountId?: string;
};

type UpdateAccountBody = {
    playerName?: string;
};

type UpdateRoomMemberBody = {
    bIsReady?: boolean;
};

type StartGameBody = {
    roomId?: string;
};

type ServerLaunchCandidate = {
    launchBatPath: string;
    packageName: string;
    mtimeMs: number;
};

const PORT = Number(process.env.PORT || 7788);
const DATA_FILE = process.env.GAME_SERVER_DATA
    ? path.resolve(process.env.GAME_SERVER_DATA)
    : path.resolve(__dirname, "..", "data", "state.json");
const REPO_ROOT = path.resolve(__dirname, "..", "..", "..", "..");
const PACKAGE_BUILDS_DIR = path.resolve(REPO_ROOT, "PackageBuilds");
const SERVER_LAUNCH_BAT_RELATIVE_PATH = path.join("Server", "WindowsServer", "RunServerWithLog.bat");
const PACKAGED_SERVER_PORT = Number(process.env.PACKAGED_SERVER_PORT || 7777);
const PACKAGED_SERVER_ADDRESS = resolvePackagedServerAddress();

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

function nowIso(): string {
    return new Date().toISOString();
}

function makeId(prefix: string): string {
    return `${prefix}_${crypto.randomBytes(8).toString("hex")}`;
}

function normalizeString(value: unknown, fallback = ""): string {
    return typeof value === "string" ? value.trim() : fallback;
}

function logServer(message: string): void {
    console.log(`[${nowIso()}][GameServer] ${message}`);
}

function isPrivateIPv4(address: string): boolean {
    const parts = address.split(".").map((part) => Number(part));
    if (parts.length !== 4 || parts.some((part) => !Number.isInteger(part) || part < 0 || part > 255)) {
        return false;
    }

    return parts[0] === 10
        || (parts[0] === 172 && parts[1] >= 16 && parts[1] <= 31)
        || (parts[0] === 192 && parts[1] === 168);
}

function getLanIPv4Address(): string | undefined {
    const networkInterfaces = os.networkInterfaces();
    const fallbackAddresses: string[] = [];

    for (const entries of Object.values(networkInterfaces) as any[]) {
        for (const entry of entries ?? []) {
            if (!entry || entry.family !== "IPv4" || entry.internal) {
                continue;
            }

            const address = normalizeString(entry.address);
            if (!address) {
                continue;
            }

            if (isPrivateIPv4(address)) {
                return address;
            }

            fallbackAddresses.push(address);
        }
    }

    return fallbackAddresses[0];
}

function resolvePackagedServerAddress(): string {
    const configuredAddress = normalizeString(process.env.PACKAGED_SERVER_ADDRESS);
    if (configuredAddress) {
        return configuredAddress;
    }

    const lanAddress = getLanIPv4Address();
    if (lanAddress) {
        return `${lanAddress}:${PACKAGED_SERVER_PORT}`;
    }

    return `127.0.0.1:${PACKAGED_SERVER_PORT}`;
}

// Load the persisted test-server state. Missing or empty files start from a clean in-memory shape.
function loadState(): ServerState {
    if (!fs.existsSync(DATA_FILE)) {
        return createEmptyState();
    }

    const raw = fs.readFileSync(DATA_FILE, "utf8");
    if (!raw.trim()) {
        return createEmptyState();
    }

    return {
        ...createEmptyState(),
        ...JSON.parse(raw),
    };
}

// Runtime presence should never survive a server restart; account records stay persisted for login reuse.
function resetRuntimeStateOnStartup(state: ServerState): ServerState {
    const hadRuntimeState = state.lobby.onlineAccountIds.length > 0 || Object.keys(state.rooms).length > 0;
    state.lobby = {
        onlineAccountIds: [],
    };
    state.rooms = {};

    if (hadRuntimeState) {
        logServer("Startup reset cleared lobby and rooms.");
    }

    return state;
}

// Persist the whole state after every mutating route. This keeps the server intentionally simple for local testing.
function saveState(state: ServerState): void {
    fs.mkdirSync(path.dirname(DATA_FILE), { recursive: true });
    fs.writeFileSync(DATA_FILE, `${JSON.stringify(state, null, 2)}\n`, "utf8");
    logServer(`State saved. accounts=${Object.keys(state.accounts).length}, online=${state.lobby.onlineAccountIds.length}, rooms=${Object.keys(state.rooms).length}`);
}

const state = resetRuntimeStateOnStartup(loadState());
saveState(state);

function sendJson(response: any, statusCode: number, body: unknown): void {
    response.writeHead(statusCode, {
        "Access-Control-Allow-Origin": "*",
        "Access-Control-Allow-Methods": "GET,POST,PATCH,DELETE,OPTIONS",
        "Access-Control-Allow-Headers": "Content-Type,Authorization",
        "Content-Type": "application/json; charset=utf-8",
    });
    response.end(JSON.stringify(body));
}

function sendError(response: any, statusCode: number, message: string, details?: unknown): void {
    logServer(`Error response. status=${statusCode}, message=${message}`);
    sendJson(response, statusCode, {
        ok: false,
        error: message,
        details,
    });
}

// Read and parse a JSON request body, with a small size guard to avoid runaway local test requests.
function readJsonBody<TBody extends object>(request: any): Promise<TBody> {
    return new Promise((resolve, reject) => {
        let rawBody = "";

        request.on("data", (chunk: unknown) => {
            rawBody += String(chunk);
            if (rawBody.length > 1024 * 1024) {
                reject(new Error("Request body is too large"));
                request.destroy();
            }
        });

        request.on("end", () => {
            if (!rawBody.trim()) {
                resolve({} as TBody);
                return;
            }

            try {
                resolve(JSON.parse(rawBody) as TBody);
            } catch {
                reject(new Error("Request body must be valid JSON"));
            }
        });

        request.on("error", reject);
    });
}

function getAccount(accountId: AccountId): Account | undefined {
    return state.accounts[accountId];
}

function toPublicAccount(account: Account | undefined): PublicAccount | undefined {
    if (!account) {
        return undefined;
    }

    return {
        accountId: account.accountId,
        playerName: account.playerName,
        createdAt: account.createdAt,
        updatedAt: account.updatedAt,
        lastLoginAt: account.lastLoginAt,
        isOnline: state.lobby.onlineAccountIds.includes(account.accountId),
    };
}

// Login is also registration: unknown account IDs are created immediately, known accounts get a fresh session.
function loginOrRegisterAccount(accountId: unknown, playerName: unknown): LoginResult {
    const resolvedAccountId = normalizeString(accountId) || makeId("account");
    const resolvedPlayerName = normalizeString(playerName) || "Player";
    const existing = getAccount(resolvedAccountId);

    if (existing) {
        existing.playerName = resolvedPlayerName;
        existing.sessionToken = makeId("session");
        existing.lastLoginAt = nowIso();
        existing.updatedAt = nowIso();
        logServer(`Account login. accountId=${existing.accountId}, playerName=${existing.playerName}`);
        return {
            account: existing,
            bRegistered: false,
        };
    }

    const account: Account = {
        accountId: resolvedAccountId,
        playerName: resolvedPlayerName,
        sessionToken: makeId("session"),
        createdAt: nowIso(),
        updatedAt: nowIso(),
        lastLoginAt: nowIso(),
    };

    state.accounts[account.accountId] = account;
    logServer(`Account registered. accountId=${account.accountId}, playerName=${account.playerName}`);
    return {
        account,
        bRegistered: true,
    };
}

// Mark an account as visible in the lobby. This is idempotent so repeated enter/login calls are harmless.
function enterLobby(accountId: AccountId): Account | undefined {
    const account = getAccount(accountId);
    if (!account) {
        return undefined;
    }

    if (!state.lobby.onlineAccountIds.includes(accountId)) {
        state.lobby.onlineAccountIds.push(accountId);
        logServer(`Lobby enter. accountId=${accountId}, online=${state.lobby.onlineAccountIds.length}`);
    }

    account.updatedAt = nowIso();
    return account;
}

function leaveLobby(accountId: AccountId): void {
    const previousOnlineCount = state.lobby.onlineAccountIds.length;
    state.lobby.onlineAccountIds = state.lobby.onlineAccountIds.filter((id) => id !== accountId);
    if (state.lobby.onlineAccountIds.length !== previousOnlineCount) {
        logServer(`Lobby leave. accountId=${accountId}, online=${state.lobby.onlineAccountIds.length}`);
    }
}

// Convert server-side account data into the Unreal-facing room member shape.
function toRoomMember(account: Account, options: Partial<Pick<RoomMemberData, "bIsHost" | "bIsReady">> = {}): RoomMemberData {
    return {
        PlayerId: account.accountId,
        PlayerName: account.playerName || "Player",
        bIsReady: Boolean(options.bIsReady),
        bIsHost: Boolean(options.bIsHost),
    };
}

function toPublicRoom(room: StoredRoomData): RoomData {
    return {
        RoomId: room.RoomId,
        RoomName: room.RoomName,
        MaxPlayers: room.MaxPlayers,
        bIsSynchronized: true,
        Members: room.Members,
        CreatedAt: room.CreatedAt,
        UpdatedAt: room.UpdatedAt,
        ServerAddress: room.ServerAddress,
    };
}

function getRoom(roomId: RoomId): StoredRoomData | undefined {
    return state.rooms[roomId];
}

// Create a room and automatically make the host an online lobby account.
function createRoom(hostAccountId: AccountId, roomName: unknown, maxPlayers: unknown): RouteResult<StoredRoomData> {
    const host = getAccount(hostAccountId);
    if (!host) {
        return { error: "Host account does not exist", statusCode: 404 };
    }

    const resolvedMaxPlayers = Math.max(1, Math.min(Number(maxPlayers) || 8, 64));
    const roomId = makeId("room");
    const room: StoredRoomData = {
        RoomId: roomId,
        RoomName: normalizeString(roomName) || "Lobby",
        MaxPlayers: resolvedMaxPlayers,
        Members: [toRoomMember(host, { bIsHost: true })],
        CreatedAt: nowIso(),
        UpdatedAt: nowIso(),
    };

    state.rooms[roomId] = room;
    enterLobby(hostAccountId);
    logServer(`Room created. roomId=${roomId}, hostAccountId=${hostAccountId}, roomName=${room.RoomName}, maxPlayers=${room.MaxPlayers}`);
    return { value: room };
}

// Join is idempotent: existing members are refreshed instead of duplicated.
function joinRoom(roomId: RoomId, accountId: AccountId): RouteResult<StoredRoomData> {
    const room = getRoom(roomId);
    const account = getAccount(accountId);

    if (!room) {
        return { error: "Room does not exist", statusCode: 404 };
    }

    if (!account) {
        return { error: "Account does not exist", statusCode: 404 };
    }

    const existingMember = room.Members.find((member) => member.PlayerId === accountId);
    if (existingMember) {
        existingMember.PlayerName = account.playerName || existingMember.PlayerName;
        room.UpdatedAt = nowIso();
        logServer(`Room join refreshed. roomId=${roomId}, accountId=${accountId}, members=${room.Members.length}`);
        return { value: room };
    }

    if (room.Members.length >= room.MaxPlayers) {
        return { error: "Room is full", statusCode: 409 };
    }

    room.Members.push(toRoomMember(account));
    room.UpdatedAt = nowIso();
    enterLobby(accountId);
    logServer(`Room joined. roomId=${roomId}, accountId=${accountId}, members=${room.Members.length}`);
    return { value: room };
}

// Remove a member, delete empty rooms, and transfer host ownership when the host leaves.
function leaveRoom(roomId: RoomId, accountId: AccountId): RouteResult<StoredRoomData | undefined> & { roomDeleted?: boolean } {
    const room = getRoom(roomId);
    if (!room) {
        return { error: "Room does not exist", statusCode: 404 };
    }

    const wasHost = room.Members.some((member) => member.PlayerId === accountId && member.bIsHost);
    room.Members = room.Members.filter((member) => member.PlayerId !== accountId);

    if (room.Members.length === 0) {
        delete state.rooms[roomId];
        logServer(`Room deleted after last member left. roomId=${roomId}, accountId=${accountId}`);
        return { roomDeleted: true };
    }

    if (wasHost) {
        room.Members[0].bIsHost = true;
        logServer(`Room host transferred. roomId=${roomId}, newHostAccountId=${room.Members[0].PlayerId}`);
    }

    room.UpdatedAt = nowIso();
    logServer(`Room left. roomId=${roomId}, accountId=${accountId}, members=${room.Members.length}`);
    return { value: room };
}

// Patch mutable per-member state used by the Unreal lobby UI.
function updateRoomMember(roomId: RoomId, accountId: AccountId, patch: UpdateRoomMemberBody): RouteResult<StoredRoomData> {
    const room = getRoom(roomId);
    if (!room) {
        return { error: "Room does not exist", statusCode: 404 };
    }

    const member = room.Members.find((candidate) => candidate.PlayerId === accountId);
    if (!member) {
        return { error: "Room member does not exist", statusCode: 404 };
    }

    if (typeof patch.bIsReady === "boolean") {
        member.bIsReady = patch.bIsReady;
        logServer(`Room member ready changed. roomId=${roomId}, accountId=${accountId}, bIsReady=${member.bIsReady}`);
    }

    const account = getAccount(accountId);
    if (account) {
        member.PlayerName = account.playerName || member.PlayerName;
    }

    room.UpdatedAt = nowIso();
    return { value: room };
}

// Build the aggregate lobby payload used by the client to inspect online accounts and rooms together.
function getLobbySnapshot(): { onlineAccounts: PublicAccount[]; rooms: RoomData[] } {
    return {
        onlineAccounts: state.lobby.onlineAccountIds
            .map((accountId) => toPublicAccount(getAccount(accountId)))
            .filter((account): account is PublicAccount => Boolean(account)),
        rooms: Object.values(state.rooms).map(toPublicRoom),
    };
}

function findLatestServerLaunchBat(): RouteResult<string> {
    if (!fs.existsSync(PACKAGE_BUILDS_DIR)) {
        return { error: `PackageBuilds directory does not exist: ${PACKAGE_BUILDS_DIR}`, statusCode: 404 };
    }

    const candidates: ServerLaunchCandidate[] = fs.readdirSync(PACKAGE_BUILDS_DIR, { withFileTypes: true })
        .filter((entry: any) => entry.isDirectory() && /^NoOutsiders_Ver_/.test(entry.name))
        .map((entry: any): ServerLaunchCandidate => {
            const packagePath = path.join(PACKAGE_BUILDS_DIR, entry.name);
            const launchBatPath = path.join(packagePath, SERVER_LAUNCH_BAT_RELATIVE_PATH);
            return {
                launchBatPath,
                packageName: entry.name,
                mtimeMs: fs.statSync(packagePath).mtimeMs,
            };
        })
        .filter((candidate: ServerLaunchCandidate) => fs.existsSync(candidate.launchBatPath))
        .sort((a: ServerLaunchCandidate, b: ServerLaunchCandidate) => b.packageName.localeCompare(a.packageName) || b.mtimeMs - a.mtimeMs);

    const latest = candidates[0];
    if (!latest) {
        return { error: "No packaged server RunServerWithLog.bat was found", statusCode: 404 };
    }

    return { value: latest.launchBatPath };
}

function startPackagedGameServer(roomId: RoomId): RouteResult<{ launchBatPath: string; room: StoredRoomData; serverAddress: string }> {
    const room = getRoom(roomId);
    if (!room) {
        return { error: "Room does not exist", statusCode: 404 };
    }

    if (room.ServerAddress) {
        return {
            value: {
                launchBatPath: "",
                room,
                serverAddress: room.ServerAddress,
            },
        };
    }

    const launchBatResult = findLatestServerLaunchBat();
    if (launchBatResult.error || !launchBatResult.value) {
        return { error: launchBatResult.error, statusCode: launchBatResult.statusCode };
    }

    const launchBatPath = launchBatResult.value;
    const launcher = childProcess.spawn(
        "cmd.exe",
        ["/c", "start", "NoOutsiders Server", launchBatPath],
        {
            cwd: path.dirname(launchBatPath),
            detached: true,
            stdio: "ignore",
            windowsHide: false,
        });

    launcher.unref();

    const serverAddress = PACKAGED_SERVER_ADDRESS;
    room.ServerAddress = serverAddress;
    room.UpdatedAt = nowIso();
    saveState(state);

    logServer(`Packaged server launched. bat=${launchBatPath}`);
    return {
        value: {
            launchBatPath,
            room,
            serverAddress,
        },
    };
}

// Tiny router for the local GameServer API. It deliberately avoids external dependencies so the tool is easy to run.
async function route(request: any, response: any): Promise<void> {
    const url = new URL(request.url, `http://${request.headers.host || "localhost"}`);
    const method = request.method || "GET";
    const parts = url.pathname.split("/").filter(Boolean).map(decodeURIComponent);
    logServer(`Request ${method} ${url.pathname}`);

    if (method === "OPTIONS") {
        sendJson(response, 204, {});
        return;
    }

    if (method === "GET" && parts.length === 1 && parts[0] === "health") {
        sendJson(response, 200, {
            ok: true,
            service: "NoOutsiders GameServer",
            time: nowIso(),
        });
        return;
    }

    if (method === "GET" && parts.length === 1 && parts[0] === "state") {
        sendJson(response, 200, {
            ok: true,
            data: {
                accounts: Object.values(state.accounts).map(toPublicAccount),
                lobby: getLobbySnapshot(),
            },
        });
        return;
    }

    if (method === "POST" && parts.length === 2 && parts[0] === "game" && parts[1] === "start") {
        const body = await readJsonBody<StartGameBody>(request);
        const result = startPackagedGameServer(normalizeString(body.roomId));
        if (result.error || !result.value) {
            sendError(response, result.statusCode || 500, result.error || "Start game failed");
            return;
        }

        sendJson(response, 200, {
            ok: true,
            data: {
                launchBatPath: result.value.launchBatPath,
                serverAddress: result.value.serverAddress,
                room: toPublicRoom(result.value.room),
            },
        });
        return;
    }

    if (method === "POST" && parts.length === 2 && parts[0] === "accounts" && parts[1] === "login") {
        const body = await readJsonBody<LoginBody>(request);
        const loginResult = loginOrRegisterAccount(body.accountId, body.playerName);
        const account = loginResult.account;
        enterLobby(account.accountId);
        saveState(state);
        sendJson(response, 200, {
            ok: true,
            data: {
                account: toPublicAccount(account),
                bRegistered: loginResult.bRegistered,
                sessionToken: account.sessionToken,
            },
        });
        return;
    }

    if (parts[0] === "accounts" && parts.length === 2) {
        const account = getAccount(parts[1]);
        if (!account) {
            sendError(response, 404, "Account does not exist");
            return;
        }

        if (method === "GET") {
            sendJson(response, 200, { ok: true, data: toPublicAccount(account) });
            return;
        }

        if (method === "PATCH") {
            const body = await readJsonBody<UpdateAccountBody>(request);
            const playerName = normalizeString(body.playerName);
            if (playerName) {
                account.playerName = playerName;
                account.updatedAt = nowIso();
                logServer(`Account name updated. accountId=${account.accountId}, playerName=${playerName}`);

                for (const room of Object.values(state.rooms)) {
                    const member = room.Members.find((candidate) => candidate.PlayerId === account.accountId);
                    if (member) {
                        member.PlayerName = playerName;
                        room.UpdatedAt = nowIso();
                    }
                }
            }

            saveState(state);
            sendJson(response, 200, { ok: true, data: toPublicAccount(account) });
            return;
        }
    }

    if (parts[0] === "lobby") {
        if (method === "GET" && parts.length === 1) {
            sendJson(response, 200, { ok: true, data: getLobbySnapshot() });
            return;
        }

        if (method === "POST" && parts.length === 2 && parts[1] === "enter") {
            const body = await readJsonBody<AccountBody>(request);
            const account = enterLobby(normalizeString(body.accountId));
            if (!account) {
                sendError(response, 404, "Account does not exist");
                return;
            }

            saveState(state);
            sendJson(response, 200, { ok: true, data: getLobbySnapshot() });
            return;
        }

        if (method === "POST" && parts.length === 2 && parts[1] === "leave") {
            const body = await readJsonBody<AccountBody>(request);
            leaveLobby(normalizeString(body.accountId));
            saveState(state);
            sendJson(response, 200, { ok: true, data: getLobbySnapshot() });
            return;
        }
    }

    if (parts[0] === "rooms") {
        if (method === "GET" && parts.length === 1) {
            sendJson(response, 200, { ok: true, data: Object.values(state.rooms).map(toPublicRoom) });
            return;
        }

        if (method === "POST" && parts.length === 1) {
            const body = await readJsonBody<CreateRoomBody>(request);
            const result = createRoom(normalizeString(body.hostAccountId), body.roomName, body.maxPlayers);
            if (result.error || !result.value) {
                sendError(response, result.statusCode || 400, result.error || "Create room failed");
                return;
            }

            saveState(state);
            sendJson(response, 201, { ok: true, data: toPublicRoom(result.value) });
            return;
        }

        const roomId = parts[1];
        if (method === "GET" && parts.length === 2) {
            const room = getRoom(roomId);
            if (!room) {
                sendError(response, 404, "Room does not exist");
                return;
            }

            sendJson(response, 200, { ok: true, data: toPublicRoom(room) });
            return;
        }

        if (method === "DELETE" && parts.length === 2) {
            if (!getRoom(roomId)) {
                sendError(response, 404, "Room does not exist");
                return;
            }

            delete state.rooms[roomId];
            logServer(`Room deleted. roomId=${roomId}`);
            saveState(state);
            sendJson(response, 200, { ok: true });
            return;
        }

        if (method === "POST" && parts.length === 3 && parts[2] === "join") {
            const body = await readJsonBody<AccountBody>(request);
            const result = joinRoom(roomId, normalizeString(body.accountId));
            if (result.error || !result.value) {
                sendError(response, result.statusCode || 400, result.error || "Join room failed");
                return;
            }

            saveState(state);
            sendJson(response, 200, { ok: true, data: toPublicRoom(result.value) });
            return;
        }

        if (method === "POST" && parts.length === 3 && parts[2] === "leave") {
            const body = await readJsonBody<AccountBody>(request);
            const result = leaveRoom(roomId, normalizeString(body.accountId));
            if (result.error) {
                sendError(response, result.statusCode || 400, result.error);
                return;
            }

            saveState(state);
            sendJson(response, 200, {
                ok: true,
                data: result.value ? toPublicRoom(result.value) : undefined,
                roomDeleted: Boolean(result.roomDeleted),
            });
            return;
        }

        if (method === "PATCH" && parts.length === 4 && parts[2] === "members") {
            const body = await readJsonBody<UpdateRoomMemberBody>(request);
            const result = updateRoomMember(roomId, parts[3], body);
            if (result.error || !result.value) {
                sendError(response, result.statusCode || 400, result.error || "Update room member failed");
                return;
            }

            saveState(state);
            sendJson(response, 200, { ok: true, data: toPublicRoom(result.value) });
            return;
        }
    }

    sendError(response, 404, "Route not found");
}

const server = http.createServer((request: any, response: any) => {
    route(request, response).catch((error: Error) => {
        sendError(response, 500, error.message || "Internal server error");
    });
});

server.listen(PORT, () => {
    logServer(`Listening on http://127.0.0.1:${PORT}`);
    logServer(`Packaged server travel address: ${PACKAGED_SERVER_ADDRESS}`);
    logServer(`State file: ${DATA_FILE}`);
    logServer(`Initial state. accounts=${Object.keys(state.accounts).length}, online=${state.lobby.onlineAccountIds.length}, rooms=${Object.keys(state.rooms).length}`);
});
