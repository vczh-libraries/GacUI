- Read through `REPO-ROOT/.github/copilot-instructions.md` before performing any work.
  - `copilot-instructions.md` is the guideline you should follow.
  - MUST READ `Coding Guidelines and Tools` before touching the source code.
  - MUST READ `Leveraging the Knowledge Base` and find the knowledge you may need before making any decision.
- Read through `REPO-ROOT/Project.md` before performing any work.
- Interpret the request (in the latest chat message, not including conversation history) following the steps:

## Step 1

Read the first word of the request, and read an additional instruction file if it is:
- "ask": REPO-ROOT/.github/prompts/ask.prompt.md
- "investigate": REPO-ROOT/.github/prompts/investigate.prompt.md
- "refine": REPO-ROOT/.github/prompts/refine.prompt.md
- "kb": REPO-ROOT/.github/prompts/kb.prompt.md
- "kb-sync": REPO-ROOT/.github/prompts/kb-sync.prompt.md

### Exceptions

- If the latest chat message is simply "execute and verify", it means do `execute` followed by `verify`.
- If the first word is not in the list, treat the request as if it begins with "investigate repro"

## Step 2

- Only applies when the first word is:
  - "investigate"
  - "kb"
- Read the second word if it exists, convert it to a title `# THE-WORD`.

## Step 3

Keep the remaining as is.
Treat the processed request as "the LATEST chat message" in the additional instruction file.
Follow the additional instruction file and start working immediately, there will be no more input.

## Fixing Typos

If the whole request has almost no new lines or punctuation,
it means I am typing the text by voice,
which would introduce a huge amount of typos,
you need to carefully consider possible words with similar pronunciation.

## Examples

When the request is `kb-sync`, follow `kb-sync.prompt.md` and "the LATEST chat message" becomes empty.

When the request is `kb execute`, follow `kb.prompt.md` and "the LATEST chat message" becomes
```
# Execute
```

When the request is `investigate repro this issue`, follow `investigate.prompt.md` and "the LATEST chat message" becomes
```
# Repro
this issue
```

When the request is `do this and do that`, because the first word is not in the list, follow `investigate.prompt.md`; "the LATEST chat message" becomes
```
# Repro
do this and do that
```
