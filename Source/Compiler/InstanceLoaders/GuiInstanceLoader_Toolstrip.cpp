#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{
			Ptr<workflow::WfStatement> AddControlToToolstrip(GlobalStringKey variableName, IGuiInstanceLoader::ArgumentMap& arguments, collections::List<WString>& errors)
			{
				auto block = MakePtr<WfBlockStatement>();

				FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
				{
					const auto& values = arguments.GetByIndex(index);
					if (prop == GlobalStringKey::Empty)
					{
						auto value = values[0].expression;
						auto type = values[0].type;

						Ptr<WfExpression> expr;
						if (type->CanConvertTo(description::GetTypeDescriptor<GuiControl>()))
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
GuiToolstripMenuInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripMenu, GuiMenuTemplate_StyleProvider, GuiMenuTemplate>
			class GuiToolstripMenuInstanceLoader : public BASE_TYPE
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
					:BASE_TYPE(description::TypeInfo<GuiToolstripMenu>::content.typeName, L"CreateMenuStyle", ArgumentFunction)
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (auto statement = AddControlToToolstrip(variableName, arguments, errors))
					{
						return statement;
					}
					else
					{
						return BASE_TYPE::AssignParameters(resolvingResult, typeInfo, variableName, arguments, errors);
					}
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiToolstripMenuBarInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripMenuBar, GuiControlTemplate_StyleProvider, GuiControlTemplate>
			class GuiToolstripMenuBarInstanceLoader : public BASE_TYPE
			{
			public:
				GuiToolstripMenuBarInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiToolstripMenuBar>::content.typeName, L"CreateMenuBarStyle")
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (auto statement = AddControlToToolstrip(variableName, arguments, errors))
					{
						return statement;
					}
					else
					{
						return BASE_TYPE::AssignParameters(resolvingResult, typeInfo, variableName, arguments, errors);
					}
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiToolstripToolBarInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripToolBar, GuiControlTemplate_StyleProvider, GuiControlTemplate>
			class GuiToolstripToolBarInstanceLoader : public BASE_TYPE
			{
			public:
				GuiToolstripToolBarInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiToolstripToolBar>::content.typeName, L"CreateToolBarStyle")
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiControl>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (auto statement = AddControlToToolstrip(variableName, arguments, errors))
					{
						return statement;
					}
					else
					{
						return BASE_TYPE::AssignParameters(resolvingResult, typeInfo, variableName, arguments, errors);
					}
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiToolstripButtonInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiToolstripButton, GuiToolstripButtonTemplate_StyleProvider, GuiToolstripButtonTemplate>
			class GuiToolstripButtonInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey					_SubMenu;

			public:
				GuiToolstripButtonInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiToolstripButton>::content.typeName, L"CreateToolBarButtonStyle")
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
						return GuiInstancePropertyInfo::Set(description::GetTypeDescriptor<GuiToolstripMenu>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfExpression> GetParameter(types::ResolvingResult& resolvingResult, const PropertyInfo& propertyInfo, GlobalStringKey variableName, collections::List<WString>& errors)override
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
					return BASE_TYPE::GetParameter(resolvingResult, propertyInfo, variableName, errors);
				}
			};
#undef BASE_TYPE

/***********************************************************************
Initialization
***********************************************************************/

			void LoadToolstripControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiToolstripMenuInstanceLoader);
				manager->SetLoader(new GuiToolstripMenuBarInstanceLoader);
				manager->SetLoader(new GuiToolstripToolBarInstanceLoader);
				manager->SetLoader(new GuiToolstripButtonInstanceLoader);
			}
		}
	}
}

#endif
