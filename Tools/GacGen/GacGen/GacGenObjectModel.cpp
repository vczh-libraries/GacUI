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
CodegenConfig
***********************************************************************/

bool CodegenConfig::LoadConfigString(Ptr<GuiResource> resource, const WString& name, WString& value, bool optional)
{
	if (auto includeItem = resource->GetValueByPath(L"GacGenConfig/" + name).Cast<GuiTextData>())
	{
		value = includeItem->GetText();
		return true;
	}
	else if (!optional)
	{
		PrintErrorMessage(L"error> Cannot find configuration in resource \"GacGenConfig/" + name + L"\".");
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
	if (!LoadConfigString(resource, L"Include", config->include)) return false;
	if (!LoadConfigString(resource, L"Name", config->name)) return false;
	if (!LoadConfigString(resource, L"Prefix", config->prefix)) return false;
	LoadConfigString(resource, L"PrecompiledOutput", config->precompiledOutput, true);
	LoadConfigString(resource, L"PrecompiledBinary", config->precompiledBinary, true);
	LoadConfigString(resource, L"PrecompiledCompressed", config->precompiledCompressed, true);
	return config;
}

WString CodegenConfig::GetControlClassHeaderFileName(Ptr<Instance> instance)
{
	return prefix + instance->typeName + L".h";
}

WString CodegenConfig::GetControlClassCppFileName(Ptr<Instance> instance)
{
	return prefix + instance->typeName + L".cpp";
}

WString CodegenConfig::GetPartialClassHeaderFileName()
{
	return name + L"PartialClasses.h";
}

WString CodegenConfig::GetPartialClassCppFileName()
{
	return name + L"PartialClasses.cpp";
}

WString CodegenConfig::GetGlobalHeaderFileName()
{
	return name + L".h";
}