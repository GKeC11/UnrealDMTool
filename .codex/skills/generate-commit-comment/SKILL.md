---
name: generate-commit-comment
description: Generate Chinese Git commit comments from current repository changes. Use when the user asks for commit messages, commit comments, changelist summaries, or wants submodule changes summarized as separate commit comments.
---

# Generate Commit Comment

## Overview

Use this skill to inspect Git changes and produce ready-to-paste Chinese commit comments. When submodules have internal changes, generate an independent commit comment for each changed submodule, plus a separate one for the parent repository.

Parent repository and submodule commit comments must be handled as separate commit units. Do not combine parent changes and submodule internal changes into one comment, even when they belong to the same feature.

## Workflow

1. Inspect the parent repository:
   - `git status --short`
   - `git diff --stat`
   - `git diff --name-status`
   - Read focused diffs for important source files with `git diff -- <paths>`.
   - Do not read binary or Unreal asset contents. For files such as `.uasset`, `.umap`, `.ubulk`, `.uexp`, `.locres`, images, audio, video, archives, or other known binary formats, infer the change from the path, filename, Git status, and diff stat only.
2. Detect submodule changes:
   - Use `git submodule status`, `git diff --submodule`, or `git status --short`.
   - For each changed submodule, run the same status/stat/name-status/diff checks inside that submodule with `git -C <submodule> ...`.
3. Separate generated artifacts from source changes:
   - Summarize generated JS/maps, TypeScript declaration files, caches, build outputs, or version metadata in one concise bullet.
   - Do not expand generated files line by line unless they reveal a real source-of-truth change.
4. Infer the intent from source diffs:
   - Group related edits by behavior, module, UI flow, protocol, build config, or documentation.
   - Prefer concrete behavior over file-by-file narration.
   - For binary assets, summarize what can be safely inferred from names and locations, for example maps, UI assets, data assets, textures, or generated cooked resources. Do not attempt to inspect or quote their binary contents.
5. Draft one commit comment per Git repository:
   - One for each dirty submodule.
   - One for the parent repository if it has non-submodule changes or submodule pointer/config changes.
   - Treat the parent repository and each submodule as separate outputs with their own summary, bullet list, and scope.

## Output Format

Return only commit comments that can be pasted into a Git tool. Separate parent and submodule comments clearly so they can be copied and committed independently.

When both the parent repository and submodules have changes, prefix each block with a plain repository label:

```text
Main:
<commit comment for parent repository>

Submodule <path>:
<commit comment for that submodule>
```

If only one repository has changes, the label can be omitted.

Use this structure for each repository:

```text
<type>: <short Chinese summary>

- <main change in Chinese>
- <main change in Chinese>
- <generated files, config, or docs change if applicable>
```

Recommended Conventional Commit types:

- `feat`: user-facing capability, new flow, new protocol, or new subsystem.
- `fix`: bug fix or incorrect behavior correction.
- `refactor`: structure or API changes without a direct feature claim.
- `chore`: generated files, build config, dependency, or housekeeping.
- `docs`: documentation-only changes.
- `test`: tests-only changes.

When the user explicitly asks for "Commit Comment" instead of a Conventional Commit subject, it is acceptable to omit the type prefix if the surrounding project style does not use it. Keep the body as Chinese Markdown bullets.

## Submodule Rules

- Never merge submodule internal edits into the parent repository commit comment.
- For a dirty submodule, summarize its internal changes from inside the submodule.
- In the parent repository comment, describe the submodule only as a pointer/config update, for example "更新 <submodule> 子模块引用".
- Do not mention submodule source behavior in the parent comment except as a high-level dependency or pointer update.
- If the submodule contains untracked files, mention them only when they are meaningful source/docs/assets. Call out suspicious generated or cache files separately.

## Quality Bar

- Write in concise Chinese.
- Make each bullet explain a behavior or intent, not just a filename.
- Prefer 3-6 bullets per commit comment.
- Include important caveats after the commit comments only if they affect what should be staged, for example modified caches or untracked generated files.
- Do not claim tests/builds were run unless they were actually run.
- Never read binary file contents to generate the comment. If the change is only visible through a binary filename, state the inferred asset category or purpose conservatively.

## Example

```text
feat: 接入 WebSocket GameServer 协议

- 登录流程改为连接 GameServer 后通过协议请求完成认证
- Lobby/Room/Match UI 改用统一协议 payload 解析
- 房间创建、轮询、离开和开始游戏流程适配新协议
- 更新生成的 JS、类型声明和版本信息
```
