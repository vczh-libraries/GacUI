# Localization

The GacUI XML [<LocalizedStrings/>](../.././gacui/xmlres/tag_localizedstrings.md) resource creates multi-language string template as bindable objects.

The GacUI XML [<LocalizedStringsInjection/>](../.././gacui/xmlres/tag_localizedstringsinjection.md) resource adds more languages to an existing **\<LocalizedStrings/\>**.

## \<ref.LocalizedStrings/\>

**\<ref.LocalizedStrings/\>** imports a **\<LocalizedStrings/\>** to the current UI instance: ``` <ref.LocalizedStrings Name="Strings" Class="demo::StringResource" Default="true"/> ``` - **Name**: The name of this string resource. It will also create a property in the current UI instance in the same name. The property value could be changed at runtime. Unlike **\<ref.Parameter/\>**, the instance knows how to initialize a string resource, so it doesn't create a constructor parameter. - **Class**: The full name of the string resource class. The resource should look like this to be referenced using **Class="demo::StringResource"**: ``` <LocalizedStrings ref.Class="demo::StringResource" DefaultLocale="en-US"> ... </LocalizedStrings> ``` - **Default**: Specify a default string resource. If there are multiple **\<LocalizedStrings/\>**, only one of them could be the default string resource.

## -str Binding

The value of the **-str** binding must be a [ Workflow call expression ](../.././workflow/lang/expr.md). The function could be either **METHOD** or **NAME.METHOD**. - **METHOD**: This syntax use the **default string resource**. **METHOD** must be one of the **Name** attribute of a **\<String/\>** in the referenced **\<LocalizedStrings/\>**. - **NAME.METHOD**: Unlike the above one, this syntax use the **\<ref.LocalizedStrings/\>** whose **Name** attribute is **NAME**. Once the string resource and the string item is located, the **-str** binding will check if arguments match the type requirement. Note that parameter types of the same string item in different languages should be exactly the same in one **\<LocalizedStrings/\>**, so it doesn't matter which is the current UI language. This also ensure that you are able to switch the UI language at runtime.

## Using -bind or -format on String Resources

For a string resource imported by: ``` <ref.LocalizedStrings Name="Strings" Class="demo::StringResource" Default="true"/> ``` The **-str** binding is also a data binding, so it knows when the **Strings** property is changed, and update the property value.

Assume the default string resource is **Strings**, the following binding code - **-str="Something(p1, p2, p3)"** - **-str="Strings.Something(p1, p2, p3)"** is equivalent to - **-bind="self.Strings.Something(p1, p2, p3)"** - **-format="$(self.Strings.Something(p1, p2, p3))"** when the current UI instance has a **ref.Name="self"** attribute.

## Switching the UI Language

The only way to specify the UI language is: - **C++**: GetApplication()-\>SetLocale(LOCALE); - **Workflow**: presentation::controls::GuiApplication.GetApplication().Locale = LOCALE; This configuration affect all windows in the current process.

For a string resource imported by: ``` <ref.LocalizedStrings Name="Strings" Class="demo::StringResource" Default="true"/> ``` When the application locale is changed, the **Strings** property of the current UI object will be replaced by a new object, which is from a **\<Strings/\>** in the referenced **\<LocalizedStrings/\>** for that locale. If the specified locale doesn't exist in that localized string, the default one will be used.

Such **\<LocalizedStrings/\>** will also create an interface called demo::**I**StringResource**Strings**, which becomes the type of the **Strings** property if it is imported using the above **\<ref.LocalizedStrings/\>**. Usually you don't need to use either the generated class **demo::StringResource**. or the generated interface **demo::IStringResourceStrings**.

