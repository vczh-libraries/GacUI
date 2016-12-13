#include "GacGen.h"

/***********************************************************************
CppTypeTransformation
***********************************************************************/

bool DetermineCppType(
	Ptr<CodegenConfig> config,
	Dictionary<WString, Ptr<Instance>>& instances,
	Ptr<Instance> instance,
	GuiConstructorRepr* ctor,
	GlobalStringKey& typeName,
	IGuiInstanceLoader*& loader,
	const Func<bool()>& stop
	)
{
	Ptr<GuiResourcePathResolver> resolver = new GuiResourcePathResolver(config->resource, config->resource->GetWorkingDirectory());
	Ptr<GuiInstanceContext> currentContext = instance->context;
	GuiConstructorRepr* currentCtor = ctor;
	typeName = GlobalStringKey::Empty;
	loader = 0;

	while (currentContext)
	{
		auto loadingSource = FindInstanceLoadingSource(currentContext, currentCtor);
		currentContext = loadingSource.context;
		currentCtor = currentContext ? currentContext->instance.Obj() : 0;
		typeName = loadingSource.typeName;
		loader = loadingSource.loader;

		if (stop())
		{
			return true;
		}
	}

	return false;
}

WString GetCppTypeName(ITypeDescriptor* typeDescriptor)
{
	if (typeDescriptor == description::GetTypeDescriptor<void>())
	{
		return L"void";
	}
	else
	{
		auto cppType = CppGetFullName(typeDescriptor);
		return cppType == L"" ? typeDescriptor->GetTypeName() : cppType;
	}
}

WString GetCppTypeName(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances, Ptr<Instance> instance, GuiConstructorRepr* ctor)
{
	GlobalStringKey typeName;
	IGuiInstanceLoader* loader = 0;
	if (DetermineCppType(config, instances, instance, ctor, typeName, loader, [&]()
		{
			return instances.Keys().Contains(typeName.ToString());
		}))
	{
		return typeName.ToString();
	}
	else
	{
		auto manager = GetInstanceLoaderManager();
		return GetCppTypeName(manager->GetTypeDescriptorForType(typeName));
	}
}

IGuiInstanceLoader::TypeInfo GetCppTypeInfo(Ptr<CodegenConfig> config, Dictionary<WString, Ptr<Instance>>& instances, Ptr<Instance> instance, GuiConstructorRepr* ctor)
{
	GlobalStringKey typeName;
	IGuiInstanceLoader* loader = 0;
	DetermineCppType(config, instances, instance, ctor, typeName, loader, [&]()
	{
		return loader != 0;
	});
	auto manager = GetInstanceLoaderManager();
	ITypeDescriptor* typeDescriptor = manager->GetTypeDescriptorForType(typeName);
	return IGuiInstanceLoader::TypeInfo(typeName, typeDescriptor);
}

/***********************************************************************
WorkflowTypeTransformation
***********************************************************************/

void PrintAndClearWorkflowErrors(Ptr<CodegenConfig> config, const WString& title)
{
	PrintErrorMessage(title);
	FOREACH(Ptr<ParsingError>, error, config->workflowManager->errors)
	{
		PrintErrorMessage(error->errorMessage);
	}
	config->workflowManager->errors.Clear();
}

Ptr<WfType> ParseWorkflowType(Ptr<CodegenConfig> config, const WString& workflowType)
{
	if (!config->workflowTable)
	{
		config->workflowTable = WfLoadTable();
	}
	if (!config->workflowManager)
	{
		List<WString> errors;
		WString contextCode = L"<Instance><Window/></Instance>";

		auto xmlParser = GetParserManager()->GetParser<XmlDocument>(L"XML");
		auto xml = xmlParser->TypedParse(contextCode, errors);
		auto context = GuiInstanceContext::LoadFromXml(xml, errors);

		if (errors.Count())
		{
			FOREACH(WString, error, errors)
			{
				PrintErrorMessage(L"Internal error: " + error);
			}
		}

		auto module = Workflow_CreateModuleWithUsings(context);

		config->workflowManager = new WfLexicalScopeManager(config->workflowTable);
		config->workflowManager->AddModule(module);
		config->workflowManager->Rebuild(true);
	}

	auto type = WfParseType(workflowType, config->workflowTable);
	if (!type)
	{
		PrintErrorMessage(L"Failed to parse workflow type: \"" + workflowType + L"\".");
		return nullptr;
	}

	auto module = config->workflowManager->GetModules()[0];
	auto scope = config->workflowManager->nodeScopes[module.Obj()];
	analyzer::ValidateTypeStructure(config->workflowManager.Obj(), type, ValidateTypeStragety::ReturnType);
	if (config->workflowManager->errors.Count() > 0)
	{
		PrintAndClearWorkflowErrors(config, L"Invalid workflow type: \"" + workflowType + L"\".");
		return nullptr;
	}

	return type;
}

Ptr<ITypeInfo> GetTypeInfoFromWorkflowType(Ptr<CodegenConfig> config, const WString& workflowType)
{
	auto type = ParseWorkflowType(config, workflowType);
	if (!type)
	{
		return nullptr;
	}
	
	auto module = config->workflowManager->GetModules()[0];
	auto scope = config->workflowManager->nodeScopes[module.Obj()];
	auto typeInfo = CreateTypeInfoFromType(scope.Obj(), type);
	if (config->workflowManager->errors.Count() > 0)
	{
		PrintAndClearWorkflowErrors(config, L"Invalid workflow type: \"" + workflowType + L"\".");
		return nullptr;
	}
	return typeInfo;
}

WString GetCppTypeFromTypeInfo(ITypeInfo* typeInfo)
{
	switch (typeInfo->GetDecorator())
	{
	case ITypeInfo::RawPtr:
		return GetCppTypeFromTypeInfo(typeInfo->GetElementType()) + L"*";
	case ITypeInfo::SharedPtr:
		if (typeInfo->GetElementType()->GetDecorator() == ITypeInfo::Generic)
		{
			auto type = GetCppTypeFromTypeInfo(typeInfo->GetElementType());
			if (type != L"")
			{
				return type;
			}
		}
		return L"vl::Ptr<" + GetCppTypeFromTypeInfo(typeInfo->GetElementType()) + L">";
	case ITypeInfo::Nullable:
		return L"vl::Nullable" + GetCppTypeFromTypeInfo(typeInfo->GetElementType()) + L">";
	case ITypeInfo::Generic:
		{
			auto td = typeInfo->GetTypeDescriptor();
			if (td == description::GetTypeDescriptor<IValueEnumerable>())
			{
				return L"vl::collections::LazyList<" + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(0)) + L">";
			}
			else if (td == description::GetTypeDescriptor<IValueReadonlyList>())
			{
				return L"const vl::collections::List<" + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(0)) + L">&";
			}
			else if (td == description::GetTypeDescriptor<IValueList>())
			{
				return L"vl::collections::List<" + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(0)) + L">&";
			}
			else if (td == description::GetTypeDescriptor<IValueReadonlyDictionary>())
			{
				return L"const vl::collections::Dictionary<" + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(0)) + L"," + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(1)) + L">&";
			}
			else if (td == description::GetTypeDescriptor<IValueDictionary>())
			{
				return L"vl::collections::Dictionary<" + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(0)) + L"," + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(1)) + L">&";
			}
			else if (td == description::GetTypeDescriptor<IValueFunctionProxy>())
			{
				WString result = L"vl::Func<" + GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(0)) + L"(";
				for (vint i = 1; i < typeInfo->GetGenericArgumentCount(); i++)
				{
					if (i>1)result += L", ";
					result += GetCppTypeFromTypeInfo(typeInfo->GetGenericArgument(i));
				}
				result += L")>";
				return result;
			}
		}
		return L"";
	default:
		return GetCppTypeName(typeInfo->GetTypeDescriptor());
	}
}

WString GetCppTypeNameFromWorkflowType(Ptr<CodegenConfig> config, const WString& workflowType)
{
	auto typeInfo = GetTypeInfoFromWorkflowType(config, workflowType);
	if (!typeInfo)
	{
		return L"vl::vint";
	}
	return GetCppTypeFromTypeInfo(typeInfo.Obj());
}