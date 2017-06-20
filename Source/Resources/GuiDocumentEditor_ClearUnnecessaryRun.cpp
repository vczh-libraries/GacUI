#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Clear all runs that have an empty length
***********************************************************************/

		namespace document_operation_visitors
		{
			class ClearRunVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				vint							start;

				ClearRunVisitor()
					:start(0)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						vint oldStart = start;
						run->runs[i]->Accept(this);
						if (oldStart == start)
						{
							run->runs.RemoveAt(i);
						}
					}
				}

				void VisitContent(DocumentContentRun* run)
				{
					start += run->GetRepresentationText().Length();
				}

				void Visit(DocumentTextRun* run)override
				{
					VisitContent(run);
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
					VisitContent(run);
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					VisitContent(run);
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}
			};
		}
		using namespace document_operation_visitors;

		namespace document_editor
		{
			void ClearUnnecessaryRun(DocumentParagraphRun* run)
			{
				ClearRunVisitor visitor;
				run->Accept(&visitor);
			}
		}
	}
}