# GacUI

**GPU Accelerated C++ User Interface, with WYSIWYG developing tools, XML supports, built-in data binding and MVVM features.**

## License

**Read the [LICENSE](https://github.com/vczh-libraries/GacUI/blob/master/LICENSE.md) first.**

This project is licensed under [the License repo](https://github.com/vczh-libraries/License).

Source code in this repo is for reference only, please use the source code in [the Release repo](https://github.com/vczh-libraries/Release).

You are welcome to contribute to this repo by opening pull requests.

## Document

For **Home Page**: click [here](http://vczh-libraries.github.io)

For **Gaclib**: click [here](http://vczh-libraries.github.io/doc/current/home.html)

For **GacUI**: click [here](http://vczh-libraries.github.io/doc/current/gacui/home.html)

[gaclib.net](http://www.gaclib.net/) is a mirror to github pages above.

## Content of This Project

- The [Tutorial](http://vczh-libraries.github.io/doc/current/gacui/running.html) shows how to start using this marvelous GUI library.
- The [Demos](http://vczh-libraries.github.io/demo.html) section shows you all demos using this GUI library.

### Notice

Using this library requires you to use C++ source files directly in

- The [Release folder](https://github.com/vczh-libraries/GacUI/tree/master/Release) for Vlpp, Workflow or GacUI.
- The [GacGen.exe](https://github.com/vczh-libraries/GacUI/tree/master/Tools/GacGen) if you prefer to use XML to build your UI.

All other files are for library development only.

### Content

This GUI library provides the following features:

- Develop your GUI using pure C++, [Workflow](https://github.com/vczh-libraries/Workflow) script, XML or even JavaScript (under construction).
- Cross-platform abilities.
  - for **Windows**: [Release repo](https://github.com/vczh-libraries/Release)
  - for **Linux**: [XGac repo](https://github.com/vczh-libraries/gGac) (beta release)
  - for **macOS**: [iGac repo](https://github.com/vczh-libraries/iGac) (beta release)
  - for running in **Browser** using WASM: (2.0)
- Rich control library. Container controls supports MVC and virtual list mode.
- Control template system. You can write your own.
- GPU acceleration.
- XML resource supports.
  - You can use the provided [GacGen.exe](https://github.com/vczh-libraries/GacUI/tree/master/Tools/GacGen) to generate multiple XML files with binary resources (images) into a compressed binary format.
  - This program will also generate C++ code behind for you. You can fill event handlers (but we suggest you use MVVM and Data Binding instead) in generated C++ files.
  - If you update your UI, GacGen.exe will merge your modification in C++ code and modification in XML together to generate new C++ code behind. Your update will still exist.
    - **NOTE**: You will see a very obvious mark `USERIMPL(/* ... */)` in the generated code. Here is where you want to add your code. All your modification outside of these places will be discarded on the next GacGen.exe run.
- MVVM and Data Binding.
  - You can define interfaces that required to build your own MVVM pattern in XML and [GacGen.exe](https://github.com/vczh-libraries/GacUI/tree/master/Tools/GacGen) will generate the C++ interface declaration for you.
  - You can also use [Workflow](https://github.com/vczh-libraries/Workflow) expressions in Data Binding and statements in event handlers (instead of writing C++ code in code behind).

https://gankra.github.io/blah/text-hates-you/  
https://lord.io/text-editing-hates-you-too/
