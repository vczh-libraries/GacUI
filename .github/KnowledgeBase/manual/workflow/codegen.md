# C++ Code Generation

Attributies can be applied to Workflow classes, functions and members to control the result of C++ code generation.

## @cpp:File("YourFile")

This attribute applies to **classes**. It creates a new pair of file **YourFile.h** and **YourFile.cpp** to

Since internal classes in Workflow get translated to internal classes in C++, if you assign different file name to class A and B when B is defined inside A, it causes an error.

This attribute is useful when the class has methods with **@cpp:UserImpl**.

## @cpp:UserImpl

This attribute applies to **constructors, destructors and functions** in classes.

Bodies of functions with **@cpp:UserImpl** are ignored, they are left blank in the generated file, allowing users to add custom code lines between: - **/* USER_CONTENT_BEGIN(CLASS-NAME) */** - **/* USER_CONTENT_END() */**

When a class contains methods with **@cpp:UserImpl**, custom members can also be added to the generated header file for this class between: - **/* USER_CONTENT_BEGIN(custom members of CLASS-NAME) */** - **/* USER_CONTENT_END() */**

When a file contains methods with **@cpp:UserImpl**, custom global declarations can also be added to the generated header file for this class between: - **/* USER_CONTENT_BEGIN(custom global declarations) */** - **/* USER_CONTENT_END() */**

When the Workflow script is modified and re-generated, new generated code and custom code lines in specific locations will be merged together, so your work doesn't lost.

**IMPORTANT**: If there are braces (**{** and **}**) in custom code lines, they must take the whole line, or the code merging will report errors.

## @cpp:Protected and @cpp:Private

These attributes apply to **any members** in classes, except non-auto properties.

When a class member is annotated by **@cpp:Protected**, this part of the C++ code will be generated with **protected:**. So as **@cpp:Private**.

**NOTE**: these attributes don't affect the Workflow compiler, Workflow script treats all members as public members.

## @cpp:Friend(typeof(YourType))

This attribute applies to **classes**. It generates **friend** declaration in C++ code, allowing **YourType** to access protected or private members of the current class.

## Example

### Workflow Script

``` module SampleModule; class DummyClass { func CallPrivateMethod(my: MyClass^): void { my.Method(); } } @cpp:File("MyClass") @cpp:Friend(typeof(DummyClass)) class MyClass { @cpp:UserImpl @cpp:Private func Method(): void { raise "Not Implemented!"; } } ```

### Generated C++ Files

**IMPORTANT**: - The default file name is configurable, here we assume that it is **SampleModule**. - This is not the full content of generated files. By following [Generating C++ Code](.././workflow/running/generating.md) you will see everything.

**SampleModule.h** ``` class DummyClass; class MyClass; class DummyClass : public ::vl::Object, public ::vl::reflection::Description<DummyClass> { #ifndef VCZH_DEBUG_NO_REFLECTION friend struct ::vl::reflection::description::CustomTypeDescriptorSelector<DummyClass>; #endif public: void CallPrivateMethod(::vl::Ptr<::MyClass> my); DummyClass(); }; /*********************************************************************** Global Variables and Functions ***********************************************************************/ namespace vl_workflow_global { class W05Script { public: static W05Script& Instance(); }; } ```

**SampleModule.cpp** ``` #define GLOBAL_SYMBOL ::vl_workflow_global::W05Script:: #define GLOBAL_NAME ::vl_workflow_global::W05Script::Instance(). #define GLOBAL_OBJ &::vl_workflow_global::W05Script::Instance() /*********************************************************************** Global Variables ***********************************************************************/ BEGIN_GLOBAL_STORAGE_CLASS(vl_workflow_global_W05Script) vl_workflow_global::W05Script instance; INITIALIZE_GLOBAL_STORAGE_CLASS FINALIZE_GLOBAL_STORAGE_CLASS END_GLOBAL_STORAGE_CLASS(vl_workflow_global_W05Script) namespace vl_workflow_global { /*********************************************************************** Global Functions ***********************************************************************/ W05Script& W05Script::Instance() { return Getvl_workflow_global_W05Script().instance; } } /*********************************************************************** Class (::DummyClass) ***********************************************************************/ void DummyClass::CallPrivateMethod(::vl::Ptr<::MyClass> my) { ::vl::__vwsn::This(my.Obj())->Method(); } DummyClass::DummyClass() { } #undef GLOBAL_SYMBOL #undef GLOBAL_NAME #undef GLOBAL_OBJ ```

**MyClass.h** ``` class MyClass : public ::vl::Object, public ::vl::reflection::Description<MyClass> { friend class ::DummyClass; #ifndef VCZH_DEBUG_NO_REFLECTION friend struct ::vl::reflection::description::CustomTypeDescriptorSelector<MyClass>; #endif private: void Method(); public: MyClass(); /* USER_CONTENT_BEGIN(custom members of ::MyClass) */ /* USER_CONTENT_END() */ }; ```

**MyClass.cpp** ``` #define GLOBAL_SYMBOL ::vl_workflow_global::W05Script:: #define GLOBAL_NAME ::vl_workflow_global::W05Script::Instance(). #define GLOBAL_OBJ &::vl_workflow_global::W05Script::Instance() /* USER_CONTENT_BEGIN(custom global declarations) */ /* USER_CONTENT_END() */ /*********************************************************************** Class (::MyClass) ***********************************************************************/ void MyClass::Method() {/* USER_CONTENT_BEGIN(::MyClass) */ throw ::vl::Exception(L"You should implement this function."); }/* USER_CONTENT_END() */ MyClass::MyClass() { } #undef GLOBAL_SYMBOL #undef GLOBAL_NAME #undef GLOBAL_OBJ ```

