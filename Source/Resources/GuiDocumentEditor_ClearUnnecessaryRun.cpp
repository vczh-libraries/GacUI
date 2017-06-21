#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace document_editor;

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

/***********************************************************************
Compress run to merge styles
***********************************************************************/

		namespace document_operation_visitors
		{
			class CompressStyleRunVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				List<Ptr<DocumentRun>>			replacedRuns;

				void VisitContainer(DocumentContainerRun* run)
				{
					for (vint i = run->runs.Count() - 1; i >= 0; i--)
					{
						Ptr<DocumentRun> subRun = run->runs[i];
						replacedRuns.Clear();
						subRun->Accept(this);
						if (replacedRuns.Count() > 0)
						{
							run->runs.RemoveAt(i);
							for (vint j = 0; j < replacedRuns.Count(); j++)
							{
								run->runs.Insert(i + j, replacedRuns[j]);
							}
							i += replacedRuns.Count();
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					VisitContainer(run);
					if (From(run->runs).Cast<DocumentStylePropertiesRun>().First(nullptr) != nullptr)
					{
						FOREACH(Ptr<DocumentRun>, subRun, run->runs)
						{
							if (auto styleRun = subRun.Cast<DocumentStylePropertiesRun>())
							{
								DocumentModel::MergeStyle(styleRun->style, run->style);
								replacedRuns.Add(styleRun);
							}
							else
							{
								auto parentRun = CopyRun(run).Cast<DocumentStylePropertiesRun>();
								parentRun->runs.Add(subRun);
								replacedRuns.Add(parentRun);
							}
						}
					}
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
			void ClearUnnecessaryRun(DocumentParagraphRun* run)
			{
				{
					ClearRunVisitor visitor;
					run->Accept(&visitor);
				}
				{
					CompressStyleRunVisitor visitor;
					run->Accept(&visitor);
				}
			}
		}
	}
}