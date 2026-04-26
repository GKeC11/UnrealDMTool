---
name: ue-cpp
description: Write, review, and refactor Unreal Engine C++ code for this project, especially in `Source/` and `Plugins/DMToolBox/Source/`. Use when Codex needs to add gameplay features, components, subsystems, function libraries, editor/runtime module code, or modify existing UE C++ classes. Prioritize componentized design, UE-native decoupling, explicit network synchronization decisions, and concise comments that explain non-obvious intent.
---

# UE C++

Use this skill for Unreal Engine C++ work in this repository. Favor UE-native extension points and keep the implementation easy to evolve inside the existing module structure.

## Workflow

1. Inspect the target module, the nearest `.Build.cs`, and the existing class layout before adding code.
2. Choose the smallest UE-native abstraction that solves the problem cleanly: component, subsystem, interface, function library, data asset, or actor class.
3. Decide whether the change is local-only, server-authoritative, replicated, or editor-only before writing behavior.
4. Implement the feature in the correct runtime or editor module and keep dependencies one-way where possible.
5. Add brief comments for non-obvious intent, lifecycle assumptions, or replication behavior.

## Architecture Rules

- Prefer componentized design over putting unrelated responsibilities into a single `Actor`, `Pawn`, `Controller`, or widget class.
- Use `UActorComponent` for attachable gameplay abilities or stateful behavior tied to an actor.
- Use subsystems when the responsibility is global or long-lived, such as game-instance, world, or UI coordination.
- Use function libraries only for stateless helpers. Do not hide mutable gameplay state in a blueprint or C++ library.
- Use interfaces, delegates, and narrow public methods to decouple systems instead of hard-coding direct knowledge between classes.
- Reuse the existing `Plugins/DMToolBox/Source/DMToolBox/Framework/` folders when they already match the responsibility. Keep editor-only code inside `DMToolBoxEditor`.
- Avoid introducing unnecessary hard references, circular dependencies, or tightly coupled ownership chains when a component, subsystem, or interface would keep the code cleaner.

## Network Synchronization Checklist

- For every gameplay-facing change, explicitly judge whether network synchronization is needed.
- If the state affects authority, combat, movement, interaction results, or anything multiple players must observe consistently, consider replication or RPC flow before finalizing the API.
- Decide which side owns the truth: standalone/local client, owning client, server, or all clients.
- When replication is needed, prefer clear UE patterns such as replicated properties, `ReplicatedUsing`, server/client/multicast RPCs, and authority guards.
- When replication is not needed, leave the code clearly local and document the reason if it is not obvious, especially for UI, camera, debug, or editor utilities.
- Do not bolt network support on as an afterthought if the feature's public API can be shaped correctly from the start.

## Comment Rules

- Use Chinese for comments in UE C++ code unless matching an existing third-party or engine-facing English comment block.
- Write comments for intent, constraints, lifecycle ordering, and networking assumptions.
- Keep comments short and useful. Explain why the code exists or what must stay true.
- Add comments around tricky UE behavior such as initialization order, delegate binding, ownership assumptions, or replication caveats.
- Do not add comments for obvious one-line assignments or simple getter/setter behavior.

## DMToolBox Conventions

- Match the existing naming style and macro usage already present in `DMToolBox`.
- Keep runtime gameplay code in `Plugins/DMToolBox/Source/DMToolBox/` and editor tooling in `Plugins/DMToolBox/Source/DMToolBoxEditor/`.
- Prefer extending existing framework areas such as `Camera`, `Gameplay`, `Library`, `Puerts`, or `UI` before creating a new top-level bucket.
- Prefer `DM_LOG` for DMToolBox runtime/editor code when adding diagnostic logs, so file/function and net-mode context stay consistent with the rest of the plugin.
- Treat the `Verbosity` argument of `DM_LOG` and `UE_LOG` as a compile-time macro token such as `Log`, `Warning`, or `Error`. Do not pass ternary expressions or other runtime expressions there.
- When log level depends on runtime state, branch with explicit `if/else` and call `DM_LOG` separately for each verbosity.
