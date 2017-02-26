#include "../GuiInstanceLoader.h"
#include "../GuiInstanceHelperTypes.h"
#include "../WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"
#include "../../Reflection/TypeDescriptors/GuiReflectionTemplates.h"

#ifndef VCZH_PRESENTATION_REFLECTION_INSTANCELOADERS_GUIINSTANCELOADER_TEMPLATECONTROL
#define VCZH_PRESENTATION_REFLECTION_INSTANCELOADERS_GUIINSTANCELOADER_TEMPLATECONTROL

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{
			using namespace collections;
			using namespace reflection::description;
			using namespace controls;
			using namespace compositions;
			using namespace theme;
			using namespace helper_types;
		
			using namespace elements;
			using namespace compositions;
			using namespace controls;
			using namespace templates;

			using namespace workflow;
			using namespace workflow::analyzer;

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
GuiVrtualTypeInstanceLoader
***********************************************************************/

			template<typename TControl, typename TControlStyle, typename TTemplate>
			class GuiTemplateControlInstanceLoader : public Object, public IGuiInstanceLoader
			{
				typedef Ptr<WfExpression>				ArgumentRawFunctionType(ArgumentMap&);
				typedef void							InitRawFunctionType(const WString&, Ptr<WfBlockStatement>);
				typedef Func<ArgumentRawFunctionType>	ArgumentFunctionType;
				typedef Func<InitRawFunctionType>		InitFunctionType;

			protected:
				GlobalStringKey								typeName;
				WString										styleMethod;
				ArgumentFunctionType						argumentFunction;
				InitFunctionType							initFunction;

				virtual void PrepareAdditionalArguments(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfBlockStatement> block)
				{
				}

				virtual void AddAdditionalArguments(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfNewClassExpression> createControl)
				{
				}

				virtual void PrepareAdditionalArgumentsAfterCreation(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfBlockStatement> block)
				{
				}
			public:

				static Ptr<WfExpression> CreateIThemeCall(const WString& method)
				{
					auto refPresentation = MakePtr<WfTopQualifiedExpression>();
					refPresentation->name.value = L"presentation";

					auto refTheme = MakePtr<WfChildExpression>();
					refTheme->parent = refPresentation;
					refTheme->name.value = L"theme";

					auto refITheme = MakePtr<WfChildExpression>();
					refITheme->parent = refTheme;
					refITheme->name.value = L"ITheme";

					auto refGetCurrentTheme = MakePtr<WfChildExpression>();
					refGetCurrentTheme->parent = refITheme;
					refGetCurrentTheme->name.value = L"GetCurrentTheme";

					auto call = MakePtr<WfCallExpression>();
					call->function = refGetCurrentTheme;

					auto refStyleMethod = MakePtr<WfMemberExpression>();
					refStyleMethod->parent = call;
					refStyleMethod->name.value = method;

					auto createStyle = MakePtr<WfCallExpression>();
					createStyle->function = refStyleMethod;
					return createStyle;
				}

				static Ptr<WfExpression> CreateStyleMethodArgument(const WString& method, ArgumentMap& arguments)
				{
					vint indexControlTemplate = arguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (indexControlTemplate != -1)
					{
						auto refControlStyle = MakePtr<WfReferenceExpression>();
						refControlStyle->name.value = L"<controlStyle>";

						auto refCreateArgument = MakePtr<WfMemberExpression>();
						refCreateArgument->parent = refControlStyle;
						refCreateArgument->name.value = L"CreateArgument";

						auto call = MakePtr<WfCallExpression>();
						call->function = refCreateArgument;

						return call;
					}
					else
					{
						return CreateIThemeCall(method);
					}
				}

				static Ptr<WfExpression> CreateTemplateFactory(types::ResolvingResult& resolvingResult, List<ITypeDescriptor*>& controlTemplateTds, parsing::ParsingTextPos attPosition, GuiResourceError::List& errors)
				{
					auto templateType = TypeInfoRetriver<TTemplate*>::CreateTypeInfo();
					auto factoryType = TypeInfoRetriver<Ptr<GuiTemplate::IFactory>>::CreateTypeInfo();

					auto refFactory = MakePtr<WfNewInterfaceExpression>();
					refFactory->type = GetTypeFromTypeInfo(factoryType.Obj());
					{
						auto funcCreateTemplate = MakePtr<WfFunctionDeclaration>();
						funcCreateTemplate->anonymity = WfFunctionAnonymity::Named;
						funcCreateTemplate->name.value = L"CreateTemplate";
						funcCreateTemplate->returnType = GetTypeFromTypeInfo(TypeInfoRetriver<GuiTemplate*>::CreateTypeInfo().Obj());

						auto argViewModel = MakePtr<WfFunctionArgument>();
						argViewModel->type = GetTypeFromTypeInfo(TypeInfoRetriver<Value>::CreateTypeInfo().Obj());
						argViewModel->name.value = L"<viewModel>";
						funcCreateTemplate->arguments.Add(argViewModel);

						auto block = MakePtr<WfBlockStatement>();
						funcCreateTemplate->statement = block;
						
						ITypeDescriptor* stopControlTemplateTd = nullptr;
						FOREACH(ITypeDescriptor*, controlTemplateTd, controlTemplateTds)
						{
							if (stopControlTemplateTd)
							{
								errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
									L"Precompile: Type \"" +
									controlTemplateTd->GetTypeName() +
									L"\" will never be tried, because \"" +
									stopControlTemplateTd->GetTypeName() +
									L"\", which is listed before, has a default constructor. So whatever the view model is, it will be the last choice."));
								continue;
							}

							ITypeInfo* viewModelType = nullptr;
							{
								auto ctors = controlTemplateTd->GetConstructorGroup();
								if (ctors->GetMethodCount() != 1)
								{
									errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
										L"Precompile: To use type \"" +
										controlTemplateTd->GetTypeName() +
										L"\" as a control template or item template, it should have exactly one constructor."));
									continue;
								}

								auto ctor = ctors->GetMethod(0);
								if (ctor->GetParameterCount() > 1)
								{
									errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
										L"Precompile: To use type \"" +
										controlTemplateTd->GetTypeName() +
										L"\" as a control template or item template, its constructor cannot have more than one parameter."));
									continue;
								}

								if (ctor->GetParameterCount() != 0)
								{
									viewModelType = ctor->GetParameter(0)->GetType();
								}
							}

							if (!viewModelType)
							{
								stopControlTemplateTd = controlTemplateTd;
							}

							auto subBlock = MakePtr<WfBlockStatement>();
							block->statements.Add(subBlock);

							Ptr<ITypeInfo> controlTemplateType;
							{
								auto elementType = MakePtr<TypeDescriptorTypeInfo>(controlTemplateTd, TypeInfoHint::Normal);
								auto pointerType = MakePtr<RawPtrTypeInfo>(elementType);

								controlTemplateType = pointerType;
							}

							Ptr<WfBlockStatement> returnStatBlock;
							if (viewModelType)
							{
								auto refViewModel = MakePtr<WfReferenceExpression>();
								refViewModel->name.value = L"<viewModel>";

								auto condition = MakePtr<WfTypeTestingExpression>();
								condition->test = WfTypeTesting::IsType;
								condition->expression = refViewModel;
								condition->type = GetTypeFromTypeInfo(viewModelType);

								auto ifStat = MakePtr<WfIfStatement>();
								subBlock->statements.Add(ifStat);
								ifStat->expression = condition;

								returnStatBlock = MakePtr<WfBlockStatement>();
								ifStat->trueBranch = returnStatBlock;
							}
							else
							{
								returnStatBlock = subBlock;
							}

							{
								auto createControlTemplate = MakePtr<WfNewClassExpression>();
								createControlTemplate->type = GetTypeFromTypeInfo(controlTemplateType.Obj());
								if (viewModelType)
								{
									auto refViewModel = MakePtr<WfReferenceExpression>();
									refViewModel->name.value = L"<viewModel>";

									auto cast = MakePtr<WfTypeCastingExpression>();
									cast->strategy = WfTypeCastingStrategy::Strong;
									cast->expression = refViewModel;
									cast->type = GetTypeFromTypeInfo(viewModelType);
									createControlTemplate->arguments.Add(cast);
								}

								auto varTemplate = MakePtr<WfVariableDeclaration>();
								varTemplate->type = GetTypeFromTypeInfo(templateType.Obj());
								varTemplate->name.value = L"<template>";
								varTemplate->expression = createControlTemplate;

								auto varStat = MakePtr<WfVariableStatement>();
								varStat->variable = varTemplate;
								returnStatBlock->statements.Add(varStat);
							}
							{
								auto refTemplate = MakePtr<WfReferenceExpression>();
								refTemplate->name.value = L"<template>";

								auto returnStat = MakePtr<WfReturnStatement>();
								returnStat->expression = refTemplate;
								returnStatBlock->statements.Add(returnStat);
							}
						}

						if (!stopControlTemplateTd)
						{
							auto value = MakePtr<WfStringExpression>();
							value->value.value = L"Cannot find a matched control template to create.";

							auto raiseStat = MakePtr<WfRaiseExceptionStatement>();
							raiseStat->expression = value;

							block->statements.Add(raiseStat);
						}

						auto member = MakePtr<WfClassMember>();
						member->kind = WfClassMemberKind::Override;
						member->declaration = funcCreateTemplate;

						refFactory->members.Add(member);
					}

					return refFactory;
				}

				static Ptr<WfExpression> CreateTemplateFactory(types::ResolvingResult& resolvingResult, ITypeDescriptor* controlTemplateTd, parsing::ParsingTextPos attPosition, GuiResourceError::List& errors)
				{
					List<ITypeDescriptor*> controlTemplateTds;
					controlTemplateTds.Add(controlTemplateTd);
					return CreateTemplateFactory(resolvingResult, controlTemplateTds, attPosition, errors);
				}

				static ITypeDescriptor* GetControlTemplateType(types::ResolvingResult& resolvingResult, Ptr<WfExpression> argument, const TypeInfo& controlTypeInfo, parsing::ParsingTextPos attPosition, GuiResourceError::List& errors)
				{
					auto controlTemplateNameExpr = argument.Cast<WfStringExpression>();
					if (!controlTemplateNameExpr)
					{
						errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
							L"[INTERNAL ERROR] Precompile: The value of contructor parameter \"" +
							GlobalStringKey::_ControlTemplate.ToString() +
							L"\" of type \"" +
							controlTypeInfo.typeName.ToString() +
							L"\" should be a constant representing the control template type name."));
						return nullptr;
					}

					auto controlTemplateName = controlTemplateNameExpr->value.value;
					if (wcschr(controlTemplateName.Buffer(), L';') != nullptr)
					{
						errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
							L"Precompile: \"" +
							controlTemplateNameExpr->value.value +
							L"\", which is assigned to contructor parameter \"" +
							GlobalStringKey::_ControlTemplate.ToString() +
							L" of type \"" +
							controlTypeInfo.typeName.ToString() +
							L"\", is illegal because control template should not have multiple choices."));
						return nullptr;
					}

					auto controlTemplateTd = description::GetTypeDescriptor(controlTemplateName);
					if (!controlTemplateTd)
					{
						errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
							L"Precompile: Type \"" +
							controlTemplateNameExpr->value.value +
							L"\", which is assigned to contructor parameter \"" +
							GlobalStringKey::_ControlTemplate.ToString() +
							L" of type \"" +
							controlTypeInfo.typeName.ToString() +
							L"\", does not exist."));
						return nullptr;
					}

					return controlTemplateTd;
				}

				static void GetItemTemplateType(types::ResolvingResult& resolvingResult, Ptr<WfExpression> argument, List<ITypeDescriptor*>& tds, const TypeInfo& controlTypeInfo, const WString& propertyName, parsing::ParsingTextPos attPosition, GuiResourceError::List& errors)
				{
					auto controlTemplateNameExpr = argument.Cast<WfStringExpression>();
					if (!controlTemplateNameExpr)
					{
						errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
							L"Precompile: The value of contructor parameter \"" +
							propertyName + L"\" of type \"" +
							controlTypeInfo.typeName.ToString() +
							L"\" should be a constant representing control template type names."));
						return;
					}

					List<WString> typeNames;
					SplitBySemicolon(controlTemplateNameExpr->value.value, typeNames);

					FOREACH(WString, controlTemplateName, typeNames)
					{
						auto controlTemplateTd = description::GetTypeDescriptor(controlTemplateName);
						if (!controlTemplateTd)
						{
							errors.Add(GuiResourceError(resolvingResult.resource, attPosition,
								L"Precompile: Type \"" +
								controlTemplateNameExpr->value.value +
								L"\", which is assigned to contructor parameter \"" + propertyName +
								L" of type \"" +
								controlTypeInfo.typeName.ToString() +
								L"\", does not exist."));
							continue;
						}
						tds.Add(controlTemplateTd);
					}
				}

			public:
				GuiTemplateControlInstanceLoader(const WString& _typeName, const WString& _styleMethod)
					:typeName(GlobalStringKey::Get(_typeName))
					, styleMethod(_styleMethod)
				{
				}

				GuiTemplateControlInstanceLoader(const WString& _typeName, const WString& _styleMethod, WString argumentStyleMethod)
					:typeName(GlobalStringKey::Get(_typeName))
					, styleMethod(_styleMethod)
					, argumentFunction([argumentStyleMethod](ArgumentMap& arguments){return CreateStyleMethodArgument(argumentStyleMethod, arguments);})
				{
				}

				GuiTemplateControlInstanceLoader(const WString& _typeName, const WString& _styleMethod, ArgumentRawFunctionType* _argumentFunction)
					:typeName(GlobalStringKey::Get(_typeName))
					, styleMethod(_styleMethod)
					, argumentFunction(_argumentFunction)
				{
				}

				GuiTemplateControlInstanceLoader(const WString& _typeName, const WString& _styleMethod, InitRawFunctionType* _initFunction)
					:typeName(GlobalStringKey::Get(_typeName))
					, styleMethod(_styleMethod)
					, initFunction(_initFunction)
				{
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::_ControlTemplate);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					return 0;
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeName == typeInfo.typeName;
				}

				Ptr<workflow::WfExpression> CreateInstance_ControlTemplate(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors)
				{
					Ptr<WfExpression> controlTemplate;
					parsing::ParsingTextPos attPosition;
					{
						auto index = arguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
						if (index != -1)
						{
							auto argument = arguments.GetByIndex(index)[0];
							controlTemplate = argument.expression;
							attPosition = argument.attPosition;
						}
					}

					if (controlTemplate)
					{
						if (auto controlTemplateTd = GetControlTemplateType(resolvingResult, controlTemplate, typeInfo, attPosition, errors))
						{
							auto styleType = TypeInfoRetriver<TControlStyle*>::CreateTypeInfo();

							auto refFactory = CreateTemplateFactory(resolvingResult, controlTemplateTd, attPosition, errors);
							auto createStyle = MakePtr<WfNewClassExpression>();
							createStyle->type = GetTypeFromTypeInfo(styleType.Obj());
							createStyle->arguments.Add(refFactory);
							
							return createStyle;
						}
						else
						{
							return nullptr;
						}
					}
					else
					{
						return CreateIThemeCall(styleMethod);
					}
				}

				Ptr<workflow::WfBaseConstructorCall> CreateRootInstance(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors)override
				{
					if (auto createStyleExpr = CreateInstance_ControlTemplate(resolvingResult, typeInfo, arguments, errors))
					{
						auto createControl = MakePtr<WfBaseConstructorCall>();
						createControl->type = GetTypeFromTypeInfo(TypeInfoRetriver<TControl>::CreateTypeInfo().Obj());
						createControl->arguments.Add(createStyleExpr);
						return createControl;
					}
					return nullptr;
				}

				Ptr<workflow::WfStatement> CreateInstance(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, parsing::ParsingTextPos tagPosition, GuiResourceError::List& errors)override
				{
					CHECK_ERROR(typeName == typeInfo.typeName, L"GuiTemplateControlInstanceLoader::CreateInstance# Wrong type info is provided.");
					vint indexControlTemplate = arguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);

					auto createStyleExpr = CreateInstance_ControlTemplate(resolvingResult, typeInfo, arguments, errors);
					if (!createStyleExpr)
					{
						return nullptr;
					}
				
					auto block = MakePtr<WfBlockStatement>();
					{
						auto varTemplate = MakePtr<WfVariableDeclaration>();
						varTemplate->name.value = L"<controlStyle>";
						varTemplate->expression = createStyleExpr;

						auto varStat = MakePtr<WfVariableStatement>();
						varStat->variable = varTemplate;
						block->statements.Add(varStat);
					}
					PrepareAdditionalArguments(resolvingResult, typeInfo, variableName, arguments, errors, block);
					{
						auto controlType = TypeInfoRetriver<TControl*>::CreateTypeInfo();

						auto createControl = MakePtr<WfNewClassExpression>();
						createControl->type = GetTypeFromTypeInfo(controlType.Obj());
						{
							auto refControlStyle = MakePtr<WfReferenceExpression>();
							refControlStyle->name.value = L"<controlStyle>";

							createControl->arguments.Add(refControlStyle);
						}

						if (argumentFunction)
						{
							createControl->arguments.Add(argumentFunction(arguments));
						}
						AddAdditionalArguments(resolvingResult, typeInfo, variableName, arguments, errors, createControl);

						auto refVariable = MakePtr<WfReferenceExpression>();
						refVariable->name.value = variableName.ToString();

						auto assignExpr = MakePtr<WfBinaryExpression>();
						assignExpr->op = WfBinaryOperator::Assign;
						assignExpr->first = refVariable;
						assignExpr->second = createControl;

						auto assignStat = MakePtr<WfExpressionStatement>();
						assignStat->expression = assignExpr;
						block->statements.Add(assignStat);
					}

					PrepareAdditionalArgumentsAfterCreation(resolvingResult, typeInfo, variableName, arguments, errors, block);
					if (initFunction)
					{
						initFunction(variableName.ToString(), block);
					}
					return block;
				}
			};
#endif
		}
	}
}
#endif
