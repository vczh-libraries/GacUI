#include "GuiResourceManager.h"
#include "GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace stream;
		using namespace parsing::xml;
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

			List<Ptr<GuiResource>>					anonymousResources;
			ResourceMap								resources;
			ResourceMap								instanceResources;

			class PendingResource : public Object
			{
			public:
				Ptr<GuiResourceMetadata>			metadata;
				GuiResourceUsage					usage;
				MemoryStream						memoryStream;
				SortedList<WString>					dependencies;

				Ptr<GuiResource> LoadResource()
				{
					memoryStream.SeekFromBegin(0);
					List<GuiResourceError> errors;
					auto resource = GuiResource::LoadPrecompiledBinary(memoryStream, errors);
					CHECK_ERROR(errors.Count() == 0, L"PendingResource::LoadResource()#Failed to load the resource.");
					return resource;
				}
			};
			Group<WString, Ptr<PendingResource>>	depToPendings;
			SortedList<Ptr<PendingResource>>		pendingResources;

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

			void SetResource(Ptr<GuiResource> resource, GuiResourceError::List& errors, GuiResourceUsage usage)override
			{
				auto metadata = resource->GetMetadata();
				if (metadata->name == L"")
				{
					if (anonymousResources.Contains(resource.Obj())) return;
					resource->Initialize(usage, errors);
					if (errors.Count() > 0)
					{
						return;
					}
					anonymousResources.Add(resource);
				}
				else
				{
					CHECK_ERROR(!resources.Keys().Contains(metadata->name), L"GuiResourceManager::SetResource(Ptr<GuiResource>, GuiResourceUsage)#A resource with the same name has been loaded.");

					resource->Initialize(usage, errors);
					if (errors.Count() > 0)
					{
						return;
					}
					resources.Add(metadata->name, resource);
				}
				
				if (auto record = resource->GetValueByPath(L"Precompiled/ClassNameRecord").Cast<GuiResourceClassNameRecord>())
				{
					FOREACH(WString, className, record->classNames)
					{
						instanceResources.Add(className, resource);
					}
				}

				if (metadata->name != L"")
				{
					vint index = depToPendings.Keys().IndexOf(metadata->name);
					if (index != -1)
					{
						List<Ptr<PendingResource>> prs;
						CopyFrom(prs, depToPendings.GetByIndex(index));
						depToPendings.Remove(metadata->name);

						FOREACH(Ptr<PendingResource>, pr, prs)
						{
							pr->dependencies.Remove(metadata->name);
							if (pr->dependencies.Count() == 0)
							{
								pendingResources.Remove(pr.Obj());
								SetResource(pr->LoadResource(), errors, pr->usage);
							}
						}
					}
				}
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

			void UnloadResource(const WString& name)override
			{
				vint index = resources.Keys().IndexOf(name);
				if (index != -1)
				{
					auto resource = resources.Values()[index];
					resources.Remove(name);

					if (auto record = resource->GetValueByPath(L"Precompiled/ClassNameRecord").Cast<GuiResourceClassNameRecord>())
					{
						FOREACH(WString, className, record->classNames)
						{
							instanceResources.Remove(className);
						}
					}
				}
			}

			void LoadResourceOrPending(stream::IStream& stream, GuiResourceError::List& errors, GuiResourceUsage usage)override
			{
				auto pr = MakePtr<PendingResource>();
				pr->usage = usage;
				CopyStream(stream, pr->memoryStream);

				pr->metadata = MakePtr<GuiResourceMetadata>();
				{
					pr->memoryStream.SeekFromBegin(0);
					stream::internal::ContextFreeReader reader(pr->memoryStream);
					WString metadata;
					reader << metadata;

					List<GuiResourceError> errors;
					auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML");
					auto xmlMetadata = parser->Parse({}, metadata, errors);
					CHECK_ERROR(xmlMetadata, L"GuiResourceManager::LoadResourceOrPending(stream::IStream&, GuiResourceUsage)#This resource does not contain a valid metadata.");
					pr->metadata->LoadFromXml(xmlMetadata, {}, errors);
					CHECK_ERROR(errors.Count() == 0, L"GuiResourceManager::LoadResourceOrPending(stream::IStream&, GuiResourceUsage)#This resource does not contain a valid metadata.");
				}

				CHECK_ERROR(
					pr->metadata->name != L"" || pr->dependencies.Count() == 0,
					L"GuiResourceManager::LoadResourceOrPending(stream::IStream&, GuiResourceUsage)#The name of this resource cannot be empty because it has dependencies."
				);
				CopyFrom(pr->dependencies, From(pr->metadata->dependencies).Except(resources.Keys()));

				if (pr->dependencies.Count() == 0)
				{
					SetResource(pr->LoadResource(), errors, pr->usage);
				}
				else
				{
					pendingResources.Add(pr);
					FOREACH(WString, dep, pr->dependencies)
					{
						depToPendings.Add(dep, pr);
					}
				}
			}

			void LoadResourceOrPending(stream::IStream& stream, GuiResourceUsage usage)override
			{
				GuiResourceError::List errors;
				LoadResourceOrPending(stream, errors, usage);
				CHECK_ERROR(errors.Count() == 0, L"GuiResourceManager::LoadResourceOrPending(stream::IStream&, GuiResourceUsage)#Error happened.");
			}

			void GetPendingResourceNames(collections::List<WString>& names)override
			{
				CopyFrom(names, From(pendingResources).Select([](Ptr<PendingResource> pr) {return pr->metadata->name; }));
			}
		};
		GUI_REGISTER_PLUGIN(GuiResourceManager)
	}
}
