#include "GuiInstanceLoader_TemplateControl.h"

#ifndef VCZH_DEBUG_NO_REFLECTION

namespace vl
{
	namespace presentation
	{
		namespace instance_loaders
		{

/***********************************************************************
GuiTemplateInstanceLoader
***********************************************************************/

			class GuiTemplateInstanceLoader : public Object, public IGuiInstanceLoader
			{
			protected:
				GlobalStringKey					typeName;

			public:
				GuiTemplateInstanceLoader()
				{
					typeName = GlobalStringKey::Get(description::TypeInfo<GuiTemplate>::content.typeName);
				}

				GlobalStringKey GetTypeName()override
				{
					return typeName;
				}

				void GetRequiredPropertyNames(const TypeInfo& typeInfo, collections::List<GlobalStringKey>& propertyNames)override
				{
					List<ITypeDescriptor*> tds;
					tds.Add(typeInfo.typeInfo->GetTypeDescriptor());

					for (vint i = 0; i < tds.Count(); i++)
					{
						auto td = tds[i];
						if (td != description::GetTypeDescriptor<GuiWindowTemplate>())
						{
							vint propCount = td->GetPropertyCount();
							for (vint i = 0; i < propCount; i++)
							{
								auto prop = td->GetProperty(i);
								if (prop->IsWritable() && INVLOC.EndsWith(prop->GetName(), L"Template", Locale::None))
								{
									propertyNames.Add(GlobalStringKey::Get(prop->GetName()));
								}
							}

							vint baseCount = td->GetBaseTypeDescriptorCount();
							for (vint i = 0; i < baseCount; i++)
							{
								auto baseTd = td->GetBaseTypeDescriptor(i);
								if (!tds.Contains(baseTd))
								{
									tds.Add(baseTd);
								}
							}
						}
					}
				}
			};

/***********************************************************************
Initialization
***********************************************************************/

			void LoadTemplates(IGuiInstanceLoaderManager* manager)
			{
				manager->SetLoader(new GuiTemplateInstanceLoader);
			}
		}
	}
}

#endif
