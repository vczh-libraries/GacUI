#include "GacGen.h"

void PrintErrorMessage(const WString& message)
{
	Console::SetColor(true, false, false, true);
	Console::WriteLine(message);
	Console::SetColor(true, true, true, false);
}

void PrintSuccessMessage(const WString& message)
{
	Console::SetColor(false, true, false, true);
	Console::WriteLine(message);
	Console::SetColor(true, true, true, false);
}

void PrintInformationMessage(const WString& message)
{
	Console::SetColor(true, true, false, true);
	Console::WriteLine(message);
	Console::SetColor(true, true, true, false);
}

/***********************************************************************
CodegenConfig
***********************************************************************/

WString CodegenConfig::NormalizeFolder(const WString& folder)
{
	if (folder.Length() == 0)
	{
		return L".\\";
	}
	else if (folder[folder.Length() - 1] != L'\\')
	{
		return folder + L"\\";
	}
	else
	{
		return folder;
	}
}

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

bool CodegenConfig::LoadConfigString(Ptr<GuiResourceFolder> folder, const WString& path, List<WString>& value, bool optional)
{
	WString text;
	if (LoadConfigString(folder, path, text, optional))
	{
		SplitBySemicolon(text, value);
		return true;
	}
	return false;
}

Ptr<CodegenConfig> CodegenConfig::LoadConfig(Ptr<GuiResource> resource, GuiResourceError::List& errors)
{
	Ptr<CodegenConfig> config = new CodegenConfig;
	config->resource = resource;
	config->metadata = resource->GetMetadata();

	if (auto folder = resource->GetFolderByPath(L"GacGenConfig/Cpp/"))
	{
		auto out = MakePtr<CodegenConfig::CppOutput>();
		LoadConfigString(folder, L"Resource", out->resource);
		LoadConfigString(folder, L"Compressed", out->compressed);
		LoadConfigString(folder, L"SourceFolder", out->sourceFolder, false);
		LoadConfigString(folder, L"NormalInclude", out->normalIncludes, false);
		LoadConfigString(folder, L"ReflectionInclude", out->reflectionIncludes);
		LoadConfigString(folder, L"Name", out->name);
		LoadConfigString(folder, L"CppResource", out->cppResource);
		LoadConfigString(folder, L"CppCompressed", out->cppCompressed);

		NormalizeFolder(out->sourceFolder);
		if (out->name == L"")
		{
			out->name = L"GacUIApplication";
		}
		config->cppOutput = out;
	}

	if (auto folder = resource->GetFolderByPath(L"GacGenConfig/ResX86/"))
	{
		auto out = MakePtr<CodegenConfig::ResOutput>();
		LoadConfigString(folder, L"Resource", out->resource);
		LoadConfigString(folder, L"Compressed", out->compressed);
		LoadConfigString(folder, L"Assembly", out->assembly);

		config->resOutputx32 = out;
	}

	if (auto folder = resource->GetFolderByPath(L"GacGenConfig/ResX64/"))
	{
		auto out = MakePtr<CodegenConfig::ResOutput>();
		LoadConfigString(folder, L"Resource", out->resource);
		LoadConfigString(folder, L"Compressed", out->compressed);
		LoadConfigString(folder, L"Assembly", out->assembly);

		config->resOutputx64 = out;
	}

	if (auto item = resource->GetValueByPath(L"GacGenConfig/Metadata"))
	{
		if (auto xml = item.Cast<XmlDocument>())
		{
			resource->GetMetadata()->LoadFromXml(xml, { resource }, errors);
		}
	}

	return config;
}