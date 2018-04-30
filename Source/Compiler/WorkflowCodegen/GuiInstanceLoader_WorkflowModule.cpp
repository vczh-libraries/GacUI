#include "GuiInstanceLoader_WorkflowCodegen.h"

namespace vl
{
	namespace presentation
	{
		using namespace workflow;
		using namespace workflow::analyzer;
		using namespace reflection::description;
		using namespace collections;

/***********************************************************************
Workflow_CreateModuleWithUsings
***********************************************************************/

		Ptr<workflow::WfModule> Workflow_CreateModuleWithUsings(Ptr<GuiInstanceContext> context, const WString& moduleName)
		{
			auto module = MakePtr<WfModule>();
			module->name.value = moduleName;

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

/***********************************************************************
Workflow_InstallClass
***********************************************************************/

		Ptr<workflow::WfClassDeclaration> Workflow_InstallClass(const WString& className, Ptr<workflow::WfModule> module)
		{
			auto decls = &module->declarations;
			auto reading = className.Buffer();
			while (true)
			{
				auto delimiter = wcsstr(reading, L"::");
				if (delimiter)
				{
					auto ns = MakePtr<WfNamespaceDeclaration>();
					ns->name.value = WString(reading, delimiter - reading);
					decls->Add(ns);
					decls = &ns->declarations;
				}
				else
				{
					auto ctorClass = MakePtr<WfClassDeclaration>();
					ctorClass->kind = WfClassKind::Class;
					ctorClass->constructorType = WfConstructorType::Undefined;
					ctorClass->name.value = reading;
					decls->Add(ctorClass);
					return ctorClass;
				}
				reading = delimiter + 2;
			}
		}

/***********************************************************************
Workflow_InstallCtorClass
***********************************************************************/
		
		Ptr<workflow::WfBlockStatement> Workflow_InstallCtorClass(types::ResolvingResult& resolvingResult, Ptr<workflow::WfModule> module)
		{
			auto ctorClass = Workflow_InstallClass(resolvingResult.context->className + L"Constructor", module);
			Workflow_CreateVariablesForReferenceValues(ctorClass, resolvingResult);

			auto thisParam = MakePtr<WfFunctionArgument>();
			thisParam->name.value = L"<this>";
			thisParam->type = GetTypeFromTypeInfo(resolvingResult.rootTypeInfo.typeInfo.Obj());

			auto block = MakePtr<WfBlockStatement>();

			auto func = MakePtr<WfFunctionDeclaration>();
			func->anonymity = WfFunctionAnonymity::Named;
			func->arguments.Add(thisParam);
			func->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<void>::CreateTypeInfo().Obj());
			func->statement = block;

			{
				List<WString> fragments;
				SplitTypeName(resolvingResult.context->className, fragments);
				func->name.value = L"<" + From(fragments).Aggregate([](const WString& a, const WString& b) {return a + L"-" + b; }) + L">Initialize";
			}
			{
				auto att = MakePtr<WfAttribute>();
				att->category.value = L"cpp";
				att->name.value = L"Protected";
				func->attributes.Add(att);
			}

			func->classMember = MakePtr<WfClassMember>();
			func->classMember->kind = WfClassMemberKind::Normal;
			ctorClass->declarations.Add(func);

			return block;
		}

/***********************************************************************
Variable
***********************************************************************/

		void Workflow_CreatePointerVariable(Ptr<workflow::WfClassDeclaration> ctorClass, GlobalStringKey name, description::ITypeInfo* typeInfo)
		{
			auto var = MakePtr<WfVariableDeclaration>();
			var->name.value = name.ToString();
			var->type = GetTypeFromTypeInfo(typeInfo);

			{
				auto att = MakePtr<WfAttribute>();
				att->category.value = L"cpp";
				att->name.value = L"Protected";
				var->attributes.Add(att);
			}

			if (!var->type)
			{
				if (auto ctors = typeInfo->GetTypeDescriptor()->GetConstructorGroup())
				{
					if (ctors->GetMethodCount() > 0)
					{
						auto ctor = ctors->GetMethod(0);
						var->type = GetTypeFromTypeInfo(ctor->GetReturn());
					}
				}
			}

			var->expression = CreateDefaultValue(typeInfo);

			var->classMember = MakePtr<WfClassMember>();
			var->classMember->kind = WfClassMemberKind::Normal;
			ctorClass->declarations.Add(var);
		}
		
		void Workflow_CreateVariablesForReferenceValues(Ptr<workflow::WfClassDeclaration> ctorClass, types::ResolvingResult& resolvingResult)
		{
			const auto& typeInfos = resolvingResult.typeInfos;
			for (vint i = 0; i < typeInfos.Count(); i++)
			{
				auto key = typeInfos.Keys()[i];
				auto value = typeInfos.Values()[i].typeInfo.Obj();
				Workflow_CreatePointerVariable(ctorClass, key, value);
			}
		}
	}
}