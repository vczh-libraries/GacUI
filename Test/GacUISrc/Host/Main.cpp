#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GAC_HEADER_USE_NAMESPACE
#include "..\..\..\Source\GacUI.h"
#include "..\..\..\Source\Compiler\GuiCppGen.h"
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

#ifndef VCZH_DEBUG_NO_REFLECTION
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
#endif

#define BINARY_FOLDER L"../TestCppCodegen/"
#define SOURCE_FOLDER L"../TestCppCodegen/Source/"

#ifndef VCZH_DEBUG_NO_REFLECTION

void CompileResources(const WString& name, const WString& resourcePath, const WString& outputPath, bool compressResource)
{
	{
		FilePath errorPath =	BINARY_FOLDER + name + L".UI.error.txt";
		FilePath workflowPath =	BINARY_FOLDER + name + L".UI.txt";
		FilePath binaryPath =	BINARY_FOLDER + name + L".UI.bin";
		FilePath cppFolder =	outputPath;

		List<GuiResourceError> errors;
		auto resource = GuiResource::LoadFromXml(resourcePath, errors);
		DebugCallback debugCallback;
		File(errorPath).Delete();

		auto precompiledFolder = PrecompileAndWriteErrors(resource, &debugCallback, errors, errorPath);
		auto compiled = WriteWorkflowScript(precompiledFolder, workflowPath);
		CHECK_ERROR(errors.Count() == 0, L"Error");

		auto input = MakePtr<WfCppInput>(name);
		input->multiFile = WfCppFileSwitch::Enabled;
		input->reflection = WfCppFileSwitch::Enabled;
		input->comment = L"Source: Host.sln";
		input->normalIncludes.Add(L"../../../../Source/GacUI.h");
		input->normalIncludes.Add(L"../Helpers.h");
		input->reflectionIncludes.Add(L"../../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h");
		auto output = WriteCppCodesToFile(compiled, input, cppFolder);
		WriteEmbeddedResource(resource, input, output, compressResource, cppFolder / (name + L"Resource.cpp"));

		WriteBinaryResource(resource, false, true, binaryPath);
		{
			FileStream fileStream(binaryPath.GetFullPath(), FileStream::ReadOnly);
			resource = GuiResource::LoadPrecompiledBinary(fileStream, errors);
			CHECK_ERROR(errors.Count() == 0, L"Error");
		}
		GetResourceManager()->SetResource(name, resource, GuiResourceUsage::InstanceClass);
	}
}

void OpenMainWindow()
{
	{
		auto theme = UnboxValue<Ptr<ThemeTemplates>>(Value::Create(L"darkskin::Theme"));
		RegisterTheme(L"DarkSkin", theme);
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

	CompileResources(L"DarkSkin",	LR"(Resources/DarkSkin/Resource.xml)",			L"../TestCppCodegen/Source/", true);
	CompileResources(L"Demo",		LR"(Resources/FullControlTest/Resource.xml)",	L"../TestCppCodegen/Source/", false);
	OpenMainWindow();
#endif
}