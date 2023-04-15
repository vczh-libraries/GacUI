#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "../../../Source/GacUI.h"
#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#include "../../../Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUIReflection.h"
#include <Windows.h>

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::reflection::description;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	int result = SetupWindowsDirect2DRenderer();
#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

extern void GuiMain_GrammarIntellisense();

WString GetResourcePath()
{
#ifdef _WIN64
	return GetApplication()->GetExecutableFolder() + L"../../../Resources/";
#else
	return GetApplication()->GetExecutableFolder() + L"../../Resources/";
#endif
}

void GuiMain()
{
#ifdef VCZH_64
#define DARKSKIN_BINARY			L"../GacUISrc/Generated_DarkSkin/Resource_x64/DarkSkin.UI.bin"
#define FULLCONTROLTEST_BINARY	L"../GacUISrc/Generated_FullControlTest/Resource_x64/Demo.UI.bin"
#else
#define DARKSKIN_BINARY			L"../GacUISrc/Generated_DarkSkin/Resource_x86/DarkSkin.UI.bin"
#define FULLCONTROLTEST_BINARY	L"../GacUISrc/Generated_FullControlTest/Resource_x86/Demo.UI.bin"
#endif
	LoadGuiFakeDialogServiceUITypes();
	{
		FileStream fileStream(GetResourcePath() + DARKSKIN_BINARY, FileStream::ReadOnly);
		GetResourceManager()->LoadResourceOrPending(fileStream, GuiResourceUsage::InstanceClass);
	}
	{
		FileStream fileStream(GetResourcePath() + FULLCONTROLTEST_BINARY, FileStream::ReadOnly);
		GetResourceManager()->LoadResourceOrPending(fileStream, GuiResourceUsage::InstanceClass);
	}
	{
		auto theme = UnboxValue<Ptr<ThemeTemplates>>(Value::Create(L"darkskin::Theme"));
		RegisterTheme(theme);
	}
	{
		auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::MainWindow"));
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		GetApplication()->Run(window);
		delete window;
	}
}