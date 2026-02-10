# Review

- Check out `Accessing Task Documents` for context about mentioned `*.md` files.
- All `*.md` and `*.ps1` files should exist; you should not create any new files unless explicitly instructed.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- Your goal is to review a document as one member of a 4-model review panel.
- The `KnowledgeBase` and `Learning` folders mentioned in this document are in `REPO-ROOT/.github/`.
- Each model writes its review to a separate file.
- You are only allowed to create or update your own review file (except during `Final Review`).
- Each round of review should consider learnings from `KnowledgeBase/Learning.md`, `Learning/Learning_Coding.md`, and `Learning/Learning_Testing.md` if they exist.

## Identify Yourself

- You are one of the 4 models in the review panel. Identify yourself:
  - If you are GPT, your file name is `GPT`.
  - If you are Claude (Opus), your file name is `Opus`.
  - If you are Grok, your file name is `Grok`.
  - If you are Gemini, your file name is `Gemini`.
- Use your file name in all file operations below.

## Step 1. Identify the Review Target

- Find the title in the LATEST chat message:
  - `# Scrum`: review `Copilot_Scrum.md`, focus only on unfinished tasks (those marked `- [ ]` instead of `- [*]`).
  - `# Design`: review `Copilot_Task.md`.
  - `# Plan`: review `Copilot_Planning.md`.
  - `# Execution`: review `Copilot_Execution.md`.
  - `# Final`: skip all remaining steps and go to the `Final Review` section.
- If there is nothing: it means you are accidentally stopped. Please continue your work.

## Step 2. Determine the Current Round Index

- Look for existing `Copilot_Review_INDEX_MODEL.md` files, you can search for `Copilot_Review_*_*.md`.
- If no review files exist, the current round index is `1`.
- Otherwise find the highest `INDEX`:
  - If all files exist, the current round index is `INDEX + 1`.
    - `Copilot_Review_INDEX_GPT.md`
    - `Copilot_Review_INDEX_Opus.md`
    - `Copilot_Review_INDEX_Grok.md`
    - `Copilot_Review_INDEX_Gemini.md`
  - If not all 4 files exist, the current round index is that `INDEX`.
- If your file for the current round already exists, report that you have already completed this round and stop.
- You must be awared that, some model may already started the current round, so this is a way to determine the round index without race condition.

## Step 3. Read Context

- If the current round index is `1`, read the target document identified in Step 1.
  - For `Copilot_Scrum.md`, focus only on unfinished tasks.
- If the current round index is greater than `1`, read all 4 review files from the previous round (round index - 1) to collect other models' opinions.
- Read learning files if they exist for additional context.

## Step 4. Write Your Review

- Create file: `Copilot_Review_{RoundIndex}_{YourFileName}.md`

### Review File Structure

The file must follow this structure:

```
# Review Target: {TargetDocumentName}

# Opinion

{
  Your complete summarized feedback and suggestions for the target document.
  You should not omit anything what is in any documents in the previous round, this is what complete means.
}
```

When the current round is greater than `1`, append a `# Replies` section:

```
# Replies

## Reply to {ModelName}

{Your response to their opinion, or AGREE if you fully agree and have nothing to add.}

... (repeat for each model except yourself)
```

- If you fully agree with a model's opinion and have nothing to add, write only `AGREE`.

## Final Review (only when `# Final` appears in the LATEST chat message)

Ignore this section if there is no `# Final` in the LATEST chat message.

### Step F1. Verify Convergence

- Find the latest round of review files.
- Check that in the latest round, every model's reply to every other model is `AGREE`.
- If not all replies are `AGREE`, report that the review has not converged and stop.

### Step F2. Identify the Target Document

- Read any review file to find `# Review Target` and identify which document was reviewed.

### Step F3. Create the Summary

- Read all models' files in the last round.
- Collect all unique ideas, feedback, and suggestions.
- Consolidate them into `Copilot_Review.md` as a cohesive set of actionable feedback.

### Step F4. Apply the Review

- Apply `Copilot_Review.md` to the target document as if it were a user comment:
  - According to the review target, follow one of the instruction files:
    - For `Copilot_Scrum.md`, follow `REPO-ROOT/.github/0-scrum.prompt.md`.
    - For `Copilot_Task.md`, follow `REPO-ROOT/.github/1-design.prompt.md`.
    - For `Copilot_Planning.md`, follow `REPO-ROOT/.github/2-planning.prompt.md`.
    - For `Copilot_Execution.md`, follow `REPO-ROOT/.github/4-execution.prompt.md`.
  - Treat the LATEST chat message as `# Update` followed by the content of `Copilot_Review.md`.
  - Update the target document content according to the review feedback.

### Step F5. Clean Up

- Delete all `Copilot_Review*.md` files, including the round files and `Copilot_Review.md` itself.
