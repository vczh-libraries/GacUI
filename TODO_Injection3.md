This is a continuation of previous works:
- TODO_Injection.md
- TODO_Injection2.md

When you are on Linux, you are working on `wGac` repo.
When you are on macOS, you are working on `iGac` repo.
According to previous works, make sure new features work on `wGac` or `iGac`:
- Make sure `./import.sh` and `./syncProj.sh` properly updates source files.
- `./test.sh --app:renderer` with `/remoteDT`, `/remoteLocale`, `/remoteFS`, `/corehost`.
- As described in TODO_Injection.md, verification could be done by adding temporary fake injection implementation on the renderer side, but it should not be committed.

commit and push all local changes once finishing.

## Details

### Details (wGac specific)

### Details (iGac specific)
