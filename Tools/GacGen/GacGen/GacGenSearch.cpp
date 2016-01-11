#include "GacGen.h"

/***********************************************************************
SearchAllFields
***********************************************************************/

class SearchAllFieldsVisitor : public Object, public GuiValueRepr::IVisitor
{
protected:
	Ptr<GuiInstanceContext>							context;
	Dictionary<WString, GuiConstructorRepr*>&		fields;
	IGuiInstanceLoader::TypeInfo					typeInfo;

public:
	SearchAllFieldsVisitor(Ptr<GuiInstanceContext> _context, Dictionary<WString, GuiConstructorRepr*>& _fields)
		:context(_context)
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
			auto loadingSource = FindInstanceLoadingSource(context, repr);
			
			auto name = repr->instanceName.ToString();
			if (name.Length() > 0 && name[0] != L'<')
			{
				fields.Add(repr->instanceName.ToString(), repr);
			}
		}
		Visit((GuiAttSetterRepr*)repr);
	}
};

void SearchAllFields(Ptr<GuiInstanceContext> context, Dictionary<WString, GuiConstructorRepr*>& fields)
{
	SearchAllFieldsVisitor visitor(context, fields);
	context->instance->Accept(&visitor);
}

/***********************************************************************
SearchAllSchemas
***********************************************************************/

void SearchAllSchemas(const Regex& regexClassName, Ptr<GuiResourceFolder> folder, List<WString>& schemaPaths, List<Ptr<GuiInstanceSharedScript>>& schemas)
{
	FOREACH(Ptr<GuiResourceItem>, item, folder->GetItems())
	{
		if (auto schema = item->GetContent().Cast<GuiInstanceSharedScript>())
		{
			if (schema->language == L"Workflow-ViewModel")
			{
				schemaPaths.Add(item->GetResourcePath());
				schemas.Add(schema);
			}
		}
	}
	FOREACH(Ptr<GuiResourceFolder>, subFolder, folder->GetFolders())
	{
		SearchAllSchemas(regexClassName, subFolder, schemaPaths, schemas);
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
		if (instances.Keys().Contains(context->className)) continue;
		auto match = regexClassName.MatchHead(context->className);
		if (!match)
		{
			PrintErrorMessage(L"Skip code generation for \"" + context->className + L"\" because this type name is illegal.");
			continue;
		}

		auto loadingSource = FindInstanceLoadingSource(context, context->instance.Obj());
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
		SearchAllFields(context, instance->fields);

		instances.Add(context->className, instance);
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
	Dictionary<WString, EventHandlerInfo>&			eventHandlers;
	IGuiInstanceLoader::TypeInfo					typeInfo;

public:
	SearchAllEventHandlersVisitor(Ptr<CodegenConfig> _config, Dictionary<WString, Ptr<Instance>>& _instances, Ptr<Instance> _instance, Dictionary<WString, EventHandlerInfo>& _eventHandlers)
		:config(_config)
		, instances(_instances)
		, instance(_instance)
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

	static Ptr<EventHandlerInfo> GetEventInfo(const IGuiInstanceLoader::TypeInfo& typeInfo, GlobalStringKey name)
	{
		auto eventInfo = typeInfo.typeDescriptor->GetEventByName(name.ToString(), true);
		if (!eventInfo)
		{
			return nullptr;
		}

		auto funcPtr = eventInfo->GetHandlerType();
		if (funcPtr->GetDecorator() != ITypeInfo::SharedPtr)
		{
			return nullptr;
		}

		auto funcType = funcPtr->GetElementType();
		if (funcType->GetGenericArgumentCount() != 3)
		{
			return nullptr;
		}

		auto senderType = funcType->GetGenericArgument(1);
		if (senderType->GetDecorator() != ITypeInfo::RawPtr && senderType->GetTypeDescriptor() != description::GetTypeDescriptor<GuiGraphicsComposition>())
		{
			return nullptr;
		}

		auto argumentType = funcType->GetGenericArgument(2);
		if (argumentType->GetDecorator() != ITypeInfo::RawPtr && !argumentType->GetTypeDescriptor()->CanConvertTo(description::GetTypeDescriptor<GuiEventArgs>()))
		{
			return nullptr;
		}

		auto info = MakePtr<EventHandlerInfo>();
		info->eventInfo = eventInfo;
		info->argumentType = argumentType->GetTypeDescriptor();
		return info;
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
					if (auto info = GetEventInfo(typeInfo, eventName))
					{
						eventHandlers.Add(handler->value, *info.Obj());
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

void SearchAllEventHandlers(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances, Ptr<Instance> instance, Dictionary<WString, EventHandlerInfo>& eventHandlers)
{
	SearchAllEventHandlersVisitor visitor(config, instances, instance, eventHandlers);
	instance->context->instance->Accept(&visitor);
}