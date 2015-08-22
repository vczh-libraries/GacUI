#include "GacGen.h"

/***********************************************************************
Codegen::PartialClass
***********************************************************************/

void WritePartialClassHeaderFile(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<InstanceSchema>>& typeSchemas, List<WString>& typeSchemaOrder, Dictionary<WString, Ptr<Instance>>& instances)
{
	WString fileName = config->cppOutput->output + config->cppOutput->GetPartialClassHeaderFileName();
	OPEN_FILE_WITH_COMMENT(L"Partial Classes", true);

	writer.WriteLine(L"#ifndef VCZH_GACUI_RESOURCE_CODE_GENERATOR_" + config->cppOutput->name + L"_PARTIAL_CLASSES");
	writer.WriteLine(L"#define VCZH_GACUI_RESOURCE_CODE_GENERATOR_" + config->cppOutput->name + L"_PARTIAL_CLASSES");
	writer.WriteLine(L"");
	writer.WriteLine(L"#include \"" + config->cppOutput->include + L"\"");
	writer.WriteLine(L"");

	List<WString> currentNamespaces;
	
	FOREACH(WString, typeSchemaName, typeSchemaOrder)
	{
		auto instance = typeSchemas[typeSchemaName];
		WString prefix = WriteNamespace(currentNamespaces, instance->namespaces, writer);
		if (auto data = instance->schema.Cast<GuiInstanceDataSchema>())
		{
			if (data->referenceType)
			{
				writer.WriteLine(prefix + L"class " + instance->typeName + L";");
			}
			else
			{
				writer.WriteLine(prefix + L"struct " + instance->typeName + L";");
			}
		}
		else if (auto itf = instance->schema.Cast<GuiInstanceInterfaceSchema>())
		{
			writer.WriteLine(prefix + L"class " + instance->typeName + L";");
		}
	}
	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		WString prefix = WriteNamespace(currentNamespaces, instance->namespaces, writer);
		writer.WriteLine(prefix + L"class " + instance->typeName + L";");
	}
	writer.WriteLine(L"");

	FOREACH(WString, typeSchemaName, typeSchemaOrder)
	{
		auto instance = typeSchemas[typeSchemaName];
		WString prefix = WriteNamespace(currentNamespaces, instance->namespaces, writer);
		if (auto data = instance->schema.Cast<GuiInstanceDataSchema>())
		{
			if (data->referenceType)
			{
				WString parent = data->parentType == L"" ? L"vl::Object" : data->parentType;
				writer.WriteLine(prefix + L"class " + instance->typeName + L" : public " + parent + L", public vl::reflection::Description<" + instance->typeName + L">");
			}
			else
			{
				writer.WriteLine(prefix + L"struct " + instance->typeName);
			}
			writer.WriteLine(prefix + L"{");
			if (data->referenceType)
			{
				writer.WriteLine(prefix + L"public:");
			}
			FOREACH(Ptr<GuiInstancePropertySchame>, prop, data->properties)
			{
				writer.WriteLine(prefix + L"\t" + GetCppTypeNameFromWorkflowType(config, prop->typeName) + L" " + prop->name + L";");
			}
			writer.WriteLine(prefix + L"};");
		}
		else if (auto itf = instance->schema.Cast<GuiInstanceInterfaceSchema>())
		{
			WString parent = itf->parentType == L"" ? L"vl::reflection::IDescriptable" : itf->parentType;
			writer.WriteLine(prefix + L"class " + instance->typeName + L" : public virtual " + parent + L", public vl::reflection::Description<" + instance->typeName + L">");
			writer.WriteLine(prefix + L"{");
			writer.WriteLine(prefix + L"public:");
			FOREACH(Ptr<GuiInstancePropertySchame>, prop, itf->properties)
			{
				writer.WriteLine(L"");
				writer.WriteLine(prefix + L"\tvirtual " + GetCppTypeNameFromWorkflowType(config, prop->typeName) + L" Get" + prop->name + L"() = 0;");
				if (!prop->readonly)
				{
					writer.WriteLine(prefix + L"\tvirtual void Set" + prop->name + L"(" + GetCppTypeNameFromWorkflowType(config, prop->typeName) + L" value) = 0;");
				}
				if (prop->observable)
				{
					writer.WriteLine(prefix + L"\tvl::Event<void()> " + prop->name + L"Changed;");
				}
			}
			if (itf->methods.Count() > 0)
			{
				writer.WriteLine(L"");
				FOREACH(Ptr<GuiInstanceMethodSchema>, method, itf->methods)
				{
					writer.WriteString(prefix + L"\tvirtual " + GetCppTypeNameFromWorkflowType(config, method->returnType) + L" " + method->name + L"(");
					FOREACH_INDEXER(Ptr<GuiInstancePropertySchame>, argument, index, method->arguments)
					{
						if (index > 0)
						{
							writer.WriteString(L", ");
						}
						writer.WriteString(GetCppTypeNameFromWorkflowType(config, argument->typeName) + L" " + argument->name);
					}
					writer.WriteLine(L") = 0;");
				}
			}
			writer.WriteLine(prefix + L"};");
		}
		writer.WriteLine(L"");
	}

	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		WString prefix = WriteNamespace(currentNamespaces, instance->namespaces, writer);
		writer.WriteLine(prefix + L"template<typename TImpl>");
		writer.WriteLine(prefix + L"class " + instance->typeName + L"_ : public " + GetCppTypeName(instance->baseType) + L", public vl::presentation::GuiInstancePartialClass<vl::" + instance->baseType->GetTypeName() + L">, public vl::reflection::Description<TImpl>");
		writer.WriteLine(prefix + L"{");
		writer.WriteLine(prefix + L"\tfriend struct vl::reflection::description::CustomTypeDescriptorSelector<TImpl>;");
		writer.WriteLine(prefix + L"private:");
		FOREACH(Ptr<GuiInstanceParameter>, parameter, instance->context->parameters)
		{
			writer.WriteString(prefix + L"\t");
			writer.WriteString(L"Ptr<");
			writer.WriteString(parameter->className.ToString());
			writer.WriteString(L"> ");
			writer.WriteString(parameter->name.ToString());
			writer.WriteLine(L"_;");
		}
		FOREACH(Ptr<GuiInstanceProperty>, prop, instance->context->properties)
		{
			writer.WriteString(prefix + L"\t");
			writer.WriteString(GetCppTypeNameFromWorkflowType(config, prop->typeName));
			writer.WriteString(L" ");
			writer.WriteString(prop->name.ToString());
			writer.WriteLine(L"_;");
		}
		writer.WriteLine(prefix + L"protected:");
		FOREACH(Ptr<GuiInstanceState>, state, instance->context->states)
		{
			writer.WriteString(prefix + L"\t");
			writer.WriteString(GetCppTypeNameFromWorkflowType(config, state->typeName));
			writer.WriteString(L" ");
			writer.WriteString(state->name.ToString());
			writer.WriteLine(L";");
		}
		FOREACH(WString, field, instance->fields.Keys())
		{
			writer.WriteLine(prefix + L"\t" + GetCppTypeName(config, instances, instance, instance->fields[field]) + L"* " + field + L";");
		}
		writer.WriteLine(L"");
		writer.WriteString(L"\t\tvoid InitializeComponents(");
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
		writer.WriteLine(L")");
		writer.WriteLine(prefix + L"\t{");
		FOREACH_INDEXER(Ptr<GuiInstanceParameter>, parameter, index, instance->context->parameters)
		{
			writer.WriteString(prefix + L"\t\t");
			writer.WriteString(parameter->name.ToString());
			writer.WriteString(L"_ = ");
			writer.WriteString(parameter->name.ToString());
			writer.WriteLine(L";");
		}
		writer.WriteLine(prefix + L"\t\tif (InitializeFromResource())");
		writer.WriteLine(prefix + L"\t\t{");
		FOREACH(WString, field, instance->fields.Keys())
		{
			writer.WriteLine(prefix + L"\t\t\tGUI_INSTANCE_REFERENCE(" + field + L");");
		}
		writer.WriteLine(prefix + L"\t\t}");
		writer.WriteLine(prefix + L"\t\telse");
		writer.WriteLine(prefix + L"\t\t{");
		FOREACH_INDEXER(Ptr<GuiInstanceParameter>, parameter, index, instance->context->parameters)
		{
			writer.WriteString(prefix + L"\t\t\t");
			writer.WriteString(parameter->name.ToString());
			writer.WriteLine(L"_ = 0;");
		}
		writer.WriteLine(prefix + L"\t\t}");
		writer.WriteLine(prefix + L"\t}");
		writer.WriteLine(prefix + L"public:");
		writer.WriteLine(prefix + L"\t" + instance->typeName + L"_()");
		writer.WriteLine(prefix + L"\t\t:vl::presentation::GuiInstancePartialClass<" + GetCppTypeName(instance->baseType) + L">(L\"" + instance->GetFullName() + L"\")");
		if (instance->baseType == GetTypeDescriptor<GuiWindow>())
		{
			auto& atts = instance->context->instance->setters;
			vint index = atts.Keys().IndexOf(GlobalStringKey::Get(L"ControlTemplate"));
			WString controlTemplate;
			if (index != -1)
			{
				auto value = atts.Values()[index];
				if (value->binding == GlobalStringKey::Empty && value->values.Count() == 1)
				{
					if (auto text = value->values[0].Cast<GuiTextRepr>())
					{
						controlTemplate = text->text;
					}
				}
			}

			if (controlTemplate == L"")
			{
				writer.WriteLine(prefix + L"\t\t," + GetCppTypeName(instance->baseType) + L"(vl::presentation::theme::GetCurrentTheme()->CreateWindowStyle())");
			}
			else
			{
				WString lambda =
					L"\r\n"
					L"vl::collections::List<vl::reflection::description::ITypeDescriptor*> types;" L"\r\n"
					L"types.Add(vl::reflection::description::GetTypeDescriptor<" + controlTemplate + L">());" L"\r\n"
					L"auto factory = vl::presentation::templates::GuiTemplate::IFactory::CreateTemplateFactory(types);" L"\r\n"
					L"auto style = new vl::presentation::templates::GuiWindowTemplate_StyleProvider(factory);" L"\r\n"
					L"return style;" L"\r\n";
				writer.WriteLine(prefix + L"\t\t," + GetCppTypeName(instance->baseType) + L"([](){" + lambda + L"}())");
			}
		}
		else if (instance->baseType == GetTypeDescriptor<GuiCustomControl>())
		{
			writer.WriteLine(prefix + L"\t\t," + GetCppTypeName(instance->baseType) + L"(vl::presentation::theme::GetCurrentTheme()->CreateCustomControlStyle())");
		}
		FOREACH(WString, field, instance->fields.Keys())
		{
			writer.WriteLine(prefix + L"\t\t," + field + L"(0)");
		}
		writer.WriteLine(prefix + L"\t{");
		writer.WriteLine(prefix + L"\t}");
		FOREACH(Ptr<GuiInstanceParameter>, parameter, instance->context->parameters)
		{
			writer.WriteLine(L"");
			writer.WriteString(prefix + L"\t");
			writer.WriteString(L"Ptr<");
			writer.WriteString(parameter->className.ToString());
			writer.WriteString(L"> Get");
			writer.WriteString(parameter->name.ToString());
			writer.WriteLine(L"()");
			writer.WriteLine(prefix + L"\t{");
			writer.WriteLine(prefix + L"\t\treturn " + parameter->name.ToString() + L"_;");
			writer.WriteLine(prefix + L"\t}");
		}
		FOREACH(Ptr<GuiInstanceProperty>, prop, instance->context->properties)
		{
			{
				writer.WriteLine(L"");
				writer.WriteString(prefix + L"\tvl::Event<void()> ");
				writer.WriteString(prop->name.ToString());
				writer.WriteLine(L"Changed;");
			}
			{
				writer.WriteLine(L"");
				writer.WriteString(prefix + L"\t");
				writer.WriteString(GetCppTypeNameFromWorkflowType(config, prop->typeName));
				writer.WriteString(L" Get");
				writer.WriteString(prop->name.ToString());
				writer.WriteLine(L"()");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\t\treturn " + prop->name.ToString() + L"_;");
				writer.WriteLine(prefix + L"\t}");
			}
			if (!prop->readonly)
			{
				writer.WriteLine(L"");
				writer.WriteString(prefix + L"\tvoid Set");
				writer.WriteString(prop->name.ToString());
				writer.WriteString(L"(");
				writer.WriteString(GetCppTypeNameFromWorkflowType(config, prop->typeName));
				writer.WriteLine(L" value)");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\t\t" + prop->name.ToString() + L"_ = value;");
				writer.WriteLine(prefix + L"\t\t" + prop->name.ToString() + L"Changed();");
				writer.WriteLine(prefix + L"\t}");
			}
		}
		writer.WriteLine(prefix + L"};");
		writer.WriteLine(L"");
	}

	{
		List<WString> ns;
		FillReflectionNamespaces(ns);
		WString prefix = WriteNamespace(currentNamespaces, ns, writer);

		FOREACH(Ptr<InstanceSchema>, instance, typeSchemas.Values())
		{
			writer.WriteLine(prefix + L"DECL_TYPE_INFO(" + instance->GetFullName() + L")");
		}
		FOREACH(Ptr<Instance>, instance, instances.Values())
		{
			writer.WriteLine(prefix + L"DECL_TYPE_INFO(" + instance->GetFullName() + L")");
		}
		writer.WriteLine(L"");
	}

	WriteNamespaceStop(currentNamespaces, writer);

	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		if (!instance->context->codeBehind)
		{
			WriteControlClassHeaderFileContent(config, instance, writer);
			writer.WriteLine(L"");
		}
	}
	writer.WriteLine(L"");

	writer.WriteLine(L"#endif");
}

void WritePartialClassCppFile(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<InstanceSchema>>& typeSchemas, List<WString>& typeSchemaOrder, Dictionary<WString, Ptr<Instance>>& instances)
{
	WString fileName = config->cppOutput->output + config->cppOutput->GetPartialClassCppFileName();
	OPEN_FILE_WITH_COMMENT(L"Partial Classes", true);

	writer.WriteLine(L"#include \"" + config->cppOutput->GetGlobalHeaderFileName() + L"\"");
	writer.WriteLine(L"");

	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		if (!instance->context->codeBehind)
		{
			WriteControlClassCppFileContent(config, instance, writer);
			writer.WriteLine(L"");
		}
	}

	List<WString> ns, currentNamespaces;
	FillReflectionNamespaces(ns);
	WString prefix = WriteNamespace(currentNamespaces, ns, writer);
	writer.WriteLine(prefix + L"#define _ ,");
	
	FOREACH(Ptr<InstanceSchema>, instance, typeSchemas.Values())
	{
		writer.WriteLine(prefix + L"IMPL_CPP_TYPE_INFO(" + instance->GetFullName() + L")");
	}
	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		writer.WriteLine(prefix + L"IMPL_CPP_TYPE_INFO(" + instance->GetFullName() + L")");
	}
	writer.WriteLine(L"");

	FOREACH(Ptr<InstanceSchema>, instance, typeSchemas.Values())
	{
		if (auto data = instance->schema.Cast<GuiInstanceDataSchema>())
		{
			if (data->referenceType)
			{
				writer.WriteLine(prefix + L"BEGIN_CLASS_MEMBER(" + instance->GetFullName() + L")");
				if (data->parentType != L"")
				{
					writer.WriteLine(prefix + L"\tCLASS_MEMBER_BASE(" + data->parentType + L")");
				}
				writer.WriteLine(prefix + L"\tCLASS_MEMBER_CONSTRUCTOR(vl::Ptr<" + instance->GetFullName() + L">(), NO_PARAMETER)");
				FOREACH(Ptr<GuiInstancePropertySchame>, prop, data->properties)
				{
					writer.WriteLine(prefix + L"\tCLASS_MEMBER_FIELD(" + prop->name + L")");
				}
				writer.WriteLine(prefix + L"END_CLASS_MEMBER(" + instance->GetFullName() + L")");
			}
			else
			{
				writer.WriteLine(prefix + L"BEGIN_STRUCT_MEMBER(" + instance->GetFullName() + L")");
				FOREACH(Ptr<GuiInstancePropertySchame>, prop, data->properties)
				{
					writer.WriteLine(prefix + L"\tSTRUCT_MEMBER(" + prop->name + L")");
				}
				writer.WriteLine(prefix + L"END_STRUCT_MEMBER(" + instance->GetFullName() + L")");
			}
		}
		else if (auto itf = instance->schema.Cast<GuiInstanceInterfaceSchema>())
		{
			writer.WriteLine(prefix + L"BEGIN_CLASS_MEMBER(" + instance->GetFullName() + L")");
			WString parent = itf->parentType == L"" ? L"vl::reflection::IDescriptable" : itf->parentType;
			writer.WriteLine(prefix + L"\tCLASS_MEMBER_BASE(" + parent + L")");
			FOREACH(Ptr<GuiInstancePropertySchame>, prop, itf->properties)
			{
				if (prop->observable)
				{
					writer.WriteLine(prefix + L"\tCLASS_MEMBER_EVENT(" + prop->name + L"Changed)");
					if (prop->readonly)
					{
						writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(" + prop->name + L", " + prop->name + L"Changed)");
					}
					else
					{
						writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY_EVENT_FAST(" + prop->name + L", " + prop->name + L"Changed)");
					}
				}
				else
				{
					if (prop->readonly)
					{
						writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY_READONLY_FAST(" + prop->name + L")");
					}
					else
					{
						writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY_FAST(" + prop->name + L")");
					}
				}
			}
			FOREACH(Ptr<GuiInstanceMethodSchema>, method, itf->methods)
			{
				writer.WriteString(prefix + L"\tCLASS_MEMBER_METHOD(" + method->name + L", ");
				if (method->arguments.Count() == 0)
				{
					writer.WriteString(L"NO_PARAMETER");
				}
				else
				{
					writer.WriteString(L"{ ");
					FOREACH_INDEXER(Ptr<GuiInstancePropertySchame>, argument, index, method->arguments)
					{
						if (index > 0)
						{
							writer.WriteString(L" _ ");
						}
						writer.WriteString(L"L\"" + argument->name + L"\"");
					}
					writer.WriteString(L" }");
				}
				writer.WriteLine(L");");
			}
			writer.WriteLine(prefix + L"END_CLASS_MEMBER(" + instance->GetFullName() + L")");
		}
		writer.WriteLine(L"");
	}

	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		writer.WriteLine(prefix + L"BEGIN_CLASS_MEMBER(" + instance->GetFullName() + L")");
		writer.WriteLine(prefix + L"\tCLASS_MEMBER_BASE(" + GetCppTypeName(instance->baseType) + L")");
		writer.WriteString(prefix + L"\tCLASS_MEMBER_CONSTRUCTOR(" + instance->GetFullName() + L"*(");
		FOREACH_INDEXER(Ptr<GuiInstanceParameter>, parameter, index, instance->context->parameters)
		{
			if (index > 0)
			{
				writer.WriteString(L", ");
			}
			writer.WriteString(L"Ptr<");
			writer.WriteString(parameter->className.ToString());
			writer.WriteString(L">");
		}
		writer.WriteString(L"), ");
		if (instance->context->parameters.Count() == 0)
		{
			writer.WriteString(L"NO_PARAMETER");
		}
		else
		{
			writer.WriteString(L"{ ");
			FOREACH_INDEXER(Ptr<GuiInstanceParameter>, parameter, index, instance->context->parameters)
			{
				if (index > 0)
				{
					writer.WriteString(L" _ ");
				}
				writer.WriteString(L"L\"");
				writer.WriteString(parameter->name.ToString());
				writer.WriteString(L"\"");
			}
			writer.WriteString(L" }");
		}
		writer.WriteLine(L")");

		if (instance->eventHandlers.Count() > 0)
		{
			writer.WriteLine(L"");
			FOREACH(WString, name, instance->eventHandlers.Keys())
			{
				writer.WriteLine(prefix + L"\tCLASS_MEMBER_GUIEVENT_HANDLER(" + name + L", " + GetCppTypeName(instance->eventHandlers[name]) + L")");
			}
		}
		
		if (instance->context->parameters.Count() > 0)
		{
			writer.WriteLine(L"");
			FOREACH_INDEXER(Ptr<GuiInstanceParameter>, parameter, index, instance->context->parameters)
			{
				writer.WriteString(prefix + L"\tCLASS_MEMBER_PROPERTY_READONLY_FAST(");
				writer.WriteString(parameter->name.ToString());
				writer.WriteLine(L")");
			}
			FOREACH(Ptr<GuiInstanceProperty>, prop, instance->context->properties)
			{
				writer.WriteString(prefix + L"\tCLASS_MEMBER_EVENT(");
				writer.WriteString(prop->name.ToString());
				writer.WriteLine(L"Changed)");
				if (prop->readonly)
				{
					writer.WriteString(prefix + L"\tCLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(");
					writer.WriteString(prop->name.ToString());
					writer.WriteString(L", ");
					writer.WriteString(prop->name.ToString());
					writer.WriteLine(L"Changed)");
				}
				else
				{
					writer.WriteString(prefix + L"\tCLASS_MEMBER_PROPERTY_EVENT_FAST(");
					writer.WriteString(prop->name.ToString());
					writer.WriteString(L", ");
					writer.WriteString(prop->name.ToString());
					writer.WriteLine(L"Changed)");
				}
			}
			FOREACH(Ptr<GuiInstanceState>, state, instance->context->states)
			{
				writer.WriteString(prefix + L"\tCLASS_MEMBER_FIELD(");
				writer.WriteString(state->name.ToString());
				writer.WriteLine(L")");
			}
		}

		writer.WriteLine(prefix + L"END_CLASS_MEMBER(" + instance->GetFullName() + L")");
		writer.WriteLine(L"");
	}

	writer.WriteLine(prefix + L"#undef _");
	writer.WriteLine(L"");

	writer.WriteLine(prefix + L"class " + config->cppOutput->name + L"ResourceLoader : public Object, public ITypeLoader");
	writer.WriteLine(prefix + L"{");
	writer.WriteLine(prefix + L"public:");
	writer.WriteLine(prefix + L"\tvoid Load(ITypeManager* manager)");
	writer.WriteLine(prefix + L"\t{");
	FOREACH(Ptr<InstanceSchema>, instance, typeSchemas.Values())
	{
		writer.WriteLine(prefix + L"\t\tADD_TYPE_INFO(" + instance->GetFullName() + L")");
	}
	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		writer.WriteLine(prefix + L"\t\tADD_TYPE_INFO(" + instance->GetFullName() + L")");
	}
	writer.WriteLine(prefix + L"\t}");
	writer.WriteLine(L"");
	writer.WriteLine(prefix + L"\tvoid Unload(ITypeManager* manager)");
	writer.WriteLine(prefix + L"\t{");
	writer.WriteLine(prefix + L"\t}");
	writer.WriteLine(prefix + L"};");
	writer.WriteLine(L"");
	writer.WriteLine(prefix + L"class " + config->cppOutput->name + L"ResourcePlugin : public Object, public vl::presentation::controls::IGuiPlugin");
	writer.WriteLine(prefix + L"{");
	writer.WriteLine(prefix + L"public:");
	writer.WriteLine(prefix + L"\tvoid Load()override");
	writer.WriteLine(prefix + L"\t{");
	writer.WriteLine(prefix + L"\t\tGetGlobalTypeManager()->AddTypeLoader(new " + config->cppOutput->name + L"ResourceLoader);");
	writer.WriteLine(prefix + L"\t}");
	writer.WriteLine(L"");
	writer.WriteLine(prefix + L"\tvoid AfterLoad()override");
	writer.WriteLine(prefix + L"\t{");
	writer.WriteLine(prefix + L"\t}");
	writer.WriteLine(L"");
	writer.WriteLine(prefix + L"\tvoid Unload()override");
	writer.WriteLine(prefix + L"\t{");
	writer.WriteLine(prefix + L"\t}");
	writer.WriteLine(prefix + L"};");
	writer.WriteLine(prefix + L"GUI_REGISTER_PLUGIN(" + config->cppOutput->name + L"ResourcePlugin)");

	WriteNamespaceStop(currentNamespaces, writer);
	writer.WriteLine(L"");
}