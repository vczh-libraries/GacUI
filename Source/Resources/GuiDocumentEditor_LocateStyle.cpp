#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Get all container runs that contain the specified position from top to bottom
***********************************************************************/

		namespace document_operation_visitors
		{
			class LocateStyleVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				List<DocumentContainerRun*>&	locatedRuns;
				RunRangeMap&					runRanges;
				vint							position;
				bool							frontSide;

				LocateStyleVisitor(List<DocumentContainerRun*>& _locatedRuns, RunRangeMap& _runRanges, vint _position, bool _frontSide)
					:locatedRuns(_locatedRuns)
					, runRanges(_runRanges)
					, position(_position)
					, frontSide(_frontSide)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					locatedRuns.Add(run);
					Ptr<DocumentRun> selectedRun;
					for (auto subRun : run->runs)
					{
						RunRange range = runRanges[subRun.Obj()];
						if (position == range.start)
						{
							if (!frontSide)
							{
								selectedRun = subRun;
								break;
							}
						}
						else if (position == range.end)
						{
							if (frontSide)
							{
								selectedRun = subRun;
								break;
							}
						}
						else if (range.start<position && position<range.end)
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
		using namespace document_operation_visitors;

		namespace document_editor
		{
			void LocateStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint position, bool frontSide, List<DocumentContainerRun*>& locatedRuns)
			{
				LocateStyleVisitor visitor(locatedRuns, runRanges, position, frontSide);
				run->Accept(&visitor);
			}
		}
	}
}