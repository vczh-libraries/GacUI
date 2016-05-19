#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "..\..\..\Source\GacUI.h"
#include "..\..\..\Source\Compiler\GuiInstanceLoader.h"
#include "..\..\..\Source\Reflection\TypeDescriptors\GuiReflectionEvents.h"
#include "..\..\..\Source\Reflection\GuiInstancePartialClass.h"
#include <Windows.h>

using namespace vl::regex;
using namespace vl::regex_internal;
using namespace vl::parsing;
using namespace vl::parsing::definitions;
using namespace vl::parsing::tabling;
using namespace vl::parsing::xml;
using namespace vl::stream;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::collections;
using namespace vl::filesystem;

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

	ThreadLocalStorage::DisposeStorages();
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
	GuiWindow window(GetCurrentTheme()->CreateWindowStyle());
	window.GetContainerComposition()->SetPreferredMinSize(Size(600, 480));
	{
		auto textBox = g::NewMultilineTextBox();
		autocomplete_grammar::InstallTextBox(textBox);
		textBox->GetBoundsComposition()->SetAlignmentToParent(Margin(5, 5, 5, 5));
		window.AddChild(textBox);
	}
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
	GetApplication()->Run(&window);
}

void GuiMain_Resource()
{
	//{
	//	FileStream fileStream(L"Reflection.txt", FileStream::WriteOnly);
	//	BomEncoder encoder(BomEncoder::Utf16);
	//	EncoderStream encoderStream(fileStream, encoder);
	//	StreamWriter writer(encoderStream);
	//	LogTypeManager(writer);
	//}
	{
		List<WString> errors;
		auto resource = GuiResource::LoadFromXml(L"UI5.xml", errors);
		resource->Precompile(nullptr, errors);

		{
			FileStream fileStream(L"UI.error.txt", FileStream::WriteOnly);
			BomEncoder encoder(BomEncoder::Utf16);
			EncoderStream encoderStream(fileStream, encoder);
			StreamWriter writer(encoderStream);

			FOREACH(WString, error, errors)
			{
				writer.WriteLine(error);
			}
		}
		CHECK_ERROR(errors.Count() == 0, L"Error");

		{
			FileStream fileStream(L"UI.bin", FileStream::WriteOnly);
			resource->SavePrecompiledBinary(fileStream);
		}
		{
			FileStream fileStream(L"UI.bin", FileStream::ReadOnly);
			resource = GuiResource::LoadPrecompiledBinary(fileStream, errors);
		}
		GetResourceManager()->SetResource(L"Resource", resource, GuiResourceUsage::DevelopmentTool);

		{
			auto item = resource->GetValueByPath(L"Precompiled/Workflow/InstanceClass");
			auto compiled = item.Cast<GuiInstanceCompiledWorkflow>();

			WString text;
			auto& codes = compiled->assembly->insAfterCodegen->moduleCodes;
			FOREACH_INDEXER(WString, code, codeIndex, compiled->assembly->insAfterCodegen->moduleCodes)
			{
				text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(codes.Count()) + L")================================\r\n";
				text += code + L"\r\n";
			}
			File(L"UI.txt").WriteAllText(text);
		}
	}

	// UI1.xml / UI3.xml / UI4.xml / UI5.xml
	auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::MainWindow"));

	// UI2.xml
	// auto viewModel = Value::InvokeStatic(L"ViewModelBuilder", L"Build");
	// auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::MainWindow", (Value_xs(), viewModel)));

	window->ForceCalculateSizeImmediately();
	window->MoveToScreenCenter();
	GetApplication()->Run(window);
	delete window;
}

void GuiMain()
{
	UnitTestInGuiMain();
	GuiMain_Resource();
}