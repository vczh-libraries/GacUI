# Running Workflow Scripts

A workflow script can be executed in 3 ways: - Compile and run directly. - Compile to a binary file, and load it in another process. If the script need to be executed multiple times, this is the recommended way to do it. Because by not having to run the compiler, the launch time is significantly improved. - Compile and generate C++ files from the script. If the script does not change at all, this is the recommended way to do it. GacUI is one of the example, it compiles different types of XML to Workflow scripts, merges with manually written scripts and generate all of them to multiple C++ files.

In order to show all kinds of interop between Workflow and C++, [an example solution](https://github.com/vczh-libraries/Release/tree/master/Tutorial/Console_Workflow) is prepared. Projects in this solution are explained in this section.

Reflection`missing document: /vlppreflection/ref/VL__REFLECTION__DESCRIPTION.html` is very important in Workflow script, you are recommended to read the document before using Workflow.

