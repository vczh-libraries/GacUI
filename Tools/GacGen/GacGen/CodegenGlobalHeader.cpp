#include "GacGen.h"

/***********************************************************************
Codegen::GlobalHeader
***********************************************************************/

void WriteGlobalHeaderFile(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances)
{
	WString fileName = config->cppOutput->GetGlobalHeaderFileName();
	OPEN_FILE_WITH_COMMENT(config->cppOutput->name, true);

	writer.WriteLine(L"#ifndef VCZH_GACUI_RESOURCE_CODE_GENERATOR_" + config->cppOutput->name);
	writer.WriteLine(L"#define VCZH_GACUI_RESOURCE_CODE_GENERATOR_" + config->cppOutput->name);
	writer.WriteLine(L"");
	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		if (instance->context->codeBehind)
		{
			writer.WriteLine(L"#include \"" + config->cppOutput->GetControlClassHeaderFileName(instance) + L"\"");
		}
	}
	writer.WriteLine(L"");

	writer.WriteLine(L"#endif");
}