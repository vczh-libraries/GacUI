#include "../../Resources/GuiResource.h"
#include "../WorkflowCodegen/GuiInstanceLoader_WorkflowCodegen.h"

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

			template<typename TControl>
			class GuiTemplateControlInstanceLoader : public Object, public IGuiInstanceLoader
			{
				typedef Ptr<WfExpression>							ArgumentRawFunctionType(ArgumentMap&);
				typedef void										InitRawFunctionType(const WString&, Ptr<WfBlockStatement>);
				typedef Func<ArgumentRawFunctionType>				ArgumentFunctionType;
				typedef Func<InitRawFunctionType>					InitFunctionType;

			protected:
				GlobalStringKey								typeName;
				theme::ThemeName							themeName;
				ArgumentFunctionType						argumentFunction;
				InitFunctionType							initFunction;

				virtual void AddAdditionalArguments(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfNewClassExpression> createControl)
				{
				}
			public:
				static Ptr<WfExpression> CreateThemeName(theme::ThemeName themeName)
				{
					auto refExpr = Ptr(new WfChildExpression);
					refExpr->parent = GetExpressionFromTypeDescriptor(description::GetTypeDescriptor<ThemeName>());
					switch (themeName)
					{
#define THEME_NAME_CASE(TEMPLATE, CONTROL) case theme::ThemeName::CONTROL: refExpr->name.value = L ## #CONTROL; break;
						GUI_CONTROL_TEMPLATE_TYPES(THEME_NAME_CASE)
						THEME_NAME_CASE(WindowTemplate, Window)
#undef THEME_NAME_CASE
					default:
						CHECK_FAIL(L"GuiTemplateControlInstanceLoader::CreateThemeName()#Unknown theme name.");
					}
					return refExpr;
				}

			public:
				GuiTemplateControlInstanceLoader(const WString& _typeName, theme::ThemeName _themeName, ArgumentRawFunctionType* _argumentFunction = nullptr, InitRawFunctionType* _initFunction = nullptr)
					:typeName(GlobalStringKey::Get(_typeName))
					, themeName(_themeName)
				{
					if (_argumentFunction) argumentFunction = _argumentFunction;
					if (_initFunction) initFunction = _initFunction;
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeName == typeInfo.typeName;
				}

				Ptr<workflow::WfBaseConstructorCall> CreateRootInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, ArgumentMap& arguments, GuiResourceError::List& errors)override
				{
					auto createControl = Ptr(new WfBaseConstructorCall);
					createControl->type = GetTypeFromTypeInfo(TypeInfoRetriver<TControl>::CreateTypeInfo().Obj());
					createControl->arguments.Add(CreateThemeName(themeName));
					return createControl;
				}

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					CHECK_ERROR(CanCreate(typeInfo), L"GuiTemplateControlInstanceLoader::CreateInstance()#Wrong type info is provided.");

					auto block = Ptr(new WfBlockStatement);
					{
						auto controlType = TypeInfoRetriver<TControl*>::CreateTypeInfo();

						auto createControl = Ptr(new WfNewClassExpression);
						createControl->type = GetTypeFromTypeInfo(controlType.Obj());
						createControl->arguments.Add(CreateThemeName(themeName));

						if (argumentFunction)
						{
							createControl->arguments.Add(argumentFunction(arguments));
						}
						AddAdditionalArguments(resolvingResult, typeInfo, variableName, arguments, errors, createControl);

						auto refVariable = Ptr(new WfReferenceExpression);
						refVariable->name.value = variableName.ToString();

						auto assignExpr = Ptr(new WfBinaryExpression);
						assignExpr->op = WfBinaryOperator::Assign;
						assignExpr->first = refVariable;
						assignExpr->second = createControl;

						auto assignStat = Ptr(new WfExpressionStatement);
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
