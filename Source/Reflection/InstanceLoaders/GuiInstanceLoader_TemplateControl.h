#include "../GuiInstanceLoader.h"
#include "../GuiInstanceHelperTypes.h"
#include "../TypeDescriptors/GuiReflectionTemplates.h"

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
				using ArgumentRawFunctionType	= Ptr<WfExpression>();
				using InitRawFunctionType		= void(const WString&, Ptr<WfBlockStatement>);
				using ArgumentFunctionType		= Func<ArgumentRawFunctionType>;
				using InitFunctionType			= Func<InitRawFunctionType>;
			protected:
				GlobalStringKey								typeName;
				WString										styleMethod;
				ArgumentFunctionType						argumentFunction;
				InitFunctionType							initFunction;

			public:

				static Ptr<WfExpression> CreateIThemeCall(const WString& method)
				{
					auto refPresentation = MakePtr<WfTopQualifiedExpression>();
					refPresentation->name.value = L"presentation";

					auto refTheme = MakePtr<WfMemberExpression>();
					refTheme->parent = refPresentation;
					refTheme->name.value = L"theme";

					auto refITheme = MakePtr<WfMemberExpression>();
					refITheme->parent = refTheme;
					refITheme->name.value = L"ITheme";

					auto refStyleMethod = MakePtr<WfMemberExpression>();
					refStyleMethod->parent = refITheme;
					refStyleMethod->name.value = method;

					auto createStyle = MakePtr<WfCallExpression>();
					createStyle->function = refStyleMethod;
					return createStyle;
				}

				static Ptr<WfExpression> CreateStyleMethodArgument(const WString& method)
				{
					if (indexControlTemplate == -1)
					{
						auto refControlStyle = MakePtr<WfReferenceExpression>();
						refControlStyle->name.value = L"<controlStyle>";

						auto refCreateArgument = MakePtr<WfMemberExpression>();
						refCreateArgument->parent = refControlStyle;
						refCreateArgument->name.value = L"CreateArgument";

						auto call = MakePtr<WfCallExpression>();
						call->function = refCreateArgument;

						createControl->arguments.Add(call);
					}
					else
					{
						createControl->arguments.Add(CreateIThemeCall(method));
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
					, argumentFunction([argumentStyleMethod](){return CreateStyleMethodArgument(argumentStyleMethod);})
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

				Ptr<workflow::WfStatement> CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					CHECK_ERROR(typeName == typeInfo.typeName, L"GuiTemplateControlInstanceLoader::CreateInstance# Wrong type info is provided.");
					vint indexControlTemplate = arguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);

					Ptr<WfExpression> createStyleExpr;
					if (indexControlTemplate == -1)
					{
						createStyleExpr = CreateIThemeCall(styleMethod);
					}
					else
					{
						auto controlTemplateNameExpr = arguments.GetByIndex(indexControlTemplate)[0].expression.Cast<WfStringExpression>();
						if (!controlTemplateNameExpr)
						{
							errors.Add(L"Precompile: The value of contructor parameter \"" + GlobalStringKey::_ControlTemplate.ToString() + L" of type \"" + typeInfo.typeName.ToString() + L"\" should be a constant representing the control template type name.");
							return nullptr;
						}

						auto controlTemplateTd = description::GetTypeDescriptor(controlTemplateNameExpr->value.value);
						if (!controlTemplateTd)
						{
							errors.Add(L"Precompile: Type \"" + controlTemplateNameExpr->value.value + L", which is assigned to contructor parameter \"" + GlobalStringKey::_ControlTemplate.ToString() + L" of type \"" + typeInfo.typeName.ToString() + L"\", does not exist.");
							return nullptr;
						}

						Ptr<ITypeInfo> controlTemplateType;
						{
							auto elementType = MakePtr<TypeInfoImpl>(ITypeInfo::TypeDescriptor);
							elementType->SetTypeDescriptor(controlTemplateTd);

							auto pointerType = MakePtr<TypeInfoImpl>(ITypeInfo::RawPtr);
							pointerType->SetElementType(elementType);

							controlTemplateType = pointerType;
						}
						auto factoryType = TypeInfoRetriver<Ptr<GuiTemplate::IFactory>>::CreateTypeInfo();
						auto templateType = TypeInfoRetriver<GuiTemplate*>::CreateTypeInfo();

						auto refFactory = MakePtr<WfNewTypeExpression>();
						refFactory->type = GetTypeFromTypeInfo(factoryType.Obj());
						{
							auto funcCreateTemplate = MakePtr<WfFunctionDeclaration>();
							funcCreateTemplate->anonymity = WfFunctionAnonymity::Named;
							funcCreateTemplate->name.value = L"CreateTemplate";
							funcCreateTemplate->returnType = GetTypeFromTypeInfo(templateType.Obj());

							auto argViewModel = MakePtr<WfFunctionArgument>();
							argViewModel->type = GetTypeFromTypeInfo(TypeInfoRetriver<Value>::CreateTypeInfo().Obj());
							argViewModel->name.value = L"<viewModel>";
							funcCreateTemplate->arguments.Add(argViewModel);

							auto block = MakePtr<WfBlockStatement>();
							funcCreateTemplate->statement = block;

							{
								auto createControlTemplate = MakePtr<WfNewTypeExpression>();
								createControlTemplate->type = GetTypeFromTypeInfo(controlTemplateType.Obj());

								auto varTemplate = MakePtr<WfVariableDeclaration>();
								varTemplate->type = GetTypeFromTypeInfo(templateType.Obj());
								varTemplate->name.value = L"<template>";
								varTemplate->expression = createControlTemplate;

								auto varStat = MakePtr<WfVariableStatement>();
								varStat->variable = varTemplate;
								block->statements.Add(varStat);
							}
							{
								auto refTemplate = MakePtr<WfReferenceExpression>();
								refTemplate->name.value = L"<template>";

								auto returnStat = MakePtr<WfReturnStatement>();
								returnStat->expression = refTemplate;
								block->statements.Add(returnStat);
							}

							refFactory->functions.Add(funcCreateTemplate);
						}

						auto controlType = TypeInfoRetriver<TControl*>::CreateTypeInfo();
						auto styleType = TypeInfoRetriver<TControlStyle*>::CreateTypeInfo();

						auto createStyle = MakePtr<WfNewTypeExpression>();
						createStyle->type = GetTypeFromTypeInfo(styleType.Obj());
						createStyle->arguments.Add(refFactory);
						createStyleExpr = createStyle;
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
					{
						auto controlType = TypeInfoRetriver<TControl*>::CreateTypeInfo();

						auto createControl = MakePtr<WfNewTypeExpression>();
						createControl->type = GetTypeFromTypeInfo(controlType.Obj());
						{
							auto refControlStyle = MakePtr<WfReferenceExpression>();
							refControlStyle->name.value = L"<controlStyle>";

							createControl->arguments.Add(refControlStyle);
						}

						if (argumentFunction)
						{
							createControl->arguments.Add(argumentFunction());
						}

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