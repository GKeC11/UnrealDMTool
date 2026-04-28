export type LogLevel = "debug" | "info" | "warn" | "error";

const logPriority: Record<LogLevel, number> = {
    debug: 10,
    info: 20,
    warn: 30,
    error: 40,
};

function resolveLogLevel(): LogLevel {
    const configured = String(process.env.LOG_LEVEL || "debug").toLowerCase();
    if (configured === "debug" || configured === "info" || configured === "warn" || configured === "error") {
        return configured;
    }

    return "debug";
}

export class Logger {
    private readonly tag: string;

    public constructor(tag: string) {
        this.tag = tag;
    }

    public debug(message: string, data?: unknown): void {
        this.write("debug", message, data);
    }

    public info(message: string, data?: unknown): void {
        this.write("info", message, data);
    }

    public warn(message: string, data?: unknown): void {
        this.write("warn", message, data);
    }

    public error(message: string, data?: unknown): void {
        this.write("error", message, data);
    }

    private write(level: LogLevel, message: string, data?: unknown): void {
        if (logPriority[level] < logPriority[resolveLogLevel()]) {
            return;
        }

        const prefix = `[${new Date().toISOString()}][${level.toUpperCase()}][${this.tag}]`;
        if (typeof data === "undefined") {
            console.log(`${prefix} ${message}`);
            return;
        }

        console.log(`${prefix} ${message}`, data);
    }
}
