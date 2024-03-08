#include "GuiResource.h"
#include "GuiDocument.h"
#include "GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace glr::xml;
		using namespace stream;

/***********************************************************************
Image Type Resolver (Image)
***********************************************************************/

		class GuiResourceImageTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_DirectLoadXml
			, private IGuiResourceTypeResolver_DirectLoadStream
		{
		public:
			WString GetType()override
			{
				return L"Image";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return true;
			}

			IGuiResourceTypeResolver_DirectLoadXml* DirectLoadXml()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			Ptr<glr::xml::XmlElement> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				return nullptr;
			}

			void SerializePrecompiled(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content, stream::IStream& binaryStream)override
			{
				auto obj = content.Cast<GuiImageData>();
				stream::internal::ContextFreeWriter writer(binaryStream);
				FileStream fileStream(resource->GetFileAbsolutePath(), FileStream::ReadOnly);
				writer << (stream::IStream&)fileStream;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlElement> element, GuiResourceError::List& errors)override
			{
				errors.Add(GuiResourceError({ resource }, L"Image should load from file."));
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, const WString& path, GuiResourceError::List& errors)override
			{
				Ptr<INativeImage> image = GetCurrentController()->ImageService()->CreateImageFromFile(path);
				if(image)
				{
					return Ptr(new GuiImageData(image, 0));
				}
				else
				{
					errors.Add(GuiResourceError({ resource }, L"Failed to load file \"" + path + L"\"."));
					return nullptr;
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(Ptr<GuiResourceItem> resource, stream::IStream& binaryStream, GuiResourceError::List& errors)override
			{
				stream::internal::ContextFreeReader reader(binaryStream);
				MemoryStream memoryStream;
				reader << (stream::IStream&)memoryStream;

				auto image = GetCurrentController()->ImageService()->CreateImageFromStream(memoryStream);
				if (image)
				{
					return Ptr(new GuiImageData(image, 0));
				}
				else
				{
					errors.Add(GuiResourceError({ resource }, L"[BINARY] Failed to load an image from binary data in a stream."));
					return nullptr;
				}
			}
		};

/***********************************************************************
Text Type Resolver (Text)
***********************************************************************/

		class GuiResourceTextTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_DirectLoadXml
			, private IGuiResourceTypeResolver_DirectLoadStream
		{
		public:
			WString GetType()override
			{
				return L"Text";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return true;
			}

			IGuiResourceTypeResolver_DirectLoadXml* DirectLoadXml()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			Ptr<glr::xml::XmlElement> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<GuiTextData>())
				{
					auto xmlContent = Ptr(new XmlText);
					xmlContent->content.value = obj->GetText();

					auto xmlText = Ptr(new XmlElement);
					xmlText->name.value = L"Text";
					xmlText->subNodes.Add(xmlContent);

					return xmlText;
				}
				return 0;
			}

			void SerializePrecompiled(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content, stream::IStream& binaryStream)override
			{
				auto obj = content.Cast<GuiTextData>();
				stream::internal::ContextFreeWriter writer(binaryStream);
				WString text = obj->GetText();
				writer << text;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlElement> element, GuiResourceError::List& errors)override
			{
				return Ptr(new GuiTextData(XmlGetValue(element)));
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, const WString& path, GuiResourceError::List& errors)override
			{
				WString text;
				if(LoadTextFile(path, text))
				{
					return Ptr(new GuiTextData(text));
				}
				else
				{
					errors.Add(GuiResourceError({ resource }, L"Failed to load file \"" + path + L"\"."));
					return nullptr;
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(Ptr<GuiResourceItem> resource, stream::IStream& binaryStream, GuiResourceError::List& errors)override
			{
				stream::internal::ContextFreeReader reader(binaryStream);
				WString text;
				reader << text;
				return Ptr(new GuiTextData(text));
			}
		};

/***********************************************************************
Xml Type Resolver (Xml)
***********************************************************************/

		class GuiResourceXmlTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_DirectLoadXml
			, private IGuiResourceTypeResolver_DirectLoadStream
		{
		public:
			WString GetType()override
			{
				return L"Xml";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return true;
			}

			IGuiResourceTypeResolver_DirectLoadXml* DirectLoadXml()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			Ptr<glr::xml::XmlElement> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<XmlDocument>())
				{
					auto xmlXml = Ptr(new XmlElement());
					xmlXml->name.value = L"Xml";
					xmlXml->subNodes.Add(obj->rootElement);
					return xmlXml;
				}
				return nullptr;
			}

			void SerializePrecompiled(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content, stream::IStream& binaryStream)override
			{
				auto obj = content.Cast<XmlDocument>();
				WString text = GenerateToStream([&](StreamWriter& writer)
				{
					XmlPrint(obj, writer);
				});
				stream::internal::ContextFreeWriter writer(binaryStream);
				writer << text;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<glr::xml::XmlElement> element, GuiResourceError::List& errors)override
			{
				Ptr<XmlElement> root = XmlGetElements(element).First(0);
				if(root)
				{
					auto xml = Ptr(new XmlDocument);
					xml->rootElement=root;
					return xml;
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, const WString& path, GuiResourceError::List& errors)override
			{
				if (auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML"))
				{
					WString text;
					if (LoadTextFile(path, text))
					{
						return parser->Parse({ resource }, text, errors);
					}
					else
					{
						errors.Add(GuiResourceError({ resource }, L"Failed to load file \"" + path + L"\"."));
					}
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(Ptr<GuiResourceItem> resource, stream::IStream& binaryStream, GuiResourceError::List& errors)override
			{
				if (auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML"))
				{
					stream::internal::ContextFreeReader reader(binaryStream);
					WString text;
					reader << text;

					return parser->Parse({ resource }, text, errors);
				}
				return nullptr;
			}
		};

/***********************************************************************
Doc Type Resolver (Doc)
***********************************************************************/

		class GuiResourceDocTypeResolver
			: public Object
			, public IGuiResourceTypeResolver
			, private IGuiResourceTypeResolver_IndirectLoad
		{
		public:
			WString GetType()override
			{
				return L"Doc";
			}

			bool XmlSerializable()override
			{
				return true;
			}

			bool StreamSerializable()override
			{
				return true;
			}

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return true;
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<GuiResourceItem> resource, Ptr<DescriptableObject> content)override
			{
				if (auto obj = content.Cast<DocumentModel>())
				{
					return obj->SaveToXml();
				}
				return nullptr;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<GuiResourceItem> resource, Ptr<GuiResourcePathResolver> resolver, GuiResourceError::List& errors)override
			{
				if(auto xml = resource->GetContent().Cast<XmlDocument>())
				{
					Ptr<DocumentModel> model = DocumentModel::LoadFromXml(resource, xml, resolver, errors);
					return model;
				}
				return nullptr;
			}
		};

/***********************************************************************
Type Resolver Plugin
***********************************************************************/

		class GuiResourceTypeResolversPlugin : public Object, public IGuiPlugin
		{
		public:

			GUI_PLUGIN_NAME(GacUI_Res_TypeResolvers)
			{
				GUI_PLUGIN_DEPEND(GacUI_Res_ResourceResolver);
			}

			void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
				if (controllerUnrelatedPlugins)
				{
					IGuiResourceResolverManager* manager = GetResourceResolverManager();
					manager->SetTypeResolver(Ptr(new GuiResourceImageTypeResolver));
					manager->SetTypeResolver(Ptr(new GuiResourceTextTypeResolver));
					manager->SetTypeResolver(Ptr(new GuiResourceXmlTypeResolver));
					manager->SetTypeResolver(Ptr(new GuiResourceDocTypeResolver));
				}
			}

			void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins)override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiResourceTypeResolversPlugin)
	}
}
