# Update Knowledge Base

## Goal and Constraints

- Your goal is to draft a document for the knowledge base in `Copilot_KB.md`.
- You are only allowed to update `Copilot_KB.md` and the knowledge base.
- You are not allowed to modify any other files.
- Code references must be wrapped in either `single-line` or ```multi-line``` quotes.
- Check out `Leveraging the Knowledge Base`, read the entry file and understand how it is organized: `REPO-ROOT/.github/KnowledgeBase/Index.md`.
  - `Index.md` below means this file.

## Identify the Problem

- Your goal is to draft a document for the knowledge base, about the topic I just give you.
- Find `# Topic` or `# Ask` or `# Draft` or `# Improve` or `# Execute` in the LATEST chat message.
- Ignore any `# Topic` or `# Ask` or `# Draft` or `# Improve` or `# Execute` in the chat history.

- If there is a `# Topic` section: it means you are on a fresh start.
  - You should override `Copilot_KB.md` with only one title `# !!!KNOWLEDGE BASE!!!`.
  - After overriding, copy precisely my problem description in `# Topic` from the LATEST chat message under a `# DOCUMENT REQUEST`, with a new sub-section `## TOPIC`.
  - Find `Steps for Topic` section for the complete instructions.

- If there is an `# Ask` section: it means I want you to clarify your findings.
  - Copy precisely my problem description in `# Ask` from the LATEST chat message to the `# DOCUMENT REQUEST` section, with a new sub-section `## ASK`.
  - Find `Steps for Ask` section for the complete instructions.

- If there is a `# Draft` section: it means I think you are ready to draft the document based on information you found.
  - Copy precisely my problem description in `# Draft` from the LATEST chat message to the `# DOCUMENT REQUEST` section, with a new sub-section `## DRAFT`.
  - Find `Steps for Draft` section for the complete instructions.

- If there is an `# Improve` section: it means I want you to improve the draft.
  - Copy precisely my problem description in `# Improve` from the LATEST chat message to the `# DOCUMENT REQUEST` section, with a new sub-section `## IMPROVE`.
  - Find `Steps for Improve` section for the complete instructions.

- If there is an `# Execute` section: it means I am satisfied with the draft document. You are going to add it to the knowledge base.
  - Find `Steps for Execute` section for the complete instructions.

- If there is nothing: it means you are accidentally stopped. Please continue your work.
  - Read `Copilot_KB.md` thoroughly, it is highly possible that you were working on the request described in the last section in `# DOCUMENT REQUEST`.

- Existing content in `# DOCUMENT REQUEST` should be frozen, you can change it only when you find out the analysis is incorrect during my clarification (try to limit the scope to `## TOPIC`).

## Steps for Topic

- Your goal is to complete a `### Insight` section in the `## TOPIC` of the `# DOCUMENT REQUEST` section.
- The topic I would like you to research about is in the `# Topic` section in the LATEST chat message.
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

## Steps for Ask

- Your goal is to complete a `### Insight` section in the `## ASK` of the `# DOCUMENT REQUEST` section.
- The finding I would like you to clarify is in the `# Ask` section in the LATEST chat message.
- I will point out that what you are wrong about, what I still do not understand, where you should need to go deeper.
- You will have to answer my question in `### Insight` of `## ASK`, and fix `## TOPIC` under `# DOCUMENT REQUEST` if there is anything wrong.

## Steps for Draft

- Your goal is to complete a draft document for the knowledge base in `Copilot_KB.md`, append the draft after the file.
- Keep `# DOCUMENT REQUEST` unchanged, do not edit anything in it. Instead, you have to read `# DOCUMENT REQUEST` carefully.
- Extra information is provided in the `# Draft` section in the LATEST chat message.
- Make a `# DRAFT-LOCATION` section, you need to describe which part you would like to update in the knowledge base.
  - It will be a new topic under the `Design Explanation` section of a project in `Index.md` of the knowledge base.
  - But at the moment only edit `Copilot_KB.md`, do not edit `Index.md`. You are drafting a document, it will be implemented in `Index.md` only after my approval.
- Make a `# DRAFT-TITLE` section, you need to make a comprehensive but short title for the draft document.
- Make a `# DRAFT-CONTENT` section, you need to complete the content of the draft document here.
- The draft document is completely based on the source code of the project, and all findings in the `# DOCUMENT REQUEST` section.
  - You must not miss any details, you must use every single point mentioned in the document.
  - Since `# DOCUMENT REQUEST` is organized as multiple rounds of questions and answers, it cannot be just directly used as a document. You must reorganize them.
  - Similar knowledge under the same categories might be spread in different answers, pay attention to them, bring a well-organized document.
- Quality of the draft:
  - The document is for understanding the source code, so you must keep mentioning names instead of using a too high-level abstraction language.
  - You must use everything in `# DOCUMENT REQUEST`. Do not just make a summary, `# DOCUMENT REQUEST` is already a summary.
  - Multiple levels of `#` markdown topic containing bullet points are favored.

## Steps for Improve

- Your goal is to update `# DRAFT-*` based on my suggestion.
- The suggestion is in the `# Improve` section in the LATEST chat message.

## Steps for Execute

- Follow `# DRAFT-LOCATION` to add a new topic under the `Design Explanation` section of a project in `Index.md` of the knowledge base.
- Use bullet points for the description of the topic to cover the most important points so that it will be easy to identify in the future if this topic is relevant to a work to do.
- Create a file according to the hyperlink and add it to the `KnowledgeBase` project.
- The title of the document is in the `# DRAFT-TITLE` section.
- The content must be exactly and precisely the content under `# DRAFT-CONTENT`. But do not copy the `# DRAFT-CONTENT` title itself.
- Keep `Copilot_KB.md` unchanged.
