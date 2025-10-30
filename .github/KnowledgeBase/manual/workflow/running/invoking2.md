# Inheriting C++ Interfaces

Involved files: - [Main.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W03_InheritingCppInterfaces/Main.cpp)

You are able to implement a reflectable C++ interface in Workflow using anonymous interface expression. You are able to write classes in Workflow script, but classes are not allowed to implement interfaces.

For example, we can define this C++ interface, and make it reflectable: ``` class IScripting : public virtual IDescriptable, public Description<IScripting> { public: virtual void Execute(const WString& name) = 0; }; ```

By inheriting from **IDescriptable** and **Description**, instances of this interface are accessible in Workflow scripts. But in order to make Workflow scripts being able to implement this interface, more things need to be done: ``` namespace vl::reflection::description { BEGIN_INTERFACE_PROXY_SHAREDPTR(IScripting) void Execute(const WString& name)override { INVOKE_INTERFACE_PROXY(Execute, name); } END_INTERFACE_PROXY(IScripting) } ``` Unfortunately it looks boring, but this version of reflection requires you to write such code to allow it to be implemented by Workflow script code.

Here we use **BEGIN_INTERFACE_PROXY_SHAREDPTR**, because we want instances of this interface to be shared pointers.

Now, we are able to implement this interface in Workflow script: ``` module sampleModule; using myapi::*; func main(): IScripting^ { return new IScripting^ { override func Execute(name: string): void { App::Print($"Hello, $(name)!"); } }; } ``` And call it in C++: ``` auto mainFunction = LoadFunction<Ptr<myapi::IScripting>()>(globalContext, L"main"); mainFunction()->Execute(L"Gaclib"); ```

