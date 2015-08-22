#ifndef VCZH_GACGEN
#define VCZH_GACGEN

#include "GacUIReflection.h"

using namespace vl::console;
using namespace vl::collections;
using namespace vl::regex;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::parsing;
using namespace vl::parsing::tabling;
using namespace vl::parsing::xml;
using namespace vl::workflow;
using namespace vl::workflow::analyzer;

/***********************************************************************
Console
***********************************************************************/

extern void										PrintErrorMessage(const WString& message);
extern void										PrintSuccessMessage(const WString& message);
extern void										PrintInformationMessage(const WString& message);

/***********************************************************************
Object Model
***********************************************************************/

struct Instance
{
	Ptr<GuiInstanceContext>						context;
	ITypeDescriptor*							baseType;
	List<WString>								namespaces;
	WString										typeName;
	Dictionary<WString, GuiConstructorRepr*>	fields;
	Dictionary<WString, ITypeDescriptor*>		eventHandlers;

	WString										GetFullName();
};

struct InstanceSchema
{
	List<WString>								namespaces;
	WString										typeName;
	Ptr<GuiInstanceTypeSchema>					schema;

	WString										GetFullName();
};

class CodegenConfig
{
public:
	class CppOutput
	{
	public:
		WString									include;
		WString									name;
		WString									prefix;

		WString									GetControlClassHeaderFileName(Ptr<Instance> instance);
		WString									GetControlClassCppFileName(Ptr<Instance> instance);
		WString									GetPartialClassHeaderFileName();
		WString									GetPartialClassCppFileName();
		WString									GetGlobalHeaderFileName();
	};

	class ResOutput
	{
	public:
		WString									precompiledOutput;
		WString									precompiledBinary;
		WString									precompiledCompressed;
	};

	Ptr<GuiResource>							resource;
	Ptr<ParsingTable>							workflowTable;
	Ptr<WfLexicalScopeManager>					workflowManager;

	Ptr<CppOutput>								cppOutput;
	Ptr<ResOutput>								resOutput;

	static bool									LoadConfigString(Ptr<GuiResource> resource, const WString& name, WString& value, bool optional = false);
	static Ptr<CodegenConfig>					LoadConfig(Ptr<GuiResource> resource);
};

/***********************************************************************
TypeTransformation
***********************************************************************/

WString											GetCppTypeName(ITypeDescriptor* typeDescriptor);
WString											GetCppTypeName(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances, Ptr<Instance> instance, GuiConstructorRepr* ctor);
IGuiInstanceLoader::TypeInfo					GetCppTypeInfo(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances, Ptr<Instance> instance, GuiConstructorRepr* ctor);
Ptr<WfType>										ParseWorkflowType(Ptr<CodegenConfig> config, const WString& workflowType);
Ptr<ITypeInfo>									GetTypeInfoFromWorkflowType(Ptr<CodegenConfig> config, const WString& workflowType);
WString											GetCppTypeFromTypeInfo(ITypeInfo* typeInfo);
WString											GetCppTypeNameFromWorkflowType(Ptr<CodegenConfig> config, const WString& workflowType);

/***********************************************************************
Search
***********************************************************************/

void											SearchAllFields(Ptr<GuiInstanceEnvironment> env, Ptr<GuiInstanceContext> context, Dictionary<WString, GuiConstructorRepr*>& fields);
void											SearchAllSchemas(const Regex& regexClassName, Ptr<GuiResourceFolder> folder, Dictionary<WString, Ptr<InstanceSchema>>& typeSchemas, List<WString>& typeSchemaOrder);
void											SearchAllInstances(const Regex& regexClassName, Ptr<GuiResourcePathResolver> resolver, Ptr<GuiResourceFolder> folder, Dictionary<WString, Ptr<Instance>>& instances);
void											SearchAllEventHandlers(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances, Ptr<Instance> instance, Ptr<GuiInstanceEnvironment> env, Dictionary<WString, ITypeDescriptor*>& eventHandlers);

/***********************************************************************
Codegen::Utility
***********************************************************************/

void											WriteFileComment(const WString& name, bool doNotModify, StreamWriter& writer);
void											FillReflectionNamespaces(List<WString>& namespaces);
WString											WriteNamespace(List<WString>& currentNamespaces, List<WString>& namespaces, StreamWriter& writer);
void											WriteNamespaceStop(List<WString>& currentNamespaces, StreamWriter& writer);

/***********************************************************************
Codegen::EventHandlers
***********************************************************************/

WString											GetEventHandlerCommentBegin(const WString& prefix);
WString											GetEventHandlerCommentEnd(const WString& prefix);
WString											GetEventHandlerHeader(const WString& prefix, Ptr<Instance> instance, const WString& name, bool addClassName);
void											WriteControlClassHeaderFileEventHandlers(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer);
void											WriteControlClassCppFileEventHandlers(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, Group<WString, WString>& existingEventHandlers, List<WString>& additionalLines, StreamWriter& writer);

/***********************************************************************
Codegen::ControlClassContent
***********************************************************************/

void											WriteControlClassHeaderCtorArgs(Ptr<CodegenConfig> config, Ptr<Instance> instance, StreamWriter& writer);
void											WriteControlClassHeaderCtor(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer);
void											WriteControlClassCppCtor(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer);
void											WriteControlClassCppInit(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer);
void											WriteControlClassHeaderFileContent(Ptr<CodegenConfig> config, Ptr<Instance> instance, StreamWriter& writer);
void											WriteControlClassCppFileContent(Ptr<CodegenConfig> config, Ptr<Instance> instance, StreamWriter& writer);

/***********************************************************************
Codegen::FileUtility
***********************************************************************/

#define OPEN_BINARY_FILE(NAME)\
	FileStream fileStream(config->resource->GetWorkingDirectory() + fileName, FileStream::WriteOnly); \
	if (!fileStream.IsAvailable()) \
	{ \
		PrintErrorMessage(L"error> Failed to generate " + fileName); \
		return; \
	} \
	PrintSuccessMessage(L"gacgen> Generating " + fileName);

#define OPEN_FILE(NAME)\
	FileStream fileStream(config->resource->GetWorkingDirectory() + fileName, FileStream::WriteOnly); \
	if (!fileStream.IsAvailable()) \
	{ \
		PrintErrorMessage(L"error> Failed to generate " + fileName); \
		return; \
	} \
	BomEncoder encoder(BomEncoder::Utf8); \
	EncoderStream encoderStream(fileStream, encoder); \
	StreamWriter writer(encoderStream); \
	PrintSuccessMessage(L"gacgen> Generating " + fileName);

#define OPEN_FILE_WITH_COMMENT(NAME, DONOTMODIFY)\
	OPEN_FILE(NAME)\
	WriteFileComment(NAME, DONOTMODIFY, writer);

#define CANNOT_MERGE_CONTENT\
	PrintErrorMessage(L"error> Don't know how to override " + fileName + L". Please open " + config->cppOutput->GetPartialClassHeaderFileName() + L" to get the latest content in the comment and modify the file by yourself.")

/***********************************************************************
Codegen::ControlClass
***********************************************************************/

bool											TryReadFile(Ptr<CodegenConfig> config, const WString& fileName, List<WString>& lines);
void											WriteControlClassHeaderFile(Ptr<CodegenConfig> config, Ptr<Instance> instance);
void											WriteControlClassCppFile(Ptr<CodegenConfig> config, Ptr<Instance> instance);

/***********************************************************************
Codegen::PartialClass
***********************************************************************/

void											WritePartialClassHeaderFile(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<InstanceSchema>>& typeSchemas, List<WString>& typeSchemaOrder, Dictionary<WString, Ptr<Instance>>& instances);
void											WritePartialClassCppFile(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<InstanceSchema>>& typeSchemas, List<WString>& typeSchemaOrder, Dictionary<WString, Ptr<Instance>>& instances);

/***********************************************************************
Codegen::GlobalHeader
***********************************************************************/

void											WriteGlobalHeaderFile(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances);

#endif