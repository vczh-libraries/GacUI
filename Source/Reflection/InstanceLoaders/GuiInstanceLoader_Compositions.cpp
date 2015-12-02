#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiAxisInstanceLoader
***********************************************************************/

			class GuiAxisInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_AxisDirection;

			public:
				GuiAxisInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiAxis>()->GetTypeName());
					_AxisDirection = GlobalStringKey::Get(L"AxisDirection");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				bool IsCreatable(const TypeInfo& typeInfo)override
				{
					return typeName == typeInfo.typeName;
				}

				description::Value CreateInstance(Ptr<GuiInstanceEnvironment> env, const TypeInfo& typeInfo, collections::Group<GlobalStringKey, description::Value>& constructorArguments)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						vint indexAxisDirection = constructorArguments.Keys().IndexOf(_AxisDirection);	
						if (indexAxisDirection == -1)
						{
							return Value();
						}

						auto axisDirection = UnboxValue<AxisDirection>(constructorArguments.GetByIndex(indexAxisDirection)[0]);
						auto axis = new GuiAxis(axisDirection);
						return Value::From(axis);
					}
					return Value();
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_AxisDirection);
					}
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _AxisDirection)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<AxisDirection>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}
			};

/***********************************************************************
GuiCompositionInstanceLoader
***********************************************************************/

			class GuiCompositionInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiCompositionInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiGraphicsComposition>()->GetTypeName());
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
						info->acceptableTypes.Add(description::GetTypeDescriptor<IGuiGraphicsElement>());
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiGraphicsComposition*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->AddChild(control->GetBoundsComposition());
								return true;
							}
							else if(auto composition = dynamic_cast<GuiGraphicsComposition*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->AddChild(composition);
								return true;
							}
							else if (Ptr<IGuiGraphicsElement> element = propertyValue.propertyValue.GetSharedPtr().Cast<IGuiGraphicsElement>())
							{
								container->SetOwnedElement(element);
								return true;
							}
						}
					}
					return false;
				}
			};

/***********************************************************************
GuiTableCompositionInstanceLoader
***********************************************************************/

			class GuiTableCompositionInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_Rows, _Columns;

			public:
				GuiTableCompositionInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiTableComposition>()->GetTypeName());
					_Rows = GlobalStringKey::Get(L"Rows");
					_Columns = GlobalStringKey::Get(L"Columns");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Rows);
					propertyNames.Add(_Columns);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Rows || propertyInfo.propertyName == _Columns)
					{
						return GuiInstancePropertyInfo::Array(description::GetTypeDescriptor<GuiCellOption>());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiTableComposition*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == _Rows)
						{
							List<GuiCellOption> options;
							CopyFrom(options, GetLazyList<GuiCellOption>(UnboxValue<Ptr<IValueList>>(propertyValue.propertyValue)));
							container->SetRowsAndColumns(options.Count(), container->GetColumns());
							FOREACH_INDEXER(GuiCellOption, option, index, options)
							{
								container->SetRowOption(index, option);
							}
							return true;
						}
						else if (propertyValue.propertyName == _Columns)
						{
							List<GuiCellOption> options;
							CopyFrom(options, GetLazyList<GuiCellOption>(UnboxValue<Ptr<IValueList>>(propertyValue.propertyValue)));
							container->SetRowsAndColumns(container->GetRows(), options.Count());
							FOREACH_INDEXER(GuiCellOption, option, index, options)
							{
								container->SetColumnOption(index, option);
							}
							return true;
						}
					}
					return false;
				}
			};

/***********************************************************************
GuiCellCompositionInstanceLoader
***********************************************************************/

			class GuiCellCompositionInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_Site;

			public:
				GuiCellCompositionInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiCellComposition>()->GetTypeName());
					_Site = GlobalStringKey::Get(L"Site");
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(_Site);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == _Site)
					{
						return GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<SiteValue>());
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiCellComposition*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == _Site)
						{
							SiteValue site = UnboxValue<SiteValue>(propertyValue.propertyValue);
							container->SetSite(site.row, site.column, site.rowSpan, site.columnSpan);
							return true;
						}
					}
					return false;
				}
			};

/***********************************************************************
Initialization
***********************************************************************/

			void LoadCompositions(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiAxisInstanceLoader);
				manager->SetLoader(new GuiCompositionInstanceLoader);
				manager->SetLoader(new GuiTableCompositionInstanceLoader);
				manager->SetLoader(new GuiCellCompositionInstanceLoader);
			}
		}
	}
}

#endif