#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
Remove some containers that intersect with the specified range
If a run decides that itself should be removed, then replacedRuns contains all runs to replace itself
RemoveHyperlinkVisitor	: Remove all hyperlinks that intersect with the specified range
RemoveStyleNameVisitor	: Remove all style names that intersect with the specified range
ClearStyleVisitor		: Remove all styles that intersect with the specified range
***********************************************************************/

		namespace document_operation_visitors
		{
			class RemoveContainerVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&					runRanges;
				vint							start;
				vint							end;
				List<Ptr<DocumentRun>>			replacedRuns;

				RemoveContainerVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:runRanges(_runRanges)
					, start(_start)
					, end(_end)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					// TODO: (enumerable) foreach:indexed(alterable(reversed))
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						Ptr<DocumentRun> subRun = run->runs[i];
						RunRange range = runRanges[subRun.Obj()];
						if (range.start<end && start<range.end)
						{
							replacedRuns.Clear();
							subRun->Accept(this);
							if (replacedRuns.Count() != 1 || replacedRuns[0] != subRun)
							{
								run->runs.RemoveAt(i);
								// TODO: (enumerable) foreach
								for (vint j = 0; j<replacedRuns.Count(); j++)
								{
									run->runs.Insert(i + j, replacedRuns[j]);
								}
								i += replacedRuns.Count();
							}
						}
					}
					replacedRuns.Clear();
					replacedRuns.Add(Ptr(run));
				}

				void VisitContent(DocumentContentRun* run)
				{
					replacedRuns.Add(Ptr(run));
				}

				void RemoveContainer(DocumentContainerRun* run)
				{
					CopyFrom(replacedRuns, run->runs);
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

			class RemoveHyperlinkVisitor : public RemoveContainerVisitor
			{
			public:
				RemoveHyperlinkVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:RemoveContainerVisitor(_runRanges, _start, _end)
				{
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					RemoveContainer(run);
				}
			};

			class RemoveStyleNameVisitor : public RemoveContainerVisitor
			{
			public:
				RemoveStyleNameVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:RemoveContainerVisitor(_runRanges, _start, _end)
				{
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					RemoveContainer(run);
				}
			};

			class ClearStyleVisitor : public RemoveContainerVisitor
			{
			public:
				ClearStyleVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:RemoveContainerVisitor(_runRanges, _start, _end)
				{
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					RemoveContainer(run);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					RemoveContainer(run);
				}
			};
		}
		using namespace document_operation_visitors;

		namespace document_editor
		{
			void RemoveHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
			{
				RemoveHyperlinkVisitor visitor(runRanges, start, end);
				run->Accept(&visitor);
			}

			void RemoveStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
			{
				RemoveStyleNameVisitor visitor(runRanges, start, end);
				run->Accept(&visitor);
			}

			void ClearStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
			{
				ClearStyleVisitor visitor(runRanges, start, end);
				run->Accept(&visitor);
			}
		}
	}
}