#include "GuiDocumentClipboard.h"
#include "GuiParserManager.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing::xml;
		using namespace stream;

		namespace document_clipboard_visitors
		{
			class TraverseDocumentVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				TraverseDocumentVisitor()
				{
				}

				virtual void VisitContainer(DocumentContainerRun* run)
				{
					FOREACH(Ptr<DocumentRun>, childRun, run->runs)
					{
						childRun->Accept(this);
					}
				}

				void Visit(DocumentTextRun* run)override
				{
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					VisitContainer(run);
				}

				void Visit(DocumentImageRun* run)override
				{
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}
			};

			class ModifyDocumentForClipboardVisitor : public TraverseDocumentVisitor
			{
			public:
				ModifyDocumentForClipboardVisitor()
				{
				}

				void VisitContainer(DocumentContainerRun* run)override
				{
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						auto childRun = run->runs[i];
						if (childRun.Cast<DocumentEmbeddedObjectRun>())
						{
							run->runs.RemoveAt(i);
						}
					}
					TraverseDocumentVisitor::VisitContainer(run);
				}
			};

			class CollectImageRunsVisitor : public TraverseDocumentVisitor
			{
			public:
				List<Ptr<DocumentImageRun>> imageRuns;

				CollectImageRunsVisitor()
				{
				}

				void Visit(DocumentImageRun* run)override
				{
					run->source = L"res://Image_" + itow(imageRuns.Count());
					imageRuns.Add(run);
				}
			};
		}
		using namespace document_clipboard_visitors;

		void ModifyDocumentForClipboard(Ptr<DocumentModel> model)
		{
			ModifyDocumentForClipboardVisitor visitor;
			FOREACH(Ptr<DocumentParagraphRun>, paragraph, model->paragraphs)
			{
				paragraph->Accept(&visitor);
			}
		}

		Ptr<DocumentModel> LoadDocumentFromClipboardStream(stream::IStream& stream)
		{
			auto tempResource = MakePtr<GuiResource>();
			auto tempResourceItem = MakePtr<GuiResourceItem>();
			tempResource->AddItem(L"Document", tempResourceItem);
			auto tempResolver = MakePtr<GuiResourcePathResolver>(tempResource, L"");

			internal::ContextFreeReader reader(stream);
			{
				WString title;
				vint32_t version = 0;
				reader << title << version;

				if (title != L"WCF_Document" || version < 1)
				{
					return nullptr;
				}
			}

			WString xmlText;
			reader << xmlText;
			List<GuiResourceError> errors;
			auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML");
			auto xml = parser->Parse({}, xmlText, errors);
			if (errors.Count() > 0) return nullptr;

			{
				vint32_t count = 0;
				reader << count;
				for (vint i = 0; i < count; i++)
				{
					MemoryStream memoryStream;
					reader << (IStream&)memoryStream;
					if (auto image = GetCurrentController()->ImageService()->CreateImageFromStream(memoryStream))
					{
						auto imageItem = MakePtr<GuiResourceItem>();
						imageItem->SetContent(L"Image", MakePtr<GuiImageData>(image, 0));
						tempResource->AddItem(L"Image_" + itow(i), imageItem);
					}
				}
			}

			auto document = DocumentModel::LoadFromXml(tempResourceItem, xml, tempResolver, errors);
			return document;
		}

		void SaveDocumentToClipboardStream(Ptr<DocumentModel> model, stream::IStream& stream)
		{
			CollectImageRunsVisitor visitor;
			FOREACH(Ptr<DocumentParagraphRun>, paragraph, model->paragraphs)
			{
				paragraph->Accept(&visitor);
			}

			internal::ContextFreeWriter writer(stream);
			{
				WString title = L"WCF_Document";
				vint32_t version = 1;
				writer << title << version;
			}
			{
				auto xmlText = GenerateToStream([&](StreamWriter& streamWriter)
				{
					auto xml = model->SaveToXml();
					XmlPrint(xml, streamWriter);
				});
				writer << xmlText;
			}
			{
				vint32_t count = (vint32_t)visitor.imageRuns.Count();
				writer << count;

				FOREACH(Ptr<DocumentImageRun>, imageRun, visitor.imageRuns)
				{
					MemoryStream memoryStream;
					if (imageRun->image)
					{
						auto format = imageRun->image->GetFormat();
						if (format == INativeImage::Gif)
						{
							format = INativeImage::Png;
						}

						imageRun->image->SaveToStream(memoryStream, format);
					}
					
					writer << (IStream&)memoryStream;
				}
			}
		}
	}
}