#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Get the hyperlink run that contains the specified position
***********************************************************************/

		namespace document_operation_visitors
		{
			class LocateHyperlinkVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				Ptr<DocumentHyperlinkRun>		hyperlink;
				RunRangeMap&					runRanges;
				vint							start;
				vint							end;

				LocateHyperlinkVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:runRanges(_runRanges)
					, start(_start)
					, end(_end)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					Ptr<DocumentRun> selectedRun;
					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						RunRange range = runRanges[subRun.Obj()];
						if (range.start <= start && end <= range.end)
						{
							selectedRun = subRun;
							break;
						}
					}

					if (selectedRun)
					{
						selectedRun->Accept(this);
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
					hyperlink = run;
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
		using namespace document_operation_visitors;

		namespace document_editor
		{
			Ptr<DocumentHyperlinkRun> LocateHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
			{
				LocateHyperlinkVisitor visitor(runRanges, start, end);
				run->Accept(&visitor);
				return visitor.hyperlink;
			}
		}
	}
}