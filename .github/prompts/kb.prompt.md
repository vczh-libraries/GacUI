# Update Knowledge Base

- Check out `Accessing Task Documents` and `Accessing Script Files` in `REPO-ROOT/.github/copilot-instructions.md` for context about mentioned `*.md` and `*.ps1` files.
- All `*.md` and `*.ps1` files should exist; you should not create any new files unless explicitly instructed.
  - The `Copilot_KB.md` file should already exist, it may or may not contain content from the last knowledge base writing.
  - If you cannot find the file, you are looking at a wrong folder.
- Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md`, find knowledge and documents for this project in `REPO-ROOT/.github/KnowledgeBase/Index.md`.
  - `Index.md` below means this file.

## Goal and Constraints

- Your goal is to draft a document for the knowledge base in `Copilot_KB.md`.
- You are only allowed to update `Copilot_KB.md` and the knowledge base.
- You are not allowed to modify any other files.
- The phrasing of the request may look like asking for code change, but your actual work is to write the design document.
- Code references must be wrapped in either `single-line` or ```multi-line``` quotes.

## Copilot_KB.md Structure

- `# !!!KNOWLEDGE BASE!!!`: This file always begins with this title.
- `# DESIGN REQUEST`: The exact copy of the problem description i gave you.
- `# INSIGHT`: Your insight about the topic after deep research.
- `# ASKS`:
  - `## QUESTION`: The exact copy of the question I gave you.
  - `### ANSWER`: Your insight about the question after deep research.
- `# DRAFT`:
  - `## DRAFT REQUEST`: The exact copy of the draft request I have you.
  - `## IMPROVEMENTS`:
    - `### IMPROVEMENT`: The exact copy of the improvement request I have you.
  - `## (API|DESIGN) EXPLANATION`: The title of the drafting KB document, and where to put the document in `Index.md` and surrounding anchors.
  - `## DOCUMENT`: The drafting KB document.

## Identify the Problem

- Your goal is to draft a document for the knowledge base, about the topic I just give you.
- Find `# Topic` or `# Ask` or `# Draft` or `# Improve` or `# Execute` in the LATEST chat message.
- Ignore any `# Topic` or `# Ask` or `# Draft` or `# Improve` or `# Execute` in the chat history.
  - Ignore any of these titles in the chat history.
  - If there is nothing:
    - It means you are accidentally stopped. Please continue your work.
    - Read `Copilot_KB.md` thoroughly, it is highly possible that you were working on the request described in the last section in `# DOCUMENT REQUEST`.

### Research on a Topic (only when "# Topic" appears in the latest chat message)

- You should override `Copilot_KB.md` with only one title `# !!!KNOWLEDGE BASE!!!`.
  - DO clear the content of the file before researching.
  - DO NOT do researching and then fix each section one by one.
- After overriding, copy precisely my problem description in `# Topic` from the LATEST chat message under `# DOCUMENT REQUEST`.
- Add another title `# INSIGHT`.
- Add another title `# ASKS`.
- Add another title `# DRAFT`, with all sub-sections under it without content.
- Find `Steps for Topic` section for the complete instructions for filling `# INSIGHT`.
- DO NOT TOUCH the `# DRAFT` part, when researching for a certain topic no document drafting is expected.

### Answer a Question (only when "# Ask" appears in the latest chat message)

- Copy precisely my question in `# Ask` from the LATEST chat message to a new sub-section `## QUESTION` under `# ASKS`.
- The new `## QUESTION` and `### ANSWER` should be added before `# DRAFT` as it is the latest question.
- Find `Steps for Ask` section for the complete instructions for filling `### ANSWER`.
- DO NOT TOUCH the `# DRAFT` part, when researching for a certain topic no document drafting is expected.

### Draft the KB Document (only when "# Draft" appears in the latest chat message)

- Copy precisely my draft request in `# Draft` from the LATEST chat message to the `## DRAFT REQUEST` section.
- Find `Steps for Draft` section for the complete instructions for filling `## (API|DESIGN) EXPLANATION` and `## DOCUMENT`.

### Improve the KB Document (only when "# Improve" appears in the latest chat message)

- Copy precisely my improvement request in a new `# Improve` from the LATEST chat message to a new sub-section `### IMPROVEMENT` section under `## IMPROVEMENTS`.
  - The new `### IMPROVEMENT` should be added before `## DOCUMENT` as it is the latest improvement.
- Find `Steps for Improve` section for the complete instructions for updating `## DOCUMENT`.

### Add the KB Document to KB (only when "# Execute" appears in the latest chat message)

- Find `Steps for Execute` section for the complete instructions.

## Steps for Topic

- Your goal is to complete the `# INSIGHT`.
- The topic I would like you to research about is already copied to `# DESIGN REQUEST`.
- The topic is around a feature of the project. It involves multiple places of the source code across many components.
- You need to find out the details of the code logic about:
  - The entry point.
  - The core part.
  - Whether there are multiple branches of cases, find all of them.
  - Whether there are recursive calls, find the structure.
- Explain in detail about the design and:
  - architecture
  - organization of components
  - execution flows
  - design patterns (if applicable)
- Each point you mention should provide proof from the source code.
- When listing cases from a choice, try your best to include all of them.
- You are writing all of this so that, when you need to change it in the future, you know what to change.
- You should keep the content compact.
  - When you refer to the source code, do not include code snippets.
  - Say the function name, and if the function is big, a little bit more to locate the code in it.
  - Do not use line number as source codes are subject to change rapidly.
  - The document is for understanding the source code, so you must keep mentioning names instead of using a too high-level abstraction language.
  - Avoid doing pure code translation, focus more on the design and the logic, focus more on why and how.

## Steps for Ask

- Your goal is to complete the `### ANSWER` section in the `## QUESTION` section.
- The finding I would like you to clarify is already copied to the last `## QUESTION`.
- I will point out that what you are wrong about, what I still do not understand, where you should need to go deeper.
- You will have to answer my question in `### ANSWER`.
- Fix `# INSIGHT` if there is anything wrong, the insight to update is between `# INSIGHT` and `# ASKS`.

## Steps for Draft

- Your goal is to complete the `## (API|DESIGN) EXPLANATION` and `## DOCUMENT` section.
- Keep everything before `# DRAFT` unchanged, DO NOT edit anything in it.
- Read everything before `# DRAFT` carefully, you are going to draft the document based on every information there, including necessary supporting materials mentioned there.
- Extra information is already copied to `## DRAFT REQUEST` if any.
- You are not going to edit `Index.md` at the moment, you are only editing `Copilot_KB.md`.

### Decide the Type of the Document

- You are going to decide which project this document belongs to.
- You are going to decide the type of the document.
- Change `## (API|DESIGN) EXPLANATION` to `## API EXPLANATION (PROJECT)` if it is more about the usage and contract of APIs.
  - It means eventually a new section will be added under `### Choosing APIs` under the specified project in `Index.md`.
- Change `## (API|DESIGN) EXPLANATION` to `## DESIGN EXPLANATION (PROJECT)` if it is more about how a set of APIs are working together and the design and implementation of the source code.
  - It means eventually a new section will be added under `### Design Explanation` under the specified project in `Index.md`.
- Fill the section to describe which part you would like to update in the knowledge base.

### Draft the Document

- You are going to draft the document under `## DOCUMENT`.
- The draft document is completely based on the source code of the project, and all findings before `# DRAFT`.
  - You must not miss any details, you must use every single point mentioned in the document.
  - Since `# DOCUMENT REQUEST` is organized as multiple rounds of questions and answers, it cannot be just directly used as a document. You must reorganize them.
  - Similar knowledge under the same categories might be spread in different answers, pay attention to them, bring a well-organized document.
- Quality of the draft:
  - The document is for understanding the source code, so you must keep mentioning names instead of using a too high-level abstraction language.
  - You must use everything before `# DRAFT` with details. Do not just make a summary, `# DOCUMENT REQUEST` is already a summary.
  - Multiple levels of `#` markdown topic containing bullet points are favored.

## Steps for Improve

- Your goal is to update `## DOCUMENT` based on my suggestion.
- The finding I would like you to clarify is already copied to the last `### IMPROVEMENT`.
- You are not going to edit `Index.md` at the moment, you are only editing `Copilot_KB.md`.

## Steps for Execute

- There is `## API EXPLANATION (PROJECT)` or `## DESIGN EXPLANATION (PROJECT)` section filled with the title and content of the document you drafted.
  - If it is `## API EXPLANATION (PROJECT)`:
    - A new section will be added under `### Choosing APIs` under the specified project in `Index.md`.
    - At the end of the new section, a `[API Explanation]()` link is expected.
  - If it is `## DESIGN EXPLANATION (PROJECT)`:
    - A new section will be added under `### Design Explanation` under the specified project in `Index.md`.
    - At the end of the new section, a `[Design Explanation]()` link is expected.
- Create a new section in `Index.md`.
  - Following `Leveraging the Knowledge Base` in `REPO-ROOT/.github/copilot-instructions.md` to figure out the expected format.
  - The document is super long, you have to carefully figure out where is the exact place to add the new section.
  - Use bullet points for the description of the topic to cover the most important points so that it will be easy to identify in the future if this topic is relevant to a work to do.
  - Create a document file according to the hyperlink.
  - The title of the new document file should follow `## API EXPLANATION (PROJECT)` or `## DESIGN EXPLANATION (PROJECT)`.
  - The content must be exactly and precisely the content under `## DOCUMENT`. But do not copy the `## DOCUMENT` title itself.
- Keep `Copilot_KB.md` unchanged.
