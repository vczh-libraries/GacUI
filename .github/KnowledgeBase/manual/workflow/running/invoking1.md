# Invoking C++ Classes

Involved files: - [Main.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W02_InvokingCppClasses/Main.cpp)

It is allowed to use reflectable types in Workflow script. Here we define a type to read and write from a console window. Registering`missing document: /vlppreflection/ref/VL__REFLECTION__DESCRIPTION.html` this class is required, but the code will not be pasted here.

Firstly we define a reflectable C++ class: ``` namespace myapi { class App : public Object, public Description<App> { public: static WString Get() { return Console::Read(); } static WString Get(const WString& message) { Console::Write(message); return Console::Read(); } static void Print(const WString& text) { Console::WriteLine(text); } }; } ```

Secondly, we use this class in the Workflow script: ``` module sampleModule; using myapi::*; func main(): void { var name = App::Get("Please enter your name:"); App::Print($"Hello, $(name)!"); } ```

Workflow uses wildcard to import names like **using myapi::*;**. In fact, you can use it to use part of a name. For example, in GacUI we have **vl::presentation::controls::GuiWindow**, it is registered using **presentation::controls::GuiWindow**. You are able to use **Window** in the script by **using presentation::controls::Gui*;**.

**IMPORTANT**: although in C++ we are free to create a class in either a raw pointer or a shared pointer, but when registering a contructor, a decision must be made. Some constructors create raw pointers, some constructors create shared pointers.

Here we cannot write **new App*()** or **new App^()**, because there is no registered contructor. But when one constructor is registered, and the constructor returns **Ptr\<App\>**, then only **new App^()** is allowed.

