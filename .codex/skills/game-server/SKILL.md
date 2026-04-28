---
name: game-server
description: Work on the TypeScript Node.js GameServer under `Plugins/DMToolBox/GameServer`. Use when Codex edits, reviews, or extends GameServer server code, protocols, connection handling, routing, room/lobby/account logic, utilities, or documentation. Require necessary logs through `src/Utils/Logger.ts` and concise comments for non-obvious server behavior.
---

# GameServer

Use this skill for TypeScript GameServer work in `Plugins/DMToolBox/GameServer`.

## Workflow

1. Inspect the target module and nearby protocol/types before editing.
2. Keep the code aligned with the existing TypeScript style: PascalCase file names for custom source files, CommonJS-compatible imports, strict typing, and small focused classes or helpers.
3. Route client messages through `MessageRouter` and keep protocol numbers in `src/Protocol/*Define.ts`.
4. Add or update logs through `src/Utils/Logger.ts` when behavior changes.
5. Add concise comments only where they explain intent, protocol constraints, lifecycle ordering, or non-obvious edge cases.

## Logging Rules

- Use `Logger` from `src/Utils/Logger.ts`; do not introduce direct `console.log`, `console.warn`, or `console.error` in GameServer source code.
- Create one logger per class or module with a stable tag, for example `new Logger("GameServer")` or `new Logger("ConnectionManager")`.
- Log important lifecycle events: server start/stop, WebSocket connection open/close, authentication result, room creation/join/leave, heartbeat cleanup, and fatal or recoverable errors.
- Use `debug` for high-frequency message flow and diagnostic details, `info` for normal lifecycle milestones, `warn` for recoverable abnormal states, and `error` for failed operations or exceptions.
- Include useful correlation fields in log messages, especially `connection id`, `userId`, `roomId`, and protocol `type` when available.
- Avoid logging sensitive values such as tokens, passwords, or raw credentials. If needed, log presence or a short non-sensitive identifier instead.
- Do not spam logs inside tight loops or per-frame style paths unless the message is `debug` and genuinely useful for diagnosis.

## Comment Rules

- Prefer Chinese comments in GameServer source unless matching existing English code or public protocol names.
- Explain why a branch, guard, protocol rule, cleanup step, or handshake detail exists.
- Comment WebSocket framing, lifecycle cleanup, heartbeat behavior, authentication assumptions, and protocol compatibility when the intent is not obvious.
- Do not add comments for simple assignments, straightforward getters/setters, or code that is already clear from names and types.

## Architecture Rules

- Keep connection lifecycle logic in `ConnectionManager` and per-client state in `ConnectionContext`.
- Keep protocol response construction in `ResponseHelper` instead of duplicating response envelopes.
- Keep protocol enum ranges organized by domain: base/system, account, lobby, and room.
- Prefer narrow typed payload interfaces for new request/response shapes instead of `any`.
- Validate malformed client input early and return protocol errors consistently.
- Avoid cross-module direct coupling for business flows; register handlers through `MessageRouter` or a focused module boundary.

## Verification

- Run the GameServer TypeScript build or the closest project build script after code changes when available.
- If build tooling is unavailable, at least inspect changed imports, exported types, protocol enum usage, and logger usage before finishing.
