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
			class ModifyDocumentForClipboardVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				ModifyDocumentForClipboardVisitor()
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						auto childRun = run->runs[i];
						if (childRun.Cast<DocumentImageRun>() || childRun.Cast<DocumentEmbeddedObjectRun>())
						{
							run->runs.RemoveAt(i);
						}
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
			StreamWriter streamWriter(stream);
			auto xml = model->SaveToXml();
			XmlPrint(xml, streamWriter);
		}
	}
}