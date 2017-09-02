#include "GuiResourceManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace stream;
		using namespace reflection::description;
		using namespace controls;

/***********************************************************************
Class Name Record (ClassNameRecord)
***********************************************************************/

		class GuiResourceClassNameRecordTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_DirectLoadStream
		{
		public:
			WString GetType()override
			{
				return L"ClassNameRecord";
			}

			bool XmlSerializable()override
			{
				return false;
			}

			bool StreamSerializable()override
			{
				return true;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			void SerializePrecompiled(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content, stream::IStream& stream)override
			{
				if (auto obj = content.Cast<GuiResourceClassNameRecord>())
				{
					internal::ContextFreeWriter writer(stream);
					writer << obj->classNames;
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(Ptr<GuiResourceItem> resource, stream::IStream& stream, GuiResourceError::List& errors)override
			{
				internal::ContextFreeReader reader(stream);

				auto obj = MakePtr<GuiResourceClassNameRecord>();
				reader << obj->classNames;
				return obj;
			}
		};

/***********************************************************************
IGuiInstanceResourceManager
***********************************************************************/

		IGuiResourceManager* resourceManager = nullptr;

		IGuiResourceManager* GetResourceManager()
		{
			return resourceManager;
		}

		class GuiResourceManager : public Object, public IGuiResourceManager, public IGuiPlugin
		{
		protected:
			typedef Dictionary<WString, Ptr<GuiResource>>					ResourceMap;

			ResourceMap								resources;
			ResourceMap								instanceResources;

		public:

			GUI_PLUGIN_NAME(GacUI_Res_Resource)
			{
				GUI_PLUGIN_DEPEND(GacUI_Res_ResourceResolver);
			}

			void Load()override
			{
				resourceManager = this;
				IGuiResourceResolverManager* manager = GetResourceResolverManager();
				manager->SetTypeResolver(new GuiResourceClassNameRecordTypeResolver);
			}

			void Unload()override
			{
				resourceManager = nullptr;
			}

			bool SetResource(const WString& name, Ptr<GuiResource> resource, GuiResourceUsage usage)override
			{
				vint index = resources.Keys().IndexOf(name);
				if (index != -1) return false;
				
				resource->Initialize(usage);
				resources.Add(name, resource);
				
				auto record = resource->GetValueByPath(L"Precompiled/ClassNameRecord").Cast<GuiResourceClassNameRecord>();
				FOREACH(WString, className, record->classNames)
				{
					instanceResources.Add(className, resource);
				}
				return true;
			}

			Ptr<GuiResource> GetResource(const WString& name)override
			{
				vint index = resources.Keys().IndexOf(name);
				return index == -1 ? nullptr : resources.Values()[index];
			}

			Ptr<GuiResource> GetResourceFromClassName(const WString& classFullName)override
			{
				vint index = instanceResources.Keys().IndexOf(classFullName);
				if (index == -1) return nullptr;
				return instanceResources.Values()[index];
			}
		};
		GUI_REGISTER_PLUGIN(GuiResourceManager)
	}
}
