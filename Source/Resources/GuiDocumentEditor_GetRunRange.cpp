#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Calculate range informations for each run object
***********************************************************************/

		namespace document_operation_visitors
		{
			class GetRunRangeVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&					runRanges;
				vint							start;

				GetRunRangeVisitor(RunRangeMap& _runRanges)
					:runRanges(_runRanges)
					, start(0)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					RunRange range;
					range.start = start;
					for (auto subRun : run->runs)
					{
						subRun->Accept(this);
					}
					range.end = start;
					runRanges.Add(run, range);
				}

				void VisitContent(DocumentContentRun* run)
				{
					RunRange range;
					range.start = start;
					start += run->GetRepresentationText().Length();
					range.end = start;
					runRanges.Add(run, range);
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
			void GetRunRange(DocumentParagraphRun* run, RunRangeMap& runRanges)
			{
				GetRunRangeVisitor visitor(runRanges);
				run->Accept(&visitor);
			}
		}
	}
}