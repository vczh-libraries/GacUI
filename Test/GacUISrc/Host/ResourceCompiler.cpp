#include "ResourceCompiler.h"
#include <Windows.h>

using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::stream;
using namespace vl::workflow::cppcodegen;

/***********************************************************************
DebugCallback
***********************************************************************/

void DebugCallback::OnLoadEnvironment()
{
#if defined VCZH_MSVC && defined _DEBUG
	OutputDebugString(L"    Workflow: Loading metadata from registered types ...\r\n");
#endif
}

void DebugCallback::OnInitialize(analyzer::WfLexicalScopeManager* manager)
{
#if defined VCZH_MSVC && defined _DEBUG
	OutputDebugString(L"    Workflow: Creating metadata from declarations ...\r\n");
#endif
}

void DebugCallback::OnValidateModule(Ptr<WfModule> module)
{
#if defined VCZH_MSVC && defined _DEBUG
	OutputDebugString((L"    Workflow: Validating module " + module->name.value + L" ...\r\n").Buffer());
#endif
}

void DebugCallback::OnGenerateMetadata()
{
#if defined VCZH_MSVC && defined _DEBUG
	OutputDebugString(L"    Workflow: Generating metadata ...\r\n");
#endif
}

void DebugCallback::OnGenerateCode(Ptr<WfModule> module)
{
#if defined VCZH_MSVC && defined _DEBUG
	OutputDebugString((L"    Workflow: Generating code for module " + module->name.value + L" ...\r\n").Buffer());
#endif
}

void DebugCallback::OnGenerateDebugInfo()
{
#if defined VCZH_MSVC && defined _DEBUG
	OutputDebugString(L"    Workflow: Generating debug information ...\r\n");
#endif
}

IWfCompilerCallback* DebugCallback::GetCompilerCallback()
{
	return this;
}

void DebugCallback::PrintPassName(vint passIndex)
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

void DebugCallback::OnPerPass(vint passIndex)
{
	PrintPassName(passIndex);
}

void DebugCallback::OnPerResource(vint passIndex, Ptr<GuiResourceItem> resource)
{
	PrintPassName(passIndex);
	OutputDebugString((L"    " + resource->GetResourcePath() + L"\r\n").Buffer());
}

/***********************************************************************
CompileResources
***********************************************************************/

void CompileResources(const WString& name, const WString& resourcePath, const WString& outputBinaryFolder, const WString& outputCppFolder, bool compressResource)
{
	FilePath errorPath = outputBinaryFolder + name + L".UI.error.txt";
	FilePath workflowPath = outputBinaryFolder + name + L".UI.txt";
	FilePath binaryPath = outputBinaryFolder + name + L".UI.bin";

	List<GuiResourceError> errors;
	auto resource = GuiResource::LoadFromXml(resourcePath, errors);
	DebugCallback debugCallback;
	File(errorPath).Delete();

	auto precompiledFolder = PrecompileAndWriteErrors(resource, &debugCallback, errors, errorPath);
	auto compiled = WriteWorkflowScript(precompiledFolder, workflowPath);
	CHECK_ERROR(errors.Count() == 0, L"Error");

	if (outputCppFolder != L"")
	{
		auto input = MakePtr<WfCppInput>(name);
		input->multiFile = WfCppFileSwitch::Enabled;
		input->reflection = WfCppFileSwitch::Enabled;
		input->comment = L"Source: Host.sln";
		input->normalIncludes.Add(L"../../../../Source/GacUI.h");
		input->normalIncludes.Add(L"../Helpers.h");
		input->reflectionIncludes.Add(L"../../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h");

		FilePath cppFolder = outputCppFolder;
		auto output = WriteCppCodesToFile(compiled, input, cppFolder);
		WriteEmbeddedResource(resource, input, output, compressResource, cppFolder / (name + L"Resource.cpp"));
	}

	WriteBinaryResource(resource, false, true, binaryPath);
	{
		FileStream fileStream(binaryPath.GetFullPath(), FileStream::ReadOnly);
		resource = GuiResource::LoadPrecompiledBinary(fileStream, errors);
		CHECK_ERROR(errors.Count() == 0, L"Error");
	}
	GetResourceManager()->SetResource(name, resource, GuiResourceUsage::InstanceClass);
}