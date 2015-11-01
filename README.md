# GacUI
### GPU Accelerated C++ User Interface, with WYSIWYG developing tools, XML supports, built-in data binding and MVVM features.

Refer to http://gaclib.net for more information.

* The [Getting Started](http://www.gaclib.net/GettingStart.html) section tells you how to begin using this marvelous GUI library.
* The [DEMOS](http://www.gaclib.net/Demos.html) section show you all demos using this GUI library.
* The [DOCUMENT](http://www.gaclib.net/Document/reference_gacui.html) section show you all simple documentation organized by C++ classes and functions.

### Notice
The release of the GacUI project is put in [vczh-libraries/Release](https://github.com/vczh-libraries/Release).

Using this library requires you to use C++ source files (you should put them together) in

* The [Import folder](https://github.com/vczh-libraries/GacUI/tree/master/Import) for Vlpp and Workflow.
* The [Release folder](https://github.com/vczh-libraries/GacUI/tree/master/Release) for this library.
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
* MVVM and Data Binding.
    * You can define interfaces that required to build your own MVVM pattern in XML and [GacGen.exe](https://github.com/vczh-libraries/GacUI/tree/master/Tools/GacGen) will generate the C++ interface declaration for you.
    * You can also use [Workflow](https://github.com/vczh-libraries/Workflow) expressions in Data Binding and statements in event handlers (instead of writing C++ code in code behind).
