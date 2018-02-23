#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{
			Ptr<workflow::WfStatement> AddControlToToolstrip(GlobalStringKey variableName, IGuiInstanceLoader::ArgumentMap& arguments, GuiResourceError::List& errors)
			{
				auto block = MakePtr<WfBlockStatement>();

				FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
				{
					const auto& values = arguments.GetByIndex(index);
					if (prop == GlobalStringKey::Empty)
					{
						auto value = values[0].expression;
						auto td = values[0].typeInfo->GetTypeDescriptor();

						Ptr<WfExpression> expr;
						if (td->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
						{
							auto refControl = MakePtr<WfReferenceExpression>();
							refControl->name.value = variableName.ToString();

							auto refToolstripItems = MakePtr<WfMemberExpression>();
							refToolstripItems->parent = refControl;
							refToolstripItems->name.value = L"ToolstripItems";

							auto refAdd = MakePtr<WfMemberExpression>();
							refAdd->parent = refToolstripItems;
							refAdd->name.value = L"Add";

							auto call = MakePtr<WfCallExpression>();
							call->function = refAdd;
							call->arguments.Add(value);

							expr = call;
						}

						if (expr)
						{
							auto stat = MakePtr<WfExpressionStatement>();
							stat->expression = expr;
							block->statements.Add(stat);
						}
					}
				}

				if (block->statements.Count() > 0)
				{
					return block;
				}
				return nullptr;
			}

/***********************************************************************
GuiToolstripInstanceLoaderBase
***********************************************************************/

			template<typename TBaseType>
			class GuiToolstripInstanceLoaderBase : public TBaseType
			{
			private:
				using TypeInfo = typename TBaseType::TypeInfo;

			public:
				using ArgumentMap = IGuiInstanceLoader::ArgumentMap;
				using PropertyInfo = IGuiInstanceLoader::PropertyInfo;

				GuiToolstripInstanceLoaderBase(const WString& _typeName, theme::ThemeName themeName, Ptr<WfExpression>(*_argumentFunction)(ArgumentMap&))
					:TBaseType(_typeName, themeName, _argumentFunction)
				{
				}

				GuiToolstripInstanceLoaderBase(const WString& _typeName, theme::ThemeName themeName)
					:TBaseType(_typeName, themeName)
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
					TBaseType::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(TypeInfoRetriver<GuiControl*>::CreateTypeInfo());
					}
					return TBaseType::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					if (auto statement = AddControlToToolstrip(variableName, arguments, errors))
					{
						return statement;
					}
					else
					{
						return TBaseType::AssignParameters(precompileContext, resolvingResult, typeInfo, variableName, arguments, attPosition, errors);
					}
				}
			};

/***********************************************************************
GuiToolstripMenuInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripMenu>
			class GuiToolstripMenuInstanceLoader : public GuiToolstripInstanceLoaderBase<BASE_TYPE>
			{
			public:
				static Ptr<WfExpression> ArgumentFunction(ArgumentMap&)
				{
					auto expr = MakePtr<WfLiteralExpression>();
					expr->value = WfLiteralValue::Null;
					return expr;
				}
			public:
				GuiToolstripMenuInstanceLoader()
					:GuiToolstripInstanceLoaderBase<BASE_TYPE>(description::TypeInfo<GuiToolstripMenu>::content.typeName, theme::ThemeName::Menu, ArgumentFunction)
				{
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiToolstripButtonInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripButton>
			class GuiToolstripButtonInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey					_SubMenu;

			public:
				GuiToolstripButtonInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiToolstripButton>::content.typeName, theme::ThemeName::ToolstripButton)
				{
					_SubMenu = GlobalStringKey::Get(L"SubMenu");
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_SubMenu);
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _SubMenu)
					{
						return GuiInstancePropertyInfo::Set(TypeInfoRetriver<GuiToolstripMenu*>::CreateTypeInfo());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfExpression> GetParameter(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const PropertyInfo& propertyInfo, GlobalStringKey variableName, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					if (propertyInfo.propertyName == _SubMenu)
					{
						auto refControl = MakePtr<WfReferenceExpression>();
						refControl->name.value = variableName.ToString();

						auto refEnsureToolstripSubMenu = MakePtr<WfMemberExpression>();
						refEnsureToolstripSubMenu->parent = refControl;
						refEnsureToolstripSubMenu->name.value = L"EnsureToolstripSubMenu";

						auto call = MakePtr<WfCallExpression>();
						call->function = refEnsureToolstripSubMenu;

						return call;
					}
					return BASE_TYPE::GetParameter(precompileContext, resolvingResult, propertyInfo, variableName, attPosition, errors);
				}
			};
#undef BASE_TYPE

/***********************************************************************
Initialization
***********************************************************************/

			void LoadToolstripControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiToolstripMenuInstanceLoader);
				manager->SetLoader(new GuiToolstripButtonInstanceLoader);
			}
		}
	}
}

#endif
