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
					run->source = L"clipboard://" + itow(imageRuns.Count());
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
			{
				vint32_t count = 0;
				if (stream.Read(&count, sizeof(count)) != sizeof(count)) return nullptr;
				for (vint i = 0; i < count; i++)
				{
					vint32_t size = 0;
					if (stream.Read(&size, sizeof(size)) != sizeof(size)) return nullptr;
					if (size > 0)
					{
						Array<char> buffer(size);
						if (stream.Read(&buffer[0], size) != size) return nullptr;
						auto image = GetCurrentController()->ImageService()->CreateImageFromMemory(&buffer[0], buffer.Count());
					}
				}
			}

			StreamReader streamReader(stream);
			auto text = streamReader.ReadToEnd();
			List<GuiResourceError> errors;

			auto parser = GetParserManager()->GetParser<XmlDocument>(L"XML");
			auto xml = parser->Parse({}, text, errors);
			if (errors.Count() > 0) return nullptr;

			auto tempResource = MakePtr<GuiResource>();
			auto tempResourceItem = MakePtr<GuiResourceItem>();
			tempResource->AddItem(L"Document", tempResourceItem);
			auto tempResolver = MakePtr<GuiResourcePathResolver>(tempResource, L"");

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
			{
				vint32_t count = visitor.imageRuns.Count();
				stream.Write(&count, sizeof(count));
				FOREACH(Ptr<DocumentImageRun>, imageRun, visitor.imageRuns)
				{
					stream::MemoryStream memoryStream;
					if (imageRun->image)
					{
						imageRun->image->SaveToStream(memoryStream);
					}
					
					count = (vint32_t)memoryStream.Size();
					stream.Write(&count, sizeof(count));
					if (count > 0)
					{
						stream.Write(memoryStream.GetInternalBuffer(), count);
					}
				}
			}

			StreamWriter streamWriter(stream);
			auto xml = model->SaveToXml();
			XmlPrint(xml, streamWriter);
		}
	}
}