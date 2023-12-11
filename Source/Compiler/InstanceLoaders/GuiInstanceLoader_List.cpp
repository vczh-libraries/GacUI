#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

			template<typename TItemTemplateStyle>
			Ptr<WfStatement> CreateSetControlTemplateStyle(types::ResolvingResult& resolvingResult, GlobalStringKey variableName, Ptr<WfExpression> argument, const WString& propertyName)
			{
				auto createStyle = Ptr(new WfNewClassExpression);
				createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<TItemTemplateStyle>>::CreateTypeInfo().Obj());
				createStyle->arguments.Add(argument);

				auto refControl = Ptr(new WfReferenceExpression);
				refControl->name.value = variableName.ToString();

				auto refStyleProvider = Ptr(new WfMemberExpression);
				refStyleProvider->parent = refControl;
				refStyleProvider->name.value = propertyName;

				auto assign = Ptr(new WfBinaryExpression);
				assign->op = WfBinaryOperator::Assign;
				assign->first = refStyleProvider;
				assign->second = createStyle;

				auto stat = Ptr(new WfExpressionStatement);
				stat->expression = assign;
				return stat;
			}

/***********************************************************************
GuiComboButtonInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiComboButton>
			class GuiComboButtonInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey						_DropdownControl;

				void AddAdditionalArguments(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfNewClassExpression> createControl)override
				{
					vint indexListControl = arguments.Keys().IndexOf(_DropdownControl);
					if (indexListControl != -1)
					{
						createControl->arguments.Add(arguments.GetByIndex(indexListControl)[0].expression);
					}
				}
			public:
				GuiComboButtonInstanceLoader()
					:BASE_TYPE(L"presentation::controls::GuiComboButton", theme::ThemeName::ComboBox)
				{
					_DropdownControl = GlobalStringKey::Get(L"DropdownControl");
				}

				void GetRequiredPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_DropdownControl);
					}
					BASE_TYPE::GetRequiredPropertyNames(precompileContext, typeInfo, propertyNames);
				}

				void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(precompileContext, typeInfo, propertyNames);
					BASE_TYPE::GetPropertyNames(precompileContext, typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _DropdownControl)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<GuiControl*>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					return BASE_TYPE::GetPropertyType(precompileContext, propertyInfo);
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiComboBoxInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiComboBoxListControl>
			class GuiComboBoxInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey						_ListControl;

				void AddAdditionalArguments(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfNewClassExpression> createControl)override
				{
					vint indexListControl = arguments.Keys().IndexOf(_ListControl);
					if (indexListControl != -1)
					{
						createControl->arguments.Add(arguments.GetByIndex(indexListControl)[0].expression);
					}
				}
			public:
				GuiComboBoxInstanceLoader()
					:BASE_TYPE(L"presentation::controls::GuiComboBox", theme::ThemeName::ComboBox)
				{
					_ListControl = GlobalStringKey::Get(L"ListControl");
				}

				void GetRequiredPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_ListControl);
					}
					BASE_TYPE::GetRequiredPropertyNames(precompileContext, typeInfo, propertyNames);
				}

				void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(precompileContext, typeInfo, propertyNames);
					BASE_TYPE::GetPropertyNames(precompileContext, typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _ListControl)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<GuiSelectableListControl*>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					return BASE_TYPE::GetPropertyType(precompileContext, propertyInfo);
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiTreeViewInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiTreeView>
			class GuiTreeViewInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey		_Nodes;

			public:
				GuiTreeViewInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiTreeView>::content.typeName, theme::ThemeName::TreeView)
				{
					_Nodes = GlobalStringKey::Get(L"Nodes");
				}

				void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const typename BASE_TYPE::TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Nodes);
					BASE_TYPE::GetPropertyNames(precompileContext, typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const typename BASE_TYPE::PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Nodes)
					{
						return GuiInstancePropertyInfo::Collection(TypeInfoRetriver<Ptr<tree::MemoryNodeProvider>>::CreateTypeInfo());
					}
					return BASE_TYPE::GetPropertyType(precompileContext, propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const typename BASE_TYPE::TypeInfo& typeInfo, GlobalStringKey variableName, typename BASE_TYPE::ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = Ptr(new WfBlockStatement);

					// TODO: (enumerable) foreach on group
					for (auto [prop, index] : indexed(arguments.Keys()))
					{
						if (prop == _Nodes)
						{
							auto refControl = Ptr(new WfReferenceExpression);
							refControl->name.value = variableName.ToString();

							auto refNodes = Ptr(new WfMemberExpression);
							refNodes->parent = refControl;
							refNodes->name.value = L"Nodes";

							auto refChildren = Ptr(new WfMemberExpression);
							refChildren->parent = refNodes;
							refChildren->name.value = L"Children";

							auto refAdd = Ptr(new WfMemberExpression);
							refAdd->parent = refChildren;
							refAdd->name.value = L"Add";

							auto call = Ptr(new WfCallExpression);
							call->function = refAdd;
							call->arguments.Add(arguments.GetByIndex(index)[0].expression);

							auto stat = Ptr(new WfExpressionStatement);
							stat->expression = call;
							block->statements.Add(stat);
						}
					}

					if (block->statements.Count() > 0)
					{
						return block;
					}
					return BASE_TYPE::AssignParameters(precompileContext, resolvingResult, typeInfo, variableName, arguments, attPosition, errors);
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiBindableTreeViewInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiBindableTreeView>
			class GuiBindableTreeViewInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey		_ReverseMappingProperty;

				void AddAdditionalArguments(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfNewClassExpression> createControl)override
				{
					vint indexReverseMappingProperty = arguments.Keys().IndexOf(_ReverseMappingProperty);
					if (indexReverseMappingProperty != -1)
					{
						createControl->arguments.Add(arguments.GetByIndex(indexReverseMappingProperty)[0].expression);
					}
				}
			public:
				GuiBindableTreeViewInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiBindableTreeView>::content.typeName, theme::ThemeName::TreeView)
				{
					_ReverseMappingProperty = GlobalStringKey::Get(L"ReverseMappingProperty");
				}

				void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const typename BASE_TYPE::TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_ReverseMappingProperty);
					BASE_TYPE::GetPropertyNames(precompileContext, typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const typename BASE_TYPE::PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _ReverseMappingProperty)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<WritableItemProperty<description::Value>>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					return BASE_TYPE::GetPropertyType(precompileContext, propertyInfo);
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiTreeNodeInstanceLoader
***********************************************************************/

			class GuiTreeNodeInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey							typeName;
				GlobalStringKey							_Text, _Image, _Tag;

			public:
				GuiTreeNodeInstanceLoader()
					:typeName(GlobalStringKey::Get(L"presentation::controls::tree::TreeNode"))
				{
					_Text = GlobalStringKey::Get(L"Text");
					_Image = GlobalStringKey::Get(L"Image");
					_Tag = GlobalStringKey::Get(L"Tag");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(GuiResourcePrecompileContext& precompileContext, const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Text);
					propertyNames.Add(_Image);
					propertyNames.Add(_Tag);
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(GuiResourcePrecompileContext& precompileContext, const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Text)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<WString>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						info->bindability = GuiInstancePropertyInfo::Bindable;
						return info;
					}
					else if (propertyInfo.propertyName == _Image)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Ptr<GuiImageData>>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						info->bindability = GuiInstancePropertyInfo::Bindable;
						return info;
					}
					else if (propertyInfo.propertyName == _Tag)
					{
						return GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Value>::CreateTypeInfo());
					}
					else if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::Collection(TypeInfoRetriver<Ptr<tree::MemoryNodeProvider>>::CreateTypeInfo());
					}
					return IGuiInstanceLoader::GetPropertyType(precompileContext, propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					if (CanCreate(typeInfo))
					{
						auto createItem = Ptr(new WfNewClassExpression);
						createItem->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<tree::TreeViewItem>>::CreateTypeInfo().Obj());

						vint imageIndex = arguments.Keys().IndexOf(_Image);
						vint textIndex = arguments.Keys().IndexOf(_Text);

						if (imageIndex != -1 || textIndex != -1)
						{
							if (imageIndex == -1)
							{
								auto nullExpr = Ptr(new WfLiteralExpression);
								nullExpr->value = WfLiteralValue::Null;
								createItem->arguments.Add(nullExpr);
							}
							else
							{
								createItem->arguments.Add(arguments.GetByIndex(imageIndex)[0].expression);
							}

							if (textIndex == -1)
							{
								createItem->arguments.Add(Ptr(new WfStringExpression));
							}
							else
							{
								createItem->arguments.Add(arguments.GetByIndex(textIndex)[0].expression);
							}
						}

						auto createNode = Ptr(new WfNewClassExpression);
						createNode->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<tree::MemoryNodeProvider>>::CreateTypeInfo().Obj());
						createNode->arguments.Add(createItem);

						auto refNode = Ptr(new WfReferenceExpression);
						refNode->name.value = variableName.ToString();

						auto assign = Ptr(new WfBinaryExpression);
						assign->op = WfBinaryOperator::Assign;
						assign->first = refNode;
						assign->second = createNode;

						auto stat = Ptr(new WfExpressionStatement);
						stat->expression = assign;
						return stat;
					}
					return nullptr;
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = Ptr(new WfBlockStatement);

					// TODO: (enumerable) foreach on group
					for (auto [prop, index] : indexed(arguments.Keys()))
					{
						if (prop == GlobalStringKey::Empty)
						{
							auto refNode = Ptr(new WfReferenceExpression);
							refNode->name.value = variableName.ToString();

							auto refChildren = Ptr(new WfMemberExpression);
							refChildren->parent = refNode;
							refChildren->name.value = L"Children";

							auto refAdd = Ptr(new WfMemberExpression);
							refAdd->parent = refChildren;
							refAdd->name.value = L"Add";

							auto call = Ptr(new WfCallExpression);
							call->function = refAdd;
							call->arguments.Add(arguments.GetByIndex(index)[0].expression);

							auto stat = Ptr(new WfExpressionStatement);
							stat->expression = call;
							block->statements.Add(stat);
						}
						else if (prop == _Tag)
						{
							{
								auto refNode = Ptr(new WfReferenceExpression);
								refNode->name.value = variableName.ToString();

								auto refData = Ptr(new WfMemberExpression);
								refData->parent = refNode;
								refData->name.value = L"Data";

								auto castExpr = Ptr(new WfTypeCastingExpression);
								castExpr->strategy = WfTypeCastingStrategy::Strong;
								castExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<tree::TreeViewItem>>::CreateTypeInfo().Obj());
								castExpr->expression = refData;

								auto refProp = Ptr(new WfMemberExpression);
								refProp->parent = castExpr;
								refProp->name.value = L"tag";

								auto assign = Ptr(new WfBinaryExpression);
								assign->op = WfBinaryOperator::Assign;
								assign->first = refProp;
								assign->second = arguments.GetByIndex(index)[0].expression;

								auto stat = Ptr(new WfExpressionStatement);
								stat->expression = assign;
								block->statements.Add(stat);
							}

							if (prop != _Tag)
							{
								auto refNode = Ptr(new WfReferenceExpression);
								refNode->name.value = variableName.ToString();

								auto refNotifyDataModified = Ptr(new WfMemberExpression);
								refNotifyDataModified->parent = refNode;
								refNotifyDataModified->name.value = L"NotifyDataModified";

								auto call = Ptr(new WfCallExpression);
								call->function = refNotifyDataModified;

								auto stat = Ptr(new WfExpressionStatement);
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

			void LoadListControls(IGuiInstanceLoaderManager* manager)
			{
				manager->CreateVirtualType(
					GlobalStringKey::Get(description::TypeInfo<GuiComboBoxListControl>::content.typeName),
					Ptr(new GuiComboBoxInstanceLoader)
					);

				manager->SetLoader(Ptr(new GuiComboButtonInstanceLoader));
				manager->SetLoader(Ptr(new GuiTreeViewInstanceLoader));
				manager->SetLoader(Ptr(new GuiBindableTreeViewInstanceLoader));
				
				manager->CreateVirtualType(
					GlobalStringKey::Get(description::TypeInfo<tree::MemoryNodeProvider>::content.typeName),
					Ptr(new GuiTreeNodeInstanceLoader)
					);
			}
		}
	}
}

#endif
