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

Ptr<CodegenConfig> CodegenConfig::LoadConfig(Ptr<GuiResource> resource)
{
	Ptr<CodegenConfig> config = new CodegenConfig;
	config->resource = resource;

	if (auto folder = resource->GetFolderByPath(L"GacGenConfig/Cpp/"))
	{
		auto out = MakePtr<CodegenConfig::CppOutput>();
		LoadConfigString(folder, L"Resource", out->resource, true);
		LoadConfigString(folder, L"Compressed", out->compressed, true);
		LoadConfigString(folder, L"SourceFolder", out->sourceFolder, true);
		LoadConfigString(folder, L"NormalInclude", out->normalInclude);
		LoadConfigString(folder, L"ReflectionInclude", out->reflectionInclude);
		LoadConfigString(folder, L"Name", out->name);

		NormalizeFolder(out->sourceFolder);
		if (out->name == L"")
		{
			out->name = L"GacUIApplication";
		}
		config->cppOutput = out;
	}

	if (auto folder = resource->GetFolderByPath(L"GacGenConfig/Res/"))
	{
		auto out = MakePtr<CodegenConfig::ResOutput>();
		LoadConfigString(folder, L"Resource", out->resource, true);
		LoadConfigString(folder, L"Compressed", out->compressed, true);

		config->resOutput = out;
	}

	return config;
}