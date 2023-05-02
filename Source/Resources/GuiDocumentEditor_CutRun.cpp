#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace document_editor;

/***********************************************************************
Cut all runs into pieces so that a run either completely outside or inside the specified range
If a run decides that itself should be cut, then leftRun and rightRun contains new run that will be inserted before and after it
***********************************************************************/

		namespace document_operation_visitors
		{
			class CutRunVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&					runRanges;
				vint							position;
				Ptr<DocumentRun>				leftRun;
				Ptr<DocumentRun>				rightRun;

				CutRunVisitor(RunRangeMap& _runRanges, vint _position)
					:runRanges(_runRanges)
					, position(_position)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					vint leftCount = 0;
					Ptr<DocumentRun> selectedRun;

					for (auto subRun : run->runs)
					{
						RunRange range = runRanges[subRun.Obj()];
						if (range.start<position)
						{
							leftCount++;
							if (position<range.end)
							{
								selectedRun = subRun;
							}
						}
						else
						{
							break;
						}
					}

					if (selectedRun)
					{
						selectedRun->Accept(this);
						if (leftRun && rightRun)
						{
							run->runs.RemoveAt(leftCount - 1);
							run->runs.Insert(leftCount - 1, leftRun);
							run->runs.Insert(leftCount, rightRun);
						}
					}

					Ptr<DocumentContainerRun> leftContainer = CopyRun(run).Cast<DocumentContainerRun>();
					Ptr<DocumentContainerRun> rightContainer = CopyRun(run).Cast<DocumentContainerRun>();
					// TODO: (enumerable) foreach
					for (vint i = 0; i<run->runs.Count(); i++)
					{
						(i<leftCount ? leftContainer : rightContainer)->runs.Add(run->runs[i]);
					}
					leftRun = leftContainer;
					rightRun = rightContainer;
				}

				void Visit(DocumentTextRun* run)override
				{
					RunRange range = runRanges[run];

					auto leftText = Ptr(new DocumentTextRun);
					leftText->text = run->text.Sub(0, position - range.start);

					auto rightText = Ptr(new DocumentTextRun);
					rightText->text = run->text.Sub(position - range.start, range.end - position);

					leftRun = leftText;
					rightRun = rightText;
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
					leftRun = 0;
					rightRun = 0;
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					leftRun = 0;
					rightRun = 0;
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
			void CutRun(DocumentParagraphRun* run, RunRangeMap& runRanges, vint position, Ptr<DocumentRun>& leftRun, Ptr<DocumentRun>& rightRun)
			{
				CutRunVisitor visitor(runRanges, position);
				run->Accept(&visitor);
				leftRun = visitor.leftRun;
				rightRun = visitor.rightRun;
			}
		}
	}
}