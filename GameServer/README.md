
# Node.js GameServer 实现方案（TypeScript 版 · 大驼峰命名）

**版本**：3.0  
**适用架构**：UE5 客户端 + Node.js 大厅服务 + UE5 DS 战斗  
**通信方式**：WebSocket（`ws` 库，JSON 消息，int 协议号）  
**业务模式**：创建房间 / 加入房间（无自动匹配）  
**语言**：TypeScript  
**命名规范**：自定义文件/类统一大驼峰（PascalCase），保留标准文件（`package.json`、`tsconfig.json`、`.env`）原样  

---

## 1. 概述

GameServer 是一个独立的 Node.js 进程，负责处理所有非实时游戏逻辑（账号、大厅、房间），并通过 WebSocket 长连接与 UE5 客户端通信。战斗由 UE5 Dedicated Server（DS）接管。  
本方案遵循以下原则：

- **库**：原生 `ws` 库，无额外框架封装。
- **协议**：消息顶层格式 `{ type: int, payload: object }`，所有协议号由 `Protocol` 目录统一定义。
- **模块化**：各业务模块通过 `MessageRouter` 解耦，模块间不直接引用。
- **配置分离**：环境相关/敏感配置放在 `.env`，应用通用参数放在 `config/Default.json`。
- **类型安全**：全面使用 TypeScript，开启严格模式。

---

## 2. 项目目录结构

```
game-server/
├── src/
│   ├── Server.ts                     # 入口，启动游戏服务器
│   ├── GameServer.ts                 # 核心协调器类
│   ├── MessageRouter.ts              # 消息分发路由
│   ├── ConnectionManager.ts          # 连接生命周期管理
│   ├── ConnectionContext.ts          # 连接上下文
│   ├── Modules/
│   │   ├── Account/
│   │   │   ├── AccountModule.ts
│   │   │   └── AuthService.ts
│   │   ├── Lobby/
│   │   │   └── LobbyModule.ts        # 房间列表、公告等（不含匹配）
│   │   └── Room/
│   │       ├── RoomModule.ts
│   │       └── DedicatedServerManager.ts  # 原 dsManager
│   ├── Protocol/
│   │   ├── ProtocolDefine.ts         # 基础协议号 + 范围定义
│   │   ├── AccountDefine.ts
│   │   ├── LobbyDefine.ts
│   │   └── RoomDefine.ts
│   └── Utils/
│       ├── Logger.ts
│       └── ResponseHelper.ts
├── config/
│   └── Default.json                  # 应用通用配置
├── .env                              # 敏感/环境配置
├── package.json
├── tsconfig.json
└── .gitignore
```

---

## 3. TypeScript 配置与依赖

### 3.1 `tsconfig.json`（关键项）
```json
{
  "compilerOptions": {
    "target": "ES2022",
    "module": "commonjs",
    "outDir": "./dist",
    "rootDir": "./src",
    "strict": true,
    "esModuleInterop": true
  },
  "include": ["src/**/*"]
}
```

### 3.2 依赖安装
```bash
npm init -y
npm install ws jsonwebtoken dotenv
npm install -D typescript @types/ws @types/jsonwebtoken @types/node
```

### 3.3 启动脚本
在 `package.json` 中：
```json
"scripts": {
  "build": "tsc",
  "start": "node dist/Server.js",
  "dev": "tsc && node dist/Server.js"
}
```

---

## 4. 核心类型定义

### 4.1 通用消息结构
```ts
export interface GameMessage {
  type: number;
  payload: Record<string, any>;
}
```

### 4.2 连接上下文 Context
```ts
import WebSocket from 'ws';

export interface ConnectionContext {
  id: string;
  ws: WebSocket;
  userId?: string;
  isAuthenticated: boolean;
  lastHeartbeat: number;
  sessionData: Record<string, any>;
}

export function createConnectionContext(ws: WebSocket, id: string): ConnectionContext {
  return {
    id,
    ws,
    isAuthenticated: false,
    lastHeartbeat: Date.now(),
    sessionData: {}
  };
}
```

### 4.3 消息处理函数类型
```ts
export type MessageHandler = (
  ctx: ConnectionContext,
  payload: any
) => void | Promise<void>;
```

---

## 5. 协议定义

### 5.1 协议号分段
| 范围   | 用途         |
|--------|--------------|
| 1 ~ 99 | 基础系统     |
| 100~199| 账号模块     |
| 200~299| 大厅模块     |
| 300~399| 房间模块     |

### 5.2 各 Define 文件示例

```ts
// Protocol/ProtocolDefine.ts
export enum ProtocolBase {
  PING = 1,
  PONG = 2,
  ERROR = 99,
}

// Protocol/AccountDefine.ts
export enum ProtocolAccount {
  AUTH_REQUEST = 101,
  AUTH_RESPONSE = 102,
  TOKEN_REFRESH = 103,
}

// Protocol/LobbyDefine.ts (本次可能空置，按需添加)
export enum ProtocolLobby {
  // ROOM_LIST = 201,
  // FRIEND_INVITE = 202,
}

// Protocol/RoomDefine.ts
export enum ProtocolRoom {
  CREATE_ROOM = 301,
  ROOM_CREATED = 302,
  JOIN_ROOM = 303,
  ROOM_UPDATE = 304,
  START_GAME = 305,
  GAME_STARTING = 306,
}
```

> **同步要求**：UE5 客户端需维护相同的协议号枚举（C++ `MessageTypes.h`）。

---

## 6. 核心模块设计

### 6.1 Server.ts（入口）
- 加载 `dotenv` 与 `config/Default.json`
- 合并配置（环境变量覆盖 JSON 默认值）
- 创建 `GameServer` 实例并启动
- 处理 `SIGTERM` 信号优雅关闭

### 6.2 GameServer.ts（协调器）
- 维护 `WebSocket.Server` 实例
- 持有 `ConnectionManager`、`MessageRouter`、各业务模块实例
- 提供核心方法：
  - `sendToClient(ctx: ConnectionContext, type: number, payload: object): void`
  - `broadcast(type: number, payload: object, filter?: (ctx: ConnectionContext) => boolean): void`
- 监听 WebSocket 事件：
  - `connection`：创建 Context 并交给 ConnectionManager
  - `message`：解析 JSON，特殊处理 `PING (1)` 直接回复 `PONG` 并刷新心跳，其余交由 `MessageRouter.dispatch`
  - `close`/`error`：清理连接

### 6.3 MessageRouter.ts
- 私有路由表 `Map<number, MessageHandler>`
- `register(protocolId: number, handler: MessageHandler): void`
- `dispatch(protocolId: number, ctx: ConnectionContext, payload: any): void`
  - 路由未找到 → 回复 `ERROR` (99)
  - 处理函数异常 → 日志记录且回复通用错误
- 可在此层加入鉴权中间件（检查 `ctx.isAuthenticated` 与协议号范围）

### 6.4 ConnectionManager.ts
- 管理 `Map<string, ConnectionContext>` 活跃连接
- 定期扫描超时连接（当前时间 - `lastHeartbeat` > 90s），`ws.terminate()` 断开
- 连接关闭时触发清理回调：移除匹配队列（如果有），离开房间，广播房间更新

### 6.5 AccountModule / AuthService
- **AuthService**：封装 JWT 签发与验证（`jwt.sign`, `jwt.verify`）
- **AccountModule**：
  - 注册 `AUTH_REQUEST (101)` 处理登录
  - 验证账号密码后签发 Token，更新 `ctx.userId`、`ctx.isAuthenticated`
  - 回复 `AUTH_RESPONSE (102) { success, token }`

### 6.6 LobbyModule（简化版）
- 无匹配队列，可提供大厅基础功能：
  - 房间列表广播（从 RoomModule 获取公开房间数据）
  - 后续可扩展好友邀请、聊天等

### 6.7 RoomModule / DedicatedServerManager
- **RoomModule**：
  - 内部状态：`Map<string, Room>`
  - `createRoom(ownerId, params)` → 调用 `DedicatedServerManager.startDS()` 获取地址 → 生成 `roomId` → 回复 `ROOM_CREATED (302)`
  - `joinRoom(userId, roomId)` → 验证容量 → 加入玩家列表 → 广播 `ROOM_UPDATE (304)`
  - `startGame(roomId, requesterId)` → 校验房主 → 通知双方 `GAME_STARTING (306)`，携带 DS 地址与 Token
- **DedicatedServerManager**：
  - 封装 DS 进程生命周期，初期使用 `child_process.spawn` 启动 UE5 服务器
  - 返回 `{ address: string, port: number }`
  - 连接关闭或房间销毁时停止对应 DS
  - 接口设计便于后续替换为云端 API / Kubernetes

---

## 7. 工具模块

### 7.1 Logger.ts
- 分级日志：`debug`, `info`, `warn`, `error`
- 自动添加时间戳与模块标签（如 `[RoomModule]`）
- 生产环境可屏蔽 `debug`，后续可对接文件日志或远程日志服务

### 7.2 ResponseHelper.ts
- 静态方法 `build(type: number, payload: object): string` 直接返回 JSON 字符串
- `buildError(code: number, message: string): string` 生成 `{ type: 99, payload: { errorCode, message } }`

---

## 8. 消息流详解（登录→创建房间→加入房间→进入 DS）

### 阶段一：连接建立
1. 客户端 A、B 分别向 GameServer 发起 WebSocket 连接。
2. GameServer 接受连接，为各自创建 `ConnectionContext`，初始化未认证状态，并交给 `ConnectionManager` 管理。

### 阶段二：登录（客户端 A 与 B 各自执行）
1. **客户端 A** 发送 `{ type: 101, payload: { account, password } }`。
2. GameServer 接收后调用 `MessageRouter.dispatch(101, ctxA, payload)`。
3. Router 寻址到 `AccountModule.onAuth`，模块校验凭证，通过后使用 `AuthService` 签发 JWT，更新 `ctxA.userId`、`ctxA.isAuthenticated = true`。
4. 通过 `GameServer.sendToClient(ctxA, 102, { success: true, token })` 回复客户端 A。
5. **客户端 B** 执行完全相同的流程，完成登录并持有其独立的 Token。

### 阶段三：创建房间（由房主 A 发起）
1. 客户端 A 发送 `{ type: 301, payload: { map: "DeathMatch", maxPlayers: 4 } }`。
2. GameServer → MessageRouter → `RoomModule.onCreateRoom(ctxA, payload)`。
3. `RoomModule` 调用 `DedicatedServerManager.startDS()`。
4. `DedicatedServerManager` 通过 `child_process.spawn` 启动 UE5 DS 可执行文件，等待 DS 就绪后获得 `{ address, port }`。
5. `RoomModule` 创建房间对象（生成唯一 roomId，记录房主、地图、最大人数等信息），将 DS 地址与房间关联。
6. 通过 `sendToClient(ctxA, 302, { roomId, address: "192.168.1.100:7777" })` 回复房主，告知房间已创建。

### 阶段四：加入房间（由客户端 B 发起）
1. 客户端 B 获得 roomId（例如通过大厅的房间列表，或房主分享），发送 `{ type: 303, payload: { roomId } }`。
2. Router 分发至 `RoomModule.onJoinRoom(ctxB, payload)`。
3. RoomModule 校验房间是否存在、是否未满、是否处于等待状态，检查通过后将 B 的 userId 加入房间玩家列表。
4. 向房间内所有玩家（目前 A 和 B）广播 `ROOM_UPDATE (304)`，payload 包含完整玩家列表 `{ players: [{userId, name}, ...] }`，确保双方 UI 同步显示当前成员。

### 阶段五：开始游戏（由房主 A 发起）
1. 客户端 A 在房间满员或准备就绪后，发送 `{ type: 305, payload: { roomId } }`。
2. Router 分发至 `RoomModule.onStartGame(ctxA, payload)`。
3. RoomModule 检查请求者是否为房主，确认房间状态允许开始，更新房间状态为 `playing`。
4. 向房间内所有玩家（A 和 B）分别发送 `GAME_STARTING (306)`，payload 中携带 `{ address: "192.168.1.100:7777", token: "<AuthToken>" }`，确保双方拿到连接 DS 的必要信息。

### 阶段六：进入 DS 战斗
1. 客户端 A、B 收到 `GAME_STARTING` 后，各自调用本地 `ClientTravel` 或等效方法，携带 Token 直连 DS 地址。
2. DS 验证 Token 有效性后，将双方纳入游戏会话，游戏同步正式开始。
3. WebSocket 连接仍可保持用于好友聊天等非战斗功能，但战斗数据完全走 DS 协议。

---

## 9. 心跳与断线处理

- **应用层心跳**：客户端每 45 秒发送 `PING (1)`，服务器回复 `PONG (2)` 并刷新 `context.lastHeartbeat`。
- **超时检测**：`ConnectionManager` 每 10 秒扫描一次，超过 90 秒未心跳则 `ws.terminate()`。
- **清理逻辑**：
  - 从 `ConnectionManager` 移除上下文
  - 若在房间内，视为离开房间，更新房间列表并广播 `ROOM_UPDATE`
  - 若为房主，可考虑关闭房间（根据策略）

---

## 10. 配置说明

### `.env` （不入库）
```
WS_PORT=8080
JWT_SECRET=change-me-to-a-random-string
DS_EXECUTABLE_PATH=/game/YourProject/Binaries/Linux/YourProjectServer
```

### `config/Default.json` （入版本库）
```json
{
  "heartbeat": {
    "intervalMs": 45000,
    "timeoutMs": 90000
  },
  "room": {
    "defaultMaxPlayers": 4
  },
  "ds": {
    "startTimeoutMs": 30000
  },
  "logLevel": "info"
}
```

启动时在 `Server.ts` 中合并：
```ts
const defaultConfig = require('../config/Default.json');
const config = {
  ...defaultConfig,
  wsPort: process.env.WS_PORT || 8080,
  jwtSecret: process.env.JWT_SECRET,
  dsPath: process.env.DS_EXECUTABLE_PATH,
};
```

---

## 11. 开发顺序建议

1. **项目初始化**  
   - 搭建目录结构，初始化 `npm` 与 TypeScript 配置。
2. **基础框架**  
   - 实现 `Logger.ts`、`ResponseHelper.ts`。
   - 实现 `GameServer.ts`（WebSocket 创建与连接管理）。
   - 实现 `ConnectionContext.ts`、`ConnectionManager.ts`、`MessageRouter.ts`。
3. **协议定义**  
   - 创建 `Protocol/` 下四个文件，确定所有协议号常量。
4. **账号系统**  
   - 实现 `AuthService` 与 `AccountModule`，支持登录与 Token。
5. **房间系统**  
   - 实现 `RoomModule` 与 `DedicatedServerManager`。
   - 用模拟 DS 地址验证房间创建、加入、开始游戏流程。
6. **大厅**  
   - 实现基础 `LobbyModule`（房间列表推送等）。
7. **联调**  
   - 与 UE5 客户端对接完整流程，加入心跳和异常处理测试。

---

## 12. 扩展预留

- **数据库/持久化**：Room 状态可抽象 `RoomStore` 接口，当前内存实现，后续替换为 Redis/数据库。
- **多进程水平扩展**：引入 Redis Pub/Sub 同步房间信息、连接迁移等（需重构 ConnectionManager）。
- **DS 云管**：`DedicatedServerManager` 预留云 API 实现，可替换本地 `spawn` 为 Kubernetes Operator。
- **协议自动生成**：基于 Protocol 目录的 TypeScript 枚举，可编写脚本生成 UE C++ 对应枚举，保证双端一致。

---
