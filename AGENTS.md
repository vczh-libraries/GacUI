- Read through `REPO-ROOT/.github/copilot-instructions.md` before performing any work.
  - `copilot-instructions.md` is the guideline you should follow.
  - MUST READ `Coding Guidelines and Tools` before touching the source code.
  - MUST READ `Leveraging the Knowledge Base` and find the knowledge you may need before making any decision.
- Read through `REPO-ROOT/Project.md` before performing any work.
- Interpret the request (in the latest chat message, not including conversation history) following the steps:

## Step 1

Read the first phrase of the request, and read an additional instruction file if it is:
- "ask": REPO-ROOT/.github/prompts/ask.prompt.md
- "investigate": REPO-ROOT/.github/prompts/investigate.prompt.md
- "review": REPO-ROOT/.github/prompts/review.prompt.md
- "refine": REPO-ROOT/.github/prompts/refine.prompt.md
- "kb": REPO-ROOT/.github/prompts/kb.prompt.md
- "kb refine": REPO-ROOT/.github/prompts/kb-refine.prompt.md

### Exceptions

The following rules apply when the first phrase is not in the list:
- If the request looks like a research or question, treat the request as if it begins with "ask".
- If the request looks like a coding work, treat the request as if it begins with "investigate repro".
- If the request looks like other non-coding work, just execute the request directly. Such works usually include but not limit to:
  - Planning.
  - Document refining.
  - Daily maintenance.
  - Script fixing, especially for build/CI.

## Step 2

- Only applies when the first phrase is:
  - "investigate"
  - "kb"
- Read the next word if it exists, convert it to a title `# THE-WORD`.

## Step 3

- Keep the remaining as is.
  - Special treatment only for `investigate repro`:
    - If the remaining is just tagging a file, replace the tagging with the content of the file.
    - If the remaining is empty, use the content of `REPO-ROOT/TODO_TASK.md`.
  - Special treatment only for `review`:
    - If the remaining is empty, tag `REPO-ROOT/TODO_TASK.md`.
- Treat the processed request as "the LATEST chat message" in the additional instruction file.
- Follow the additional instruction file and start working immediately, there will be no more input.

## Fixing Typos

If the whole request has almost no new lines or punctuation,
it means I am typing the text by voice,
which would introduce a huge amount of typos,
you need to carefully consider possible words with similar pronunciation.

## Examples

When the request is `refine`, follow `refine.prompt.md` and "the LATEST chat message" becomes empty.

When the request is `kb execute`, follow `kb.prompt.md` and "the LATEST chat message" becomes
```
# Execute
```

When the request is `review`, follow `review.prompt.md` and "the LATEST chat message" becomes
```
<tagging REPO-ROOT/TODO_TASK.md>
```
Special treatment applied

When the request is `investigate repro <tagging REPO-ROOT/TODO_ANOTHER_TASK.md>`, follow `investigate.prompt.md` and "the LATEST chat message" becomes
```
# Repro
<CONTENT OF REPO-ROOT/TODO_ANOTHER_TASK.md>
```
Special treatment applied

When the request is `investigate repro this issue`, follow `investigate.prompt.md` and "the LATEST chat message" becomes
```
# Repro
this issue
```

When the request is `do this and do that`, because the first phrase is not in the list, follow `investigate.prompt.md`; "the LATEST chat message" becomes
```
# Repro
do this and do that
```
