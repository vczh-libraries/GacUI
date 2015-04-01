#include "GuiResource.h"
#include "GuiDocument.h"
#include "GuiParserManager.h"
#include "../Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace controls;
		using namespace parsing;
		using namespace parsing::tabling;
		using namespace parsing::xml;
		using namespace stream;

/***********************************************************************
Image Type Resolver
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

			void Precompile(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
			}

			IGuiResourceTypeResolver_DirectLoadXml* DirectLoadXml()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			Ptr<parsing::xml::XmlElement> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (auto obj = resource.Cast<GuiImageData>())
				{
					FileStream fileStream(obj->GetFilePath(), FileStream::ReadOnly);
					if (fileStream.IsAvailable())
					{
						auto xmlContent = MakePtr<XmlCData>();
						xmlContent->content.value = BinaryToHex(fileStream);

						auto xmlImage = MakePtr<XmlElement>();
						xmlImage->name.value = L"Image";
						xmlImage->subNodes.Add(xmlContent);
						return xmlImage;
					}
				}
				return 0;
			}

			void SerializePrecompiled(Ptr<DescriptableObject> resource, stream::IStream& stream)override
			{
				auto obj = resource.Cast<GuiImageData>();
				stream::internal::Writer writer(stream);
				FileStream fileStream(obj->GetFilePath(), FileStream::ReadOnly);
				writer << (stream::IStream&)fileStream;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<parsing::xml::XmlElement> element, collections::List<WString>& errors)override
			{
				MemoryStream stream;
				HexToBinary(stream, XmlGetValue(element));
				stream.SeekFromBegin(0);
				auto image = GetCurrentController()->ImageService()->CreateImageFromStream(stream);
				if (image)
				{
					return new GuiImageData(image, 0);
				}
				else
				{
					errors.Add(L"Failed to load an image from binary data in xml.");
					return 0;
				}
			}

			Ptr<DescriptableObject> ResolveResource(const WString& path, collections::List<WString>& errors)override
			{
				Ptr<INativeImage> image = GetCurrentController()->ImageService()->CreateImageFromFile(path);
				if(image)
				{
					return new GuiImageData(image, 0, path);
				}
				else
				{
					errors.Add(L"Failed to load file \"" + path + L"\".");
					return 0;
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(stream::IStream& stream, collections::List<WString>& errors)
			{
				stream::internal::Reader reader(stream);
				MemoryStream memoryStream;
				reader << (stream::IStream&)memoryStream;

				auto image = GetCurrentController()->ImageService()->CreateImageFromStream(memoryStream);
				if (image)
				{
					return new GuiImageData(image, 0);
				}
				else
				{
					errors.Add(L"Failed to load an image from binary data in a stream.");
					return 0;
				}
			}
		};

/***********************************************************************
Text Type Resolver
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

			void Precompile(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
			}

			IGuiResourceTypeResolver_DirectLoadXml* DirectLoadXml()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			Ptr<parsing::xml::XmlElement> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (auto obj = resource.Cast<GuiTextData>())
				{
					auto xmlContent = MakePtr<XmlText>();
					xmlContent->content.value = obj->GetText();

					auto xmlText = MakePtr<XmlElement>();
					xmlText->name.value = L"Text";
					xmlText->subNodes.Add(xmlContent);

					return xmlText;
				}
				return 0;
			}

			void SerializePrecompiled(Ptr<DescriptableObject> resource, stream::IStream& stream)override
			{
				auto obj = resource.Cast<GuiTextData>();
				stream::internal::Writer writer(stream);
				WString text = obj->GetText();
				writer << text;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<parsing::xml::XmlElement> element, collections::List<WString>& errors)override
			{
				return new GuiTextData(XmlGetValue(element));
			}

			Ptr<DescriptableObject> ResolveResource(const WString& path, collections::List<WString>& errors)override
			{
				WString text;
				if(LoadTextFile(path, text))
				{
					return new GuiTextData(text);
				}
				else
				{
					errors.Add(L"Failed to load file \"" + path + L"\".");
					return 0;
				}
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(stream::IStream& stream, collections::List<WString>& errors)
			{
				stream::internal::Reader reader(stream);
				WString text;
				reader << text;
				return new GuiTextData(text);
			}
		};

/***********************************************************************
Xml Type Resolver
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

			void Precompile(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
			}

			IGuiResourceTypeResolver_DirectLoadXml* DirectLoadXml()override
			{
				return this;
			}

			IGuiResourceTypeResolver_DirectLoadStream* DirectLoadStream()override
			{
				return this;
			}

			Ptr<parsing::xml::XmlElement> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (auto obj = resource.Cast<XmlDocument>())
				{
					auto xmlXml = MakePtr<XmlElement>();
					xmlXml->name.value = L"Xml";
					xmlXml->subNodes.Add(obj->rootElement);
					return xmlXml;
				}
				return 0;
			}

			void SerializePrecompiled(Ptr<DescriptableObject> resource, stream::IStream& stream)override
			{
				auto obj = resource.Cast<XmlDocument>();
				MemoryStream buffer;
				{
					StreamWriter writer(buffer);
					XmlPrint(obj, writer);
				}
				{
					buffer.SeekFromBegin(0);
					StreamReader reader(buffer);
					WString text = reader.ReadToEnd();

					stream::internal::Writer writer(stream);
					writer << text;
				}
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<parsing::xml::XmlElement> element, collections::List<WString>& errors)override
			{
				Ptr<XmlElement> root = XmlGetElements(element).First(0);
				if(root)
				{
					Ptr<XmlDocument> xml=new XmlDocument;
					xml->rootElement=root;
					return xml;
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(const WString& path, collections::List<WString>& errors)override
			{
				if(auto parser=GetParserManager()->GetParser<XmlDocument>(L"XML"))
				{
					WString text;
					if(LoadTextFile(path, text))
					{
						return parser->TypedParse(text, errors);
					}
					else
					{
						errors.Add(L"Failed to load file \"" + path + L"\".");
					}
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResourcePrecompiled(stream::IStream& stream, collections::List<WString>& errors)
			{
				stream::internal::Reader reader(stream);
				WString text;
				reader << text;

				auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML");
				return parser->TypedParse(text, errors);
			}
		};

/***********************************************************************
Doc Type Resolver
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

			WString GetPreloadType()override
			{
				return L"Xml";
			}

			bool IsDelayLoad()override
			{
				return true;
			}

			void Precompile(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
			}

			IGuiResourceTypeResolver_IndirectLoad* IndirectLoad()override
			{
				return this;
			}

			Ptr<DescriptableObject> Serialize(Ptr<DescriptableObject> resource, bool serializePrecompiledResource)override
			{
				if (auto obj = resource.Cast<DocumentModel>())
				{
					return obj->SaveToXml();
				}
				return 0;
			}

			Ptr<DescriptableObject> ResolveResource(Ptr<DescriptableObject> resource, Ptr<GuiResourcePathResolver> resolver, collections::List<WString>& errors)override
			{
				Ptr<XmlDocument> xml = resource.Cast<XmlDocument>();
				if(xml)
				{
					Ptr<DocumentModel> model=DocumentModel::LoadFromXml(xml, resolver, errors);
					return model;
				}
				return 0;
			}
		};

/***********************************************************************
Type Resolver Plugin
***********************************************************************/

		class GuiResourceTypeResolverPlugin : public Object, public IGuiPlugin
		{
		public:
			void Load()override
			{
			}

			void AfterLoad()override
			{
				IGuiResourceResolverManager* manager=GetResourceResolverManager();
				manager->SetTypeResolver(new GuiResourceImageTypeResolver);
				manager->SetTypeResolver(new GuiResourceTextTypeResolver);
				manager->SetTypeResolver(new GuiResourceXmlTypeResolver);
				manager->SetTypeResolver(new GuiResourceDocTypeResolver);
			}

			void Unload()override
			{
			}
		};
		GUI_REGISTER_PLUGIN(GuiResourceTypeResolverPlugin)
	}
}