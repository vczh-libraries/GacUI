#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Remove text run contents with the specified range, or other content runs that intersect with the range
If a run decides that itself should be removed, then replacedRuns contains all runs to replace itself
***********************************************************************/

		namespace document_operation_visitors
		{
			class RemoveRunVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&					runRanges;
				vint							start;
				vint							end;
				List<Ptr<DocumentRun>>			replacedRuns;

				RemoveRunVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:runRanges(_runRanges)
					, start(_start)
					, end(_end)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					if (start == end) return;
					// TODO: (enumerable) foreach:indexed(alterable(reversed))
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						Ptr<DocumentRun> subRun = run->runs[i];
						RunRange range = runRanges[subRun.Obj()];

						vint maxStart = range.start > start ? range.start : start;
						vint minEnd = range.end < end ? range.end : end;
						if (maxStart < minEnd)
						{
							subRun->Accept(this);
							if (replacedRuns.Count() == 0 || subRun != replacedRuns[0])
							{
								run->runs.RemoveAt(i);
								// TODO: (enumerable) foreach
								for (vint j = 0; j<replacedRuns.Count(); j++)
								{
									run->runs.Insert(i + j, replacedRuns[j]);
								}
							}
						}
					}
					replacedRuns.Clear();
					replacedRuns.Add(Ptr(run));
				}

				void Visit(DocumentTextRun* run)override
				{
					replacedRuns.Clear();
					RunRange range = runRanges[run];

					if (start <= range.start)
					{
						if (end<range.end)
						{
							run->text = run->text.Sub(end - range.start, range.end - end);
							replacedRuns.Add(Ptr(run));
						}
					}
					else
					{
						if (end<range.end)
						{
							auto firstRun = Ptr(new DocumentTextRun);
							auto secondRun = Ptr(new DocumentTextRun);

							firstRun->text = run->text.Sub(0, start - range.start);
							secondRun->text = run->text.Sub(end - range.start, range.end - end);

							replacedRuns.Add(firstRun);
							replacedRuns.Add(secondRun);
						}
						else
						{
							run->text = run->text.Sub(0, start - range.start);
							replacedRuns.Add(Ptr(run));
						}
					}
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
					replacedRuns.Clear();
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					replacedRuns.Clear();
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
			void RemoveRun(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
			{
				RemoveRunVisitor visitor(runRanges, start, end);
				run->Accept(&visitor);
			}
		}
	}
}