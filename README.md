# GacUI
### GPU Accelerated C++ User Interface, with WYSIWYG developing tools, XML supports, built-in data binding and MVVM features.

**Read the [LICENSE](https://github.com/vczh-libraries/GacUI/blob/master/LICENSE.md) first.**

# Notice
- The release of the GacUI project is put in [vczh-libraries/Release](https://github.com/vczh-libraries/Release).
- **DO NOT** download source code from this repo **if you are not intended to create push request**. This is not for end users.

# Content of This Project

New website:
 - http://www.gaclib.net
 - https://vczh-libraries.github.io

This website is under construction, but it is somehow useable now. The old one (http://gaclib.net) is deprecated. After the new website is finished, I will update all domains to the new website.

* The [Tutorial](http://www.gaclib.net/#~/Tutorial) section tells you how to begin using this marvelous GUI library.
* The [DEMOES](http://www.gaclib.net/#~/Demo) section show you all demos using this GUI library.
* The [DOCUMENT](http://www.gaclib.net/Document.html#~/) section show you all simple documentation organized by C++ classes and functions.

### Notice

Using this library requires you to use C++ source files directly in

* The [Release folder](https://github.com/vczh-libraries/GacUI/tree/master/Release) for Vlpp, Workflow or GacUI.
* The [GacGen.exe](https://github.com/vczh-libraries/GacUI/tree/master/Tools/GacGen) if you prefer to use XML to build your UI.

All other files are for librari development only.

### Content
This GUI library provides the following features:
* Develop your GUI using pure C++, [Workflow](https://github.com/vczh-libraries/Workflow) script, XML or even JavaScript (under construction)
* Cross-platform abilities. 
    * for **Windows**: Just use the code here
    * for **Linux**: https://github.com/vczh-libraries/XGac
    * for **OSX**: https://github.com/vczh-libraries/iGac
    * for running in **Browser!**: https://github.com/vczh-libraries/GacJS (under construction)
* Rich control library. Container controls supports MVC and virtual list mode.
* Control template system. You can write your own 
* GPU acceleration.
* XML resource supports.
    * You can use the provided [GacGen.exe](https://github.com/vczh-libraries/GacUI/tree/master/Tools/GacGen) to generate multiple XML files with binary resources (images) into a compressed binary format.
    * This program will also generate C++ code behind for you. You can fill event handlers (but we suggest you use MVVM and Data Binding instead) in generated C++ files.
    * If you update your UI, GacGen.exe will merge your modification in C++ code and modification in XML together to generate new C++ code behind. Your update will still exist.
        * **NOTE**: You will see a very obvious mark `USERIMPL(/* ... */)` in the generated code. Here is where you want to add your code. All your modification outside of these places will be discarded on the next GacGen.exe run.
* MVVM and Data Binding.
    * You can define interfaces that required to build your own MVVM pattern in XML and [GacGen.exe](https://github.com/vczh-libraries/GacUI/tree/master/Tools/GacGen) will generate the C++ interface declaration for you.
    * You can also use [Workflow](https://github.com/vczh-libraries/Workflow) expressions in Data Binding and statements in event handlers (instead of writing C++ code in code behind).
