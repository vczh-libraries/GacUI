# \<LocalizedStrings\>

After a **GacUI XML Resource** is converted to a resource binary: - This resource does not exist in the resource binary. - The equivalent Workflow script will be included in the Workflow assembly part in the resource binary. - If you use the one specified in **GacGenConfig/Cpp**, the Workflow assembly is also not included in the resource binary, because you are supposed to use the generated C++ code.

\<LocalizedStrings/\> generate a class with a local property and methods returning text. By using data binding to bind these text to UI, changing the locale will also change texts in UI.

[Here](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_localizedstrings/Resource.xml) is a sample for \<LocalizedStrings/\>. In this same, there is a \<LocalizedStrings/\> like this: ``` <LocalizedStrings name="StringResource"> <LocalizedStrings ref.Class="sample::StringResource" DefaultLocale="en-US"> <Strings Locales="en-US"> <String Name="Date" Text="Date: $(0:LongDate)"/> <String Name="Week" Text=", $(0:Date:dddd)"/> <String Name="Time" Text="Time: $(0:LongTime)"/> <String Name="Message" Text="Good morning, $(0)!"/> <String Name="Title" Text="Localization"/> <String Name="SelectLocal" Text="Selected Locale:"/> <String Name="SelectName" Text="Enter Your Name:"/> <String Name="OK" Text="OK"/> <String Name="Cancel" Text="Cancel"/> </Strings> <Strings Locales="zh-CN"> <String Name="Date" Text="日期：$(0:LongDate)"/> <String Name="Week" Text="，$(0:Date:dddd)"/> <String Name="Time" Text="时间：$(0:LongTime)"/> <String Name="Message" Text="早上好，$(0)！"/> <String Name="Title" Text="本地化"/> <String Name="SelectLocal" Text="语言设置："/> <String Name="SelectName" Text="输入名字："/> <String Name="OK" Text="确定"/> <String Name="Cancel" Text="取消"/> </Strings> <Strings Locales="ja-JP"> <String Name="Date" Text="日付：$(0:LongDate)"/> <String Name="Week" Text=" $(0:Date:dddd)"/> <String Name="Time" Text="時刻：$(0:LongTime)"/> <String Name="Message" Text="おはようございます，$(0)さん！"/> <String Name="Title" Text="ローカライゼーション"/> <String Name="SelectLocal" Text="選択したロケール："/> <String Name="SelectName" Text="名前を入力してください："/> <String Name="OK" Text="OK"/> <String Name="Cancel" Text="キャンセル"/> </Strings> </LocalizedStrings> </LocalizedStrings> ```

There are two attributes for \<LocalizedStrings/\>: - **ref.Class**: The class name. This is used in \<ref.LocalizedStrings/\> in \<Instance/\>. - **DefaultLocale** The default locale name. If the translation for a locale does not exist, it uses the **DefaultLocale**.

In any \<String/\>, **Name** is the method name of this text item, **Text** is the content. Content consists of the following items: - Normal text. - **$($)**, which is the $ character. - **$(ARGUMENT-INDEX)**: A string parameter. Parameters are zero-based. - **$(ARGUMENT-INDEX:TYPE)**: A typed parameter. - **$(ARGUMENT-INDEX:TYPE:FORMAT)**: A typed parameter with specified format. Type must be one of the following: - ShortDate - LongDate - YearMonthData - ShortTime - LongTime - Date - Time - Number - Currency If the type is **Date** or **Time**, the format must be compatible to Locale::FormatTime`missing document: /vlppos/ref/VL__LOCALE__FORMATTIME@__VL__WSTRING(__VL__WSTRING_CONST_&,___VL.html`.

There are three attributes for \<ref.LocalizedStrings/\> in \<Instance/\>: - **Name**: The property name of this string resource. This property is added to the instance, and it could be **null**. - **Class**: **ref.Class** in the \<LocalizedStrings/\>. - **Default**: **true** or **false**. Only one string resource could be default.

After the configuration is ready, a text item could be accessed in one of the following ways: - **Property-str="METHOD(ARGUMENT, ...)"** - **Property-str="NAME.METHOD(ARGUMENT, ...)"** - **self.NAME.METHOD(ARGUMENT, ...)**, note that **self.NAME** could be **null**, the **??** operator is recommended.

**NAME** must be the name of one of the  \<ref.LocalizedStrings/\>. If it is not specified, the default one kicks in.

**METHOD** must be one of the text item name in the string resource.

**ARGUMENTS** could be any valid Workflow expression that is a string or can be implicitly converted to a string.

When the UI is created, the locale is set to **Application::GetApplication().Locale**, which is the default locale in the OS. To change the locale of the UI, just change **Application::GetApplication().Locale**, and all string resources will be updated if it is used in **-str**, **-format** or **-bind**.

Here is how it looks like:

- Source code: [kb_localizedstrings](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_localizedstrings/Resource.xml) - ![](https://gaclib.net/doc/gacui/kb_localizedstrings.gif)

