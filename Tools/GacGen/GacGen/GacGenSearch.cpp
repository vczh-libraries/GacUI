#include "GacGen.h"

/***********************************************************************
SearchAllFields
***********************************************************************/

class SearchAllFieldsVisitor : public Object, public GuiValueRepr::IVisitor
{
protected:
	Ptr<GuiInstanceEnvironment>						env;
	Dictionary<WString, GuiConstructorRepr*>&		fields;
	IGuiInstanceLoader::TypeInfo					typeInfo;

public:
	SearchAllFieldsVisitor(Ptr<GuiInstanceEnvironment> _env, Dictionary<WString, GuiConstructorRepr*>& _fields)
		:env(_env)
		, fields(_fields)
	{
	}

	void Visit(GuiTextRepr* repr)
	{
	}

	void Visit(GuiAttSetterRepr* repr)
	{
		FOREACH(Ptr<GuiAttSetterRepr::SetterValue>, setterValue, repr->setters.Values())
		{
			FOREACH(Ptr<GuiValueRepr>, value, setterValue->values)
			{
				value->Accept(this);
			}
		}
	}

	void Visit(GuiConstructorRepr* repr)
	{
		if (repr->instanceName != GlobalStringKey::Empty && !fields.Keys().Contains(repr->instanceName.ToString()))
		{
			auto loadingSource = FindInstanceLoadingSource(env->context, repr);
			
			auto name = repr->instanceName.ToString();
			if (name.Length() > 0 && name[0] != L'<')
			{
				fields.Add(repr->instanceName.ToString(), repr);
			}
		}
		Visit((GuiAttSetterRepr*)repr);
	}
};

void SearchAllFields(Ptr<GuiInstanceEnvironment> env, Ptr<GuiInstanceContext> context, Dictionary<WString, GuiConstructorRepr*>& fields)
{
	SearchAllFieldsVisitor visitor(env, fields);
	context->instance->Accept(&visitor);
}

/***********************************************************************
SearchAllSchemas
***********************************************************************/

void SearchAllSchemas(const Regex& regexClassName, Ptr<GuiResourceFolder> folder, Dictionary<WString, Ptr<InstanceSchema>>& typeSchemas, List<WString>& typeSchemaOrder)
{
	FOREACH(Ptr<GuiResourceItem>, item, folder->GetItems())
	{
		auto schema = item->GetContent().Cast<GuiInstanceSchema>();
		if (!schema) continue;

		FOREACH(Ptr<GuiInstanceTypeSchema>, typeSchema, schema->schemas)
		{
			if (typeSchemas.Keys().Contains(typeSchema->typeName)) continue;
			auto match = regexClassName.MatchHead(typeSchema->typeName);
			if (!match)
			{
				PrintErrorMessage(L"Skip code generation for \"" + typeSchema->typeName + L"\" because this type name is illegal.");
				continue;
			}

			auto instance = MakePtr<InstanceSchema>();
			if (match->Groups().Contains(L"namespace"))
			{
				CopyFrom(
					instance->namespaces,
					From(match->Groups()[L"namespace"])
						.Select([](const RegexString& str)->WString
						{
							return str.Value();
						})
					);
			}
			instance->typeName = match->Groups()[L"type"][0].Value();
			instance->schema = typeSchema;
			typeSchemaOrder.Add(typeSchema->typeName);
			typeSchemas.Add(typeSchema->typeName, instance);
		}
	}
	FOREACH(Ptr<GuiResourceFolder>, subFolder, folder->GetFolders())
	{
		SearchAllSchemas(regexClassName, subFolder, typeSchemas, typeSchemaOrder);
	}
}

/***********************************************************************
SearchAllInstances
***********************************************************************/

void SearchAllInstances(const Regex& regexClassName, Ptr<GuiResourcePathResolver> resolver, Ptr<GuiResourceFolder> folder, Dictionary<WString, Ptr<Instance>>& instances)
{
	FOREACH(Ptr<GuiResourceItem>, item, folder->GetItems())
	{
		auto context = item->GetContent().Cast<GuiInstanceContext>();
		if (!context) continue;
		if (!context->className) continue;
		if (instances.Keys().Contains(context->className.Value())) continue;
		auto match = regexClassName.MatchHead(context->className.Value());
		if (!match)
		{
			PrintErrorMessage(L"Skip code generation for \"" + context->className.Value() + L"\" because this type name is illegal.");
			continue;
		}

		Ptr<GuiInstanceEnvironment> env = new GuiInstanceEnvironment(context, resolver);
		auto loadingSource = FindInstanceLoadingSource(env->context, context->instance.Obj());
		if (!loadingSource.loader) continue;
		auto typeDescriptor = GetGlobalTypeManager()->GetTypeDescriptor(loadingSource.typeName.ToString());
		if (!typeDescriptor) continue;

		Ptr<Instance> instance = new Instance;
		instance->context = context;
		instance->baseType = typeDescriptor;
		if (match->Groups().Contains(L"namespace"))
		{
			CopyFrom(
				instance->namespaces,
				From(match->Groups()[L"namespace"])
					.Select([](const RegexString& str)->WString
					{
						return str.Value();
					})
				);
		}
		instance->typeName = match->Groups()[L"type"][0].Value();
		SearchAllFields(env, context, instance->fields);

		instances.Add(context->className.Value(), instance);
	}
	FOREACH(Ptr<GuiResourceFolder>, subFolder, folder->GetFolders())
	{
		SearchAllInstances(regexClassName, resolver, subFolder, instances);
	}
}

/***********************************************************************
SearchAllEventHandlers
***********************************************************************/

class SearchAllEventHandlersVisitor : public Object, public GuiValueRepr::IVisitor
{
protected:
	Ptr<CodegenConfig>								config;
	Dictionary<WString, Ptr<Instance>>&				instances;
	Ptr<Instance>									instance;
	Ptr<GuiInstanceEnvironment>						env;
	Dictionary<WString, ITypeDescriptor*>&			eventHandlers;
	IGuiInstanceLoader::TypeInfo					typeInfo;

public:
	SearchAllEventHandlersVisitor(Ptr<CodegenConfig> _config, Dictionary<WString, Ptr<Instance>>& _instances, Ptr<Instance> _instance, Ptr<GuiInstanceEnvironment> _env, Dictionary<WString, ITypeDescriptor*>& _eventHandlers)
		:config(_config)
		, instances(_instances)
		, instance(_instance)
		, env(_env)
		, eventHandlers(_eventHandlers)
	{
	}

	static Ptr<GuiInstancePropertyInfo> GetPropertyInfo(const IGuiInstanceLoader::TypeInfo& typeInfo, GlobalStringKey name, IGuiInstanceLoader*& loader)
	{
		loader = GetInstanceLoaderManager()->GetLoader(typeInfo.typeName);
		auto propertyInfo = IGuiInstanceLoader::PropertyInfo(typeInfo, name);
		while (loader)
		{
			if (auto info = loader->GetPropertyType(propertyInfo))
			{
				if (info->support == GuiInstancePropertyInfo::NotSupport)
				{
					return 0;
				}
				else if (info->acceptableTypes.Count() > 0)
				{
					return info;
				}
			}
			loader = GetInstanceLoaderManager()->GetParentLoader(loader);
		}
		return 0;
	}

	static Ptr<GuiInstanceEventInfo> GetEventInfo(const IGuiInstanceLoader::TypeInfo& typeInfo, GlobalStringKey name, IGuiInstanceLoader*& loader)
	{
		loader = GetInstanceLoaderManager()->GetLoader(typeInfo.typeName);
		auto propertyInfo = IGuiInstanceLoader::PropertyInfo(typeInfo, name);
		while (loader)
		{
			if (auto info = loader->GetEventType(propertyInfo))
			{
				if (info->support == GuiInstanceEventInfo::NotSupport)
				{
					return 0;
				}
				else
				{
					return info;
				}
			}
			loader = GetInstanceLoaderManager()->GetParentLoader(loader);
		}
		return 0;
	}

	void Visit(GuiTextRepr* repr)
	{
	}

	void Visit(GuiAttSetterRepr* repr)
	{
		FOREACH_INDEXER(GlobalStringKey, eventName, index, repr->eventHandlers.Keys())
		{
			auto handler = repr->eventHandlers.Values()[index];
			if (handler->binding == GlobalStringKey::Empty)
			{
				if (!eventHandlers.Keys().Contains(eventName.ToString()))
				{
					IGuiInstanceLoader* loader = 0;
					if (auto info = GetEventInfo(typeInfo, eventName, loader))
					{
						eventHandlers.Add(handler->value, info->argumentType);
					}
				}
			}
		}

		auto oldTypeInfo = typeInfo;
		FOREACH(GlobalStringKey, propertyName, repr->setters.Keys())
		{
			auto setterValue = repr->setters[propertyName];
			if (setterValue->binding == GlobalStringKey::_Set)
			{
				int a = 0;
			}
			IGuiInstanceLoader* loader = 0;
			if (auto info = GetPropertyInfo(typeInfo, propertyName, loader))
			{
				typeInfo = IGuiInstanceLoader::TypeInfo(GlobalStringKey::Get(info->acceptableTypes[0]->GetTypeName()), info->acceptableTypes[0]);
				FOREACH(Ptr<GuiValueRepr>, value, setterValue->values)
				{
					value->Accept(this);
				}
				typeInfo = oldTypeInfo;
			}
		}
		typeInfo = oldTypeInfo;
	}

	void Visit(GuiConstructorRepr* repr)
	{
		auto oldTypeInfo = typeInfo;
		typeInfo = GetCppTypeInfo(config, instances, instance, repr);
		Visit((GuiAttSetterRepr*)repr);
		typeInfo = oldTypeInfo;
	}
};

void SearchAllEventHandlers(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances, Ptr<Instance> instance, Ptr<GuiInstanceEnvironment> env, Dictionary<WString, ITypeDescriptor*>& eventHandlers)
{
	SearchAllEventHandlersVisitor visitor(config, instances, instance, env, eventHandlers);
	instance->context->instance->Accept(&visitor);
}