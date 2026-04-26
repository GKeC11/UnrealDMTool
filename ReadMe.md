# DMToolBox 使用指南

`DMToolBox` 是一个偏项目基础设施的插件，主要提供 4 类能力：

- UI 框架：`UIScreen / UILayout / Layer / 按 GameplayTag 打开 Widget`
- Gameplay 辅助：输入映射注册、关卡初始化配置读取
- Camera 辅助：运行时管理自定义相机 Actor
- Puerts 辅助：`GameInstance` TS 初始化入口、控制台命令注册

这份文档重点说明“怎么接入”和“开发时怎么用”。

## 目录概览

- `Framework/UI`
  - UI 运行时框架，负责 `UIScreen`、`UILayout`、Layer、Widget 容器
- `Framework/Library`
  - 蓝图/代码可调用的工具函数库
- `Framework/Config`
  - `DMToolBoxDeveloperSetting`，放插件级配置
- `Framework/Gameplay`
  - 关卡初始化配置、世界设置、GameInstance 扩展
- `Framework/Camera`
  - 相机 Actor 和相机管理组件
- `Framework/Puerts`
  - Puerts 辅助对象和注册逻辑
- `Framework/Network`
  - GameServer HTTP 客户端、Dedicated Server 生命周期辅助

## 网络职责边界

当前项目把“大厅同步”和“对局同步”拆成两套运行时：

- `Plugins/DMToolBox/GameServer`
  - 轻量 HTTP GameServer，负责账号、在线状态、Lobby、Room 数据同步。
  - Lobby 阶段不依赖 Unreal Dedicated Server。客户端通过 `UDMGameServerSubsystem` 调用 `/lobby`、`/rooms`、`/game/start` 等接口同步房间状态。
  - `/game/start` 用于为指定 Room 启动打包后的 Match Dedicated Server，并把 `ServerAddress` 写回 Room，供客户端跳转连接。
- Unreal Dedicated Server
  - 只负责 Match 阶段的实时对局同步。
  - Match DS 的生命周期逻辑由 `DMDedicatedServerSubsystem` 这类专用服务组件承载，不应该放进 Lobby GameMode 或公共 GameMode 基类。

因此，类似“DS 内玩家全部断开后自动关闭”的逻辑，只适用于 Match DS：

1. `UDMDedicatedServerSubsystem` 只在 Dedicated Server 的 `GameInstance` 中创建。
2. Match GameMode 在 `PostLogin` / `Logout` 中把玩家连接变化转发给 Subsystem。
3. Subsystem 记录是否曾经有玩家成功进入 DS，并在玩家进入时取消空服关闭倒计时。
4. 玩家退出后，如果当前 Match DS 人数为 0，Subsystem 启动短延迟倒计时。
5. 倒计时结束再次确认仍为空服，再请求退出 DS 进程。

Lobby 的在线状态、房间删除、房主转移、准备状态同步仍由 GameServer API 负责，不应通过 DS 玩家连接数推导。

## 快速接入

推荐按下面顺序接：

1. 启用插件并确保项目依赖 `CommonUI`、`GameplayTags`、`EnhancedInput`
2. 在项目设置里打开 `DMToolBox` Developer Settings
3. 配置 `DefaultUIScreenClass`
4. 配置 `WidgetConfigDataTable`
5. 准备一个继承 `UDMUILayout` 的布局蓝图，并在构造时注册 Layer
6. 如果关卡需要默认 UI，在 `WorldSettings` 上配置 `LevelInitializationSetting`
7. 业务逻辑里通过 `UDMUILibrary::CreateWidgetByTagToLayer` / `RemoveWidgetByTag` 打开关闭 UI

## Developer Settings

配置类在 [DMToolBoxDeveloperSetting.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Config/DMToolBoxDeveloperSetting.h:8)。

当前最重要的两个配置项：

- `DefaultUIScreenClass`
  - `UDMUISubsystem` 初始化时会实例化这个 `UIScreen`
- `WidgetConfigDataTable`
  - `UDMUILibrary` 会从这个数据表里根据 `WidgetTag` 查找 `WidgetClass`

也就是说，`DMUILibrary` 已经不再依赖硬编码路径，后续项目只需要改配置，不需要改插件源码。

## UI 框架

### 1. UISubsystem

[UDMUISubsystem](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/UI/DMUISubsystem.cpp:5) 会在 `GameInstanceSubsystem` 初始化时：

- 从 `DMToolBoxDeveloperSetting` 读取 `DefaultUIScreenClass`
- 创建当前运行时的 `UIScreen`
- 监听 `OnLocalPlayerAddedEvent`
- 在本地玩家拥有 `PlayerController` 后，为该玩家注册 Layout

这意味着：

- 你通常不需要手动创建 `UIScreen`
- 本地玩家 UI Layout 的生命周期由 `DMUISubsystem` 驱动

### 2. UIScreen

[UDMUIScreen](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/UI/DMUIScreen.h:8) 负责：

- 为每个 `LocalPlayer` 创建/缓存一个 `UDMUILayout`
- 把 Layout 加到视口
- 根据 `LocalPlayer` 取回当前 Layout

核心函数：

- `RegisterLayoutForLocalPlayer`
- `GetLayoutFromLocalPlayer`
- `GetUIScreen`

### 3. UILayout 和 Layer

[UDMUILayout](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/UI/DMUILayout.h:10) 是具体布局容器。

它的职责是：

- 保存 `LayerTag -> WidgetContainer` 的映射
- 提供 `GetLayerFromGameplayTag`
- 在 `NativeConstruct` 时做初始化

实际接法通常是：

1. 做一个继承 `UDMUILayout` 的 Widget Blueprint
2. 在蓝图里准备多个 `CommonActivatableWidgetContainer`
3. 调用 `RegisterLayer(LayerTag, LayerWidget)` 把容器和标签绑定

### 4. 按 GameplayTag 打开 UI

[UDMUILibrary](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Library/DMUILibrary.h:9) 提供了按 `GameplayTag` 打开和关闭 UI 的入口：

- `CreateWidgetByTagToLayer(WorldContextObject, WidgetTag, LayerTag)`
- `RemoveWidgetByTag(WorldContextObject, WidgetTag)`

运行逻辑：

1. 从 `DMToolBoxDeveloperSetting.WidgetConfigDataTable` 读取数据表
2. 在数据表里查找匹配的 `WidgetTag`
3. 取到对应 `WidgetClass`
4. 找到当前本地玩家的 `UILayout`
5. 把 Widget 加到 `LayerTag` 对应的容器

适合这种场景：

- `StateTree` 进入状态时弹 UI
- 玩法事件驱动 HUD / 面板开关
- 希望业务侧只传 `GameplayTag`，不直接依赖具体 Widget 类

## Widget 配置表

数据结构在 [DMWidgetConfig.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/UI/DMWidgetConfig.h:8)。

每一行至少要有：

- `WidgetTag`
- `WidgetClass`

推荐做法：

- 一个 Widget 对应一个稳定的 `GameplayTag`
- 把 UI 类集中放进同一张数据表
- 用 `LayerTag` 决定显示到哪个 UI Layer，而不是在业务代码里硬编码层级引用

## 关卡初始化

### 1. WorldSetting

[ADMWorldSetting](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Gameplay/Core/DMWorldSetting.h:7) 扩展了世界设置，提供：

- `LevelInitializationSetting`

### 2. LevelInitializationSetting

[UDMLevelInitializationSetting](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Gameplay/Data/DMLevelInitializationSetting.h:21) 是一个 `PrimaryDataAsset`，当前主要用于：

- 配置关卡启动时默认要创建的 Widget

结构 `FLevelInitializationSetting_WidgetConfig` 包含：

- `Layer`
- `WidgetClass`

也就是说，一个关卡可以声明“进入这个关卡时，默认往某个 Layer 加哪些 Widget”。

### 3. SystemLibrary

[UDMSystemLibrary](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Library/DMSystemLibrary.h:7) 提供：

- `GetLevelInitializationSetting(UWorld*)`

适合在 `UILayout`、关卡启动逻辑、或初始化脚本里读取当前世界对应的初始化配置。

## 输入辅助

[UDMGameplayLibrary](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Library/DMGameplayLibrary.h:7) 提供两个常用函数：

- `RegisterInputMappingContext`
- `UnregisterInputMappingContext`

适合：

- UI 打开时注册一套输入映射
- UI 关闭时移除对应输入映射
- 不同玩法阶段切换输入上下文

## Camera 框架

### 1. CameraManagerComponent

[UDMCameraManagerComponent](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Camera/DMCameraManagerComponent.h:12) 是最主要的相机管理入口。

它负责：

- 解析目标 `PlayerController`
- 生成/维护 `ADMCameraActor`
- 在 Pawn 或控制器变化时刷新相机绑定
- 可选自动设置 ViewTarget

常用配置：

- `CameraActorClass`
- `PlayerIndex`
- `bAutoSetViewTarget`
- `PlayerControllerOverride`

常用函数：

- `SetPlayerController`
- `RefreshCamera`

适合：

- 固定相机玩法
- 俯视角 / 跟随视角切换
- 把相机逻辑从 Pawn 里抽出来集中管理

### 2. CameraActor

`ADMCameraActor` 是插件自己的相机 Actor，配合 `CameraManagerComponent` 使用。  
如果你需要项目级相机行为，通常会继承它再扩展。

## Puerts 接入点

### 1. DMGameInstance

[UDMGameInstance](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Gameplay/Core/DMGameInstance.h:5) 在原生 `GameInstance` 之上提供了：

- `TS_Init()`

这是一个 `BlueprintImplementableEvent`，适合当作 TS 初始化桥接入口。

### 2. DMPuertsLibrary

[UDMPuertsLibrary](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Library/DMPuertsLibrary.h:7) 提供：

- `RegisterConsoleCommand`

适合：

- 给 TS 或调试工具注册控制台命令
- 做开发期调试入口

## 典型用法

### 场景 1：StateTree 打开一个面板

1. 在 `WidgetConfigDataTable` 里配置 `WidgetTag -> WidgetClass`
2. 在 `UILayout` 蓝图里注册 `LayerTag -> LayerWidget`
3. 在业务逻辑里调用：

```cpp
UDMUILibrary::CreateWidgetByTagToLayer(this, WidgetTag, LayerTag);
```

状态退出时调用：

```cpp
UDMUILibrary::RemoveWidgetByTag(this, WidgetTag);
```

### 场景 2：关卡自带默认 UI

1. 新建 `UDMLevelInitializationSetting` 资产
2. 配置默认要创建的 `WidgetConfigs`
3. 在关卡的 `WorldSettings` 上把 `LevelInitializationSetting` 指过去
4. 在布局初始化逻辑里读取这个配置并创建默认 Widget

### 场景 3：UI 控制输入

1. UI 打开时注册 `InputMappingContext`
2. UI 关闭时移除 `InputMappingContext`
3. 把输入切换逻辑放到 Widget 激活/关闭生命周期里

## 使用建议

- 把 `WidgetTag` 和 `LayerTag` 当成稳定协议，不要把业务逻辑绑死在具体 Widget 类上
- `DefaultUIScreenClass`、`WidgetConfigDataTable` 都走配置，不要再写死路径
- `DMToolBox` 更适合承载基础设施，不建议把具体玩法逻辑直接塞进插件
- 如果项目已经有自己的 UI 架构，优先复用 `DMUILibrary` 的按 Tag 打开能力，而不是强行替换整套 UI 体系

## 当前项目里最常用的入口

- [DMToolBoxDeveloperSetting.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Config/DMToolBoxDeveloperSetting.h:8)
- [DMUILibrary.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Library/DMUILibrary.h:9)
- [DMUIScreen.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/UI/DMUIScreen.h:8)
- [DMUILayout.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/UI/DMUILayout.h:10)
- [DMLevelInitializationSetting.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Gameplay/Data/DMLevelInitializationSetting.h:21)
- [DMCameraManagerComponent.h](/d:/UGit/NoOutsiders/Plugins/DMToolBox/Source/DMToolBox/Framework/Camera/DMCameraManagerComponent.h:12)
