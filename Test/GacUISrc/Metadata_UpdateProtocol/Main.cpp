#define GAC_HEADER_USE_NAMESPACE

#include "../../../Source/Compiler/RemoteProtocol/GuiRemoteProtocolCompiler.h"
#ifdef VCZH_MSVC
#include <Windows.h>
#endif

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

#if defined VCZH_MSVC
WString GetExePath()
{
	wchar_t buffer[65536];
	GetModuleFileName(NULL, buffer, sizeof(buffer) / sizeof(*buffer));
	vint pos = -1;
	vint index = 0;
	while (buffer[index])
	{
		if (buffer[index] == L'\\' || buffer[index] == L'/')
		{
			pos = index;
		}
		index++;
	}
	return WString::CopyFrom(buffer, pos + 1);
}

WString GetRemoteProtocolPath()
{
#ifdef _WIN64
	return GetExePath() + L"../../../../Source/PlatformProviders/Remote/Protocol/";
#else
	return GetExePath() + L"../../../Source/PlatformProviders/Remote/Protocol/";
#endif
}
#elif defined VCZH_GCC
WString GetRemoteProtocolPath()
{
	return L"../../../Source/PlatformProviders/Remote/Protocol/";
}
#endif

void WriteFileIfChanged(FilePath filePath, WString code)
{
	File file = filePath;
	if (file.Exists())
	{
		auto originalCode = file.ReadAllTextByBom();
		if (originalCode == code)
		{
			return;
		}
	}
	file.WriteAllText(code, false, BomEncoder::Utf8);
}

#if defined VCZH_MSVC
int wmain(int argc, wchar_t* argv[])
#elif defined VCZH_GCC
int main(int argc, char* argv[])
#endif
{
	remoteprotocol::Parser parser;
	List<WString> schemaNames;
	Dictionary<WString, Ptr<GuiRpSchema>> schemas;

	File(FilePath(GetRemoteProtocolPath()) / L"Protocols.txt").ReadAllLinesByBom(schemaNames);
	for (auto [schemaName, index] : indexed(schemaNames))
	{
		WString code = File(FilePath(GetRemoteProtocolPath()) / (schemaName + L".txt")).ReadAllTextByBom();
		auto schema = parser.ParseSchema(code, index);
		schemas.Add(schemaName, schema);
	}

	auto mergedSchema = Ptr(new GuiRpSchema);
	for (auto schemaName : schemaNames)
	{
		CopyFrom(mergedSchema->declarations, schemas[schemaName]->declarations, true);
	}

	List<GuiRpError> errors;
	auto symbols = CheckRemoteProtocolSchema(mergedSchema, errors);
	CHECK_ERROR(errors.Count() == 0, L"Error detected in schema.");

	WriteFileIfChanged(
		FilePath(GetRemoteProtocolPath()) / L"Metadata" / L"Protocols.json",
		GenerateToStream([&](StreamWriter& writer)
		{
			json_visitor::AstVisitor visitor(writer);
			visitor.printTokenCodeRange = false;
			visitor.printAstCodeRange = false;
			visitor.Print(mergedSchema.Obj());
		}));

	GuiRpCppConfig config;
	config.headerFileName = L"GuiRemoteProtocolSchema.h";
	config.headerGuard = L"VCZH_PRESENTATION_GUIREMOTECONTROLLER_REMOTEPROTOCOLSCHEMA";
	config.headerInclude = L"../../GuiRemoteProtocolSchemaShared.h";
	config.cppNamespace = L"vl::presentation::remoteprotocol";
	config.builderMacroPrefix = L"GACUI_REMOTEPROTOCOL";

	WriteFileIfChanged(
		FilePath(GetRemoteProtocolPath()) / L"Generated" / L"GuiRemoteProtocolSchema.h",
		GenerateToStream([&](StreamWriter& writer)
		{
			GenerateRemoteProtocolHeaderFile(mergedSchema, symbols, config, writer);
		}));

	WriteFileIfChanged(
		FilePath(GetRemoteProtocolPath()) / L"Generated" / L"GuiRemoteProtocolSchema.cpp",
		GenerateToStream([&](StreamWriter& writer)
		{
			GenerateRemoteProtocolCppFile(mergedSchema, symbols, config, writer);
		}));
	return 0;
}