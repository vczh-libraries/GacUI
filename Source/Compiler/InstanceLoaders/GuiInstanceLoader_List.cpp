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
				auto createStyle = MakePtr<WfNewClassExpression>();
				createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<TItemTemplateStyle>>::CreateTypeInfo().Obj());
				createStyle->arguments.Add(argument);

				auto refControl = MakePtr<WfReferenceExpression>();
				refControl->name.value = variableName.ToString();

				auto refStyleProvider = MakePtr<WfMemberExpression>();
				refStyleProvider->parent = refControl;
				refStyleProvider->name.value = propertyName;

				auto assign = MakePtr<WfBinaryExpression>();
				assign->op = WfBinaryOperator::Assign;
				assign->first = refStyleProvider;
				assign->second = createStyle;

				auto stat = MakePtr<WfExpressionStatement>();
				stat->expression = assign;
				return stat;
			}

/***********************************************************************
GuiVirtualTextListControlInstanceLoader
***********************************************************************/

			class GuiVirtualTextListControlInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiVirtualTextListControlInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiVirtualTextList>::content.typeName);
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::_ItemTemplate);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::_ItemTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<TemplateProperty<GuiTextListItemTemplate>>::CreateTypeInfo());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::_ItemTemplate)
						{
							if (auto stat = CreateSetControlTemplateStyle<GuiTextListItemTemplate_ItemStyleProvider>(resolvingResult, variableName, arguments.GetByIndex(index)[0].expression, L"StyleProvider"))
							{
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
GuiComboBoxInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiComboBoxListControl, GuiComboBoxTemplate_StyleProvider, GuiComboBoxTemplate>
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
					:BASE_TYPE(L"presentation::controls::GuiComboBox", L"CreateComboBoxStyle")
				{
					_ListControl = GlobalStringKey::Get(L"ListControl");
				}

				void GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_ListControl);
					}
					BASE_TYPE::GetRequiredPropertyNames(typeInfo, propertyNames);
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					GetRequiredPropertyNames(typeInfo, propertyNames);
					propertyNames.Add(GlobalStringKey::_ItemTemplate);
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _ListControl)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<GuiSelectableListControl*>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					else if (propertyInfo.propertyName == GlobalStringKey::_ItemTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<TemplateProperty<GuiControlTemplate>>::CreateTypeInfo());
						return info;
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::_ItemTemplate)
						{
							if (auto stat = CreateSetControlTemplateStyle<GuiControlTemplate_ItemStyleProvider>(resolvingResult, variableName, arguments.GetByIndex(index)[0].expression, L"StyleProvider"))
							{
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
#undef BASE_TYPE

/***********************************************************************
GuiListViewInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<TControl, GuiListViewTemplate_StyleProvider, GuiListViewTemplate>
			template<typename TControl>
			class GuiListViewInstanceLoaderBase : public BASE_TYPE
			{
			protected:
				GlobalStringKey		_View, _IconSize;

				void PrepareAdditionalArgumentsAfterCreation(types::ResolvingResult& resolvingResult, const typename BASE_TYPE::TypeInfo& typeInfo, GlobalStringKey variableName, typename BASE_TYPE::ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfBlockStatement> block)override
				{
					auto view = ListViewViewType::Detail;
					Ptr<WfExpression> iconSize;
					{
						vint indexView = arguments.Keys().IndexOf(_View);
						if (indexView != -1)
						{
							auto viewArgument = arguments.GetByIndex(indexView)[0];
							if (auto inferExpr = viewArgument.expression.template Cast<WfInferExpression>())
							{
								if (auto refExpr = inferExpr->expression.template Cast<WfReferenceExpression>())
								{
									auto enumType = description::GetTypeDescriptor<ListViewViewType>()->GetEnumType();
									vint index = enumType->IndexOfItem(refExpr->name.value);
									if (index == -1)
									{
										goto ILLEGAL_VIEW_PROPERTY;
									}
									
									view = UnboxValue<ListViewViewType>(enumType->ToEnum(enumType->GetItemValue(index)));
									goto FINISH_VIEW_PROPERTY;
								}
							}
						ILLEGAL_VIEW_PROPERTY:
							errors.Add(GuiResourceError({ resolvingResult.resource }, viewArgument.attPosition,
								L"Precompile: The value of property \"View\" of type \"" +
								typeInfo.typeName.ToString() +
								L"\" is not in a correct format: \"BigIcon | SmallIcon | List | Tile | Information | Detail\"."));
						FINISH_VIEW_PROPERTY:;
						}

						vint indexIconSize = arguments.Keys().IndexOf(_IconSize);
						if (indexIconSize != -1)
						{
							iconSize = arguments.GetByIndex(indexIconSize)[0].expression;
						}
					}

					Ptr<ITypeInfo> itemStyleType;
					switch (view)
					{
#define VIEW_TYPE_CASE(NAME) case ListViewViewType::NAME: itemStyleType = TypeInfoRetriver<Ptr<list::ListView##NAME##ContentProvider>>::CreateTypeInfo(); break;
						VIEW_TYPE_CASE(BigIcon)
						VIEW_TYPE_CASE(SmallIcon)
						VIEW_TYPE_CASE(List)
						VIEW_TYPE_CASE(Tile)
						VIEW_TYPE_CASE(Information)
						VIEW_TYPE_CASE(Detail)
#undef VIEW_TYPE_CASE
					}

					auto createStyle = MakePtr<WfNewClassExpression>();
					createStyle->type = GetTypeFromTypeInfo(itemStyleType.Obj());

					if (iconSize)
					{
						createStyle->arguments.Add(iconSize);

						auto falseValue = MakePtr<WfLiteralExpression>();
						falseValue->value = WfLiteralValue::False;
						createStyle->arguments.Add(falseValue);
					}
					else
					{
						{
							auto ctorExpr = MakePtr<WfConstructorExpression>();
							{
								auto argument = MakePtr<WfConstructorArgument>();
								{
									auto key = MakePtr<WfReferenceExpression>();
									key->name.value = L"x";
									argument->key = key;

									auto value = MakePtr<WfIntegerExpression>();
									value->value.value = L"32";
									argument->value = value;
								}
								ctorExpr->arguments.Add(argument);
							}
							{
								auto argument = MakePtr<WfConstructorArgument>();
								{
									auto key = MakePtr<WfReferenceExpression>();
									key->name.value = L"y";
									argument->key = key;

									auto value = MakePtr<WfIntegerExpression>();
									value->value.value = L"32";
									argument->value = value;
								}
								ctorExpr->arguments.Add(argument);
							}

							auto iconSizeValue = MakePtr<WfInferExpression>();
							iconSizeValue->type = GetTypeFromTypeInfo(TypeInfoRetriver<Size>::CreateTypeInfo().Obj());
							iconSizeValue->expression = ctorExpr;

							createStyle->arguments.Add(iconSizeValue);
						}
						{
							auto trueValue = MakePtr<WfLiteralExpression>();
							trueValue->value = WfLiteralValue::True;
							createStyle->arguments.Add(trueValue);
						}
					}

					auto refControl = MakePtr<WfReferenceExpression>();
					refControl->name.value = variableName.ToString();

					auto refChangeItemStyle = MakePtr<WfMemberExpression>();
					refChangeItemStyle->parent = refControl;
					refChangeItemStyle->name.value = L"ChangeItemStyle";

					auto call = MakePtr<WfCallExpression>();
					call->function = refChangeItemStyle;
					call->arguments.Add(createStyle);

					auto stat = MakePtr<WfExpressionStatement>();
					stat->expression = call;
					block->statements.Add(stat);
				}

			public:
				GuiListViewInstanceLoaderBase()
					:BASE_TYPE(description::TypeInfo<TControl>::content.typeName, L"CreateListViewStyle")
				{
					_View = GlobalStringKey::Get(L"View");
					_IconSize = GlobalStringKey::Get(L"IconSize");
				}

				void GetPropertyNames(const typename BASE_TYPE::TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_View);
						propertyNames.Add(_IconSize);
					}
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const typename BASE_TYPE::PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _View)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<ListViewViewType>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					else if (propertyInfo.propertyName == _IconSize)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Size>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}
			};
#undef BASE_TYPE

			class GuiListViewInstanceLoader : public GuiListViewInstanceLoaderBase<GuiListView>
			{
			};

			class GuiBindableListViewInstanceLoader : public GuiListViewInstanceLoaderBase<GuiBindableListView>
			{
			};

/***********************************************************************
GuiVirtualTreeViewInstanceLoader
***********************************************************************/

			class GuiVirtualTreeViewInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiVirtualTreeViewInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiVirtualTreeView>::content.typeName);
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::_ItemTemplate);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::_ItemTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<TemplateProperty<GuiTreeItemTemplate>>::CreateTypeInfo());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::_ItemTemplate)
						{
							if (auto stat = CreateSetControlTemplateStyle<GuiTreeItemTemplate_ItemStyleProvider>(resolvingResult, variableName, arguments.GetByIndex(index)[0].expression, L"NodeStyleProvider"))
							{
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
GuiTreeViewInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<TControl, GuiTreeViewTemplate_StyleProvider, GuiTreeViewTemplate>
			template<typename TControl>
			class GuiTreeViewInstanceLoaderBase : public BASE_TYPE
			{
			protected:
				bool				bindable;
				GlobalStringKey		_Nodes, _IconSize;

				void PrepareAdditionalArgumentsAfterCreation(types::ResolvingResult& resolvingResult, const typename BASE_TYPE::TypeInfo& typeInfo, GlobalStringKey variableName, typename BASE_TYPE::ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfBlockStatement> block)override
				{
					vint indexIconSize = arguments.Keys().IndexOf(_IconSize);
					if (indexIconSize != -1)
					{
						Ptr<ITypeInfo> itemStyleType = TypeInfoRetriver<Ptr<tree::TreeViewNodeItemStyleProvider>>::CreateTypeInfo();

						auto createStyle = MakePtr<WfNewClassExpression>();
						createStyle->type = GetTypeFromTypeInfo(itemStyleType.Obj());
						{
							auto iconSize = arguments.GetByIndex(indexIconSize)[0].expression;
							createStyle->arguments.Add(iconSize);
						}
						{
							auto falseValue = MakePtr<WfLiteralExpression>();
							falseValue->value = WfLiteralValue::False;
							createStyle->arguments.Add(falseValue);
						}

						auto refControl = MakePtr<WfReferenceExpression>();
						refControl->name.value = variableName.ToString();

						auto refSetNodeStyleProvider = MakePtr<WfMemberExpression>();
						refSetNodeStyleProvider->parent = refControl;
						refSetNodeStyleProvider->name.value = L"SetNodeStyleProvider";

						auto call = MakePtr<WfCallExpression>();
						call->function = refSetNodeStyleProvider;
						call->arguments.Add(createStyle);

						auto stat = MakePtr<WfExpressionStatement>();
						stat->expression = call;
						block->statements.Add(stat);
					}
				}

			public:
				GuiTreeViewInstanceLoaderBase(bool _bindable)
					:BASE_TYPE(description::TypeInfo<TControl>::content.typeName, L"CreateTreeViewStyle")
					, bindable(_bindable)
				{
					_Nodes = GlobalStringKey::Get(L"Nodes");
					_IconSize = GlobalStringKey::Get(L"IconSize");
				}

				void GetPropertyNames(const typename BASE_TYPE::TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (CanCreate(typeInfo))
					{
						propertyNames.Add(_IconSize);
					}
					if (!bindable)
					{
						propertyNames.Add(_Nodes);
					}
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const typename BASE_TYPE::PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _IconSize)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Size>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						return info;
					}
					else if (propertyInfo.propertyName == _Nodes)
					{
						if (!bindable)
						{
							return GuiInstancePropertyInfo::Collection(TypeInfoRetriver<Ptr<tree::MemoryNodeProvider>>::CreateTypeInfo());
						}
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const typename BASE_TYPE::TypeInfo& typeInfo, GlobalStringKey variableName, typename BASE_TYPE::ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						if (prop == _Nodes)
						{
							auto refControl = MakePtr<WfReferenceExpression>();
							refControl->name.value = variableName.ToString();

							auto refNodes = MakePtr<WfMemberExpression>();
							refNodes->parent = refControl;
							refNodes->name.value = L"Nodes";

							auto refChildren = MakePtr<WfMemberExpression>();
							refChildren->parent = refNodes;
							refChildren->name.value = L"Children";

							auto refAdd = MakePtr<WfMemberExpression>();
							refAdd->parent = refChildren;
							refAdd->name.value = L"Add";

							auto call = MakePtr<WfCallExpression>();
							call->function = refAdd;
							call->arguments.Add(arguments.GetByIndex(index)[0].expression);

							auto stat = MakePtr<WfExpressionStatement>();
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

			class GuiTreeViewInstanceLoader : public GuiTreeViewInstanceLoaderBase<GuiTreeView>
			{
			public:
				GuiTreeViewInstanceLoader()
					:GuiTreeViewInstanceLoaderBase<GuiTreeView>(false)
				{
				}
			};

			class GuiBindableTreeViewInstanceLoader : public GuiTreeViewInstanceLoaderBase<GuiBindableTreeView>
			{
			public:
				GuiBindableTreeViewInstanceLoader()
					:GuiTreeViewInstanceLoaderBase<GuiBindableTreeView>(true)
				{
				}
			};

/***********************************************************************
GuiBindableDataGridInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiBindableDataGrid, GuiListViewTemplate_StyleProvider, GuiListViewTemplate>
			class GuiBindableDataGridInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey		typeName;
				GlobalStringKey		_ViewModelContext;
				GlobalStringKey		_Columns;
				
				void AddAdditionalArguments(types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceError::List& errors, Ptr<WfNewClassExpression> createControl)override
				{
					auto indexViewModelContext = arguments.Keys().IndexOf(_ViewModelContext);
					if (indexViewModelContext == -1)
					{
						auto nullExpr = MakePtr<WfLiteralExpression>();
						nullExpr->value = WfLiteralValue::Null;
						createControl->arguments.Add(nullExpr);
					}
					else
					{
						createControl->arguments.Add(arguments.GetByIndex(indexViewModelContext)[0].expression);
					}
				}
			public:
				GuiBindableDataGridInstanceLoader()
					:BASE_TYPE(description::TypeInfo<GuiBindableDataGrid>::content.typeName, L"CreateListViewStyle")
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiBindableDataGrid>::content.typeName);
					_ViewModelContext = GlobalStringKey::Get(L"ViewModelContext");
					_Columns = GlobalStringKey::Get(L"Columns");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_ViewModelContext);
					propertyNames.Add(_Columns);
					BASE_TYPE::GetPropertyNames(typeInfo, propertyNames);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _ViewModelContext)
					{
						auto info = GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Value>::CreateTypeInfo());
						info->usage = GuiInstancePropertyInfo::ConstructorArgument;
						info->bindability = GuiInstancePropertyInfo::Bindable;
						return info;
					}
					else if (propertyInfo.propertyName == _Columns)
					{
						return GuiInstancePropertyInfo::Collection(TypeInfoRetriver<Ptr<list::BindableDataColumn>>::CreateTypeInfo());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						if (prop == _Columns)
						{
							auto refControl = MakePtr<WfReferenceExpression>();
							refControl->name.value = variableName.ToString();

							auto refAddBindableColumn = MakePtr<WfMemberExpression>();
							refAddBindableColumn->parent = refControl;
							refAddBindableColumn->name.value = L"AddBindableColumn";

							auto call = MakePtr<WfCallExpression>();
							call->function = refAddBindableColumn;
							call->arguments.Add(arguments.GetByIndex(index)[0].expression);

							auto stat = MakePtr<WfExpressionStatement>();
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

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Text);
					propertyNames.Add(_Image);
					propertyNames.Add(_Tag);
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Text)
					{
						return GuiInstancePropertyInfo::Assign(TypeInfoRetriver<WString>::CreateTypeInfo());
					}
					else if (propertyInfo.propertyName == _Image)
					{
						return GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Ptr<GuiImageData>>::CreateTypeInfo());
					}
					else if (propertyInfo.propertyName == _Tag)
					{
						return GuiInstancePropertyInfo::Assign(TypeInfoRetriver<Value>::CreateTypeInfo());
					}
					else if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::Collection(TypeInfoRetriver<Ptr<tree::MemoryNodeProvider>>::CreateTypeInfo());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				Ptr<workflow::WfStatement> CreateInstance(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos tagPosition, GuiResourceError::List& errors)override
				{
					if (CanCreate(typeInfo))
					{
						auto createItem = MakePtr<WfNewClassExpression>();
						createItem->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<tree::TreeViewItem>>::CreateTypeInfo().Obj());

						auto createNode = MakePtr<WfNewClassExpression>();
						createNode->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<tree::MemoryNodeProvider>>::CreateTypeInfo().Obj());
						createNode->arguments.Add(createItem);

						auto refNode = MakePtr<WfReferenceExpression>();
						refNode->name.value = variableName.ToString();

						auto assign = MakePtr<WfBinaryExpression>();
						assign->op = WfBinaryOperator::Assign;
						assign->first = refNode;
						assign->second = createNode;

						auto stat = MakePtr<WfExpressionStatement>();
						stat->expression = assign;
						return stat;
					}
					return nullptr;
				}

				Ptr<workflow::WfStatement> AssignParameters(GuiResourcePrecompileContext& precompileContext, types::ResolvingResult& resolvingResult, const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, GuiResourceTextPos attPosition, GuiResourceError::List& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						if (prop == GlobalStringKey::Empty)
						{
							auto refNode = MakePtr<WfReferenceExpression>();
							refNode->name.value = variableName.ToString();

							auto refChildren = MakePtr<WfMemberExpression>();
							refChildren->parent = refNode;
							refChildren->name.value = L"Children";

							auto refAdd = MakePtr<WfMemberExpression>();
							refAdd->parent = refChildren;
							refAdd->name.value = L"Add";

							auto call = MakePtr<WfCallExpression>();
							call->function = refAdd;
							call->arguments.Add(arguments.GetByIndex(index)[0].expression);

							auto stat = MakePtr<WfExpressionStatement>();
							stat->expression = call;
							block->statements.Add(stat);
						}
						else if (prop == _Text || prop == _Image || prop == _Tag)
						{
							{
								auto refNode = MakePtr<WfReferenceExpression>();
								refNode->name.value = variableName.ToString();

								auto refData = MakePtr<WfMemberExpression>();
								refData->parent = refNode;
								refData->name.value = L"Data";

								auto castExpr = MakePtr<WfTypeCastingExpression>();
								castExpr->strategy = WfTypeCastingStrategy::Strong;
								castExpr->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<tree::TreeViewItem>>::CreateTypeInfo().Obj());
								castExpr->expression = refData;

								auto refProp = MakePtr<WfMemberExpression>();
								refProp->parent = castExpr;
								if (prop == _Text)
								{
									refProp->name.value = L"text";
								}
								else if (prop == _Image)
								{
									refProp->name.value = L"image";
								}
								else if (prop == _Tag)
								{
									refProp->name.value = L"tag";
								}

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refProp;
								assign->second = arguments.GetByIndex(index)[0].expression;

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = assign;
								block->statements.Add(stat);
							}

							if (prop != _Tag)
							{
								auto refNode = MakePtr<WfReferenceExpression>();
								refNode->name.value = variableName.ToString();

								auto refNotifyDataModified = MakePtr<WfMemberExpression>();
								refNotifyDataModified->parent = refNode;
								refNotifyDataModified->name.value = L"NotifyDataModified";

								auto call = MakePtr<WfCallExpression>();
								call->function = refNotifyDataModified;

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

			void LoadListControls(IGuiInstanceLoaderManager* manager)
			{
				manager->CreateVirtualType(
					GlobalStringKey::Get(description::TypeInfo<GuiComboBoxListControl>::content.typeName),
					new GuiComboBoxInstanceLoader
					);

				manager->SetLoader(new GuiVirtualTextListControlInstanceLoader);

				manager->SetLoader(new GuiListViewInstanceLoader);
				manager->SetLoader(new GuiBindableListViewInstanceLoader);

				manager->SetLoader(new GuiVirtualTreeViewInstanceLoader);
				manager->SetLoader(new GuiTreeViewInstanceLoader);
				manager->SetLoader(new GuiBindableTreeViewInstanceLoader);

				manager->SetLoader(new GuiBindableDataGridInstanceLoader);
				
				manager->CreateVirtualType(
					GlobalStringKey::Get(description::TypeInfo<tree::MemoryNodeProvider>::content.typeName),
					new GuiTreeNodeInstanceLoader
					);
			}
		}
	}
}

#endif
