#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "ResourceCompiler.h"
#include <Windows.h>

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::reflection::description;

//#define GUI_GRAPHICS_RENDERER_GDI
#define GUI_GRAPHICS_RENDERER_DIRECT2D

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
#ifdef GUI_GRAPHICS_RENDERER_GDI
	int result=SetupWindowsGDIRenderer();
#endif
#ifdef GUI_GRAPHICS_RENDERER_DIRECT2D
	int result = SetupWindowsDirect2DRenderer();
#endif

#if VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}

extern void UnitTestInGuiMain();

namespace autocomplete_grammar
{
	extern void InstallTextBox(GuiMultilineTextBox* textBox);
}

void GuiMain_GrammarIntellisense()
{
	GuiWindow window(theme::ThemeName::Window);
	window.GetContainerComposition()->SetPreferredMinSize(Size(600, 480));
	{
		auto textBox = new GuiMultilineTextBox(theme::ThemeName::MultilineTextBox);
		autocomplete_grammar::InstallTextBox(textBox);
		textBox->GetBoundsComposition()->SetAlignmentToParent(Margin(5, 5, 5, 5));
		window.AddChild(textBox);
	}
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
	GetApplication()->Run(&window);
}

#ifndef VCZH_DEBUG_NO_REFLECTION

void OpenMainWindow()
{
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

#endif

void GuiMain()
{
	UnitTestInGuiMain();
#ifndef VCZH_DEBUG_NO_REFLECTION
	{
		FileStream fileStream(L"Reflection.txt", FileStream::WriteOnly);
		BomEncoder encoder(BomEncoder::Utf16);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		LogTypeManager(writer);
	}

#define BINARY_FOLDER L"../TestCppCodegen/"
#define SOURCE_FOLDER L"../TestCppCodegen/Source/"
#define COMPILE_DARKSKIN
//#define COMPILE_DEMO

	List<WString> dependencies;
#ifdef COMPILE_DARKSKIN
	LoadResource(CompileResources(L"DarkSkin",	dependencies,	LR"(Resources/DarkSkin/Resource.xml)",			BINARY_FOLDER, SOURCE_FOLDER, true));
#else
	LoadResource(L"../TestCppCodegen/DarkSkin.UI.bin");
#endif

#ifdef COMPILE_DEMO
	LoadResource(CompileResources(L"Demo",		dependencies,	LR"(Resources/FullControlTest/Resource.xml)",	BINARY_FOLDER, SOURCE_FOLDER, false));
#else
	LoadResource(L"../TestCppCodegen/Demo.UI.bin");
#endif

#undef BINARY_FOLDER
#undef SOURCE_FOLDER

	OpenMainWindow();
#endif
}