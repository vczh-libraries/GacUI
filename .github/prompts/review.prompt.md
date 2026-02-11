# Review

- Check out `Accessing Task Documents` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md` files.
- All `*.md` and `*.ps1` files should exist; you should not create any new files unless explicitly instructed.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- Your goal is to review a document as one member of a 4-model review panel.
- The `KnowledgeBase` and `Learning` folders mentioned in this document are in `REPO-ROOT/.github/`.
- The mentioned `Copilot_Review.md` and `Copilot_Review_*_*.md` files are in `REPO-ROOT/.github/TaskLogs/`.
- Each model writes its review to a separate file.
- When you are asked to create a `Copilot_Review_*_*.md`, You are only allowed to create your own review file.
- Each round of review should consider knowledges from the knowledge base.
- Each round of review should consider learnings from `KnowledgeBase/Learning.md`, `KnowledgeBase/Learning.md`, `Learning/Learning_Coding.md`, and `Learning/Learning_Testing.md` if they exist.

## Identify Yourself

- You are one of the 4 models in the review panel. Identify yourself:
  - If you are GPT, your file name fragment is `GPT`.
  - If you are Claude (Opus), your file name fragment is `Opus`.
  - If you are Grok, your file name fragment is `Grok`.
  - If you are Gemini, your file name fragment is `Gemini`.
- Use your file name fragment in all file operations below.

## Step 1. Identify the Target Document to Review

- Find the title in the LATEST chat message:
  - `# Scrum`: review `Copilot_Scrum.md`, begins from `# TASKS` until the end, focus only on unfinished tasks (those marked `- [ ]` instead of `- [*]`).
  - `# Design`: review `Copilot_Task.md`, begins from `# INSIGHTS AND REASONING` until the end.
  - `# Plan`: review `Copilot_Planning.md`, begins from `# EXECUTION PLAN` until the end.
  - `# Execution`: review `Copilot_Execution.md`, begins from `# EXECUTION PLAN` until the end.
  - `# Final`: skip all remaining steps and go to the `Final Review` section.
  - `# Apply`: skip all remaining steps and go to the `Apply Review` section.
- If there is nothing: it means you are accidentally stopped. Please continue your work.

## Step 2. Determine the Current Round Index

- Look for your own `Copilot_Review_PREVIOUSINDEX_{YourFileNameFragment}.md` file.
  - If it exists, the current round index is biggest `PREVIOUSINDEX` + 1.
  - If none exists, the current round index is `1`.
- If the current round index is greater that 1, here are a list of review files from the previous round:
  - `Copilot_Review_PREVIOUSINDEX_GPT.md`
  - `Copilot_Review_PREVIOUSINDEX_Opus.md`
  - `Copilot_Review_PREVIOUSINDEX_Grok.md`
  - `Copilot_Review_PREVIOUSINDEX_Gemini.md`

## Step 3. Read Context

- Read the target document identified in Step 1.
  - For `Copilot_Scrum.md`, focus only on unfinished tasks.
- If the current round index is greater than `1`, read all review files from the previous round to collect other models' opinions.
  - Their opinion of the review.
  - Their replies to you.

## Step 4. Write Your Review

- Create file: `Copilot_Review_{RoundIndex}_{YourFileNameFragment}.md`
- You need to consolidate all information from Step 3.
- Find what inspires you, what you agree with, and what you disagree with.
- Complete the document following the format:
  - `# Review Target: {TargetDocumentName}`: the name of the document you are reviewing.
  - `## Opinion`:
    - Your complete summarized feedback and suggestions for the target document.
    - You should not omit anything what is in any documents in the previous round, this is what complete means.
  - `## Replies`: this section exists only when the current round index is greater than `1`.
    - Find `## Opinion` in every `Copilot_Review_{RoundIndex-1}_{ModelName}.md` except yours.
    - If you totally agree with a model, add this section: `### AGREE with {ModelName}` with no content. If you have anything to add, put them in your own `## Opinion`.
    - If you partially or totally disagree with a model, add this section: `### DISAGREE with {ModelName}` and explain why you disagree and what you think is correct.
- The following sections are about what you need to pay attention to when reviewing the target document.
- After finishing the review document, stops.

### Review the Architecture

- This applies when the document talks about architecture and interface design.
- I prefer interface design with SOLID
  - Single responsibility: each interface or class should have one responsibility.
  - Open-closed principle: software entities should be open for extension but closed for modification.
  - Liskov substitution: objects of a superclass should be replaceable with objects of a subclass without affecting the correctness of the program.
  - Interface segregation: many client-specific interfaces are better than one general-purpose interface.
  - Dependency inversion: depend on abstractions, not on concretions.
- More importantly, the design should be compatible with existing constructions and patterns.

### Review the Data Structure and Algorithm

- This applies when the document talks about selecting or adding data structures and algorithms.
- When possible, use existing types and algorithms from libraries in the `Import` folder, especially important primitive types and collection types.
  - Only when the mission is performance sensitive, low level constructions are allowed.
- Prefer algorithms with lower time and space complexity.

### Review the Code Quality

- This applies when the document tasks about actual code implementation.
- The most important rule is that the code should look like other files in the codebase.
- Code styles could be a little bit different between features and testing.
- TRY YOUR BEST to prevent from code duplication.

### Code in Feature

- Feature code usually refer to files in the `Source` folder.
- Header files should follow the style of existing header files, ensuring that no cyclic dependencies and guarded to prevent from being included multiple times.
- When a header file `Something.h` matches with multiple cpp files, names of these cpp files usually named after `Something_Category.cpp`.
- Naming convention follow .NET framework guidelines, although these are C++ code.

### Code in Testing

- Testing code usually refer to files in the `Test` folder.
- Header files in testing code are important, as they usually contain shared constructions. Reuse them as much as possible.
- If multiple test files test again the same thing:
  - It is highly possibly that helper functions you need already exists. Reuse them as much as possible.
  - When test patterns are obvious, you should follow the pattern.

### Review with Learnings

- Learnings from the past are important, they are written in:
  - `KnowledgeBase/Learning.md`
  - `Learning/Learning_Coding.md`
  - `Learning/Learning_Testing.md`
- These files contains some concentrated ideas from reviews in the past.
- Each item has a `[SCORE]` with their title in the `# Orders` section.
- Pay attention to those with high scores, they are mistakes that are frequently made.
- Apply all learnings to the document and find out what could be improved.

### Review with the Knowledge Base

- None should be conflict with the knowledge base.
- But in some rare cases where the knowledge base is not precisely describing the code, you should point them out.

## Final Review (only when `# Final` appears in the LATEST chat message)

Ignore this section if there is no `# Final` in the LATEST chat message.

### Step F1. Verify Convergence

- Find the latest round of review files.
- Check that in the latest round, every models agree with each other.
  - If any reply is not agree, report that the review has not converged and stop.

### Step F2. Identify the Target Document

- Identify all review files from the last round. Read their `# Review Target`, they should be the same.

### Step F3. Create the Summary

- Read the `## Opinion` section from each review file from the last round.
- Consolidate all options into a single review opinion.
- Write the review opinion to `Copilot_Review.md` as a cohesive set of actionable feedback.
  - The only title in this file should be `# Review Target: {TargetDocumentName}`.
  - The content should not contain any title.
  - DO NOT mention which model offers which opinion, the review opinion should be a cohesive whole, not a collection of separate opinions.
  - Ignore any comments against `# !!!SOMETHING!!!`.
- Stops.

## Apply Review (only when `# Apply` appears in the LATEST chat message)

Ignore this section if there is no `# Apply` in the LATEST chat message.

### Step A1. Identify the Target Document

- The title of `Copilot_Review.md` is `# Review Target: {TargetDocumentName}`. This is the target document to apply the review opinion.
- According to the target document, follow one of the instruction files:
  - For `Copilot_Scrum.md`, follow `REPO-ROOT/.github/prompts/0-scrum.prompt.md`.
  - For `Copilot_Task.md`, follow `REPO-ROOT/.github/prompts/1-design.prompt.md`.
  - For `Copilot_Planning.md`, follow `REPO-ROOT/.github/prompts/2-planning.prompt.md`.
  - For `Copilot_Execution.md`, follow `REPO-ROOT/.github/prompts/4-execution.prompt.md`.

### Step A2. Apply the Review

- Treat the LATEST chat message as `# Update` followed by the content of `Copilot_Review.md`.
  - Do not include the title of `Copilot_Review.md` in the content.
- Follow the specific instruction file to update the target document with the review opinion.
  - Skip the part that adding a new `# Update` section to the target document.

### Step A3. Clean Up

- Delete all `Copilot_Review_*_*.md` files.
- Delete `Copilot_Review.md`.
- Stops.
