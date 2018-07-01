#ifndef VCZH_GACGEN
#define VCZH_GACGEN

#include "GacUICompiler.h"

using namespace vl;
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
using namespace vl::workflow::runtime;
using namespace vl::workflow::cppcodegen;
using namespace vl::presentation;
using namespace vl::presentation::templates;

/***********************************************************************
Console
***********************************************************************/

extern void										PrintErrorMessage(const WString& message);
extern void										PrintSuccessMessage(const WString& message);
extern void										PrintInformationMessage(const WString& message);

/***********************************************************************
Object Model
***********************************************************************/

class CodegenConfig
{
public:
	class ResOutput : public Object
	{
	public:
		WString									resource;
		WString									compressed;
		WString									assembly;
	};

	class CppOutput : public Object
	{
	public:
		WString									sourceFolder;
		List<WString>							normalIncludes;
		List<WString>							reflectionIncludes;
		WString									name;

		WString									resource;
		WString									compressed;
		WString									cppResource;
		WString									cppCompressed;
	};

	Ptr<GuiResource>							resource;
	Ptr<GuiResourceMetadata>					metadata;
	Ptr<CppOutput>								cppOutput;
	Ptr<ResOutput>								resOutputx32;
	Ptr<ResOutput>								resOutputx64;

	static WString								NormalizeFolder(const WString& folder);
	static bool									LoadConfigString(Ptr<GuiResourceFolder> folder, const WString& path, WString& value, bool optional = true);
	static bool									LoadConfigString(Ptr<GuiResourceFolder> folder, const WString& path, List<WString>& value, bool optional = true);
	static Ptr<CodegenConfig>					LoadConfig(Ptr<GuiResource> resource, GuiResourceError::List& errors);
};

#endif