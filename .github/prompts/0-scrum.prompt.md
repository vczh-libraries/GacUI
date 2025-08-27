# Scrum

## Maintaining Knowledge Base

You, as an experienced C++ developer for large scale systems, will need to maintain the knowledge base by keeping the documentation up to date, ensuring accuracy and clarity, and adding new information as needed.
During performing any tasks, if you find anything is missing but important, you need to enrich the knowledge base.

In `KnowledgeBase` project (`KnowledgeBase.vcxitems`), there is an `Index.md` file.
This file serves as the main entry point for the knowledge base, providing an overview of the content and structure of the documentation.

### Project Overview Section

Under the `Project Overview` section, there are sections for each project covered in the knowledge base.
You will find short explanation and imported source files for each project.
You need to find out in `copilot-instructions.md` what project you are in and what imported projects are available.
If a project is imported, the `Files from Imports` guide you to the source code.
For a currently editing project, source files are in the current solution.

### Guidance Section

Under the `Guidance` section, there are detailed guidelines about how to use utilities offered by each project.
Under each section of the project, guidelines are categorized under different topics, each topic is a section starting with `####`.
Under each topics, there are hyperlinks to detailed guideline pages.
The title of the hyperlink will be a little task you want to do, for example "How to read a file".

#### Organization

Each guideline file is organized and named after this pattern: `KB_Project_Category_TaskDescription.md`.
For example, "How to read a file" will surely using `VlppOS`, so the file name will be `KB_VlppOS_FileSystem_HowToReadAFile.md`.
The `TaskDescription` part must be compact so we don't create super long file names, but the guideline title could be more descriptive.

#### Solution File

In each guideline file, the title will be the task description in the hyperlink (descriptive version instead of the compact file name version).
There may be multiple solutions to the same, each solution will be in the same guideline file, organized in this way:

```markdown
# How To Read a File

## Using vl::stream::FileStream
blahblahblah

### Work with vl::stream::StreamReader to read the file as text
blahblahblah

### Handle UTF Encoding
blahblahblah

## Using vl::filesystem::File
blahblahblah
```

## Experiences and Learnings Section

Under the `Experiences and Learnings` section, you will find:
- insights and reflections on the development process.
- coding style, some common or repeat requirements from me.
- best practices and lessons learned from previous performed tasks.

You need to create your own category, maintain the content as a list of hyperlinks.
Each hyperlink points to a file named after this pattern: `Exp_Category_Description.md`.

## Maintaining the `KnowledgeBase` project in the Solution

Once you create a hyperlink with a file, you will need to add this file to the `KnowledgeBase` project.
