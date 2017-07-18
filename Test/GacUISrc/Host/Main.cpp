#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "..\..\..\Source\GacUI.h"
#ifndef VCZH_DEBUG_NO_REFLECTION
#include "..\..\..\Source\Compiler\GuiInstanceLoader.h"
#include "..\..\..\Source\Reflection\GuiInstanceCompiledWorkflow.h"
#include "..\..\..\Source\Compiler\WorkflowCodegen\GuiInstanceLoader_WorkflowCodegen.h"
#endif
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
using namespace vl::workflow;
using namespace vl::workflow::cppcodegen;

#define GUI_GRAPHICS_RENDERER_GDI
//#define GUI_GRAPHICS_RENDERER_DIRECT2D

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

int CALLBACK EnumFontFamExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam)
{
	auto fontFamilies = (SortedList<WString>*)lParam;
	if (lpelfe->lfFaceName[0] != L'@'&&!fontFamilies->Contains(lpelfe->lfFaceName))
	{
		fontFamilies->Add(lpelfe->lfFaceName);
	}
	return -1;
}

void EnumerateFontFamilies(const Func<void(const WString&)>& callback)
{
	HDC dc = GetDC(NULL);
	LOGFONT logFont;
	ZeroMemory(&logFont, sizeof(logFont));
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfFaceName[0] = L'\0';

	SortedList<WString> fontFamilies;
	EnumFontFamiliesEx(dc, &logFont, &EnumFontFamExProc, (LPARAM)&fontFamilies, 0);

	FOREACH(WString, fontFamily, fontFamilies)
	{
		callback(fontFamily);
	}
	ReleaseDC(NULL, dc);
}

#define BINARY_FOLDER L"../TestCppCodegen/"
#define SOURCE_FOLDER L"../TestCppCodegen/Source/"

class DebugCallback : public Object, public IGuiResourcePrecompileCallback, public IWfCompilerCallback
{
public:
	vint lastPassIndex = -1;

	void OnLoadEnvironment()override
	{
#if defined VCZH_MSVC && defined _DEBUG
		OutputDebugString(L"    Workflow: Loading metadata from registered types ...\r\n");
#endif
	}

	void OnInitialize(analyzer::WfLexicalScopeManager* manager)override
	{
#if defined VCZH_MSVC && defined _DEBUG
		OutputDebugString(L"    Workflow: Creating metadata from declarations ...\r\n");
#endif
	}

	void OnValidateModule(Ptr<WfModule> module)override
	{
#if defined VCZH_MSVC && defined _DEBUG
		OutputDebugString((L"    Workflow: Validating module " + module->name.value + L" ...\r\n").Buffer());
#endif
	}

	void OnGenerateMetadata()override
	{
#if defined VCZH_MSVC && defined _DEBUG
		OutputDebugString(L"    Workflow: Generating metadata ...\r\n");
#endif
	}

	void OnGenerateCode(Ptr<WfModule> module)override
	{
#if defined VCZH_MSVC && defined _DEBUG
		OutputDebugString((L"    Workflow: Generating code for module " + module->name.value + L" ...\r\n").Buffer());
#endif
	}

	void OnGenerateDebugInfo()override
	{
#if defined VCZH_MSVC && defined _DEBUG
		OutputDebugString(L"    Workflow: Generating debug information ...\r\n");
#endif
	}

	IWfCompilerCallback* GetCompilerCallback()override
	{
		return this;
	}

	void PrintPassName(vint passIndex)
	{
		if (lastPassIndex != passIndex)
		{
			lastPassIndex = passIndex;
#if defined VCZH_MSVC && defined _DEBUG

#define PRINT_PASS(PASS)\
			case IGuiResourceTypeResolver_Precompile::PASS:\
				OutputDebugString((itow(passIndex + 1) + L"/" + itow(IGuiResourceTypeResolver_Precompile::Instance_Max + 1) + L": " L ## #PASS L"\r\n").Buffer());\
				break;\

			switch (passIndex)
			{
			PRINT_PASS(Workflow_Collect)
			PRINT_PASS(Workflow_Compile)
			PRINT_PASS(Instance_CollectInstanceTypes)
			PRINT_PASS(Instance_CompileInstanceTypes)
			PRINT_PASS(Instance_CollectEventHandlers)
			PRINT_PASS(Instance_CompileEventHandlers)
			PRINT_PASS(Instance_GenerateInstanceClass)
			PRINT_PASS(Instance_CompileInstanceClass)
			}
#endif
		}
	}

	void OnPerPass(vint passIndex)override
	{
		PrintPassName(passIndex);
	}

	void OnPerResource(vint passIndex, Ptr<GuiResourceItem> resource)override
	{
		PrintPassName(passIndex);
		OutputDebugString((L"    " + resource->GetResourcePath() + L"\r\n").Buffer());
	}
};

void GuiMain_Resource()
{
#ifndef VCZH_DEBUG_NO_REFLECTION
	{
		FileStream fileStream(L"Reflection.txt", FileStream::WriteOnly);
		BomEncoder encoder(BomEncoder::Utf16);
		EncoderStream encoderStream(fileStream, encoder);
		StreamWriter writer(encoderStream);
		LogTypeManager(writer);
	}
	{
		List<GuiResourceError> errors;
		auto resource = GuiResource::LoadFromXml(LR"(Resources/FullControlTest/Resource.xml)", errors);
		DebugCallback debugCallback;
		auto precompiledFolder = resource->Precompile(&debugCallback, errors);
		auto compiled = precompiledFolder ? precompiledFolder->GetValueByPath(L"Workflow/InstanceClass").Cast<GuiInstanceCompiledWorkflow>() : nullptr;

		{
			List<WString> output;
			GuiResourceError::SortAndLog(errors, output);
			File(BINARY_FOLDER L"UI.error.txt").WriteAllLines(output, true, BomEncoder::Utf16);
		}
		if (compiled)
		{
			WString text;
			if (compiled->assembly)
			{
				auto& codes = compiled->assembly->insAfterCodegen->moduleCodes;
				FOREACH_INDEXER(WString, code, codeIndex, codes)
				{
					text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(codes.Count()) + L")================================\r\n";
					text += code + L"\r\n";
				}
			}
			else
			{
				FOREACH_INDEXER(GuiInstanceCompiledWorkflow::ModuleRecord, moduleRecord, codeIndex, compiled->modules)
				{
					WString code;
					{
						MemoryStream stream;
						{
							StreamWriter writer(stream);
							WfPrint(moduleRecord.module, L"", writer);
						}
						stream.SeekFromBegin(0);
						{
							StreamReader reader(stream);
							code = reader.ReadToEnd();
						}
					}
					text += L"================================(" + itow(codeIndex + 1) + L"/" + itow(compiled->modules.Count()) + L")================================\r\n";
					text += code + L"\r\n";
				}
			}
			File(BINARY_FOLDER L"UI.txt").WriteAllText(text);
		}
		CHECK_ERROR(errors.Count() == 0, L"Error");

		{
			auto input = MakePtr<WfCppInput>(L"Demo");
			input->comment = L"Source: Host.sln";
			input->normalIncludes.Add(L"../../../../Source/GacUI.h");
			input->normalIncludes.Add(L"../Helpers.h");
			input->reflectionIncludes.Add(L"../../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h");
			auto output = GenerateCppFiles(input, compiled->metadata.Obj());
			FOREACH_INDEXER(WString, fileName, index, output->cppFiles.Keys())
			{
				WString code = output->cppFiles.Values()[index];
				File file(L"../TestCppCodegen/Source/" + fileName);

				if (file.Exists())
				{
					WString inputText;
					BomEncoder::Encoding inputEncoding;
					bool inputBom;
					file.ReadAllTextWithEncodingTesting(inputText, inputEncoding, inputBom);
					code = MergeCppFileContent(inputText, code);
				}

				if (file.Exists())
				{
					WString inputText;
					BomEncoder::Encoding inputEncoding;
					bool inputBom;
					file.ReadAllTextWithEncodingTesting(inputText, inputEncoding, inputBom);
					if (inputText == code)
					{
						continue;
					}
				}
				file.WriteAllText(code, true, BomEncoder::Utf8);
			}
		}

		{
			FileStream fileStream(BINARY_FOLDER L"UI.bin", FileStream::WriteOnly);
			resource->SavePrecompiledBinary(fileStream);
		}
		{
			FileStream fileStream(BINARY_FOLDER L"UI.bin", FileStream::ReadOnly);
			resource = GuiResource::LoadPrecompiledBinary(fileStream, errors);
			CHECK_ERROR(errors.Count() == 0, L"Error");
		}
		GetResourceManager()->SetResource(L"Resource", resource, GuiResourceUsage::InstanceClass);
	}

	// FullControlTest / UI1 / UI3 / UI4 / UI5 / UI7 / UI8 / UI9
	auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::MainWindow"));

	// UI2
	// auto viewModel = Value::InvokeStatic(L"ViewModelBuilder", L"Build");
	// auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::MainWindow", (Value_xs(), viewModel)));

	// UI6
	// auto window = UnboxValue<GuiWindow*>(Value::Create(L"demo::MainWindow"));
	// auto fontList = UnboxValue<GuiTextList*>(Value::From(window).GetProperty(L"fontList"));
	// EnumerateFontFamilies([fontList](const WString& fontFamily)
	// {
	// 	fontList->GetItems().Add(new list::TextItem(fontFamily));
	// });

	window->ForceCalculateSizeImmediately();
	window->MoveToScreenCenter();
	GetApplication()->Run(window);
	delete window;
#endif
}

void GuiMain()
{
	// win7::Win7Theme theme;
	// SetCurrentTheme(&theme);
	UnitTestInGuiMain();
	GuiMain_Resource();
}