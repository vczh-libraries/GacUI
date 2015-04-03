#include "GacGen.h"

/***********************************************************************
Codegen::ControlClass
***********************************************************************/

bool TryReadFile(Ptr<CodegenConfig> config, const WString& fileName, List<WString>& lines)
{
	FileStream fileStream(config->resource->GetWorkingDirectory() + fileName, FileStream::ReadOnly);
	if (!fileStream.IsAvailable())
	{
		return false;
	}

	BomDecoder decoder;
	DecoderStream decoderStream(fileStream, decoder);
	StreamReader reader(decoderStream);

	while (!reader.IsEnd())
	{
		lines.Add(reader.ReadLine());
	}

	for (vint i = lines.Count() - 1; i >= 0; i--)
	{
		if (lines[i] != L"") break;
		lines.RemoveAt(i);
	}
	return true;
}

void WriteControlClassHeaderFile(Ptr<CodegenConfig> config, Ptr<Instance> instance)
{
	Regex regexCtor(L"^(<prefix>/s*)" + instance->typeName + L"/([^)]*/);");

	WString fileName = config->GetControlClassHeaderFileName(instance);
	List<WString> lines;
	if (TryReadFile(config, fileName, lines))
	{
		WString prefix;
		for (vint i = 0; i < instance->namespaces.Count(); i++)
		{
			prefix += L"\t";
		}
		vint begin = lines.IndexOf(GetEventHandlerCommentBegin(prefix + L"\t"));
		vint end = lines.IndexOf(GetEventHandlerCommentEnd(prefix + L"\t"));
		if (begin == -1 || end == -1)
		{
			CANNOT_MERGE_CONTENT;
			return;
		}
		
		OPEN_FILE(instance->typeName);
		PrintInformationMessage(L"gacgen> Merging content into " + fileName);

		for (vint i = 0; i < begin; i++)
		{
			writer.WriteLine(lines[i]);
		}
		WriteControlClassHeaderFileEventHandlers(config, instance, prefix, writer);
		for (vint i = end + 1; i < lines.Count(); i++)
		{
			if (auto match = regexCtor.MatchHead(lines[i]))
			{
				auto prefix = match->Groups()[L"prefix"][0].Value();
				WriteControlClassHeaderCtor(config, instance, prefix, writer);
			}
			else
			{
				writer.WriteLine(lines[i]);
			}
		}
	}
	else
	{
		OPEN_FILE_WITH_COMMENT(instance->typeName, false);

		writer.WriteLine(L"#ifndef VCZH_GACUI_RESOURCE_CODE_GENERATOR_" + config->name + L"_" + instance->typeName);
		writer.WriteLine(L"#define VCZH_GACUI_RESOURCE_CODE_GENERATOR_" + config->name + L"_" + instance->typeName);
		writer.WriteLine(L"");
		writer.WriteLine(L"#include \"" + config->GetPartialClassHeaderFileName() + L"\"");
		writer.WriteLine(L"");
		WriteControlClassHeaderFileContent(config, instance, writer);
		writer.WriteLine(L"#endif");
	}
}

void WriteControlClassCppFile(Ptr<CodegenConfig> config, Ptr<Instance> instance)
{
	Regex regexCtor(L"^(<prefix>/s*)" + instance->typeName + L"::" + instance->typeName + L"/([^)]*/)");
	Regex regexInit(L"^(<prefix>/s*)InitializeComponents/([^)]*/);");

	WString fileName = config->GetControlClassCppFileName(instance);
	List<WString> lines;
	if (TryReadFile(config, fileName, lines))
	{
		WString prefix;
		for (vint i = 0; i < instance->namespaces.Count(); i++)
		{
			prefix += L"\t";
		}
		vint begin = lines.IndexOf(GetEventHandlerCommentBegin(prefix));
		vint end = lines.IndexOf(GetEventHandlerCommentEnd(prefix));
		if (begin == -1 || end == -1)
		{
			CANNOT_MERGE_CONTENT;
			return;
		}

		Group<WString, WString> existingEventHandlers;
		List<WString> additionalLines;

		bool inCode = false;
		WString eventHandlerName;
		for (vint i = begin + 1; i < end; i++)
		{
			WString line = lines[i];
			if (!inCode)
			{
				if (eventHandlerName != L"")
				{
					inCode = true;
				}
				else
				{
					bool allSpaces = true;
					for (vint j = 0; j < line.Length(); j++)
					{
						wchar_t c = line[j];
						if (c != L' '&&c != L'\t')
						{
							allSpaces = false;
							break;
						}
					}

					if (!allSpaces)
					{
						FOREACH(WString, name, instance->eventHandlers.Keys())
						{
							if (line == GetEventHandlerHeader(prefix, instance, name, true))
							{
								eventHandlerName = name;
								break;
							}
						}

						if (eventHandlerName == L"")
						{
							inCode = true;
						}
					}
				}
			}

			if (inCode)
			{
				if (eventHandlerName == L"")
				{
					additionalLines.Add(line);
				}
				else
				{
					existingEventHandlers.Add(eventHandlerName, line);
				}

				if (line == prefix + L"}")
				{
					inCode = false;
					if (eventHandlerName == L"")
					{
						additionalLines.Add(L"");
					}
					else
					{
						eventHandlerName = L"";
					}
				}
			}
		}
		
		OPEN_FILE(instance->typeName);
		PrintInformationMessage(L"gacgen> Merging content into " + fileName);

		for (vint i = 0; i < begin; i++)
		{
			writer.WriteLine(lines[i]);
		}
		WriteControlClassCppFileEventHandlers(config, instance, prefix, existingEventHandlers, additionalLines, writer);
		for (vint i = end + 1; i < lines.Count(); i++)
		{
			if (auto match = regexCtor.MatchHead(lines[i]))
			{
				auto prefix = match->Groups()[L"prefix"][0].Value();
				WriteControlClassCppCtor(config, instance, prefix, writer);
			}
			else if (auto match = regexInit.MatchHead(lines[i]))
			{
				auto prefix = match->Groups()[L"prefix"][0].Value();
				WriteControlClassCppInit(config, instance, prefix, writer);
			}
			else
			{
				writer.WriteLine(lines[i]);
			}
		}
	}
	else
	{
		OPEN_FILE_WITH_COMMENT(instance->typeName, false);
		writer.WriteLine(L"#include \"" + config->GetGlobalHeaderFileName() + L"\"");
		writer.WriteLine(L"");
		WriteControlClassCppFileContent(config, instance, writer);
	}
}