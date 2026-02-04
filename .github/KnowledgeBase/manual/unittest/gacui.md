# Running GacUI in Unit Test Framework

The most simple way to start the unit test framework would be:
```c++
#include <GacUI.UnitTest.h>
using namespace vl;
using namespace vl::unittest;
using namespace vl::presentation::unittest;

#ifdef VCZH_MSVC
int wmain(int argc , wchar_t* argv[])
#else
int main(int argc, char** argv)
#endif
{
    UnitTestFrameworkConfig config;
    config.snapshotFolder = L"...";
    config.resourceFolder = L"...";
    GacUIUnitTest_Initialize(&config);
    int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
    GacUIUnitTest_Finalize();
#ifdef VCZH_CHECK_MEMORY_LEAKS
    _CrtDumpMemoryLeaks();
#endif
    return result;
}
```


A typical test file would be:
```c++
#include <GacUI.UnitTest.h>
using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;

TEST_FILE
{
    const auto resourceSingleButton = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiButton" ClientSize="x:320 y:240">
        <Button ref.Name="button" Text="This is a Button">
          <ev.Clicked-eval>...</ev.Clicked-eval>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

    TEST_CASE(L"ClickOnMouseUp")
    {
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Ready", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto button = FindObjectByName<GuiButton>(window, L"button");
                auto location = protocol->LocationOf(button);
                protocol->LClick(location);
            });
            protocol->OnNextIdleFrame(L"Close", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                window->Hide();
            });
        });
        GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
            WString::Unmanaged(L"Controls/Basic/GuiButton/ClickOnMouseUp"),
            WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
            resourceSingleButton
            );
    });
}
```
Or if you are loading an existing window instead of making one from GacUI XML resource:
```c++
#include <GacUI.UnitTest.h>
using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::unittest;

TEST_FILE
{
    TEST_CASE(L"ClickOnMouseUp")
    {
        GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
        {
            protocol->OnNextIdleFrame(L"Ready", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                auto button = FindObjectByName<GuiButton>(window, L"button");
                auto location = protocol->LocationOf(button);
                protocol->LClick(location);
            });
            protocol->OnNextIdleFrame(L"Close", [=]()
            {
                auto window = GetApplication()->GetMainWindow();
                window->Hide();
            });
        });
        GacUIUnitTest_LinkGuiMainProxy([=](
            vl::presentation::unittest::UnitTestRemoteProtocol* protocol,
            vl::presentation::unittest::IUnitTestContext* context,
            const vl::presentation::unittest::UnitTestMainFunc& previousMainProxy
            )
        {
            {
                vl::presentation::remoteprotocol::ControllerGlobalConfig globalConfig;
#if defined VCZH_WCHAR_UTF16
                globalConfig.documentCaretFromEncoding = vl::presentation::remoteprotocol::CharacterEncoding::UTF16;
#elif defined VCZH_WCHAR_UTF32
                globalConfig.documentCaretFromEncoding = vl::presentation::remoteprotocol::CharacterEncoding::UTF32;
#endif
                protocol->GetEvents()->OnControllerConnect(globalConfig);
            }
            auto theme = vl::Ptr(darkskin::Theme);
            vl::presentation::theme::RegisterTheme(theme);
            {
                gacuisrc_unittest::MainWindow window;
                window.MoveToScreenCenter();
                previousMainProxy(protocol, context);
                vl::presentation::controls::GetApplication()->Run(&window);
            }
            vl::presentation::theme::UnregisterTheme(theme->Name);
        });
        GacUIUnitTest_Start(WString::Unmanaged(L"Controls/Basic/GuiButton/ClickOnMouseUp"));
    });
});
```


## About Snapshots and Frames

This test case will create a bunch of file in**UnitTestFrameworkConfig::snapshotFolder**, under the path**Controls/Basic/GuiButton**. There will be**ClickOnMouseUp.json**and two**ClickOnMouseUp/frame_*.json**, with some other**ClickOnMouseUp.*.json**files. If you**git commit**them, after running a test case you can easily find out which test cases have been affected.

In this test case, the unit test framework does the following thing:
- Compile the resource and run**gacuisrc_unittest::MainWindow**as the main window, offering no arguments.
- Wait until the layout finishes.
- Save the snapshot of the current UI to**frame_0.json**.
- Click the button.
- Wait until the layout finishes.
- Save the snapshot of the current UI to**frame_1.json**.
- exit.

Each frame needs to make some visible UI changes, the unit test framework will detect and fail if no visible UI change has been made.

Each frame should not call blocking functions directly. For example, if a button displays a dialog, the**ShowDialog**or**ShowModal**or**ShowModalAndDelete**function only returns after the dialog is closed, such functions are blocking functions. In case when clicking a button displaying a dialog, you should use:
```c++
GetApplication()->InvokeInMainThread(window, [=]()
{
    protocol->LClick(location);
});
```
The next frame will starts right after the dialog is shown, and you can then interact with it.**ShowModalAsync**does not have such limitation.

So the title of the frame should describe what the UI looks like before the action, or just describe what the previous frame has done, as a snapshot is taken combining with the title name before executing the code inside a frame.

## Using Images

**UnitTestFrameworkConfig::resourceFolder**is the working directly when compiling the GacUI XML resource. If it needs to add other resource files, e.g. images, you can put them there.

The following code shows how to reference image files in the resource:
```xml
<Resource>
  <Folder name="UnitTestConfig" content="Link">ListViewImagesData.xml</Folder>
  <Folder name="ListViewImages" content="Link">ListViewImagesFolder.xml</Folder>
  ...
</Resource>
```
By saying this,**ListViewImagesData.xml**and**ListViewImagesFolder.xml**should be put under**UnitTestFrameworkConfig::resourceFolder**.

**ListViewImagesData.xml**
```xml
<Folder>
  <Xml name="ImageData">
    <ImageData>
      <Image Path="ListViewImages/LargeImages/Cert" Format="Png" Width="32" Height="32"/>
      ...
    </ImageData>
  </Xml>
</Folder>
```


**ListViewImagesFolder.xml**
```xml
<Folder>
  <Folder name="LargeImages">
    <Image name="Cert" content="File">Cert_32.png</Image>
    ...
  </Folder>
  ...
</Folder>
```


The unit test framework will not actually read the image file, but in order to perform UI layouting, it will instead read**UnitTestConfig/ImageData**to search for the metadata of the image. In this example,**res://ListViewImages/LargeImages/Cert**will match the record**Format="Png" Width="32" Height="32"**.

