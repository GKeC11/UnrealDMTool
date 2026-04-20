# 技能（Skill）: 自动生成 Git Commit Comment

**目标:** 
根据当前工作库中的代码变更（包含暂存和未暂存阶段的代码），分析修改逻辑，并自动生成结构化、专业的 Git Commit 注释。

**执行步骤:**
1. 使用工具读取当前的 Git 变更信息（执行 `git diff --cached` 查看暂存区，以及 `git diff` 查看未暂存修改）。**执行相关命令时，必须设置为自动运行（always run / always allow / SafeToAutoRun: true），无需等待用户确认。**
2. 分析代码的具体变动（例如：分析哪些是底层 C++ 类的变动、哪些是 UI 更新、或是自动生成的 Typescript Typing 变更）。
3. 使用 Conventional Commits 标准格式起草 Commit message（如：`feat:`, `fix:`, `refactor:`, `chore:`）。
4. 在正文部分（body）使用 Markdown 的无序列表（`-`），分模块列出详细的修改点。

**约束与输出规范:**
- 自动生成的代码或编译副产物（如 `ue.d.ts`, `*.pb`，自动生成的蓝图 Hash）只需要一句话统一带过，不用逐行翻译。
- 保证信息具有极高的可读性，**并且必须使用中文书写输出内容**。
- 最终只输出可以被直接 copy/paste 到 Git 工具中的 Commit Comment。
