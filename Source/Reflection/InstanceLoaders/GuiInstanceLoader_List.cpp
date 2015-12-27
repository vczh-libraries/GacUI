#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

			template<typename IItemTemplateStyle, typename ITemplate>
			Ptr<WfStatement> CreateSetControlTemplateStyle(GlobalStringKey variableName, Ptr<WfExpression> argument, const IGuiInstanceLoader::TypeInfo& controlTypeInfo, const WString& propertyName, collections::List<WString>& errors)
			{
				using Helper = GuiTemplateControlInstanceLoader<Value, Value, ITemplate>;
				List<ITypeDescriptor*> controlTemplateTds;
				Helper::GetItemTemplateType(argument, controlTemplateTds, controlTypeInfo, GlobalStringKey::_ItemTemplate.ToString(), errors);

				if (controlTemplateTds.Count() > 0)
				{
					auto refFactory = Helper::CreateTemplateFactory(controlTemplateTds, errors);
					auto createStyle = MakePtr<WfNewTypeExpression>();
					createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<IItemTemplateStyle>>::CreateTypeInfo().Obj());
					createStyle->arguments.Add(refFactory);

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

				return nullptr;
			}

/***********************************************************************
GuiSelectableListControlInstanceLoader
***********************************************************************/

			class GuiSelectableListControlInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiSelectableListControlInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiSelectableListControl>::TypeName);
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
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::_ItemTemplate)
						{
							if (auto stat = CreateSetControlTemplateStyle<GuiListItemTemplate_ItemStyleProvider, GuiListItemTemplate>(variableName, arguments.GetByIndex(index)[0].expression, typeInfo, L"StyleProvider", errors))
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
GuiVirtualTreeViewInstanceLoader
***********************************************************************/

			class GuiVirtualTreeViewInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiVirtualTreeViewInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiVirtualTreeView>::TypeName);
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
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						const auto& values = arguments.GetByIndex(index);
						if (prop == GlobalStringKey::_ItemTemplate)
						{
							if (auto stat = CreateSetControlTemplateStyle<GuiTreeItemTemplate_ItemStyleProvider, GuiTreeItemTemplate>(variableName, arguments.GetByIndex(index)[0].expression, typeInfo, L"NodeStyleProvider", errors))
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
GuiListViewInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<TControl, GuiListViewTemplate_StyleProvider, GuiListViewTemplate>
			template<typename TControl>
			class GuiListViewInstanceLoaderBase : public BASE_TYPE
			{
			protected:
				bool				bindable;
				GlobalStringKey		_View, _IconSize;

				void PrepareAdditionalArgumentsAfterCreation(const typename BASE_TYPE::TypeInfo& typeInfo, GlobalStringKey variableName, typename BASE_TYPE::ArgumentMap& arguments, collections::List<WString>& errors, Ptr<WfBlockStatement> block)override
				{
					auto view = ListViewViewType::Detail;
					Ptr<WfExpression> iconSize;
					{
						vint indexView = arguments.Keys().IndexOf(_View);
						if (indexView != -1)
						{
							auto value = arguments.GetByIndex(indexView)[0].expression;
							auto viewValue = ParseConstantArgument<ListViewViewType>(value, typeInfo, L"View", L"", errors);
							view = UnboxValue<ListViewViewType>(viewValue);
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

					auto createStyle = MakePtr<WfNewTypeExpression>();
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
							auto stringValue = MakePtr<WfStringExpression>();
							stringValue->value.value = L"x:32 y:32";

							auto iconSizeValue = MakePtr<WfTypeCastingExpression>();
							iconSizeValue->type = GetTypeFromTypeInfo(TypeInfoRetriver<Size>::CreateTypeInfo().Obj());
							iconSizeValue->expression = stringValue;

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
					:BASE_TYPE(description::TypeInfo<TControl>::TypeName, L"CreateListViewStyle")
				{
					_View = GlobalStringKey::Get(L"View");
					_IconSize = GlobalStringKey::Get(L"IconSize");
				}

				void GetConstructorParameters(const typename BASE_TYPE::TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == BASE_TYPE::GetTypeName())
					{
						propertyNames.Add(_View);
						propertyNames.Add(_IconSize);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const typename BASE_TYPE::PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _View)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<ListViewViewType>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					else if (propertyInfo.propertyName == _IconSize)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Size>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}
			};
#undef BASE_TYPE

			class GuiListViewInstanceLoader : public GuiListViewInstanceLoaderBase<GuiListView>
			{
			};

#define BASE_TYPE GuiListViewInstanceLoaderBase<GuiBindableListView>
			class GuiBindableListViewInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey		_ItemSource;

				void AddAdditionalArguments(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors, Ptr<WfNewTypeExpression> createControl)override
				{
					vint indexItemSource = arguments.Keys().IndexOf(_ItemSource);
					if (indexItemSource != -1)
					{
						createControl->arguments.Add(arguments.GetByIndex(indexItemSource)[0].expression);
					}
				}
			public:
				GuiBindableListViewInstanceLoader()
				{
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					BASE_TYPE::GetConstructorParameters(typeInfo, propertyNames);
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_ItemSource);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _ItemSource)
					{
						if (bindable)
						{
							auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
							info->scope = GuiInstancePropertyInfo::Constructor;
							info->required = true;
							info->bindable = true;
							return info;
						}
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}
			};
#undef BASE_TYPE

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

				void PrepareAdditionalArgumentsAfterCreation(const typename BASE_TYPE::TypeInfo& typeInfo, GlobalStringKey variableName, typename BASE_TYPE::ArgumentMap& arguments, collections::List<WString>& errors, Ptr<WfBlockStatement> block)override
				{
					vint indexIconSize = arguments.Keys().IndexOf(_IconSize);
					if (indexIconSize != -1)
					{
						Ptr<ITypeInfo> itemStyleType = TypeInfoRetriver<Ptr<tree::TreeViewNodeItemStyleProvider>>::CreateTypeInfo();

						auto createStyle = MakePtr<WfNewTypeExpression>();
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
				}

			public:
				GuiTreeViewInstanceLoaderBase()
					:BASE_TYPE(description::TypeInfo<TControl>::TypeName, L"CreateTreeViewStyle")
				{
					_Nodes = GlobalStringKey::Get(L"Nodes");
					_IconSize = GlobalStringKey::Get(L"IconSize");
				}

				void GetPropertyNames(const typename BASE_TYPE::TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (!bindable)
					{
						propertyNames.Add(_Nodes);
					}
				}

				void GetConstructorParameters(const typename BASE_TYPE::TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == BASE_TYPE::GetTypeName())
					{
						propertyNames.Add(_Nodes);
						propertyNames.Add(_IconSize);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const typename BASE_TYPE::PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Nodes)
					{
						if (!bindable)
						{
							return GuiInstancePropertyInfo::Collection(description::GetTypeDescriptor<tree::MemoryNodeProvider>());
						}
					}
					else if (propertyInfo.propertyName == _IconSize)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Size>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const typename BASE_TYPE::TypeInfo& typeInfo, GlobalStringKey variableName, typename BASE_TYPE::ArgumentMap& arguments, collections::List<WString>& errors)override
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
					return nullptr;
				}
			};
#undef BASE_TYPE

			class GuiTreeViewInstanceLoader : public GuiTreeViewInstanceLoaderBase<GuiTreeView>
			{
			};

#define BASE_TYPE GuiTreeViewInstanceLoaderBase<GuiBindableTreeView>
			class GuiBindableTreeViewInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey		_ItemSource;

				void AddAdditionalArguments(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors, Ptr<WfNewTypeExpression> createControl)override
				{
					vint indexItemSource = arguments.Keys().IndexOf(_ItemSource);
					if (indexItemSource != -1)
					{
						createControl->arguments.Add(arguments.GetByIndex(indexItemSource)[0].expression);
					}
				}
			public:
				GuiBindableTreeViewInstanceLoader()
				{
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					BASE_TYPE::GetConstructorParameters(typeInfo, propertyNames);
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_ItemSource);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _ItemSource)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						info->bindable = true;
						return info;
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiBindableTextListInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiBindableTextList, GuiTextListTemplate_StyleProvider, GuiTextListTemplate>
			class GuiBindableTextListInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey					_ItemSource;
				
				void AddAdditionalArguments(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors, Ptr<WfNewTypeExpression> createControl)override
				{
					vint indexItemSource = arguments.Keys().IndexOf(_ItemSource);
					if (indexItemSource != -1)
					{
						createControl->arguments.Add(arguments.GetByIndex(indexItemSource)[0].expression);
					}
				}
			public:
				GuiBindableTextListInstanceLoader(const WString& type)
					:BASE_TYPE(
						L"presentation::controls::GuiBindable" + type + L"TextList",
						L"CreateTextListStyle",
						L"Create" + type + L"TextListItemStyle"
						)
				{
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}
				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_ItemSource);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _ItemSource)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiBindableDataColumnInstanceLoader
***********************************************************************/

			class GuiBindableDataColumnInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey		typeName;
				GlobalStringKey		_VisualizerTemplates;
				GlobalStringKey		_EditorTemplate;

			public:
				GuiBindableDataColumnInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<list::BindableDataColumn>::TypeName);
					_VisualizerTemplates = GlobalStringKey::Get(L"VisualizerTemplates");
					_EditorTemplate = GlobalStringKey::Get(L"EditorTemplate");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_VisualizerTemplates);
					propertyNames.Add(_EditorTemplate);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _VisualizerTemplates)
					{
						return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					}
					else if (propertyInfo.propertyName == _EditorTemplate)
					{
						return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					auto block = MakePtr<WfBlockStatement>();

					FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
					{
						if (prop == _VisualizerTemplates)
						{
							using Helper = GuiTemplateControlInstanceLoader<Value, Value, GuiGridVisualizerTemplate>;
							List<ITypeDescriptor*> controlTemplateTds;
							Helper::GetItemTemplateType(arguments.GetByIndex(index)[0].expression, controlTemplateTds, typeInfo, _EditorTemplate.ToString(), errors);

							if (controlTemplateTds.Count() > 0)
							{
								FOREACH_INDEXER(ITypeDescriptor*, controlTemplateTd, index, controlTemplateTds)
								{
									auto refFactory = Helper::CreateTemplateFactory(controlTemplateTd, errors);
									auto createStyle = MakePtr<WfNewTypeExpression>();
									if (index == 0)
									{
										createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<GuiBindableDataVisualizer::Factory>>::CreateTypeInfo().Obj());
									}
									else
									{
										createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<GuiBindableDataVisualizer::DecoratedFactory>>::CreateTypeInfo().Obj());
									}
									createStyle->arguments.Add(refFactory);
									{
										auto refContainer = MakePtr<WfReferenceExpression>();
										refContainer->name.value = variableName.ToString();
										createStyle->arguments.Add(refContainer);
									}
									if (index > 0)
									{
										auto refPreviousFactory = MakePtr<WfReferenceExpression>();
										refPreviousFactory->name.value = L"<factory>" + itow(index - 1);
										createStyle->arguments.Add(refPreviousFactory);
									}

									auto varDecl = MakePtr<WfVariableDeclaration>();
									varDecl->name.value = L"<factory>" + itow(index);
									varDecl->expression = createStyle;

									auto stat = MakePtr<WfVariableStatement>();
									stat->variable = varDecl;
									block->statements.Add(stat);
								}

								auto refContainer = MakePtr<WfReferenceExpression>();
								refContainer->name.value = variableName.ToString();

								auto refVisualizerFactory = MakePtr<WfMemberExpression>();
								refVisualizerFactory->parent = refContainer;
								refVisualizerFactory->name.value = L"Visualizer";
									
								auto refLastFactory = MakePtr<WfMemberExpression>();
								refLastFactory->parent = refContainer;
								refLastFactory->name.value = L"<factory>" + itow(controlTemplateTds.Count() - 1);

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refVisualizerFactory;
								assign->second = refLastFactory;

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = assign;
								block->statements.Add(stat);
							}
						}
						else if (prop == _EditorTemplate)
						{
							using Helper = GuiTemplateControlInstanceLoader<Value, Value, GuiGridEditorTemplate>;
							List<ITypeDescriptor*> controlTemplateTds;
							Helper::GetItemTemplateType(arguments.GetByIndex(index)[0].expression, controlTemplateTds, typeInfo, _EditorTemplate.ToString(), errors);

							if (controlTemplateTds.Count() > 0)
							{
								auto refFactory = Helper::CreateTemplateFactory(controlTemplateTds, errors);
								auto createStyle = MakePtr<WfNewTypeExpression>();
								createStyle->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<GuiBindableDataEditor::Factory>>::CreateTypeInfo().Obj());
								createStyle->arguments.Add(refFactory);
								{
									auto refContainer = MakePtr<WfReferenceExpression>();
									refContainer->name.value = variableName.ToString();
									createStyle->arguments.Add(refContainer);
								}

								auto refContainer = MakePtr<WfReferenceExpression>();
								refContainer->name.value = variableName.ToString();

								auto refEditorFactory = MakePtr<WfMemberExpression>();
								refEditorFactory->parent = refContainer;
								refEditorFactory->name.value = L"EditorFactory";

								auto assign = MakePtr<WfBinaryExpression>();
								assign->op = WfBinaryOperator::Assign;
								assign->first = refEditorFactory;
								assign->second = createStyle;

								auto stat = MakePtr<WfExpressionStatement>();
								stat->expression = assign;
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
GuiBindableDataGridInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiBindableDataGrid, GuiListViewTemplate_StyleProvider, GuiListViewTemplate>
			class GuiBindableDataGridInstanceLoader : public BASE_TYPE
			{
			protected:
				GlobalStringKey		typeName;
				GlobalStringKey		_ItemSource;
				GlobalStringKey		_ViewModelContext;
				GlobalStringKey		_Columns;
				
				void AddAdditionalArguments(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors, Ptr<WfNewTypeExpression> createControl)override
				{
					auto indexItemSource = arguments.Keys().IndexOf(_ItemSource);
					createControl->arguments.Add(arguments.GetByIndex(indexItemSource)[0].expression);

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
					:BASE_TYPE(description::TypeInfo<GuiBindableDataGrid>::TypeName, L"CreateListViewStyle")
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiBindableDataGrid>::TypeName);
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
					_ViewModelContext = GlobalStringKey::Get(L"ViewModelContext");
					_Columns = GlobalStringKey::Get(L"Columns");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Columns);
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_ItemSource);
						propertyNames.Add(_ViewModelContext);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Columns)
					{
						return GuiInstancePropertyInfo::Collection(description::GetTypeDescriptor<list::BindableDataColumn>());
					}
					else if (propertyInfo.propertyName == _ItemSource)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						info->bindable = true;
						return info;
					}
					else if (propertyInfo.propertyName == _ViewModelContext)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->bindable = true;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
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
					return nullptr;
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
						return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
					}
					else if (propertyInfo.propertyName == _Image)
					{
						return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<GuiImageData>());
					}
					else if (propertyInfo.propertyName == _Tag)
					{
						return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
					}
					else if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::Collection(description::GetTypeDescriptor<tree::MemoryNodeProvider>());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool CanCreate(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				Ptr<workflow::WfStatement> CreateInstance(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						auto createItem = MakePtr<WfNewTypeExpression>();
						createItem->type = GetTypeFromTypeInfo(TypeInfoRetriver<Ptr<tree::TreeViewItem>>::CreateTypeInfo().Obj());

						auto createNode = MakePtr<WfNewTypeExpression>();
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

				Ptr<workflow::WfStatement> AssignParameters(const TypeInfo& typeInfo, GlobalStringKey variableName, ArgumentMap& arguments, collections::List<WString>& errors)override
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
				manager->SetLoader(new GuiSelectableListControlInstanceLoader);
				manager->SetLoader(new GuiVirtualTreeViewInstanceLoader);

				manager->SetLoader(new GuiListViewInstanceLoader);
				manager->SetLoader(new GuiBindableListViewInstanceLoader);

				manager->SetLoader(new GuiTreeViewInstanceLoader);
				manager->SetLoader(new GuiBindableTreeViewInstanceLoader);

				manager->SetLoader(new GuiBindableTextListInstanceLoader(L""));
				manager->SetLoader(new GuiBindableTextListInstanceLoader(L"Check"));
				manager->SetLoader(new GuiBindableTextListInstanceLoader(L"Radio"));

				manager->SetLoader(new GuiBindableDataColumnInstanceLoader);
				manager->SetLoader(new GuiBindableDataGridInstanceLoader);
				
				manager->CreateVirtualType(
					GlobalStringKey::Get(description::TypeInfo<tree::MemoryNodeProvider>::TypeName),
					new GuiTreeNodeInstanceLoader
					);
			}
		}
	}
}

#endif
