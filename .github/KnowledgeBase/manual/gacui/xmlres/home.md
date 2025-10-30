# GacUI XML Resource

**GacUI XML Resource** is a very convenient way to create GacUI application.

**GacBuild.ps1** will perform increment building on multiple GacUI resource files that are depending on each other. There are two options in general to use the compiled resource files: - `empty list item` Compile everything in a binary file, - pros: It offers a huge flexibility of developing UI, UI can be loaded at runtime dynamically. - cons: The UI is running on a script engine, it is significantly slower that C++. and it also requires the application to enable reflection`missing document: /vlppreflection/home.html`, which usually cause the executable file to grow by 20MB. - `empty list item` Compile only resource in a binary file and generate C++ files for UI, - pros: All UI are converted to C++ code, the application can statically link to them and keep a high performance at runtime. And it offers an option to disable reflection`missing document: /vlppreflection/home.html`, which significantly reduces the size of the executable file. - cons: By disabling reflection, Workflow script is able to control UI, everything should be known at compile time and be in C++.

**If you decide to include Workflow script in resource binaries instead of generating C++ files**, [GacBuild.ps1](https://github.com/vczh-libraries/Release/tree/master/Tools) will generate different resource binaries for x86 and x64 platforms. Please be careful to load the correct one.

If you want to keep your executable file stand-alone, there is an extra option to generate the resource binary into the C++ code. In this case, you don't have to load the resource binary from file, the generated C++ code will take care it for you.

