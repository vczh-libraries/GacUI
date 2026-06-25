# Writing Project.md

`Project.md` is the repository map that tells the agent where to build, what to run, which generated files are protected, and which platform folders matter. The Release copy only points to the GacUI example; an application repository should replace it with concrete project facts.

## Recommended Shape

A useful `Project.md` usually contains:
- Solution to work on: the solution file and the meaning of `SOLUTION-ROOT`.
- Projects for verification: unit-test, CLI, code-generation, metadata, or UI projects that must run after relevant changes.
- Execution order: required ordering when one project generates files consumed by another project.
- Files not allowed to modify directly: generated folders, imported dependencies, released binaries, and generated baselines.
- Code generation triggers: which files require running generators and which outputs they refresh.
- Platform notes: Linux `Test/Linux` folders, unsupported platforms, special tools, and available configurations.
- Application-specific debugging notes: services, ports, sample data, dialogs, or local servers that matter for the app.

## Small Application Example

A small GacUI application can use this shape:
```markdown
# Project Specific Instruction

## Solution to Work On

You are working on the solution `REPO-ROOT/App/App.sln`,
therefore `SOLUTION-ROOT` is `REPO-ROOT/App`.

## Projects for Verification

The `REPO-ROOT/App/AppTests/AppTests.vcxproj` project is the unit test project.
When any `*.h`, `*.cpp`, `*.xml`, or Workflow script file is changed, build the solution
and run `AppTests`.

The `REPO-ROOT/App/App/App.vcxproj` project is the interactive GacUI application.
Run it when the change affects windows, dialogs, command routing, or application startup.

## Files not Allowed to Modify

Files in these folders are generated and must not be edited directly:
- `REPO-ROOT/App/Generated`
- `REPO-ROOT/App/Resources/Compiled`

Fix the source resource or generator input instead.

## Code Generation

If `REPO-ROOT/App/Resources/*.xml` changes, run `GacGen` through the project build.
If generated files change, rebuild and run `AppTests`.

## Linux Specific

This repository has no Linux build for the application.
Do not try to create one unless the task explicitly asks for it.
```

## Large Repository Patterns

The library repositories use the same idea with more detail:
- Vlpp, VlppOS, and VlppRegex define one unit-test solution and one Linux folder.
- VlppReflection adds metadata generation and metadata round-trip tests when reflection types change.
- VlppParser2 and Workflow list ordered generator and test projects because later projects consume files produced by earlier projects.
- GacUI lists generated folders, reflection metadata projects, GacUI compiler triggers, remote protocol generation, AutomationService-enabled applications, and Linux project folders.

Use these examples as patterns, not templates to copy blindly. The important part is that every instruction names the exact project, folder, generated output, or trigger that applies to the application repository.

