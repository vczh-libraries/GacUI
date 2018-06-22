#include "GuiDocumentClipboard.h"

namespace vl
{
	namespace presentation
	{

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
	}
}