#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "../Host/ResourceCompiler.h"
#include "../TestCppCodegen/Source/DarkSkinReflection.h"
#include "../TestCppCodegen/Source/DemoReflection.h"
#include "../../../Source/NativeWindow/Windows/WinNativeWindow.h"
#include "resource.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::reflection::description;

//#define GUI_GRAPHICS_RENDERER_GDI
#define GUI_GRAPHICS_RENDERER_DIRECT2D

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
#ifdef GUI_GRAPHICS_RENDERER_GDI
	int result = SetupWindowsGDIRenderer();
#endif
#ifdef GUI_GRAPHICS_RENDERER_DIRECT2D
	windows::SetWindowDefaultIcon(MAINICON);
	int result = SetupWindowsDirect2DRenderer();
#endif

#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

#ifndef VCZH_DEBUG_NO_REFLECTION

void AttachDumpCompositionMenu(GuiWindow* window)
{
	auto menu = new GuiToolstripMenu(theme::ThemeName::Menu, window);
	window->AddControlHostComponent(menu);
	{
		auto menuItem = new GuiToolstripButton(theme::ThemeName::MenuItemButton);
		menuItem->SetText(L"Dump Composition to " + FilePath(L"TestXml.xml").GetFullPath());
		menu->GetToolstripItems().Add(menuItem);

		window->GetBoundsComposition()->GetEventReceiver()->rightButtonUp.AttachLambda([=](auto, auto arguments)
		{
			menu->ShowPopup(window, Point(arguments.x, arguments.y));
		});

		menuItem->Clicked.AttachLambda([=](auto, auto)
		{
			FileStream fileStream(L"TestXml.xml", FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf8);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);

			GuiGraphicsComposition* composition = window->GetBoundsComposition();
			while (composition->GetParent())
			{
				composition = composition->GetParent();
			}

			DumpComposition(composition, writer);
			GetCurrentController()->DialogService()->ShowMessageBox(window->GetNativeWindow(), L"Finished!", L"Dump Composition");
		});
	}
	{
		auto menuItem = new GuiToolstripButton(theme::ThemeName::MenuItemButton);
		menuItem->SetText(L"Open New Window");
		menu->GetToolstripItems().Add(menuItem);

		menuItem->Clicked.AttachLambda([=](auto, auto)
		{
			auto newWindow = new GuiWindow(theme::ThemeName::Window);
			newWindow->SetText(L"New Window");
			newWindow->SetClientSize(Size(640, 480));
			newWindow->ForceCalculateSizeImmediately();
			newWindow->MoveToScreenCenter();
			newWindow->ShowModalAndDelete(window, []() {});
		});
	}
}

void OpenMainWindow()
{
	{
		auto theme = MakePtr<darkskin::Theme>();
		RegisterTheme(theme);
	}
	{
		auto theme = UnboxValue<Ptr<ThemeTemplates>>(Value::Create(L"demo::TestTheme"));
		RegisterTheme(theme);
	}
	{
		auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::TestWindow"));
		AttachDumpCompositionMenu(window);
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		GetApplication()->Run(window);
		delete window;
	}
}

#endif

void GuiMain()
{
#ifndef VCZH_DEBUG_NO_REFLECTION
	LoadDarkSkinTypes();
	LoadDemoTypes();

	List<WString> names;
	names.Add(L"Resource");

	Group<WString, WString> deps;

	Dictionary<WString, FilePath> paths;
	FOREACH(WString, name, names)
	{
		List<WString> dependencies;
		vint index = deps.Keys().IndexOf(name);
		if (index != -1)
		{
			CopyFrom(dependencies, deps.GetByIndex(index));
		}
		paths.Add(name, CompileResources(name, dependencies, L"Resources/" + name + L".xml", L"./", L"", false));
		LoadResource(paths[name]);
	}

	FOREACH(WString, name, names)
	{
		GetResourceManager()->UnloadResource(name);
	}
	FOREACH(WString, name, From(names).Reverse())
	{
		LoadResource(paths[name]);
	}

	names.Clear();
	GetResourceManager()->GetPendingResourceNames(names);
	CHECK_ERROR(names.Count() == 0, L"Error");

	OpenMainWindow();
#endif
}