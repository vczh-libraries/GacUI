#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

			template<typename IItemTemplateStyle, typename ITemplate>
			Ptr<WfStatement> CreateSetControlTemplateStyle(GlobalStringKey variableName, Ptr<WfExpression> argument, const WString& propertyName, collections::List<WString>& errors)
			{
				using Helper = GuiTemplateControlInstanceLoader<void, IItemTemplateStyle, ITemplate>;
				List<ITypeDescriptor*> controlTemplateTds;
				Helper::GetItemTemplateType(argument, controlTemplateTds, errors);

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
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiSelectableListControl>()->GetTypeName());
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
					if (typeInfo.typeName == GetTypeName())
					{
						auto block = MakePtr<WfBlockStatement>();

						FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
						{
							const auto& values = arguments.GetByIndex(index);
							if (prop == GlobalStringKey::_ItemTemplate)
							{
								if (auto stat = CreateSetControlTemplateStyle<GuiListItemTemplate_ItemStyleProvider, GuiListItemTemplate>(variableName, arguments.GetByIndex(index)[0].expression, L"StyleProvider", errors))
								{
									block->statements.Add(stat);
								}
							}
						}

						if (block->statements.Count() > 0)
						{
							return block;
						}
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
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiVirtualTreeView>()->GetTypeName());
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
					if (typeInfo.typeName == GetTypeName())
					{
						auto block = MakePtr<WfBlockStatement>();

						FOREACH_INDEXER(GlobalStringKey, prop, index, arguments.Keys())
						{
							const auto& values = arguments.GetByIndex(index);
							if (prop == GlobalStringKey::_ItemTemplate)
							{
								if (auto stat = CreateSetControlTemplateStyle<GuiTreeItemTemplate_ItemStyleProvider, GuiTreeItemTemplate>(variableName, arguments.GetByIndex(index)[0].expression, L"NodeStyleProvider", errors))
								{
									block->statements.Add(stat);
								}
							}
						}

						if (block->statements.Count() > 0)
						{
							return block;
						}
					}
					return nullptr;
				}
			};

/***********************************************************************
GuiListViewInstanceLoader
***********************************************************************/

			class GuiListViewInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				bool				bindable;
				GlobalStringKey		typeName;
				GlobalStringKey		_View, _IconSize, _ItemSource;

			public:
				GuiListViewInstanceLoader(bool _bindable)
					:bindable(_bindable)
				{
					if (bindable)
					{
						typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableListView>()->GetTypeName());
					}
					else
					{
						typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiListView>()->GetTypeName());
					}
					_View = GlobalStringKey::Get(L"View");
					_IconSize = GlobalStringKey::Get(L"IconSize");
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				bool IsCreatable(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						Ptr<IValueEnumerable> itemSource;
						ListViewViewType viewType = ListViewViewType::Detail;
						GuiListViewBase::IStyleProvider* styleProvider = 0;
						Size iconSize;
						{
							vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);
							if (indexItemSource != -1)
							{
								itemSource = UnboxValue<Ptr<IValueEnumerable>>(constructorArguments.GetByIndex(indexItemSource)[0]);
							}
							else if (bindable)
							{
								return Value();
							}

							vint indexView = constructorArguments.Keys().IndexOf(_View);
							if (indexView != -1)
							{
								viewType = UnboxValue<ListViewViewType>(constructorArguments.GetByIndex(indexView)[0]);
							}

							vint indexIconSize = constructorArguments.Keys().IndexOf(_IconSize);
							if (indexIconSize != -1)
							{
								iconSize = UnboxValue<Size>(constructorArguments.GetByIndex(indexIconSize)[0]);
							}

							vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
							if (indexControlTemplate == -1)
							{
								styleProvider = GetCurrentTheme()->CreateListViewStyle();
							}
							else
							{
								auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
								styleProvider = new GuiListViewTemplate_StyleProvider(factory);
							}
						}

						GuiVirtualListView* listView = 0;
						if (bindable)
						{
							listView = new GuiBindableListView(styleProvider, itemSource);
						}
						else
						{
							listView = new GuiListView(styleProvider);
						}
						switch (viewType)
						{
	#define VIEW_TYPE_CASE(NAME)\
						case ListViewViewType::NAME:\
							if (iconSize == Size())\
							{\
								listView->ChangeItemStyle(new list::ListView##NAME##ContentProvider);\
							}\
							else\
							{\
								listView->ChangeItemStyle(new list::ListView##NAME##ContentProvider(iconSize, false));\
							}\
							break;\

							VIEW_TYPE_CASE(BigIcon)
							VIEW_TYPE_CASE(SmallIcon)
							VIEW_TYPE_CASE(List)
							VIEW_TYPE_CASE(Tile)
							VIEW_TYPE_CASE(Information)
							VIEW_TYPE_CASE(Detail)

	#undef VIEW_TYPE_CASE
						}

						return Value::From(listView);
					}
					return Value();
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(GlobalStringKey::_ControlTemplate);
						propertyNames.Add(_View);
						propertyNames.Add(_IconSize);
						if (bindable)
						{
							propertyNames.Add(_ItemSource);
						}
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					else if (propertyInfo.propertyName == _View)
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
					else if (propertyInfo.propertyName == _ItemSource)
					{
						if (bindable)
						{
							auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
							info->scope = GuiInstancePropertyInfo::Constructor;
							info->required = true;
							return info;
						}
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}
			};

/***********************************************************************
GuiTreeViewInstanceLoader
***********************************************************************/

			class GuiTreeViewInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				bool				bindable;
				GlobalStringKey		typeName;
				GlobalStringKey		_IconSize, _ItemSource, _Nodes;

			public:
				GuiTreeViewInstanceLoader(bool _bindable)
					:bindable(_bindable)
				{
					if (bindable)
					{
						typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableTreeView>()->GetTypeName());
					}
					else
					{
						typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiTreeView>()->GetTypeName());
					}
					_IconSize = GlobalStringKey::Get(L"IconSize");
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
					_Nodes = GlobalStringKey::Get(L"Nodes");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				bool IsCreatable(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);
						GuiVirtualTreeView::IStyleProvider* styleProvider = 0;
						{
							vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
							if (indexControlTemplate == -1)
							{
								styleProvider = GetCurrentTheme()->CreateTreeViewStyle();
							}
							else
							{
								auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
								styleProvider = new GuiTreeViewTemplate_StyleProvider(factory);
							}
						}

						GuiVirtualTreeView* treeView = 0;
						if (bindable)
						{
							if (indexItemSource == -1)
							{
								return Value();
							}

							auto itemSource = constructorArguments.GetByIndex(indexItemSource)[0];
							treeView = new GuiBindableTreeView(styleProvider, itemSource);
						}
						else
						{
							treeView = new GuiTreeView(styleProvider);
						}

						vint indexIconSize = constructorArguments.Keys().IndexOf(_IconSize);
						if (indexIconSize != -1)
						{
							auto iconSize = UnboxValue<Size>(constructorArguments.GetByIndex(indexIconSize)[0]);
							treeView->SetNodeStyleProvider(new tree::TreeViewNodeItemStyleProvider(iconSize, false));
						}

						return Value::From(treeView);
					}
					return Value();
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (!bindable)
					{
						propertyNames.Add(_Nodes);
					}
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(GlobalStringKey::_ControlTemplate);
						propertyNames.Add(_IconSize);
						if (bindable)
						{
							propertyNames.Add(_ItemSource);
						}
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Nodes)
					{
						if (!bindable)
						{
							return GuiInstancePropertyInfo::Collection(description::GetTypeDescriptor<tree::MemoryNodeProvider>());
						}
					}
					else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					else if (propertyInfo.propertyName == _ItemSource)
					{
						if (bindable)
						{
							auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
							info->scope = GuiInstancePropertyInfo::Constructor;
							info->required = true;
							return info;
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

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiTreeView*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == _Nodes)
						{
							auto item = UnboxValue<Ptr<tree::MemoryNodeProvider>>(propertyValue.propertyValue);
							container->Nodes()->Children().Add(item);
							return true;
						}
					}
					return false;
				}
			};

/***********************************************************************
GuiBindableTextListInstanceLoader
***********************************************************************/

			class GuiBindableTextListInstanceLoader : public Object, public IGuiInstanceLoader
			{
				typedef Func<list::TextItemStyleProvider::ITextItemStyleProvider*()>		ItemStyleProviderFactory;
			protected:
				GlobalStringKey					typeName;
				ItemStyleProviderFactory		itemStyleProviderFactory;
				GlobalStringKey					_ItemSource;

			public:
				GuiBindableTextListInstanceLoader(const WString& type, const ItemStyleProviderFactory& factory)
					:typeName(GlobalStringKey::Get(L"presentation::controls::GuiBindable" + type + L"TextList"))
					, itemStyleProviderFactory(factory)
				{
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				bool IsCreatable(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);
						if (indexItemSource != -1)
						{
							GuiTextListTemplate_StyleProvider* styleProvider = 0;
							{
								vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
								if (indexControlTemplate != -1)
								{
									auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
									styleProvider = new GuiTextListTemplate_StyleProvider(factory);
								}
							}

							auto itemSource = UnboxValue<Ptr<IValueEnumerable>>(constructorArguments.GetByIndex(indexItemSource)[0]);
							GuiBindableTextList* control = 0;
							if (styleProvider)
							{
								control = new GuiBindableTextList(styleProvider, styleProvider->CreateArgument(), itemSource);
							}
							else
							{
								control = new GuiBindableTextList(GetCurrentTheme()->CreateTextListStyle(), itemStyleProviderFactory(), itemSource);
							}
							return Value::From(control);
						}
					}
					return Value();
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(GlobalStringKey::_ControlTemplate);
						propertyNames.Add(_ItemSource);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
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
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<list::BindableDataColumn>()->GetTypeName());
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

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<list::BindableDataColumn*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == _VisualizerTemplates)
						{
							List<WString> types;
							SplitBySemicolon(propertyValue.propertyValue.GetText(), types);
							Ptr<list::IDataVisualizerFactory> factory;
							FOREACH(WString, type, types)
							{
								auto templateFactory = CreateTemplateFactory(type);
								if (factory)
								{
									factory = new GuiBindableDataVisualizer::DecoratedFactory(templateFactory, container, factory);
								}
								else
								{
									factory = new GuiBindableDataVisualizer::Factory(templateFactory, container);
								}
							}

							container->SetVisualizerFactory(factory);
							return true;
						}
						else if (propertyValue.propertyName == _EditorTemplate)
						{
							auto templateFactory = CreateTemplateFactory(propertyValue.propertyValue.GetText());
							auto factory = new GuiBindableDataEditor::Factory(templateFactory, container);
							container->SetEditorFactory(factory);
							return true;
						}
					}
					return false;
				}
			};

/***********************************************************************
GuiBindableDataGridInstanceLoader
***********************************************************************/

			class GuiBindableDataGridInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey		typeName;
				GlobalStringKey		_ItemSource;
				GlobalStringKey		_ViewModelContext;
				GlobalStringKey		_Columns;

			public:
				GuiBindableDataGridInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableDataGrid>()->GetTypeName());
					_ItemSource = GlobalStringKey::Get(L"ItemSource");
					_ViewModelContext = GlobalStringKey::Get(L"ViewModelContext");
					_Columns = GlobalStringKey::Get(L"Columns");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				bool IsCreatable(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						vint indexItemSource = constructorArguments.Keys().IndexOf(_ItemSource);	
						if (indexItemSource == -1)
						{
							return Value();
						}

						GuiBindableDataGrid::IStyleProvider* styleProvider = 0;
						vint indexControlTemplate = constructorArguments.Keys().IndexOf(GlobalStringKey::_ControlTemplate);
						if (indexControlTemplate == -1)
						{
							styleProvider = GetCurrentTheme()->CreateListViewStyle();
						}
						else
						{
							auto factory = CreateTemplateFactory(constructorArguments.GetByIndex(indexControlTemplate)[0].GetText());
							styleProvider = new GuiListViewTemplate_StyleProvider(factory);
						}
					
						auto itemSource = UnboxValue<Ptr<IValueEnumerable>>(constructorArguments.GetByIndex(indexItemSource)[0]);

						Value viewModelContext;
						vint indexViewModelContext = constructorArguments.Keys().IndexOf(_ViewModelContext);
						if (indexViewModelContext != -1)
						{
							viewModelContext = constructorArguments.GetByIndex(indexViewModelContext)[0];
						}

						auto dataGrid = new GuiBindableDataGrid(styleProvider, itemSource, viewModelContext);
						return Value::From(dataGrid);
					}
					return Value();
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Columns);
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(GlobalStringKey::_ControlTemplate);
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
					else if (propertyInfo.propertyName == GlobalStringKey::_ControlTemplate)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					else if (propertyInfo.propertyName == _ItemSource)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<IValueEnumerable>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						info->required = true;
						return info;
					}
					else if (propertyInfo.propertyName == _ViewModelContext)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<Value>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiBindableDataGrid*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == _Columns)
						{
							auto column = UnboxValue<Ptr<list::BindableDataColumn>>(propertyValue.propertyValue);
							container->AddBindableColumn(column);
							return true;
						}
					}
					return false;
				}
			};

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

				bool IsCreatable(const TypeInfo& typeInfo)override
				{
					return typeInfo.typeName == GetTypeName();
				}

				description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						Ptr<tree::TreeViewItem> item = new tree::TreeViewItem;
						Ptr<tree::MemoryNodeProvider> node = new tree::MemoryNodeProvider(item);
						return Value::From(node);
					}
					return Value();
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

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<tree::MemoryNodeProvider*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == _Text)
						{
							if (auto item = container->GetData().Cast<tree::TreeViewItem>())
							{
								item->text = UnboxValue<WString>(propertyValue.propertyValue);
								container->NotifyDataModified();
								return true;
							}
						}
						else if (propertyValue.propertyName == _Image)
						{
							if (auto item = container->GetData().Cast<tree::TreeViewItem>())
							{
								item->image = UnboxValue<Ptr<GuiImageData>>(propertyValue.propertyValue);
								container->NotifyDataModified();
								return true;
							}
						}
						else if (propertyValue.propertyName == _Tag)
						{
							if (auto item = container->GetData().Cast<tree::TreeViewItem>())
							{
								item->tag = propertyValue.propertyValue;
								return true;
							}
						}
						else if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							auto item = UnboxValue<Ptr<tree::MemoryNodeProvider>>(propertyValue.propertyValue);
							container->Children().Add(item);
							return true;
						}
					}
					return false;
				}
			};

/***********************************************************************
Initialization
***********************************************************************/

			void LoadListControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiSelectableListControlInstanceLoader);
				manager->SetLoader(new GuiVirtualTreeViewInstanceLoader);
				manager->SetLoader(new GuiListViewInstanceLoader(false));
				manager->SetLoader(new GuiTreeViewInstanceLoader(false));
				manager->SetLoader(new GuiBindableTextListInstanceLoader(L"", [](){return GetCurrentTheme()->CreateTextListItemStyle(); }));
				manager->SetLoader(new GuiListViewInstanceLoader(true));
				manager->SetLoader(new GuiTreeViewInstanceLoader(true));
				manager->SetLoader(new GuiBindableDataColumnInstanceLoader);
				manager->SetLoader(new GuiBindableDataGridInstanceLoader);

				auto bindableTextListName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiBindableTextList>()->GetTypeName());
				manager->CreateVirtualType(bindableTextListName, new GuiBindableTextListInstanceLoader(L"Check", [](){return GetCurrentTheme()->CreateCheckTextListItemStyle(); }));
				manager->CreateVirtualType(bindableTextListName, new GuiBindableTextListInstanceLoader(L"Radio", [](){return GetCurrentTheme()->CreateRadioTextListItemStyle(); }));
				
				manager->CreateVirtualType(
					GlobalStringKey::Get(description::GetTypeDescriptor<tree::MemoryNodeProvider>()->GetTypeName()),
					new GuiTreeNodeInstanceLoader
					);
			}
		}
	}
}

#endif