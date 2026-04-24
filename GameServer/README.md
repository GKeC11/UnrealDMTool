# NoOutsiders GameServer

Lightweight HTTP game server for managing:

- Account data, including player names and session tokens.
- Lobby data, including currently online accounts.
- Room data shaped like the Unreal `FNORoomData` / `FNORoomMemberData` structs.

The server is written in TypeScript. Main source: `src/server.ts`.

It uses only Node built-in modules and stores state in `GameServer/data/state.json`.

## Start

```powershell
cd Plugins/DMToolBox/GameServer
npm start
```

On Windows, double-click `StartGameServer.bat` in this directory.

Useful development commands:

```powershell
npm run check
npm run build
```

Optional environment variables:

- `PORT`: HTTP port. Defaults to `7788`.
- `GAME_SERVER_DATA`: state file path. Defaults to `GameServer/data/state.json`.

## Main APIs

```http
GET  /health
GET  /state
POST /accounts/login
GET  /accounts/:accountId
PATCH /accounts/:accountId
POST /lobby/enter
POST /lobby/leave
GET  /lobby
GET  /rooms
POST /rooms
GET  /rooms/:roomId
POST /rooms/:roomId/join
POST /rooms/:roomId/leave
PATCH /rooms/:roomId/members/:accountId
DELETE /rooms/:roomId
```

Example login:

```json
{
  "accountId": "player_001",
  "playerName": "Alice"
}
```

`POST /accounts/login` logs in an existing account, or registers it immediately when `accountId` does not exist. The response `data.bRegistered` is `true` only for newly-created accounts.

Example create room:

```json
{
  "hostAccountId": "player_001",
  "roomName": "Lobby",
  "maxPlayers": 8
}
```

Room responses use Unreal-style field names:

```json
{
  "RoomId": "room_xxx",
  "RoomName": "Lobby",
  "MaxPlayers": 8,
  "bIsSynchronized": true,
  "Members": [
    {
      "PlayerId": "player_001",
      "PlayerName": "Alice",
      "bIsReady": false,
      "bIsHost": true
    }
  ]
}
```
