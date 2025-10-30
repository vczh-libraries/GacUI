# Interop with C++ View Model

It is recommended to use the [MVVM](https://en.wikipedia.org/wiki/Model%E2%80%93view%E2%80%93viewmodel) pattern with GacUI: - `empty list item` **Model**: It refers to a domain model. Code in **Model** solves real problem that users need. It could be an object-oriented design of a domain, it could also be a data access layer to the data in database, from the internet, or any possible way. - `empty list item` **ViewModel**: It refers to a set of interface exposing the **Model** in a convient way for **Views**. Data organization in **Model** doesn't have to be UI friendly. It is the **ViewModel** who exposes a right set of operations, to make **Views** access **Model** convenient. For example, there are many applications offer list sorting to users. Data in **Model** is not sorted, it could be indexed, because there is so many way to sort the data, it is a waste of storage to cache all sorting result. When a user sort the data in a specific way, it is the **ViewModel** who handels the sorting. the **ViewModel** accesses the data, return the data organization in a sorted way. - `empty list item` **View**: A **View** could be the actual UI of an application, it could also be a set of unit tests. **Views** access **ViewModel**, typically it doesn't access **Model**.

In GacUI, **ViewModels** are defined as a set of interfaces in [Workflow script files](../.././gacui/xmlres/tag_script.md). **Views** are defined as a set of [windows and controls](../.././gacui/xmlres/instance/root_instance.md).

All **ViewModels** that are needed in a **View** are defined as [<ref.Parameter/>](../.././gacui/xmlres/tag_instance.md), which represents constructor arguments to that **View**. The **Class** attributes of such **\<ref.Parameter/\>** will be interfaces for **ViewModels**.

In the GacUI entry point (the **GuiMain** function), a main window will be created. If the main window needs **ViewModels**, the main window **\<Instance/\>** will have one or more **\<ref.Parameter/\>**, then the constructor of the main window will have one or more constructor arguments.

Now, **GuiMain** just needs to create classes that implement these interfaces, and gives all of them to the constructor of the main window, then **ViewModels** and the **View** is connected together.

## Sample

[This](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_HelloWorlds/MVVM) is a very simple example for using MVVM in GacUI.

![](https://gaclib.net/doc/gacui/adv_mvvm.gif)

The **ViewModel** is an interface: ``` interface IViewModel { func GetUserName() : string; func SetUserName(value : string) : void; prop UserName : string {GetUserName, SetUserName} func GetPassword() : string; func SetPassword(value : string) : void; prop Password : string {GetPassword, SetPassword} func GetUserNameError() : string; event UserNameErrorChanged(); prop UserNameError : string {GetUserNameError : UserNameErrorChanged} func GetPasswordError() : string; event PasswordErrorChanged(); prop PasswordError : string {GetPasswordError : PasswordErrorChanged} func SignUp() : bool; } ``` It defines all **View** needs to know: - Accessing the user name and the password. - Retriving validation results for the user name and the password.

When the content of a text box is changed, the data is stored to the view model. This is implemented using data bindings to the view model object: ``` <ref.Parameter Name="ViewModel" Class="vm::IViewModel"/> <Window ref.Name="self" Text="Let's Sign Up!" ClientSize="x:320 y:320"> <att.ViewModel-set UserName-bind="textBoxUserName.Text" Password-bind="textBoxPassword.Text"/> ... ```

When the validation result is changed, errors are displayed to the UI immediately. This is implemented using data bindings to UI objects: ``` <SolidLabel Text-bind="ViewModel.UserNameError"/> ```

When the user name or the password is stored to the view model, validation results are updated automatically. This is implemented in a C++ class which inherits the interface: ``` class ViewModel : public Object, public virtual vm::IViewModel { private: WString userName; WString password; Regex regexLcLetters; Regex regexUcLetters; Regex regexNumbers; public: ViewModel() :regexLcLetters(L"[a-z]") , regexUcLetters(L"[A-Z]") , regexNumbers(L"[0-9]") { } ... WString GetPassword()override { return password; } void SetPassword(const WString& value)override { password = value; PasswordErrorChanged(); } WString GetPasswordError()override { if (password == L"") { return L"Password should not be empty."; } bool containsLowerCases = regexLcLetters.Match(password); bool containsUpperCases = regexUcLetters.Match(password); bool containsNumbers = regexNumbers.Match(password); if (!containsLowerCases || !containsUpperCases || !containsNumbers) { return L"Password should contains at least one lower case letter, one upper case letter and one digit."; } return L""; } }; ```

It is very easy to connect the **ViewModel** to the **View**: ``` void GuiMain() { ... auto viewModel = Ptr(new ViewModel); auto window = new helloworld::MainWindow(viewModel); ... } ```

Collection objects or tree-like objects can be binded to [list controls](../.././gacui/components/controls/list/home.md) directly. Data bindings accept any [workflow expressions](../.././workflow/lang/bind.md) that compiles. MVVM in GacUI is very powerful, you are free to choose any way to define **ViewModel** interfaces, and it is always possible to bind it to the UI.

