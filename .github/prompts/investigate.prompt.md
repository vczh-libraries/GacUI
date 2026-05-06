# Investigate

- Check out `Accessing Task Documents`, `(Windows Specific) Accessing Script Files`, and `(Linux Specific) Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md`, `*.ps1` and `*.sh` files.
- Check out `(Windows Specific) External Tools Environment and Context` and `(Linux Specific) External Tools Environment and Context` in `REPO-ROOT/.github/copilot-instructions.md` for accessing scripts for testing and debugging.
- Check out `REPO-ROOT/Project.md` to find out what solutions you need to build.
- All `*.md`, `*.ps1` and `*.sh` files should exist; you should not create any new files unless explicitly instructed.
  - The `Copilot_Investigate.md` file should already exist, it may or may not contain content from the last investigation.
  - If you cannot find the file, you are looking at a wrong folder.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find the knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.

## Goal and Constraints

- Your goal is to finish an investigation document in `Copilot_Investigate.md` to address a problem.
- You are recommended to modify unit tests, build, run, and debug to understand the problem, or verify your root cause analysis while tracing a bug.

## Copilot_Investigate.md Structure

- `# !!!INVESTIGATE!!!`: This file always begins with this title.
- `# PROBLEM DESCRIPTION`: An exact copy of the problem description I gave you.
- `# UPDATES`
  - `## (UPDATE|REPORT)`: There could be multiple occurrences. Each one has an exact copy of the update description I gave you.
- `# TEST`: Test cases to confirm or define the problem.
- `# PROPOSALS`: A list of proposed solutions, followed by details about each proposal.
  - `- No.X TITLE OF PROPOSAL`, with `[CONFIRMED]` or `[DENIED]` after the proposal is tested and proven to be correct or incorrect. X is an increasing number starting from 1.
  - `## No.X TITLE OF PROPOSAL`: Details of the proposal.
    - `### CODE CHANGE`: The change you have done to the source code to implement the proposal.
    - `### (CONFIRMED|DENIED|DENIED BY USER)`: Appears after the proposal is tested and proven to be correct or incorrect, with a comprehensive explanation about why.

## Step 1. Identify the Problem

- The problem I would like to solve is in the chat messages sent with this request.
- Find `# Repro` or `# Continue` or `# Report`.
  - Ignore any of these titles in the chat history.
  - If there is nothing: it means `# Continue`.

### Repro the Problem (only when "# Repro" appears in the LATEST chat message)

Ignore this section if there is no "# Repro" in the LATEST chat message
I am starting a fresh new request.

- Before replacing `Copilot_Investigate.md`, read its current content.
  - If it contains anything beyond the `# !!!INVESTIGATE!!!` title, run `copilotRemember.ps1` without arguments to archive the old investigation first.
- You should override `Copilot_Investigate.md` with only one title `# !!!INVESTIGATE!!!`.
  - At the moment, `Copilot_Investigate.md` may contain the last investigation. Even if it may look like the document is already finished for the current investigation, always clean it up.
- After overriding, copy precisely my problem description in `# Repro` from the LATEST chat message under `# PROBLEM DESCRIPTION`.
- Add empty `# UPDATES`, `# TEST` and `# PROPOSALS` sections.
- Jump to `Step 2` directly.

### Continue Investigation (only when "# Continue" appears in the LATEST chat message)

Ignore this section if there is no "# Continue" in the LATEST chat message (or when the LATEST chat message has no content)
I am going to propose a change to `Copilot_Investigate.md`.

- When there is any content under `# Continue` in the LATEST chat message:
  - Copy precisely my problem description in `# Continue` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## UPDATE`.
  - The new `## UPDATE` should be appended to the end of the existing `# UPDATES` section (aka before `# TEST`).
  - If I do not agree with any proposal marked with `[CONFIRMED]`:
    - Mark these proposals with `[DENIED]`, change related `### CONFIRMED` in those proposals to `### DENIED BY USER`, and write a comprehensive explanation in each proposal about why.
- Jump to `Step 5` directly.

### Generate Report (only when "# Report" appears in the LATEST chat message)

Ignore this section if there is no "# Report" in the LATEST chat message
I am going to propose a change to `Copilot_Investigate.md`.

- When there is any content under `# Report` in the LATEST chat message:
  - Copy precisely my problem description in `# Report` from the LATEST chat message to the `# UPDATES` section, with a new sub-section `## REPORT`.
  - The new `## REPORT` should be appended to the end of the existing `# UPDATES` section (aka before `# TEST`).
  - If I do not agree with any proposal marked with `[CONFIRMED]`:
    - Mark these proposals with `[DENIED]`, change related `### CONFIRMED` in those proposals to `### DENIED BY USER`, and write a comprehensive explanation in each proposal about why.
- Add a `# REPORT` at the end of the document:
  - You are going to explain all `[CONFIRMED]` proposals, why they work, and how they solve the issue.
  - You are going to compare any `[CONFIRMED]` proposals, and explain how each one is better (pros) or worse (cons) than the others.
- Skip all remaining steps, you are good from here.

## Step 2. Construct Test Cases

Only when I suggest that the problem is impossible to reproduce with test cases, write `N/A` under `# TEST` and skip `Step 2.`.

- You are going to construct proper test cases to address the problem.
- If I suggest a way to confirm the issue:
  - Write the idea under `# TEST`.
  - Finish actual test cases.
  - Build the unit test to ensure test cases compile.
- If I suggest that test cases already exist:
  - Find the test cases and summarize the idea under `# TEST`.
  - Build the unit test to ensure test cases compile.
- If I did not suggest any way to confirm the issue:
  - You need to come up with your own idea to confirm the issue.
  - Write the idea under `# TEST`.
  - Finish actual test cases.
  - Build the unit test to ensure test cases compile.
- Finish `# TEST`
  - Besides the idea of the test cases, you should also understand the criteria for success. Write them in `# TEST` as well.
  - Passing test cases is the basic criterion. There might be other conditions; make sure you are clear about them, and write them down in `# TEST`.
  - Such conditions usually include:
    - Find out any clue in logs.
    - Attach a debugger to confirm something happens or not.

## Step 3. Confirm the Problem

If `N/A` is written under `# TEST`, it means the problem is not possible to reproduce with test cases, skip this step.
If the title is `# TEST [CONFIRMED]`, it means the problem is already proven confirmable by the test cases, skip this step.

- Run the test cases and confirm that the problem is reproduced.
- If you cannot confirm, you need to figure out why, and jump back to `Step 2.` to adjust the test case.
- If you can confirm, you need to update the title to `# TEST [CONFIRMED]`. Save your progress by:
  - git commit all changes
  - git push to the current branch, but if it fails because it needs a pull/merge, or because of a network issue, skip pushing.

## Step 4. Propose Solutions

Propose any solution you can think of and write them down in the document, you must save the document before trying each proposal.

- Do necessary analysis to propose solutions to the problem.
  - You are recommended to read related source code and documents to understand the problem.
  - You are recommended to run the test cases, see if there is something wrong.
  - You are recommended to check logs from the test cases.
  - You are recommended to debug the test cases.
  - Try your best to collect any useful information.
- It is fine to have multiple proposals. When you come up with any, record it under `# PROPOSALS`; a new list item and a new section should be added for each proposal:
  - Add a new proposal with `- No.X TITLE OF PROPOSAL`
  - Add a new proposal with `## No.X TITLE OF PROPOSAL` for details, you are going to add it at the end of the `# PROPOSALS`.
    - With an empty section `### CODE CHANGE` at the end of the proposal.
  - Confirmation of the proposal will be finished later, so do not include any mark for now.
- Save your progress by:
  - git commit all changes
  - git push to the current branch, but if it fails because it needs a pull/merge, or because of a network issue, skip pushing.

## Step 5. Confirm the Proposal

- Find a proposal that has not been confirmed.
  - Check out the list under `# PROPOSALS`, pick the first proposal that is without `[CONFIRMED]` or `[DENIED]`.
  - If every proposal is marked `[DENIED]`, you need to work out new proposals, jump to `Step 4`.
  - If every proposal is marked, but some are `[CONFIRMED]`, you are good and stop here.
    - If there is only one `[CONFIRMED]` proposal, reapply the change according to its `### CODE CHANGE` section, run the test cases to confirm all test cases pass and extra conditions are satisfied.
- Understand and implement the proposal.
- Run the test cases to confirm the proposal.
  - Ensure all test cases passed.
    - If related test cases failed, it means the proposal is denied.
    - If unrelated test cases failed, it means the proposal introduces regression issues:
      - Try to work out a fix, remember to update the current `## No.X TITLE OF PROPOSAL` with the fix details.
      - Re-run the test cases until you come to a conclusion.
  - Ensure all conditions are verified.
    - You need to carefully read the `# TEST` section.
    - You might be required to check logs or debug the test cases to confirm the condition.
- If the proposal is proven to be useful, mark the proposal with `[CONFIRMED]`, and write a comprehensive explanation in `### CONFIRMED`.
- If the proposal is proven to be invalid, mark the proposal with `[DENIED]`, and write a comprehensive explanation in `### DENIED`.
- After trying a proposal:
  - You need to record what you updated in the source code under `### CODE CHANGE` of the proposal.
  - You need to record what you have done to confirm the problem under `### CONFIRMED` or `### DENIED` of the proposal.
  - Save your progress by:
    - git commit `Copilot_Investigate.md`.
    - revert all other changes, so that you can continue for the next one. This is why it is important to record your changes.
    - git push to the current branch, but if it fails because it needs a pull/merge, or because of a network issue, skip pushing.
  - Jump back to `Step 5` to try the next proposal.
