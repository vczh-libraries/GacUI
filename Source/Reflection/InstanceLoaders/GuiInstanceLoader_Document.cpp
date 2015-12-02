#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiDocumentItemInstanceLoader
***********************************************************************/

			class GuiDocumentItemInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;
				GlobalStringKey					_Name;

			public:
				GuiDocumentItemInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::GetTypeDescriptor<GuiDocumentItem>()->GetTypeName());
					_Name = GlobalStringKey::Get(L"Name");
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
						vint indexName = constructorArguments.Keys().IndexOf(_Name);	
						if (indexName == -1)
						{
							return Value();
						}

						auto name = UnboxValue<WString>(constructorArguments.GetByIndex(indexName)[0]);
						auto item = MakePtr<GuiDocumentItem>(name);
						return Value::From(item);
					}
					return Value();
				}

				void GetConstructorParameters(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					if (typeInfo.typeName == GetTypeName())
					{
						propertyNames.Add(_Name);
					}
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
					else if (propertyInfo.propertyName == _Name)
					{
						auto info = GuiInstancePropertyInfo::Assign(description::GetTypeDescriptor<WString>());
						info->scope = GuiInstancePropertyInfo::Constructor;
						return info;
					}
					return IGuiInstanceLoader::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiDocumentItem*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto control = dynamic_cast<GuiControl*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->GetContainer()->AddChild(control->GetBoundsComposition());
								return true;
							}
							else if (auto composition = dynamic_cast<GuiGraphicsComposition*>(propertyValue.propertyValue.GetRawPtr()))
							{
								container->GetContainer()->AddChild(composition);
								return true;
							}
						}
					}
					return false;
				}
			};

/***********************************************************************
GuiDocumentViewerInstanceLoader
***********************************************************************/

#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentViewer, GuiDocumentViewerTemplate_StyleProvider, GuiDocumentViewerTemplate>
			class GuiDocumentViewerInstanceLoader : public BASE_TYPE
			{
			public:
				GuiDocumentViewerInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiDocumentViewer>()->GetTypeName(), L"CreateDocumentViewerStyle")
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiDocumentItem>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiDocumentCommonInterface*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto item = propertyValue.propertyValue.GetSharedPtr().Cast<GuiDocumentItem>())
							{
								container->AddDocumentItem(item);
								return true;
							}
						}
					}
					return false;
				}
			};
#undef BASE_TYPE

/***********************************************************************
GuiDocumentLabelInstanceLoader
***********************************************************************/
		
#define BASE_TYPE GuiTemplateControlInstanceLoader<GuiDocumentLabel, GuiDocumentLabelTemplate_StyleProvider, GuiDocumentLabelTemplate>
			class GuiDocumentLabelInstanceLoader : public BASE_TYPE
			{
			public:
				GuiDocumentLabelInstanceLoader()
					:BASE_TYPE(description::GetTypeDescriptor<GuiDocumentLabel>()->GetTypeName(), L"CreateDocumentLabelStyle")
				{
				}

				void GetPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					propertyNames.Add(GlobalStringKey::Empty);
				}

				Ptr<GuiInstancePropertyInfo> GetPropertyType(const PropertyInfo& propertyInfo)override
				{
					if (propertyInfo.propertyName == GlobalStringKey::Empty)
					{
						return GuiInstancePropertyInfo::CollectionWithParent(description::GetTypeDescriptor<GuiDocumentItem>());
					}
					return BASE_TYPE::GetPropertyType(propertyInfo);
				}

				bool SetPropertyValue(PropertyValue& propertyValue)override
				{
					if (auto container = dynamic_cast<GuiDocumentCommonInterface*>(propertyValue.instanceValue.GetRawPtr()))
					{
						if (propertyValue.propertyName == GlobalStringKey::Empty)
						{
							if (auto item = propertyValue.propertyValue.GetSharedPtr().Cast<GuiDocumentItem>())
							{
								container->AddDocumentItem(item);
								return true;
							}
						}
					}
					return false;
				}
			};
#undef BASE_TYPE

/***********************************************************************
Initialization
***********************************************************************/

			void LoadDocumentControls(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiDocumentItemInstanceLoader);
				manager->SetLoader(new GuiDocumentViewerInstanceLoader);
				manager->SetLoader(new GuiDocumentLabelInstanceLoader);
			}
		}
	}
}

#endif