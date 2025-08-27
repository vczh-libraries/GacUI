# Scrum

Read through `copilot-instructions.md` carefully.
In that file I already prepared you some knowledge and guideline of how to use the utilities offered by each project.

Read through `Index.md` carefully. You goal is to pick the first incomplete category and complete every guidelines in it.

A missing guideline is a hyperlink begins with `*`. After finishing it, remove the `*`. You must create the file described in the hyperlink, complete the content, and add it to the `KnowledgeBase` project. Begins from `VlppOS` as previous projects are all finished.

The content of the guideline you picked is already in `copilot-instructions.md`.
Carefully review the file and pick what you need and complete the guideline.

Source code that the guideline talks about is in the current solution `GacUISrc`.
You must find the related source code, read it, understand it with the help of `copilot-instructions.md`.

Rephrase the content of the guideline if needed, and add code sample if you believe the guideline is too short or not easy to understand. Add only code sample around utility usage, less in general programming patterns (or even no if the utility does not connect with such pattern)

Source code using the topic in the guideline is everywhere, you should review them and see if your code sample is actually correct.

Do not make up anything that is not included in `copilot-instructions.md`.
Do not edit `copilot-instructions.md`.
You only need to edit `Index.md`, the `KnowledgeBase` project, and guideline file you just created.

Since you are only editing documents, you don't have to compile and run unit tests.

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

> # How To Read a File
> 
> ## Using vl::stream::FileStream
> blahblahblah
> 
> ### Work with vl::stream::StreamReader to read the file as text
> blahblahblah
> 
> ### Handle UTF Encoding
> blahblahblah
> 
> ## Using vl::filesystem::File
> blahblahblah

## Choosing a good topic of categories and guidelines

The whole project is huge, and C++ source files are not easy to read. The goal of guidelines is to make your (aka Copilot) work easier.
When you want to complete a small task (e.g. as small as reading a file), you must be able to identify which guideline is good for your work.
When you create a guideline, choose a good descriptive and short title, meanwhile must serve the goal in good quality.

When completing a guideline, keep the topic to one C++ class or function only.
You can contain more than one but only when they are closely related.

Content of the guideline must be short enough.
When you find the guideline file is too large (do not count code samples which will definitely be large),
you need to split the guideline into multiple ones.

## Experiences and Learnings Section

Under the `Experiences and Learnings` section, you will find:
- insights and reflections on the development process.
- coding style, some common or repeat requirements from me.
- best practices and lessons learned from previous performed tasks.

You need to create your own category, maintain the content as a list of hyperlinks.
Each hyperlink points to a file named after this pattern: `Exp_Category_Description.md`.

## Maintaining the `KnowledgeBase` project in the Solution

Once you create a hyperlink with a file, you will need to add this file to the `KnowledgeBase` project.
