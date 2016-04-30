#include "GacGen.h"

/***********************************************************************
Codegen::ControlClassContent
***********************************************************************/

void WriteControlClassHeaderCtorArgs(Ptr<CodegenConfig> config, Ptr<Instance> instance, StreamWriter& writer)
{
	FOREACH_INDEXER(Ptr<GuiInstanceParameter>, parameter, index, instance->context->parameters)
	{
		if (index > 0)
		{
			writer.WriteString(L", ");
		}
		writer.WriteString(L"Ptr<");
		writer.WriteString(parameter->className.ToString());
		writer.WriteString(L"> ");
		writer.WriteString(parameter->name.ToString());
	}
}

void WriteControlClassHeaderCtor(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer)
{
	writer.WriteString(prefix + instance->typeName + L"(");
	WriteControlClassHeaderCtorArgs(config, instance, writer);
	writer.WriteLine(L");");
}

void WriteControlClassHeaderDtor(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer)
{
	writer.WriteLine(prefix + L"~" + instance->typeName + L"();");
}

void WriteControlClassCppCtor(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer)
{
	writer.WriteString(prefix + instance->typeName + L"::" + instance->typeName + L"(");
	WriteControlClassHeaderCtorArgs(config, instance, writer);
	writer.WriteLine(L")");
}

void WriteControlClassCppDtor(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer)
{
	writer.WriteLine(prefix + instance->typeName + L"::~" + instance->typeName + L"()");
	writer.WriteLine(prefix + L"{");
	writer.WriteLine(prefix + L"\tOnDestroy();");
	writer.WriteLine(prefix + L"\tClearSubscriptions();");
	writer.WriteLine(prefix + L"}");
}

void WriteControlClassCppInit(Ptr<CodegenConfig> config, Ptr<Instance> instance, const WString& prefix, StreamWriter& writer)
{
	writer.WriteString(prefix + L"InitializeComponents(");
	FOREACH_INDEXER(Ptr<GuiInstanceParameter>, parameter, index, instance->context->parameters)
	{
		if (index > 0)
		{
			writer.WriteString(L", ");
		}
		writer.WriteString(parameter->name.ToString());
	}
	writer.WriteLine(L");");
}

void WriteControlClassHeaderFileContent(Ptr<CodegenConfig> config, Ptr<Instance> instance, StreamWriter& writer)
{
	List<WString> currentNamespaces;
	WString prefix = WriteNamespace(currentNamespaces, instance->namespaces, writer);
	WString instanceClassName;
	FOREACH(WString, ns, instance->namespaces)
	{
		instanceClassName += ns + L"::";
	}
	instanceClassName += instance->typeName;

	writer.WriteLine(prefix + L"class " + instance->typeName + L" : public " + instanceClassName + L"_<" + instanceClassName + L">");
	writer.WriteLine(prefix + L"{");
	writer.WriteLine(prefix + L"\tfriend class " + instanceClassName + L"_<" + instanceClassName + L">;");
	writer.WriteLine(prefix + L"\tfriend struct vl::reflection::description::CustomTypeDescriptorSelector<" + instanceClassName + L">;");
	writer.WriteLine(prefix + L"protected:");
	writer.WriteLine(L"");
	WriteControlClassHeaderFileEventHandlers(config, instance, prefix, writer);
	writer.WriteLine(prefix + L"public:");
	WriteControlClassHeaderCtor(config, instance, prefix + L"\t" , writer);
	WriteControlClassHeaderDtor(config, instance, prefix + L"\t" , writer);
	writer.WriteLine(prefix + L"};");
	WriteNamespaceStop(currentNamespaces, writer);
	writer.WriteLine(L"");
}

void WriteControlClassCppFileContent(Ptr<CodegenConfig> config, Ptr<Instance> instance, StreamWriter& writer)
{
	Group<WString, WString> existingEventHandlers;
	List<WString> additionalLines;
	List<WString> currentNamespaces;
	WString prefix = WriteNamespace(currentNamespaces, instance->namespaces, writer);
	WriteControlClassCppFileEventHandlers(config, instance, prefix, existingEventHandlers, additionalLines, writer);
	writer.WriteLine(L"");
	WriteControlClassCppCtor(config, instance, prefix, writer);
	writer.WriteLine(prefix + L"{");
	WriteControlClassCppInit(config, instance, prefix + L"\t", writer);
	writer.WriteLine(prefix + L"\tOnCreate();");
	writer.WriteLine(prefix + L"}");
	writer.WriteLine(L"");
	WriteControlClassCppDtor(config, instance, prefix, writer);
	WriteNamespaceStop(currentNamespaces, writer);
	writer.WriteLine(L"");
}