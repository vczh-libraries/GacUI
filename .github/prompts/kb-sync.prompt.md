# Sync Knowledge Base with other Repos

- Check out `Accessing Task Documents` and `Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md`, `*.ps1` and `*.sh` files.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

Here is a list of all repos, the `.github` folder should sync with each other:
- Vlpp
- VlppOS
- VlppRegex
- VlppReflection
- VlppParser2
- Workflow
- GacUI
- Release
- Tools (instead of `.github` the folder calls `Copilot` in this repo)
There are also folder names, and `REPO-ROOT` is one of them.

## Step 1. Sync back to Tools

The `Tools` repo is the central place to maintain all instructions and documents. But knowledge bases are updated from each repo. So the first step is to copy knowledge base from the current repo back to `Tools`:
```
& REPO-ROOT\..\Tools\Copilot\copilotInit.ps1 -UpdateKB
```

## Step 2. Sync from Tools to other Repos

```
& REPO-ROOT\..\Tools\Copilot\copilotInitAll.ps1
```

## Step 3. Commit and Push

After calling `copilotInit.ps1 -UpdateKB` and `copilotInitAll.ps1`, find out what repos are updated:
```
& REPO-ROOT\..\Tools\Tools\CheckRepo.ps1 CheckAll
```

You will get a list, now go to each repo that has uncommitted files, commit all changes and push to the current branch of each repo.
