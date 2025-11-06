# Update Knowledge Base

- Only checkout the LATEST chat message, ignore all chat history. It has the request about what to do about the knowledge base.
- Your goal is to update the knowledge base according to the request.

## Implement the Knowledge Base

- Find out the `Accessing the Knowledge Base` section. Understand the organization of the knowledge base.
- Read the `Index.md` of the knowledge base first.
- There could be multiple places as the request might include multiple objectives. For each objective:
  - Find out which project does it belong to.
  - Read through all categories, find out which category is the best fit.
    - If there is no obvious best answer, create a new category.A new category comes with a new guideline file, you must add it to the `KnowledgeBase` project.
    - Please be careful about the place to insert the new category, a new category should be added at the end of the belonging project.
  - Update the category description if you think there is anything worth menthioning.
  - Read through the file of the hyperlink in the category, update the content to reflect the change I want you to make.

## Adding or Updating an `API Explanation` guideline

- Content in such guideline must be compact. Do not repeat anything that could be read from the source code.
- Do not simply add code samples.
  - If you do, keep the code sample stick to only usage of APIs.
  - Do not add code sample just to show best practices or what it can do or what it is commonly used or something like that, describe these topics in words.
  - A code sample is only necessary when many functions or classes must be involved in a specific order. A good example would be the `TEST_FILE` structure.

# External Tools Environment and Context

- You are on Windows running in Visual Studio Code.
- Submitting CLI commands is not recommended unless you have no choice.
- There is some rules to follow to submit correct powershell commands:
  - DO NOT call `msbuild` or other executable files by yourself.
  - DO NOT create or delete any file unless explicitly directed.
  - MUST run any powershell script in this format: `& absolute-path.ps1 parameters...`.
  - MUST run tasks via Cursor for compiling and running test cases.

