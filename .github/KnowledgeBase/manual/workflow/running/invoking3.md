# Inheriting C++ Classes

Involved files: - [Main.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Console_Workflow/W04_InheritingCppClasses/Main.cpp)

Workflow classes are allowed to inherit from reflectable C++ classes. Here we have a C++ class, with two properties: ``` namespace myapi { class App : public Object, public AggregatableDescription<App> { private: WString name; List<Ptr<IValueSubscription>> subscriptions; public: static WString Get(); static WString Get(const WString& message); static void Print(const WString& text); App() = default; ~App() { FOREACH(Ptr<IValueSubscription>, sub, subscriptions) { sub->Close(); } FinalizeAggregation(); } Event<void()> NameChanged; WString GetName(); void SetName(const WString& value); List<Ptr<IValueSubscription>>& GetSubscriptions(); }; } ``` The destructor code is important, so it is not hidden.

An inheritable class need to use **AggregatableDescription** instead of **Description**. **FinalizeAggregation** is also very important. If this function is not called when destructing a Workflow class that inherits from App, it will crash by design.

Inheriting a reflectable C++ class is very straight forward, but there is no virtual methods in classes in this version of Workflow script, so you cannot override virtual methods in reflectable C++ classes.

``` module sampleModule; using myapi::*; namespace myscript { class MyApp : App { new(name: string) { Name = name; /* "this" is not allowed in binding, copy it to a variable */ var self = this; var subscription = bind($"Hello, $(self.Name)"); attach(subscription.ValueChanged, func (value: object): void { Print(cast string value); }); subscription.Open(); Subscriptions.Add(subscription); } delete { Print("Destructing myscript::MyApp"); } } } ``` Constructors and destructors are allowed in Workflow classes.

Data binding will not be explained in detail here, basically the code is tracking the changing of **$"Hello, $(self.Name)"**. When **Name** property is changed, it is obviously that the result of this expression is also changed. Whenever the data binding is changed, **ValueChanged** will be called. Here it prints the formatted string to the console window.

All classes and interfaces defined in Workflow scripts will be registered when loadong an assembly. This is why you cannot load it twice, because types are conflict with each other.

When a reflectable class is loaded, regardless whether it is implemented in Workflow or C++, the way to call does not change. But since it inherits from **App**, it is much easier to get an instance of **App**. ``` auto myapp = UnboxValue<Ptr<App>>(Value::Create( L"myscript::MyApp", ( Value_xs(), WString(L"vczh") ))); myapp->SetName(L"Vczh Libraries++"); myapp->SetName(L"Workflow"); myapp->SetName(L"Gaclib"); ``` This code creates **myscript::MyApp** with a string passed to the constructor. It changes **Name** three times, so four lines will be printed to the console window. The last one is from the destructor.

