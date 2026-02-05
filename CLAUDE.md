- Read through `REPO-ROOT/.github/copilot-instructions.md` before performing any work.
- `copilot-instructions.md` is the guideline you should follow.
  - MUST READ `Coding Guidelines and Tools` before touching the source code.
  - MUST READ `Leveraging the Knowledge Base` and find knowledge you may need before making any decision.
- Interpret the request (in the latest chat message, not including conversation history) following the steps:

## Step 1

Read the first word of the request, and read an additional instruction file when it is:
- "scrum": REPO-ROOT/.github/prompts/0-scrum.prompt.md
- "design": REPO-ROOT/.github/prompts/1-design.prompt.md
- "plan": REPO-ROOT/.github/prompts/2-planning.prompt.md
- "summarize": REPO-ROOT/.github/prompts/3-summarizing.prompt.md
- "execute": REPO-ROOT/.github/prompts/4-execution.prompt.md
- "verify": REPO-ROOT/.github/prompts/5-verifying.prompt.md
- "ask": REPO-ROOT/.github/prompts/ask.prompt.md
- "code": REPO-ROOT/.github/prompts/code.prompt.md

### Exceptions

- If the latest chat message is simply "execute and verify", it means do `execute` followed by `verify`.
- If the first word is not in the list:
  - Follow REPO-ROOT/.github/prompts/code.prompt.md
  - Skip `Step 2`

## Step 2

Read the second word if it exists, convert it to a title `# THE-WORD`.

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

When the request is `scrum`, follow `0-scrum.prompt.md` and "the LATEST chat message" becomes empty.

When the request is `scrum learn`, follow `0-scrum.prompt.md` and "the LATEST chat message" becomes
```
# Learn
```

When the request is `design problem next`, follow `1-design.prompt.md` and "the LATEST chat message" becomes
```
# Problem
next
```

When the request is `do this and do that`, since the first word is not in the list, so follow `code.prompt.md`, skipping `Step 2` and "the LATEST chat message" becomes
```
do this and do that
```
