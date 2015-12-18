#include "GuiInstanceLoader_WorkflowCodegen.h"
#include "../TypeDescriptors/GuiReflectionEvents.h"
#include "../../Resources/GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace reflection::description;
		using namespace collections;

/***********************************************************************
Workflow_CreateEmptyModule
***********************************************************************/

		Ptr<workflow::WfModule> Workflow_CreateEmptyModule(Ptr<GuiInstanceContext> context)
		{
			auto module = MakePtr<WfModule>();
			vint index = context->namespaces.Keys().IndexOf(GlobalStringKey());
			if (index != -1)
			{
				auto nss = context->namespaces.Values()[index];
				FOREACH(Ptr<GuiInstanceNamespace>, ns, nss->namespaces)
				{
					auto path = MakePtr<WfModuleUsingPath>();
					module->paths.Add(path);

					auto pathCode = ns->prefix + L"*" + ns->postfix;
					auto reading = pathCode.Buffer();
					while (reading)
					{
						auto delimiter = wcsstr(reading, L"::");
						auto begin = reading;
						auto end = delimiter ? delimiter : begin + wcslen(reading);

						auto wildcard = wcschr(reading, L'*');
						if (wildcard >= end)
						{
							wildcard = nullptr;
						}

						auto item = MakePtr<WfModuleUsingItem>();
						path->items.Add(item);
						if (wildcard)
						{
							if (begin < wildcard)
							{
								auto fragment = MakePtr<WfModuleUsingNameFragment>();
								item->fragments.Add(fragment);
								fragment->name.value = WString(begin, vint(wildcard - begin));
							}
							{
								auto fragment = MakePtr<WfModuleUsingWildCardFragment>();
								item->fragments.Add(fragment);
							}
							if (wildcard + 1 < end)
							{
								auto fragment = MakePtr<WfModuleUsingNameFragment>();
								item->fragments.Add(fragment);
								fragment->name.value = WString(wildcard + 1, vint(end - wildcard - 1));
							}
						}
						else if (begin < end)
						{
							auto fragment = MakePtr<WfModuleUsingNameFragment>();
							item->fragments.Add(fragment);
							fragment->name.value = WString(begin, vint(end - begin));
						}

						if (delimiter)
						{
							reading = delimiter + 2;
						}
						else
						{
							reading = nullptr;
						}
					}
				}
			}
			return module;
		}
		
		Ptr<workflow::WfModule> Workflow_CreateModuleWithInitFunction(Ptr<GuiInstanceContext> context, types::VariableTypeInfoMap& typeInfos, description::ITypeDescriptor* rootTypeDescriptor, Ptr<workflow::WfStatement> functionBody)
		{
			auto module = Workflow_CreateEmptyModule(context);
			Workflow_CreateVariablesForReferenceValues(module, typeInfos);

			auto thisParam = MakePtr<WfFunctionArgument>();
			thisParam->name.value = L"<this>";
			{
				Ptr<TypeInfoImpl> elementType = new TypeInfoImpl(ITypeInfo::TypeDescriptor);
				elementType->SetTypeDescriptor(rootTypeDescriptor);

				Ptr<TypeInfoImpl> pointerType = new TypeInfoImpl(ITypeInfo::RawPtr);
				pointerType->SetElementType(elementType);

				thisParam->type = GetTypeFromTypeInfo(pointerType.Obj());
			}

			auto resolverParam = MakePtr<WfFunctionArgument>();
			resolverParam->name.value = L"<resolver>";
			{
				Ptr<TypeInfoImpl> elementType = new TypeInfoImpl(ITypeInfo::TypeDescriptor);
				elementType->SetTypeDescriptor(description::GetTypeDescriptor<GuiResourcePathResolver>());

				Ptr<TypeInfoImpl> pointerType = new TypeInfoImpl(ITypeInfo::RawPtr);
				pointerType->SetElementType(elementType);

				resolverParam->type = GetTypeFromTypeInfo(pointerType.Obj());
			}

			auto func = MakePtr<WfFunctionDeclaration>();
			func->anonymity = WfFunctionAnonymity::Named;
			func->name.value = L"<initialize-instance>";
			func->arguments.Add(thisParam);
			func->arguments.Add(resolverParam);
			func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());
			func->statement = functionBody;
			module->declarations.Add(func);

			return module;
		}

/***********************************************************************
Variable
***********************************************************************/

		void Workflow_CreatePointerVariable(Ptr<workflow::WfModule> module, GlobalStringKey name, description::ITypeDescriptor* type)
		{
			auto var = MakePtr<WfVariableDeclaration>();
			var->name.value = name.ToString();
			{
				Ptr<TypeInfoImpl> elementType = new TypeInfoImpl(ITypeInfo::TypeDescriptor);
				elementType->SetTypeDescriptor(type);

				Ptr<TypeInfoImpl> pointerType = new TypeInfoImpl(ITypeInfo::RawPtr);
				pointerType->SetElementType(elementType);

				var->type = GetTypeFromTypeInfo(pointerType.Obj());
			}

			auto literal = MakePtr<WfLiteralExpression>();
			literal->value = WfLiteralValue::Null;
			var->expression = literal;

			module->declarations.Add(var);
		}
		
		void Workflow_CreateVariablesForReferenceValues(Ptr<workflow::WfModule> module, types::VariableTypeInfoMap& typeInfos)
		{
			for (vint i = 0; i < typeInfos.Count(); i++)
			{
				auto key = typeInfos.Keys()[i];
				auto value = typeInfos.Values()[i].typeDescriptor;
				Workflow_CreatePointerVariable(module, key, value);
			}
		}
	}
}