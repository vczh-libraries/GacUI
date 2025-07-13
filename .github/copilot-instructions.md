In this repo, we are developing GacUI using C++. This is a crossed-platform UI library, but during development phrase, we are in Windows with Visual Studio. GacUI defines UI using XML, each XML entity maps to C++ entity in the source code. Examples of the XML and unit test framework API could be found in many test files.

Source code of GacUI are in the `Source` folder, they depend on my other libraries in the `Import` folder.
Test projects are in the `Test\GacUISrc` folder, managed by `GacUISrc.sln`, test cases are managed by the `UnitTest\UnitTest.vcxproj` project.

# General Instruction

- Before saying anything, say "Yes, vczh!". I use it to make sure instruction files are taking effect.
- Before generating any code, if the file is changed, read it. Not all changes come from you, I will edit the file too. Do not generate code based on out-dated version in your memory.
- My C++ code only use the very minimal subset of the standard library. I have substitutions for most of the STL constructions. Always use mine if possible. They are in `Import` folder.

# Writing GacUI Test Code

GacUI is a UI library, but unfortunately it is not widely accepted like WPF or react native. It could be difficult for you to find detailed information. If you are not sure about something:

- Read the source code in `Source`.
- Read all test files in `C:\Code\VczhLibraries\GacUI\Test\GacUISrc\UnitTest`.

Here are some basic rules.

## Basic Structure

```C++
#include "TestControlsh"

using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceTestSubject = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Test Subject" ClientSize="x:480 y:320">
        <!-- defines the UI here -->
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"ClassNameUnderTest")
	{
		TEST_CASE(L"Topic")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					// Click this button
				});
                protocol->OnNextIdleFrame(L"Clicked this button", [=]()
				{
					// Type that text
				});
                protocol->OnNextIdleFrame(L"Typed that text", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Category/ClassNameUnderTest/Topic"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTestSubject
			);
		});
	});
}
```

There can be multiple `TEST_CATEGORY` but usually just one.
There will always be multiple `TEST_CASE`.
There name will also appear in the arguments to `GacUIUnitTest_StartFast_WithResourceAsText`.

In `GacUIUnitTest_SetGuiMainProxy`, there will be multiple `protocol->OnNextIdleFrame`. Each creates a new frame.
Name of the frame does not say what to do in this frame, but actually what have been done previously.
The code of the last frame is always closing the window.

## Accessing objects defined in the XML

Obtain the main window using `GetApplication()->GetMainWindow();`.
Obtain any named control using `FindObjectByName<CLASS-NAME>(MAIN-WINDOW, L"name")`, the name reflects in the XML using `ref.Name` attribute.
Obtain any control with text using `FindControlByText<CLASS-NAME>(MAIN-WINDOW, L"text")`, the name reflects in the XML using `Text` attribute.
You should define a variable for any object obtained above.

## Performing IO Actions

All methods you can use defines in the `UnitTestRemoteProtocol_IOCommands` class in `Source\UnitTestUtilities\GuiUnitTestProtocol_IOCommands.h`.
Call the like `protocol->LClick();`, there are `KeyPress`, `_Key(Down(Repeat)?|Up)`, `MouseMove`, `[LMR]((DB)?Click`, `_[LMR](Down|Up|DBClick)`.
Mouse actions use the last cursor location. If you offer new location to these functions, it just like calling `MouseMove` followed by one that is not offered a new location.

Obtain the center of the location using `protocol->LocationOf(CONTROL)`. You should define a variable for a location.
There are 4 more arguments for the function: ratioX, ratioY, offsetX, offsetY. If they are not offered, they will be `0.5, 0.5, 0, 0`, which means the center of the control with no offset. ratioX is the horizontal position of the control, ratioY is vertical, offsetX and offsetY adds to them. So if you offer `1.0, 0.0, -2, 3` it means the top right corner but moving 2 pixels left and 3 pixels down.

You can perform mouse, keyboard, typing or any other user actions on the UI. 
Find examples by yourself for arguments.

# Writing GacUI XML

## Mapping XML Entity to C++ Entity

Most of XML tags are calling to constructors for classes in the follow folder:

- Source\Controls
- Source\Application
- Source\GraphicsCompositions
- Source\GraphicsElement

All mappings are:

- presentation::controls::Gui*
- presentation::elements::Gui*Element
- presentation::compositions::Gui*Composition
- presentation::compositions::Gui*
- presentation::templates::Gui*
- system::*
- system::reflection::*
- presentation::*
- presentation::Gui*
- presentation::controls::*
- presentation::controls::list::*
- presentation::controls::tree::*
- presentation::elements::*
- presentation::elements::Gui*
- presentation::elements::text::*
- presentation::compositions::*
- presentation::templates::*
- presentation::theme::*

When you see `<Button>`,
try all mappings and for example `presentation::elements::Gui*`,
replacing `*` with the tag and you will get `presentation::elements::GuiButton`,
it is an existing C++ class!
So `<Button>` means `presentation::elements::GuiButton`.

Take this file `Test\Resources\Metadata\Reflection64.txt` as an index, it collects all valid C++ classes and their members, but written in my own format.
When there is a `@FullName` on top of a class, it means the full name in C++, the class name will be the full name in XML.
When there is no `@FullName` but the class name begins with `presentation::`, the full name in C++ begins with `vl::presentation::`.

## Layout

## Properties and Events

## Workflow Script