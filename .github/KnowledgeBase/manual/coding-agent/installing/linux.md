# Installing Required Tools on Linux

The Linux agent workflow is driven by `.github/Ubuntu/build.sh`. It adds `.github/Ubuntu/vl/cmd` to `PATH`, runs `vmake --make`, and then builds with `vbuild`. The agent instructions deliberately route builds through this script instead of direct `cmake`, `make`, `clang++`, `g++`, or `gdb` calls.

Prepare a Linux environment with a C++ compiler toolchain, Bash, and LLDB. After copying the Release `.github` folder, make the copied scripts executable if the checkout blocks execution bits:
```bash
chmod +x .github/Ubuntu/build.sh
chmod +x .github/Ubuntu/vl/cmd/*
```

Build from the folder that contains the target `vmake` file:
- For a repository with one Linux project, use `REPO-ROOT/Test/Linux`.
- For a repository with multiple Linux projects, use `REPO-ROOT/Test/Linux/PROJECT-NAME`.
- Run `REPO-ROOT/.github/Ubuntu/build.sh` for an incremental build.
- Run `REPO-ROOT/.github/Ubuntu/build.sh -f` for a full rebuild.

Linux debug work should use `lldb` in an interactive terminal session. The copied guidelines expect the agent to start LLDB from the same folder that contains the `vmake` file so relative paths to binaries and source files remain correct.

If the application uses Release tools, run `Release/Tools/BuildExecutables.sh` in the Release repository. It produces `CodePack`, `CppMerge`, `GacGen`, and `GlrParserGen` under `Release/Tools`.

