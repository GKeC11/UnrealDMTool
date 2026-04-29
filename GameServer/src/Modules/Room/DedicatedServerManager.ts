import crypto = require("node:crypto");
import childProcess = require("node:child_process");
import fs = require("node:fs");
import os = require("node:os");
import path = require("node:path");
import { Logger } from "../../Utils/Logger";

export type DedicatedServerSession = {
    roomId: string;
    address: string;
    port: number;
    ticket: string;
    startedAt: string;
    launchBatPath: string;
};

export class DedicatedServerManager {
    private readonly logger = new Logger("DedicatedServerManager");
    private readonly sessions = new Map<string, DedicatedServerSession>();

    public startDS(roomId: string): DedicatedServerSession {
        const existing = this.sessions.get(roomId);
        if (existing) {
            this.logger.info(`Reuse dedicated server session. roomId=${roomId}, address=${existing.address}`);
            return existing;
        }

        const launchBatPath = this.resolveLaunchBatPath();
        if (!launchBatPath) {
            throw new Error("No packaged server RunServerWithLog.bat was found");
        }

        const fixedAddress = String(process.env.PACKAGED_SERVER_ADDRESS || process.env.MOCK_DS_ADDRESS || "").trim();
        const port = this.resolvePort(fixedAddress);
        const host = this.resolveHost(fixedAddress);
        const session: DedicatedServerSession = {
            roomId,
            address: `${host}:${port}`,
            port,
            ticket: crypto.randomBytes(16).toString("hex"),
            startedAt: new Date().toISOString(),
            launchBatPath,
        };

        this.launchPackagedServer(launchBatPath, roomId);
        this.sessions.set(roomId, session);
        this.logger.info(`Dedicated server launched. roomId=${roomId}, address=${session.address}, bat=${launchBatPath}`);
        return session;
    }

    public stopDS(roomId: string): void {
        const existing = this.sessions.get(roomId);
        if (!existing) {
            return;
        }

        this.sessions.delete(roomId);
        this.logger.info(`Dedicated server session removed. roomId=${roomId}, address=${existing.address}`);
    }

    private resolveHost(fixedAddress: string): string {
        const fixedHost = fixedAddress.includes(":") ? fixedAddress.split(":").slice(0, -1).join(":").trim() : "";
        return fixedHost || String(process.env.PACKAGED_SERVER_HOST || process.env.MOCK_DS_HOST || this.getLanIPv4Address() || "127.0.0.1").trim() || "127.0.0.1";
    }

    private resolvePort(fixedAddress: string): number {
        const fixedPort = Number(fixedAddress.split(":").pop());
        if (Number.isInteger(fixedPort) && fixedPort > 0) {
            return fixedPort;
        }

        const packagedPort = Number(process.env.PACKAGED_SERVER_PORT || 7777);
        return Number.isInteger(packagedPort) && packagedPort > 0 ? packagedPort : 7777;
    }

    private launchPackagedServer(launchBatPath: string, roomId: string): void {
        const launcher = childProcess.spawn(
            "cmd.exe",
            ["/c", "call", launchBatPath],
            {
                cwd: path.dirname(launchBatPath),
                detached: true,
                env: {
                    ...process.env,
                    NO_ROOM_ID: roomId,
                    NO_GAME_SERVER_ROOM_ID: roomId,
                },
                stdio: "ignore",
                windowsHide: false,
            });

        launcher.unref();
    }

    private resolveLaunchBatPath(): string | undefined {
        const configuredPath = String(process.env.PACKAGED_SERVER_LAUNCH_BAT || "").trim();
        if (configuredPath && fs.existsSync(configuredPath)) {
            return configuredPath;
        }

        return this.findLatestServerLaunchBat();
    }

    private findLatestServerLaunchBat(): string | undefined {
        const packageBuildsDir = path.resolve(this.getRepoRoot(), "PackageBuilds");
        if (!fs.existsSync(packageBuildsDir)) {
            return undefined;
        }

        const candidates = fs.readdirSync(packageBuildsDir, { withFileTypes: true })
            .filter((entry: any) => entry.isDirectory() && /^NoOutsiders_Ver_/.test(entry.name))
            .map((entry: any) => {
                const packagePath = path.join(packageBuildsDir, entry.name);
                return {
                    launchBatPath: path.join(packagePath, "Server", "WindowsServer", "RunServerWithLog.bat"),
                    packageName: entry.name,
                    mtimeMs: fs.statSync(packagePath).mtimeMs,
                };
            })
            .filter((candidate: { launchBatPath: string }) => fs.existsSync(candidate.launchBatPath))
            .sort((a: { packageName: string; mtimeMs: number }, b: { packageName: string; mtimeMs: number }) => b.packageName.localeCompare(a.packageName) || b.mtimeMs - a.mtimeMs);

        return candidates[0]?.launchBatPath;
    }

    private getRepoRoot(): string {
        return path.resolve(__dirname, "..", "..", "..", "..", "..", "..");
    }

    private getLanIPv4Address(): string | undefined {
        const networkInterfaces = os.networkInterfaces();
        const fallbackAddresses: string[] = [];

        for (const entries of Object.values(networkInterfaces) as any[]) {
            for (const entry of entries ?? []) {
                if (!entry || entry.family !== "IPv4" || entry.internal) {
                    continue;
                }

                const address = String(entry.address || "").trim();
                if (!address) {
                    continue;
                }

                if (this.isPrivateIPv4(address)) {
                    return address;
                }

                fallbackAddresses.push(address);
            }
        }

        return fallbackAddresses[0];
    }

    private isPrivateIPv4(address: string): boolean {
        const parts = address.split(".").map((part) => Number(part));
        if (parts.length !== 4 || parts.some((part) => !Number.isInteger(part) || part < 0 || part > 255)) {
            return false;
        }

        return parts[0] === 10
            || (parts[0] === 172 && parts[1] >= 16 && parts[1] <= 31)
            || (parts[0] === 192 && parts[1] === 168);
    }
}
