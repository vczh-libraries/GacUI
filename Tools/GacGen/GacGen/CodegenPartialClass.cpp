#include "GacGen.h"

WString SplitSchemaName(WString typeName, List<WString>& namespaces)
{
	auto reading = typeName.Buffer();
	while (auto split = wcsstr(reading, L"::"))
	{
		namespaces.Add(WString(reading, split - reading));
		reading = split + 2;
	}
	return reading;
}

/***********************************************************************
Codegen::PartialClassHeader
***********************************************************************/

void WritePartialClassHeaderFile(Ptr<CodegenConfig> config, Ptr<WfLexicalScopeManager> schemaManager, Dictionary<WString, Ptr<WfClassDeclaration>>& typeSchemas, List<WString>& typeSchemaOrder, Dictionary<WString, Ptr<Instance>>& instances)
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
		List<WString> namespaces;
		auto typeName = SplitSchemaName(typeSchemaName, namespaces);
		WString prefix = WriteNamespace(currentNamespaces, namespaces, writer);
		writer.WriteLine(prefix + L"class " + typeName + L";");
	}
	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		WString prefix = WriteNamespace(currentNamespaces, instance->namespaces, writer);
		writer.WriteLine(prefix + L"class " + instance->typeName + L";");
	}
	writer.WriteLine(L"");

	FOREACH(WString, typeSchemaName, typeSchemaOrder)
	{
		List<WString> namespaces;
		auto typeName = SplitSchemaName(typeSchemaName, namespaces);
		WString prefix = WriteNamespace(currentNamespaces, namespaces, writer);
		auto classDecl = typeSchemas[typeSchemaName];
		auto td = schemaManager->declarationTypes[classDecl.Obj()];

		writer.WriteString(prefix + L"class " + typeName);
		{
			auto baseCount = td->GetBaseTypeDescriptorCount();
			if (baseCount == 0)
			{
				writer.WriteString(L" : public virtual vl::reflection::IDescriptable");
			}
			else
			{
				for (vint i = 0; i < baseCount; i++)
				{
					if (i == 0)
					{
						writer.WriteString(L" : public virtual ");
					}
					else
					{
						writer.WriteString(L", public virtual ");
					}
					writer.WriteString(GetCppTypeName(td->GetBaseTypeDescriptor(i)));
				}
			}
		}
		writer.WriteLine(L", public vl::reflection::Description<" + typeName + L">");
		writer.WriteLine(prefix + L"{");
		writer.WriteLine(prefix + L"public:");

		FOREACH(Ptr<WfClassMember>, member, classDecl->members)
		{
			if (member->kind == WfClassMemberKind::Normal)
			{
				if (auto funcDecl = member->declaration.Cast<WfFunctionDeclaration>())
				{
					auto info = schemaManager->declarationMemberInfos[funcDecl.Obj()].Cast<IMethodInfo>();
					writer.WriteString(prefix + L"\tvirtual " + GetCppTypeFromTypeInfo(info->GetReturn()) + L" " + info->GetName() + L"(");
					vint count = info->GetParameterCount();
					for (vint i = 0; i < count; i++)
					{
						if (i > 0)
						{
							writer.WriteString(L", ");
						}
						auto paramInfo = info->GetParameter(i);
						writer.WriteString(GetCppTypeFromTypeInfo(paramInfo->GetType()) + L" " + paramInfo->GetName());
					}
					writer.WriteLine(L") = 0;");
				}
				else if (auto eventDecl = member->declaration.Cast<WfEventDeclaration>())
				{
					auto info = schemaManager->declarationMemberInfos[eventDecl.Obj()].Cast<IEventInfo>();
					writer.WriteString(prefix + L"\tvl::Event<void(");
					auto funcType = info->GetHandlerType()->GetElementType();
					vint count = funcType->GetGenericArgumentCount();
					for (vint i = 1; i < count; i++)
					{
						if (i > 1)
						{
							writer.WriteString(L", ");
						}
						writer.WriteString(GetCppTypeFromTypeInfo(funcType->GetGenericArgument(i)));
					}
					writer.WriteLine(L")> " + info->GetName() + L";");
				}
			}
		}
		writer.WriteLine(prefix + L"};");
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
		FOREACH(Ptr<GuiInstanceState>, state, instance->context->states)
		{
			if (auto typeInfo = GetTypeInfoFromWorkflowType(config, state->typeName))
			{
				if (typeInfo->GetDecorator() == ITypeInfo::RawPtr)
				{
					writer.WriteLine(prefix + L"\t\t," + state->name.ToString() + L"(nullptr)");
				}
			}
		}
		FOREACH(Ptr<GuiInstanceProperty>, prop, instance->context->properties)
		{
			if (auto typeInfo = GetTypeInfoFromWorkflowType(config, prop->typeName))
			{
				if (typeInfo->GetDecorator() == ITypeInfo::RawPtr)
				{
					writer.WriteLine(prefix + L"\t\t," + prop->name.ToString() + L"(nullptr)");
				}
			}
		}
		writer.WriteLine(prefix + L"\t{");
		FOREACH(Ptr<GuiInstanceState>, state, instance->context->states)
		{
			if (auto typeInfo = GetTypeInfoFromWorkflowType(config, state->typeName))
			{
				if (typeInfo->GetTypeDescriptor()->GetValueSerializer())
				{
					auto cppType = GetCppTypeFromTypeInfo(typeInfo.Obj());
					writer.WriteLine(prefix + L"\t\tthis->" + state->name.ToString() + L" = vl::reflection::description::UnboxValue<" + cppType + L">(vl::reflection::description::Value::From(L\"" + state->value + L"\", reflection::description::GetTypeDescriptor<" + cppType + L">()));");
				}
			}
		}
		FOREACH(Ptr<GuiInstanceProperty>, prop, instance->context->properties)
		{
			if (auto typeInfo = GetTypeInfoFromWorkflowType(config, prop->typeName))
			{
				if (typeInfo->GetTypeDescriptor()->GetValueSerializer())
				{
					auto cppType = GetCppTypeFromTypeInfo(typeInfo.Obj());
					writer.WriteLine(prefix + L"\t\tthis->" + prop->name.ToString() + L"_ = vl::reflection::description::UnboxValue<" + cppType + L">(vl::reflection::description::Value::From(L\"" + prop->value + L"\", reflection::description::GetTypeDescriptor<" + cppType + L">()));");
				}
			}
		}
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

		FOREACH(WString, typeName, typeSchemas.Keys())
		{
			writer.WriteLine(prefix + L"DECL_TYPE_INFO(" + typeName + L")");
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

/***********************************************************************
Codegen::PartialClassCpp
***********************************************************************/

void WritePartialClassCppFile(Ptr<CodegenConfig> config, Ptr<WfLexicalScopeManager> schemaManager, Dictionary<WString, Ptr<WfClassDeclaration>>& typeSchemas, List<WString>& typeSchemaOrder, Dictionary<WString, Ptr<Instance>>& instances)
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
	
	FOREACH(WString, typeName, typeSchemas.Keys())
	{
		writer.WriteLine(prefix + L"IMPL_CPP_TYPE_INFO(" + typeName + L")");
	}
	FOREACH(Ptr<Instance>, instance, instances.Values())
	{
		writer.WriteLine(prefix + L"IMPL_CPP_TYPE_INFO(" + instance->GetFullName() + L")");
	}
	writer.WriteLine(L"");
	

	FOREACH(WString, typeSchemaName, typeSchemas.Keys())
	{
		List<WString> namespaces;
		auto classDecl = typeSchemas[typeSchemaName];
		auto td = schemaManager->declarationTypes[classDecl.Obj()];

		writer.WriteLine(prefix + L"BEGIN_CLASS_MEMBER(" + typeSchemaName + L")");
		{
			auto baseCount = td->GetBaseTypeDescriptorCount();
			if (baseCount == 0)
			{
				writer.WriteLine(prefix + L"\tCLASS_MEMBER_BASE(vl::reflection::IDescriptable)");
			}
			else
			{
				for (vint i = 0; i < baseCount; i++)
				{
					writer.WriteLine(prefix + L"\tCLASS_MEMBER_BASE(" + GetCppTypeName(td->GetBaseTypeDescriptor(i)) + L")");
				}
			}
		}
		
		FOREACH(Ptr<WfClassMember>, member, classDecl->members)
		{
			if (member->kind == WfClassMemberKind::Normal)
			{
				if (auto funcDecl = member->declaration.Cast<WfFunctionDeclaration>())
				{
					auto info = schemaManager->declarationMemberInfos[funcDecl.Obj()].Cast<IMethodInfo>();
					writer.WriteString(prefix + L"\tCLASS_MEMBER_METHOD(" + info->GetName() + L", ");
					vint count = info->GetParameterCount();
					if (count == 0)
					{
						writer.WriteString(L"NO_PARAMETER");
					}
					else
					{
						writer.WriteString(L"{ ");
						for (vint i = 0; i<count; i++)
						{
							if (i > 0)
							{
								writer.WriteString(L" _ ");
							}
							writer.WriteString(L"L\"" + info->GetParameter(i)->GetName() + L"\"");
						}
						writer.WriteString(L" }");
					}
					writer.WriteLine(L");");
				}
				else if (auto eventDecl = member->declaration.Cast<WfEventDeclaration>())
				{
					auto info = schemaManager->declarationMemberInfos[eventDecl.Obj()].Cast<IEventInfo>();
					writer.WriteLine(prefix + L"\tCLASS_MEMBER_EVENT(" + info->GetName() + L")");
				}
			}
		}
		
		FOREACH(Ptr<WfClassMember>, member, classDecl->members)
		{
			if (member->kind == WfClassMemberKind::Normal)
			{
				if (auto propDecl = member->declaration.Cast<WfPropertyDeclaration>())
				{
					auto info = schemaManager->declarationMemberInfos[propDecl.Obj()].Cast<IPropertyInfo>();
					if (info->GetSetter())
					{
						if (info->GetValueChangedEvent())
						{
							writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY_EVENT(" + info->GetName() + L", " + info->GetGetter()->GetName() + L", " + info->GetSetter()->GetName() + L", " + info->GetValueChangedEvent()->GetName() + L")");
						}
						else
						{
							writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY(" + info->GetName() + L", " + info->GetGetter()->GetName() + L", " + info->GetSetter()->GetName() + L")");
						}
					}
					else
					{
						if (info->GetValueChangedEvent())
						{
							writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY_EVENT_READONLY(" + info->GetName() + L", " + info->GetGetter()->GetName() + L", " + info->GetValueChangedEvent()->GetName() + L")");
						}
						else
						{
							writer.WriteLine(prefix + L"\tCLASS_MEMBER_PROPERTY_READONLY(" + info->GetName() + L", " + info->GetGetter()->GetName() + L")");
						}
					}
				}
			}
		}
		writer.WriteLine(prefix + L"END_CLASS_MEMBER(" + typeSchemaName + L")");
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
				writer.WriteLine(prefix + L"\tCLASS_MEMBER_GUIEVENT_HANDLER(" + name + L", " + GetCppTypeName(instance->eventHandlers[name].argumentType) + L")");
			}
		}
		
		if (instance->context->parameters.Count() + instance->context->properties.Count() + instance->context->states.Count() > 0)
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
				writer.WriteString(prefix + L"\tCLASS_MEMBER_PROPERTY_EVENT_FAST(");
				writer.WriteString(prop->name.ToString());
				writer.WriteString(L", ");
				writer.WriteString(prop->name.ToString());
				writer.WriteLine(L"Changed)");
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
	FOREACH(WString, typeName, typeSchemas.Keys())
	{
		writer.WriteLine(prefix + L"\t\tADD_TYPE_INFO(" + typeName + L")");
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