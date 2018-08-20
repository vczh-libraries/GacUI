#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace document_editor;

/***********************************************************************
Clone the current run without its children
If clonedRun field is assigned then it will be added to the cloned container run
***********************************************************************/

		namespace document_operation_visitors
		{
			class CloneRunVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				Ptr<DocumentRun>				clonedRun;

				CloneRunVisitor(Ptr<DocumentRun> subRun)
					:clonedRun(subRun)
				{
				}

				void VisitContainer(Ptr<DocumentContainerRun> cloned)
				{
					if (clonedRun)
					{
						cloned->runs.Add(clonedRun);
					}
					clonedRun = cloned;
				}

				void Visit(DocumentTextRun* run)override
				{
					Ptr<DocumentTextRun> cloned = new DocumentTextRun;
					cloned->text = run->text;
					clonedRun = cloned;
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					Ptr<DocumentStylePropertiesRun> cloned = new DocumentStylePropertiesRun;
					cloned->style = CopyStyle(run->style);
					VisitContainer(cloned);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					Ptr<DocumentStyleApplicationRun> cloned = new DocumentStyleApplicationRun;
					cloned->styleName = run->styleName;

					VisitContainer(cloned);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					Ptr<DocumentHyperlinkRun> cloned = new DocumentHyperlinkRun;
					cloned->styleName = run->styleName;
					cloned->normalStyleName = run->normalStyleName;
					cloned->activeStyleName = run->activeStyleName;
					cloned->reference = run->reference;

					VisitContainer(cloned);
				}

				void Visit(DocumentImageRun* run)override
				{
					Ptr<DocumentImageRun> cloned = new DocumentImageRun;
					cloned->size = run->size;
					cloned->baseline = run->baseline;
					cloned->image = run->image;
					cloned->frameIndex = run->frameIndex;
					cloned->source = run->source;
					clonedRun = cloned;
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					Ptr<DocumentEmbeddedObjectRun> cloned = new DocumentEmbeddedObjectRun;
					cloned->name = run->name;
					clonedRun = cloned;
				}

				void Visit(DocumentParagraphRun* run)override
				{
					Ptr<DocumentParagraphRun> cloned = new DocumentParagraphRun;
					cloned->alignment = run->alignment;

					VisitContainer(cloned);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
Clone the current run with its children
***********************************************************************/

		namespace document_operation_visitors
		{
			class CloneRunRecursivelyVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				Ptr<DocumentRun>				clonedRun;
				RunRangeMap&					runRanges;
				vint							start;
				vint							end;
				bool							deepCopy;

				CloneRunRecursivelyVisitor(RunRangeMap& _runRanges, vint _start, vint _end, bool _deepCopy)
					:runRanges(_runRanges)
					, start(_start)
					, end(_end)
					, deepCopy(_deepCopy)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					clonedRun = 0;
					RunRange range = runRanges[run];
					if (range.start <= end && start <= range.end)
					{
						if (start <= range.start && range.end <= end && !deepCopy)
						{
							clonedRun = run;
						}
						else
						{
							Ptr<DocumentContainerRun> containerRun = CopyRun(run).Cast<DocumentContainerRun>();
							FOREACH(Ptr<DocumentRun>, subRun, run->runs)
							{
								subRun->Accept(this);
								if (clonedRun)
								{
									containerRun->runs.Add(clonedRun);
								}
							}
							clonedRun = containerRun;
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
					clonedRun = 0;
					RunRange range = runRanges[run];
					if (range.start<end && start<range.end)
					{
						if (start <= range.start && range.end <= end)
						{
							if (deepCopy)
							{
								clonedRun = CopyRun(run);
							}
							else
							{
								clonedRun = run;
							}
						}
						else
						{
							Ptr<DocumentTextRun> textRun = new DocumentTextRun;
							vint copyStart = start>range.start ? start : range.start;
							vint copyEnd = end<range.end ? end : range.end;
							if (copyStart<copyEnd)
							{
								textRun->text = run->text.Sub(copyStart - range.start, copyEnd - copyStart);
							}
							clonedRun = textRun;
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
					clonedRun = 0;
					RunRange range = runRanges[run];
					if (range.start<end && start<range.end)
					{
						if (deepCopy)
						{
							clonedRun = CopyRun(run);
						}
						else
						{
							clonedRun = run;
						}
					}
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					clonedRun = 0;
					RunRange range = runRanges[run];
					if (range.start<end && start<range.end)
					{
						if (deepCopy)
						{
							clonedRun = CopyRun(run);
						}
						else
						{
							clonedRun = run;
						}
					}
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
			Ptr<DocumentStyleProperties> CopyStyle(Ptr<DocumentStyleProperties> style)
			{
				if (!style) return nullptr;
				Ptr<DocumentStyleProperties> newStyle = new DocumentStyleProperties;

				newStyle->face = style->face;
				newStyle->size = style->size;
				newStyle->color = style->color;
				newStyle->backgroundColor = style->backgroundColor;
				newStyle->bold = style->bold;
				newStyle->italic = style->italic;
				newStyle->underline = style->underline;
				newStyle->strikeline = style->strikeline;
				newStyle->antialias = style->antialias;
				newStyle->verticalAntialias = style->verticalAntialias;

				return newStyle;
			}

			Ptr<DocumentRun> CopyRun(DocumentRun* run)
			{
				CloneRunVisitor visitor(0);
				run->Accept(&visitor);
				return visitor.clonedRun;
			}

			Ptr<DocumentRun> CopyStyledText(List<DocumentContainerRun*>& styleRuns, const WString& text)
			{
				Ptr<DocumentTextRun> textRun = new DocumentTextRun;
				textRun->text = text;

				CloneRunVisitor visitor(textRun);
				for (vint i = styleRuns.Count() - 1; i >= 0; i--)
				{
					styleRuns[i]->Accept(&visitor);
				}

				return visitor.clonedRun;
			}

			Ptr<DocumentRun> CopyRunRecursively(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, bool deepCopy)
			{
				CloneRunRecursivelyVisitor visitor(runRanges, start, end, deepCopy);
				run->Accept(&visitor);
				return visitor.clonedRun;
			}
		}
	}
}