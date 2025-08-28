# Scrum

Read through `copilot-instructions.md` carefully.
In that file I already prepared you some knowledge and guideline of how to use the utilities offered by each project.

Do not make up anything that is not included in `copilot-instructions.md`.
Do not edit `copilot-instructions.md`.
You only need to edit `Index.md`, the `KnowledgeBase` project, and guideline file you just created.

Since you are only editing documents, you don't have to compile and run unit tests.

## Goal

Your goal is to complete one `API Explanation` of the first incomplete category.
An incomplete category will be marked by `*` before the `API Explanation` hyperlink.
After completing the `API Explanation`, you need to remove the `*` from that particular hyperlink.

You need to read through the whole `Index.md` carefully, and identify which part of `copilot-instruction.md` belongs to this category, and which part does not.

Create the file with the name in the `API Explanation` hyperlink, add it to the `KnowledgeBase` project.

In the file, you must copy exactly, precisely, absolutely related content from `copilot-instruction.md`.
You can do a little twists, like making the `# Title` for the API explanation file, re-leveling titles so that the number of `#`'s make sense.

After that, review the file you just created again, and read through related source code, complete whatever that you think it is missing.
Things that does not appear in `copilot-instruction.md` is less important, but I think it is still worth mentioning.
So you can put them under `## Extra Content`, and follow the wording of paragraphs you copied.

## Using and Maintaining Knowledge Base

You, as an experienced C++ developer for large scale systems, will need to maintain the knowledge base by keeping the documentation up to date, ensuring accuracy and clarity, adding new information as needed.

In the solution, there is a `KnowledgeBase` project. The entry point is its `Index.md` file.

In `KnowledgeBase` project (`KnowledgeBase.vcxitems`), there is an `Index.md` file.
This file serves as the main entry point for the knowledge base, providing an overview of the content and structure of the documentation.

`Index.md` is organized in this way:

- Guidance: A general guidance that play a super important part repo-wide.
- Each Project: A brief description of each project and its purpose.
  - Choosing APIs: Guidelines for selecting appropriate APIs for the project.
  - Design Explanation: Insights into the design decisions made within the project.
- Experiences and Learnings: Reflections on the development process and key takeaways.

### Project/Choosing APIs

There are multiple categories under `Choosing APIs`. Each category begins with a short and accurate title `#### Category`.
A category means a set of related things that you can do with APIs from this project.

Under the category, there is overall and comprehensive description about what you can do.

Under the description, there are bullet points and each item follow the format:  `- Use CLASS-NAME for blahblajblah` (If a function does not belong to a class, you can generate `Use FUNCTION-NAME ...`).
It mentions what to do, it does not mention how to do (as this part will be in `API Explanation`).
If many classes or functions serve the same, or very similar purpose, one bullet point will mention them together.

At the end of the category, there is a hyperlink: `[API Explanation](./KB_Project_Category.md)` (no space between file name, all pascal case).

### Project/Design Explanation

(To do ...)

### Experiences and Learnings

(To do ...)

### Adding new Guidelines to `KnowledgeBase`

When you create a new guideline:
- A hyperlink must insert to a appropriate position in `Index.md`.
- The file must be added to the `KnowledgeBase` project.
