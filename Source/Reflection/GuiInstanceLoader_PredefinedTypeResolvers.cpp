#include "GuiInstanceLoader.h"
#include "TypeDescriptors/GuiReflectionEvents.h"
#include "../Resources/GuiParserManager.h"
#include "InstanceQuery/GuiInstanceQuery.h"
#include "GuiInstanceSchemaRepresentation.h"
#include "GuiInstanceSharedScript.h"
#include "GuiInstanceLoader_WorkflowCompiler.h"

namespace vl
{
	namespace presentation
	{
		using namespace parsing;
		using namespace parsing::xml;
		using namespace workflow::analyzer;
		using namespace workflow::runtime;
		using namespace reflection::description;
		using namespace collections;

#define ERROR_CODE_PREFIX L"================================================================"

/***********************************************************************
Instance Type Resolver
***********************************************************************/

		class GuiResourceInstanceTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_DirectLoadStream
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Instance";
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return 2;
			}

			void Precompile(Ptr<DescriptableObject> resource, GuiResource* rootResource, vint passIndex, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				if (passIndex == 2)
				{
					if (auto obj = resource.Cast<GuiInstanceContext>())
					{
						obj->ApplyStyles(resolver, errors);
						Workflow_PrecompileInstanceContext(obj, errors);
					}
				}
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			void SerializePrecompiled(Ptr<DescriptableObject> resource, stream::IStream& stream)override
			{
				auto obj = resource.Cast<GuiInstanceContext>();
				obj->SavePrecompiledBinary(stream);
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(stream::IStream& stream, collections::List<WString>& errors)override
			{
				return GuiInstanceContext::LoadPrecompiledBinary(stream, errors);
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (auto obj = resource.Cast<GuiInstanceContext>())
				{
					return obj->SaveToXml(serializePrecompiledResource);
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					Ptr<GuiInstanceContext> context = GuiInstanceContext::LoadFromXml(xml, errors);
					return context;
				}
				return 0;
			}
		};

/***********************************************************************
Instance Style Type Resolver
***********************************************************************/

		class GuiResourceInstanceStyleResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"InstanceStyle";
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (!serializePrecompiledResource)
				{
					if (auto obj = resource.Cast<GuiInstanceStyleContext>())
					{
						return obj->SaveToXml();
					}
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					auto context = GuiInstanceStyleContext::LoadFromXml(xml, errors);
					return context;
				}
				return 0;
			}
		};

/***********************************************************************
Instance Schema Type Resolver
***********************************************************************/

		class GuiResourceInstanceSchemaTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"InstanceSchema";
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (!serializePrecompiledResource)
				{
					if (auto obj = resource.Cast<GuiInstanceSchema>())
					{
						return obj->SaveToXml();
					}
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					auto schema = GuiInstanceSchema::LoadFromXml(xml, errors);
					return schema;
				}
				return 0;
			}
		};

/***********************************************************************
Shared Script Type Resolver
***********************************************************************/

		class GuiSharedWorkflowCache : public Object, public IGuiResourceCache
		{
		public:
			static const GlobalStringKey&					CacheTypeName;
			static const GlobalStringKey&					CacheContextName;

			List<WString>									moduleCodes;
			Ptr<WfAssembly>									assembly;
			Ptr<WfRuntimeGlobalContext>						globalContext;

			GuiSharedWorkflowCache()
			{
			}

			GuiSharedWorkflowCache(Ptr<WfAssembly> _assembly)
				:assembly(_assembly)
			{
				Initialize();
			}

			GlobalStringKey GetCacheTypeName()override
			{
				return CacheTypeName;
			}

			void Initialize()
			{
				if (!globalContext)
				{
					globalContext = new WfRuntimeGlobalContext(assembly);
					LoadFunction<void()>(globalContext, L"<initialize>")();
				}
			}
		};

		const GlobalStringKey& GuiSharedWorkflowCache::CacheTypeName = GlobalStringKey::_Shared_Workflow_Assembly_Cache;
		const GlobalStringKey& GuiSharedWorkflowCache::CacheContextName = GlobalStringKey::_Shared_Workflow_Global_Context;

		class GuiSharedWorkflowCacheResolver : public Object, public IGuiResourceCacheResolver 
		{
		public:
			GlobalStringKey GetCacheTypeName()override
			{
				return GuiSharedWorkflowCache::CacheTypeName;
			}

			bool Serialize(Ptr<IGuiResourceCache> cache, stream::IStream& stream)override
			{
				if (auto obj = cache.Cast<GuiSharedWorkflowCache>())
				{
					obj->assembly->Serialize(stream);
					return true;
				}
				else
				{
					return false;
				}
			}

			Ptr<IGuiResourceCache> Deserialize(stream::IStream& stream)override
			{
				auto assembly = new WfAssembly(stream);
				return new GuiSharedWorkflowCache(assembly);
			}
		};

		class GuiResourceSharedScriptTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_Precompile
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Script";
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return false;
			}

			vint GetMaxPassIndex()override
			{
				return 1;
			}

			void Precompile(Ptr<DescriptableObject> resource, GuiResource* rootResource, vint passIndex, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				if (passIndex == 0)
				{
					if (auto obj = resource.Cast<GuiInstanceSharedScript>())
					{
						if (obj->language == L"Workflow")
						{
							Ptr<GuiSharedWorkflowCache> cache;
							auto key = GuiSharedWorkflowCache::CacheContextName;
							auto index = rootResource->precompiledCaches.Keys().IndexOf(key);

							if (index == -1)
							{
								cache = new GuiSharedWorkflowCache;
								rootResource->precompiledCaches.Add(key, cache);
							}
							else
							{
								cache = rootResource->precompiledCaches.Values()[index].Cast<GuiSharedWorkflowCache>();
							}
							cache->moduleCodes.Add(obj->code);
						}
					}
				}
				else if (passIndex == 1)
				{
					Ptr<GuiSharedWorkflowCache> cache;
					auto key = GuiSharedWorkflowCache::CacheContextName;
					auto index = rootResource->precompiledCaches.Keys().IndexOf(key);

					if (index != -1)
					{
						auto cache = rootResource->precompiledCaches.Values()[index].Cast<GuiSharedWorkflowCache>();
						if (cache->moduleCodes.Count() > 0)
						{
							auto table = GetParserManager()->GetParsingTable(L"WORKFLOW");
							List<Ptr<ParsingError>> scriptErrors;
							cache->assembly = Compile(table, cache->moduleCodes, scriptErrors);
							cache->moduleCodes.Clear();

							if (scriptErrors.Count() > 0)
							{
								errors.Add(ERROR_CODE_PREFIX L"Failed to parse the shared workflow script");
								FOREACH(Ptr<ParsingError>, error, scriptErrors)
								{
									errors.Add(
										L"Row: " + itow(error->codeRange.start.row + 1) +
										L", Column: " + itow(error->codeRange.start.column + 1) +
										L", Message: " + error->errorMessage);
								}
							}
							else
							{
								cache->Initialize();
							}
						}
					}
				}
			}

			IGuiResourceTypeResolver_Precompile* Precompile()override
			{
				return this;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (!serializePrecompiledResource)
				{
					if (auto obj = resource.Cast<GuiInstanceSharedScript>())
					{
						return obj->SaveToXml();
					}
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if (xml)
				{
					auto schema = GuiInstanceSharedScript::LoadFromXml(xml, errors);
					return schema;
				}
				return 0;
			}
		};

/***********************************************************************
Shared Script Type Resolver
***********************************************************************/

		class GuiPredefinedTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:
			GuiPredefinedTypeResolversPlugin()
			{
			}

			void Load()override
			{
			}

			void AfterLoad()override
			{
				IGuiResourceResolverManager* manager = GetResourceResolverManager();
				manager->SetTypeResolver(new GuiResourceInstanceTypeResolver);
				manager->SetTypeResolver(new GuiResourceInstanceStyleResolver);
				manager->SetTypeResolver(new GuiResourceInstanceSchemaTypeResolver);
				manager->SetTypeResolver(new GuiResourceSharedScriptTypeResolver);
				manager->SetCacheResolver(new GuiSharedWorkflowCacheResolver);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiPredefinedTypeResolversPlugin)
	}
}