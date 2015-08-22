#include "GacGen.h"

/***********************************************************************
Instance
***********************************************************************/

WString Instance::GetFullName()
{
	return From(namespaces)
		.Aggregate(WString(), [](const WString& a, const WString& b)->WString
		{
			return a + b + L"::";
		})
		+ typeName;
}

/***********************************************************************
InstanceSchema
***********************************************************************/

WString InstanceSchema::GetFullName()
{
	return From(namespaces)
		.Aggregate(WString(), [](const WString& a, const WString& b)->WString
		{
			return a + b + L"::";
		})
		+ typeName;
}

/***********************************************************************
CodegenConfig::Output
***********************************************************************/

void CodegenConfig::Output::Initialize()
{
	if (output.Length() == 0)
	{
		output = L".\\";
	}
	else if (output[output.Length() - 1] != L'\\')
	{
		output += L"\\";
	}
}

/***********************************************************************
CodegenConfig::CppOutput
***********************************************************************/

WString CodegenConfig::CppOutput::GetControlClassHeaderFileName(Ptr<Instance> instance)
{
	return prefix + instance->typeName + L".h";
}

WString CodegenConfig::CppOutput::GetControlClassCppFileName(Ptr<Instance> instance)
{
	return prefix + instance->typeName + L".cpp";
}

WString CodegenConfig::CppOutput::GetPartialClassHeaderFileName()
{
	return name + L"PartialClasses.h";
}

WString CodegenConfig::CppOutput::GetPartialClassCppFileName()
{
	return name + L"PartialClasses.cpp";
}

WString CodegenConfig::CppOutput::GetGlobalHeaderFileName()
{
	return name + L".h";
}

/***********************************************************************
CodegenConfig
***********************************************************************/

bool CodegenConfig::LoadConfigString(Ptr<GuiResourceFolder> folder, const WString& path, WString& value, bool optional)
{
	if (auto includeItem = folder->GetValueByPath(path).Cast<GuiTextData>())
	{
		value = includeItem->GetText();
		return true;
	}
	else if (!optional)
	{
		PrintErrorMessage(L"error> Cannot find configuration in resource \"" + folder->GetName() + L"/" + path + L"\".");
		return false;
	}
	else
	{
		return true;
	}
}

Ptr<CodegenConfig> CodegenConfig::LoadConfig(Ptr<GuiResource> resource)
{
	Ptr<CodegenConfig> config = new CodegenConfig;
	config->resource = resource;

	if (auto folder = resource->GetFolderByPath(L"GacGenConfig/Cpp/"))
	{
		auto out = MakePtr<CodegenConfig::CppOutput>();
		if (!LoadConfigString(folder, L"Output", out->output)) return nullptr;
		if (!LoadConfigString(folder, L"Include", out->include)) return nullptr;
		if (!LoadConfigString(folder, L"Name", out->name)) return nullptr;
		if (!LoadConfigString(folder, L"Prefix", out->prefix)) return nullptr;

		out->Initialize();
		config->cppOutput = out;
	}

	if (auto folder = resource->GetFolderByPath(L"GacGenConfig/Res/"))
	{
		auto out = MakePtr<CodegenConfig::ResOutput>();
		if (!LoadConfigString(folder, L"Output", out->output)) return nullptr;
		LoadConfigString(folder, L"PrecompiledOutput", out->precompiledOutput, true);
		LoadConfigString(folder, L"PrecompiledBinary", out->precompiledBinary, true);
		LoadConfigString(folder, L"PrecompiledCompressed", out->precompiledCompressed, true);

		out->Initialize();
		config->resOutput = out;
	}

	return config;
}