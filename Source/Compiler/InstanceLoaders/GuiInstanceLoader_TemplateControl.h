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

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(GlobalStringKey::_ControlTemplate);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<TemplateProperty<TTemplate>>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
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
					auto index = arguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
					if (index != -1)
					{
						auto argument = arguments.GetByIndex(index)[0];

						auto createStyle = MakePtr<WfNewClassExpression>();
						createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<TControlStyle*>::CreateTypeInfo().Obj());
						createStyle->arguments.Add(argument.expression);

						return createStyle;
					}
					return CreateIThemeCall(styleMethod);
				}

				Ptr<workflow::WfBaseConstructorCall> CreateRootInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors)override
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

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					CHECK_ERROR(CanCreate(typeInfo), L"GuiTemplateControlInstanceLoader::CreateInstance# Wrong type info is provided.");
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
