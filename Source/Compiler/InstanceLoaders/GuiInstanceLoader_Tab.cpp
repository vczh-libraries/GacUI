#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiTabInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiTab, GuiTabTemplate_StyleProvider, GuiTabTemplate>
			class GuiTabInstanceLoader : public BASE_TYPE
			{
			public:
				GuiTabInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiTab>::content.typeName, L"CreateTabStyle")
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
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiTabPage>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
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
							if (type->CanConvertTo(description::GetTypeDescriptor<GuiTabPage>()))
							{
								auto refControl = MakePtr<WfReferenceExpression>();
								refControl->name.value = variableName.ToString();

								auto refCreatePage = MakePtr<WfMemberExpression>();
								refCreatePage->parent = refControl;
								refCreatePage->name.value = L"CreatePage";

								auto refMinus1 = MakePtr<WfIntegerExpression>();
								refMinus1->value.value = L"-1";

								auto call = MakePtr<WfCallExpression>();
								call->function = refCreatePage;
								call->arguments.Add(value);
								call->arguments.Add(refMinus1);

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
					return BASE_TYPE::AssignParameters(resolvingResult, typeInfo, variableName, arguments, attPosition, errors);
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiTabPageInstanceLoader
***********************************************************************/

			class GuiTabPageInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiTabPageInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiTabPage>::content.typeName);
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						auto info = GuiInstancePropertyInfo::Collection();
						info->acceptableTypes.Add(description::GetTypeDescriptor<GuiControl>());
						info->acceptableTypes.Add(description::GetTypeDescriptor<GuiGraphicsComposition>());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
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
								auto refBoundsComposition = MakePtr<WfMemberExpression>();
								refBoundsComposition->parent = value;
								refBoundsComposition->name.value = L"BoundsComposition";

								expr = refBoundsComposition;
							}
							else if (type->CanConvertTo(description::GetTypeDescriptor<GuiGraphicsComposition>()))
							{
								expr = value;
							}

							if (expr)
							{
								auto refControl = MakePtr<WfReferenceExpression>();
								refControl->name.value = variableName.ToString();

								auto refContainerComposition = MakePtr<WfMemberExpression>();
								refContainerComposition->parent = refControl;
								refContainerComposition->name.value = L"ContainerComposition";

								auto refAddChild = MakePtr<WfMemberExpression>();
								refAddChild->parent = refContainerComposition;
								refAddChild->name.value = L"AddChild";

								auto call = MakePtr<WfCallExpression>();
								call->function = refAddChild;
								call->arguments.Add(expr);

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = call;
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
			};

/***********************************************************************
Initialization
***********************************************************************/

			void LoadTabControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiTabInstanceLoader);
				manager->SetLoader(new GuiTabPageInstanceLoader);
			}
		}
	}
}

#endif