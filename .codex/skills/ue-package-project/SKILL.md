---
name: ue-package-project
description: Generate or update this Unreal Engine project's packaging entry scripts under the project `Scripts/` directory, including `PackageProject.ps1`, `PackageClient.bat`, `PackageServer.bat`, and `PackageClientAndServer.bat`. Use when Codex needs to scaffold a project's standard packaging automation, refresh those generated files after workflow changes, or adjust the generated UAT wrapper for this repository.
---

# UE Package Project

Use this skill to generate and maintain the project's packaging entry scripts.

## Workflow

1. Read `scripts/generate_package_scripts.py` before changing generation behavior.
2. Run the generator to emit or refresh `Scripts/PackageProject.ps1` and the companion `.bat` wrappers in the project root.
3. Let the generator discover the `.uproject` by walking upward from the skill folder unless the caller passes `--project-root`.
4. Keep the generated files in the project `Scripts/` directory, not inside the skill.
5. Regenerate the project scripts after changing packaging defaults, argument handling, or wrapper behavior.

## Conventions

- Generate files into the project `Scripts/` directory.
- Keep archive output under the project `PackageBuilds/` directory unless the caller passes `-ArchiveRoot`.
- Keep packaging logs under `Saved/Logs/`.
- Resolve the Unreal Engine root from `-EngineRoot`, `UE_ENGINE_ROOT`, `UNREAL_ENGINE_ROOT`, or known repo-adjacent defaults before failing.
- Preserve the existing default behavior: package client builds when no explicit target switch is passed.
- When changing UAT arguments, keep client and server flows explicit and easy to diff.
- Keep a short generated-file header so future edits are traceable back to this skill.

## Entry Points

- Generator: `scripts/generate_package_scripts.py`
- Generated PowerShell output: `Scripts/PackageProject.ps1`
- Generated client wrapper: `Scripts/PackageClient.bat`
- Generated server wrapper: `Scripts/PackageServer.bat`
- Generated combined wrapper: `Scripts/PackageClientAndServer.bat`
