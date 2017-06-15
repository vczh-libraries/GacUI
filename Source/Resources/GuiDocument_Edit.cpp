#include "GuiDocument.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

		typedef DocumentModel::RunRange			RunRange;
		typedef DocumentModel::RunRangeMap		RunRangeMap;

/***********************************************************************
document_operation_visitors::GetRunRangeVisitor
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
					,start(0)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					RunRange range;
					range.start=start;
					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						subRun->Accept(this);
					}
					range.end=start;
					runRanges.Add(run, range);
				}

				void VisitContent(DocumentContentRun* run)
				{
					RunRange range;
					range.start=start;
					start+=run->GetRepresentationText().Length();
					range.end=start;
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

				static void GetRunRange(DocumentParagraphRun* run, RunRangeMap& runRanges)
				{
					GetRunRangeVisitor visitor(runRanges);
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::LocateStyleVisitor
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
					,runRanges(_runRanges)
					,position(_position)
					,frontSide(_frontSide)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					locatedRuns.Add(run);
					Ptr<DocumentRun> selectedRun;
					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						RunRange range=runRanges[subRun.Obj()];
						if(position==range.start)
						{
							if(!frontSide)
							{
								selectedRun=subRun;
								break;
							}
						}
						else if(position==range.end)
						{
							if(frontSide)
							{
								selectedRun=subRun;
								break;
							}
						}
						else if(range.start<position && position<range.end)
						{
							selectedRun=subRun;
							break;
						}
					}

					if(selectedRun)
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

				static void LocateStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint position, bool frontSide, List<DocumentContainerRun*>& locatedRuns)
				{
					LocateStyleVisitor visitor(locatedRuns, runRanges, position, frontSide);
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::LocateHyperlinkVisitor
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
					,start(_start)
					,end(_end)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					Ptr<DocumentRun> selectedRun;
					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						RunRange range=runRanges[subRun.Obj()];
						if(range.start<=start && end<=range.end)
						{
							selectedRun=subRun;
							break;
						}
					}

					if(selectedRun)
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
					hyperlink=run;
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

				static Ptr<DocumentHyperlinkRun> LocateHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
				{
					LocateHyperlinkVisitor visitor(runRanges, start, end);
					run->Accept(&visitor);
					return visitor.hyperlink;
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::CloneRunVisitor
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
					if(clonedRun)
					{
						cloned->runs.Add(clonedRun);
					}
					clonedRun=cloned;
				}

				void Visit(DocumentTextRun* run)override
				{
					Ptr<DocumentTextRun> cloned=new DocumentTextRun;
					cloned->text=run->text;
					clonedRun=cloned;
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					Ptr<DocumentStylePropertiesRun> cloned=new DocumentStylePropertiesRun;
					cloned->style=CopyStyle(run->style);
					VisitContainer(cloned);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					Ptr<DocumentStyleApplicationRun> cloned=new DocumentStyleApplicationRun;
					cloned->styleName=run->styleName;
					
					VisitContainer(cloned);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					Ptr<DocumentHyperlinkRun> cloned=new DocumentHyperlinkRun;
					cloned->styleName=run->styleName;
					cloned->normalStyleName=run->normalStyleName;
					cloned->activeStyleName=run->activeStyleName;
					cloned->reference=run->reference;
					
					VisitContainer(cloned);
				}

				void Visit(DocumentImageRun* run)override
				{
					Ptr<DocumentImageRun> cloned=new DocumentImageRun;
					cloned->size=run->size;
					cloned->baseline=run->baseline;
					cloned->image=run->image;
					cloned->frameIndex=run->frameIndex;
					cloned->source=run->source;
					clonedRun=cloned;
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					Ptr<DocumentEmbeddedObjectRun> cloned=new DocumentEmbeddedObjectRun;
					cloned->name=run->name;
					clonedRun=cloned;
				}

				void Visit(DocumentParagraphRun* run)override
				{
					Ptr<DocumentParagraphRun> cloned=new DocumentParagraphRun;
					cloned->alignment=run->alignment;
						
					VisitContainer(cloned);
				}

				static Ptr<DocumentStyleProperties> CopyStyle(Ptr<DocumentStyleProperties> style)
				{
					Ptr<DocumentStyleProperties> newStyle=new DocumentStyleProperties;
					
					newStyle->face					=style->face;
					newStyle->size					=style->size;
					newStyle->color					=style->color;
					newStyle->backgroundColor		=style->backgroundColor;
					newStyle->bold					=style->bold;
					newStyle->italic				=style->italic;
					newStyle->underline				=style->underline;
					newStyle->strikeline			=style->strikeline;
					newStyle->antialias				=style->antialias;
					newStyle->verticalAntialias		=style->verticalAntialias;

					return newStyle;
				}

				static Ptr<DocumentRun> CopyRun(DocumentRun* run)
				{
					CloneRunVisitor visitor(0);
					run->Accept(&visitor);
					return visitor.clonedRun;
				}

				static Ptr<DocumentRun> CopyStyledText(List<DocumentContainerRun*>& styleRuns, const WString& text)
				{
					Ptr<DocumentTextRun> textRun=new DocumentTextRun;
					textRun->text=text;

					CloneRunVisitor visitor(textRun);
					for(vint i=styleRuns.Count()-1;i>=0;i--)
					{
						styleRuns[i]->Accept(&visitor);
					}

					return visitor.clonedRun;
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::CloneRunRecursivelyVisitor
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
					,start(_start)
					,end(_end)
					,deepCopy(_deepCopy)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					clonedRun=0;
					RunRange range=runRanges[run];
					if(range.start<=end && start<=range.end)
					{
						if(start<=range.start && range.end<=end && !deepCopy)
						{
							clonedRun=run;
						}
						else
						{
							Ptr<DocumentContainerRun> containerRun=CloneRunVisitor::CopyRun(run).Cast<DocumentContainerRun>();
							FOREACH(Ptr<DocumentRun>, subRun, run->runs)
							{
								subRun->Accept(this);
								if(clonedRun)
								{
									containerRun->runs.Add(clonedRun);
								}
							}
							clonedRun=containerRun;
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
					clonedRun=0;
					RunRange range=runRanges[run];
					if(range.start<end && start<range.end)
					{
						if(start<=range.start && range.end<=end)
						{
							if(deepCopy)
							{
								clonedRun=CloneRunVisitor::CopyRun(run);
							}
							else
							{
								clonedRun=run;
							}
						}
						else
						{
							Ptr<DocumentTextRun> textRun=new DocumentTextRun;
							vint copyStart=start>range.start?start:range.start;
							vint copyEnd=end<range.end?end:range.end;
							if(copyStart<copyEnd)
							{
								textRun->text=run->text.Sub(copyStart-range.start, copyEnd-copyStart);
							}
							clonedRun=textRun;
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
					clonedRun=0;
					RunRange range=runRanges[run];
					if(range.start<end && start<range.end)
					{
						if(deepCopy)
						{
							clonedRun=CloneRunVisitor::CopyRun(run);
						}
						else
						{
							clonedRun=run;
						}
					}
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					clonedRun=0;
					RunRange range=runRanges[run];
					if(range.start<end && start<range.end)
					{
						if(deepCopy)
						{
							clonedRun=CloneRunVisitor::CopyRun(run);
						}
						else
						{
							clonedRun=run;
						}
					}
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}

				static Ptr<DocumentRun> CopyRun(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, bool deepCopy)
				{
					CloneRunRecursivelyVisitor visitor(runRanges, start, end, deepCopy);
					run->Accept(&visitor);
					return visitor.clonedRun;
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::CollectStyleNameVisitor
***********************************************************************/

		namespace document_operation_visitors
		{
			class CollectStyleNameVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				List<WString>&					styleNames;

				CollectStyleNameVisitor(List<WString>& _styleNames)
					:styleNames(_styleNames)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						subRun->Accept(this);
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
					if(!styleNames.Contains(run->styleName))
					{
						styleNames.Add(run->styleName);
					}
					VisitContainer(run);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					if(!styleNames.Contains(run->normalStyleName))
					{
						styleNames.Add(run->normalStyleName);
					}
					if(!styleNames.Contains(run->activeStyleName))
					{
						styleNames.Add(run->activeStyleName);
					}
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

				static void CollectStyleName(DocumentParagraphRun* run, List<WString>& styleNames)
				{
					CollectStyleNameVisitor visitor(styleNames);
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::ReplaceStyleNameVisitor
***********************************************************************/

		namespace document_operation_visitors
		{
			class ReplaceStyleNameVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				WString							oldStyleName;
				WString							newStyleName;

				ReplaceStyleNameVisitor(const WString& _oldStyleName, const WString& _newStyleName)
					:oldStyleName(_oldStyleName)
					,newStyleName(_newStyleName)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						subRun->Accept(this);
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
					if(run->styleName==oldStyleName) run->styleName=newStyleName;
					VisitContainer(run);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					if(run->styleName==oldStyleName) run->styleName=newStyleName;
					if(run->normalStyleName==oldStyleName) run->normalStyleName=newStyleName;
					if(run->activeStyleName==oldStyleName) run->activeStyleName=newStyleName;
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

				static void ReplaceStyleName(DocumentParagraphRun* run, const WString& oldStyleName, const WString& newStyleName)
				{
					ReplaceStyleNameVisitor visitor(oldStyleName, newStyleName);
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::RemoveRunVisitor
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
					,start(_start)
					,end(_end)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					if(start==end) return;
					for(vint i=run->runs.Count()-1;i>=0;i--)
					{
						Ptr<DocumentRun> subRun=run->runs[i];
						RunRange range=runRanges[subRun.Obj()];

						if(range.start<=end && start<=range.end)
						{
							subRun->Accept(this);
							if(replacedRuns.Count()==0 || subRun!=replacedRuns[0])
							{
								run->runs.RemoveAt(i);
								for(vint j=0;j<replacedRuns.Count();j++)
								{
									run->runs.Insert(i+j, replacedRuns[j]);
								}
							}
						}
					}
					replacedRuns.Clear();
					replacedRuns.Add(run);
				}

				void Visit(DocumentTextRun* run)override
				{
					replacedRuns.Clear();
					RunRange range=runRanges[run];

					if(start<=range.start)
					{
						if(end<range.end)
						{
							run->text=run->text.Sub(end-range.start, range.end-end);
							replacedRuns.Add(run);
						}
					}
					else
					{
						if(end<range.end)
						{
							DocumentTextRun* firstRun=new DocumentTextRun;
							DocumentTextRun* secondRun=new DocumentTextRun;

							firstRun->text=run->text.Sub(0, start-range.start);
							secondRun->text=run->text.Sub(end-range.start, range.end-end);

							replacedRuns.Add(firstRun);
							replacedRuns.Add(secondRun);
						}
						else
						{
							run->text=run->text.Sub(0, start-range.start);
							replacedRuns.Add(run);
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

				static void RemoveRun(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
				{
					RemoveRunVisitor visitor(runRanges, start, end);
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::CutRunVisitor
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
					,position(_position)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					vint leftCount=0;
					Ptr<DocumentRun> selectedRun;

					FOREACH(Ptr<DocumentRun>, subRun, run->runs)
					{
						RunRange range=runRanges[subRun.Obj()];
						if(range.start<position)
						{
							leftCount++;
							if (position<range.end)
							{
								selectedRun=subRun;
							}
						}
						else
						{
							break;
						}
					}

					if(selectedRun)
					{
						selectedRun->Accept(this);
						if(leftRun && rightRun)
						{
							run->runs.RemoveAt(leftCount-1);
							run->runs.Insert(leftCount-1, leftRun);
							run->runs.Insert(leftCount, rightRun);
						}
					}
					
					Ptr<DocumentContainerRun> leftContainer=CloneRunVisitor::CopyRun(run).Cast<DocumentContainerRun>();
					Ptr<DocumentContainerRun> rightContainer=CloneRunVisitor::CopyRun(run).Cast<DocumentContainerRun>();
					for(vint i=0;i<run->runs.Count();i++)
					{
						(i<leftCount?leftContainer:rightContainer)->runs.Add(run->runs[i]);
					}
					leftRun=leftContainer;
					rightRun=rightContainer;
				}

				void Visit(DocumentTextRun* run)override
				{
					RunRange range=runRanges[run];

					Ptr<DocumentTextRun> leftText=new DocumentTextRun;
					leftText->text=run->text.Sub(0, position-range.start);

					Ptr<DocumentTextRun> rightText=new DocumentTextRun;
					rightText->text=run->text.Sub(position-range.start, range.end-position);

					leftRun=leftText;
					rightRun=rightText;
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
					leftRun=0;
					rightRun=0;
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					leftRun=0;
					rightRun=0;
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}

				static void CutRun(DocumentParagraphRun* run, RunRangeMap& runRanges, vint position, Ptr<DocumentRun>& leftRun, Ptr<DocumentRun>& rightRun)
				{
					CutRunVisitor visitor(runRanges, position);
					run->Accept(&visitor);
					leftRun=visitor.leftRun;
					rightRun=visitor.rightRun;
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::ClearRunVisitor
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
					for(vint i=run->runs.Count()-1;i>=0;i--)
					{
						vint oldStart=start;
						run->runs[i]->Accept(this);
						if(oldStart==start)
						{
							run->runs.RemoveAt(i);
						}
					}
				}

				void VisitContent(DocumentContentRun* run)
				{
					start+=run->GetRepresentationText().Length();
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

				static void ClearRun(DocumentParagraphRun* run)
				{
					ClearRunVisitor visitor;
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::AddContainerVisitor
***********************************************************************/

		namespace document_operation_visitors
		{
			class AddContainerVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&							runRanges;
				vint									start;
				vint									end;
				bool									insertStyle;

				virtual Ptr<DocumentContainerRun>		CreateContainer()=0;

				AddContainerVisitor(RunRangeMap& _runRanges, vint _start, vint _end)
					:runRanges(_runRanges)
					,start(_start)
					,end(_end)
					,insertStyle(false)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					for(vint i=run->runs.Count()-1;i>=0;i--)
					{
						Ptr<DocumentRun> subRun=run->runs[i];
						RunRange range=runRanges[subRun.Obj()];
						if(range.start<end && start<range.end)
						{
							insertStyle=false;
							subRun->Accept(this);
							if(insertStyle)
							{
								Ptr<DocumentContainerRun> containerRun=CreateContainer();
								run->runs.RemoveAt(i);
								containerRun->runs.Add(subRun);
								run->runs.Insert(i, containerRun);
							}
						}
					}
					insertStyle=false;
				}

				void Visit(DocumentTextRun* run)override
				{
					insertStyle=true;
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
					insertStyle=false;
				}

				void Visit(DocumentEmbeddedObjectRun* run)override
				{
					insertStyle=false;
				}

				void Visit(DocumentParagraphRun* run)override
				{
					VisitContainer(run);
				}
			};

			class AddStyleVisitor : public AddContainerVisitor
			{
			public:
				Ptr<DocumentStyleProperties>	style;

				Ptr<DocumentContainerRun> CreateContainer()override
				{
					Ptr<DocumentStylePropertiesRun> containerRun=new DocumentStylePropertiesRun;
					containerRun->style=CloneRunVisitor::CopyStyle(style);
					return containerRun;
				}

				AddStyleVisitor(RunRangeMap& _runRanges, vint _start, vint _end, Ptr<DocumentStyleProperties> _style)
					:AddContainerVisitor(_runRanges, _start, _end)
					,style(_style)
				{
				}

				static void AddStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, Ptr<DocumentStyleProperties> style)
				{
					AddStyleVisitor visitor(runRanges, start, end, style);
					run->Accept(&visitor);
				}
			};

			class AddHyperlinkVisitor : public AddContainerVisitor
			{
			public:
				WString							reference;
				WString							normalStyleName;
				WString							activeStyleName;

				Ptr<DocumentContainerRun> CreateContainer()override
				{
					Ptr<DocumentHyperlinkRun> containerRun=new DocumentHyperlinkRun;
					containerRun->reference=reference;
					containerRun->normalStyleName=normalStyleName;
					containerRun->activeStyleName=activeStyleName;
					containerRun->styleName=normalStyleName;
					return containerRun;
				}

				AddHyperlinkVisitor(RunRangeMap& _runRanges, vint _start, vint _end, const WString& _reference, const WString& _normalStyleName, const WString& _activeStyleName)
					:AddContainerVisitor(_runRanges, _start, _end)
					,reference(_reference)
					,normalStyleName(_normalStyleName)
					,activeStyleName(_activeStyleName)
				{
				}

				static void AddHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName)
				{
					AddHyperlinkVisitor visitor(runRanges, start, end, reference, normalStyleName, activeStyleName);
					run->Accept(&visitor);
				}
			};

			class AddStyleNameVisitor : public AddContainerVisitor
			{
			public:
				WString							styleName;

				Ptr<DocumentContainerRun> CreateContainer()override
				{
					Ptr<DocumentStyleApplicationRun> containerRun=new DocumentStyleApplicationRun;
					containerRun->styleName=styleName;
					return containerRun;
				}

				AddStyleNameVisitor(RunRangeMap& _runRanges, vint _start, vint _end, const WString& _styleName)
					:AddContainerVisitor(_runRanges, _start, _end)
					,styleName(_styleName)
				{
				}

				static void AddStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end, const WString& styleName)
				{
					AddStyleNameVisitor visitor(runRanges, start, end, styleName);
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::RemoveContainerVisitor
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
					,start(_start)
					,end(_end)
				{
				}

				void VisitContainer(DocumentContainerRun* run)
				{
					for(vint i=run->runs.Count()-1;i>=0;i--)
					{
						Ptr<DocumentRun> subRun=run->runs[i];
						RunRange range=runRanges[subRun.Obj()];
						if(range.start<end && start<range.end)
						{
							replacedRuns.Clear();
							subRun->Accept(this);
							if(replacedRuns.Count()!=1 || replacedRuns[0]!=subRun)
							{
								run->runs.RemoveAt(i);
								for(vint j=0;j<replacedRuns.Count();j++)
								{
									run->runs.Insert(i+j, replacedRuns[j]);
								}
								i+=replacedRuns.Count();
							}
						}
					}
					replacedRuns.Clear();
					replacedRuns.Add(run);
				}

				void VisitContent(DocumentContentRun* run)
				{
					replacedRuns.Add(run);
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

				static void RemoveHyperlink(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
				{
					RemoveHyperlinkVisitor visitor(runRanges, start, end);
					run->Accept(&visitor);
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

				static void RemoveStyleName(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
				{
					RemoveStyleNameVisitor visitor(runRanges, start, end);
					run->Accept(&visitor);
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

				static void ClearStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, vint start, vint end)
				{
					ClearStyleVisitor visitor(runRanges, start, end);
					run->Accept(&visitor);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
document_operation_visitors::SummerizeStyleVisitor
***********************************************************************/

		namespace document_operation_visitors
		{
			class SummerizeStyleVisitor : public Object, public DocumentRun::IVisitor
			{
			public:
				RunRangeMap&							runRanges;
				DocumentModel*							model;
				vint									start;
				vint									end;
				Ptr<DocumentStyleProperties>			style;
				List<DocumentModel::ResolvedStyle>		resolvedStyles;

				SummerizeStyleVisitor(RunRangeMap& _runRanges, DocumentModel* _model, vint _start, vint _end)
					:runRanges(_runRanges)
					,model(_model)
					,start(_start)
					,end(_end)
				{
					DocumentModel::ResolvedStyle resolvedStyle;
					resolvedStyle = model->GetStyle(DocumentModel::DefaultStyleName, resolvedStyle);
					resolvedStyles.Add(resolvedStyle);
				}

				const DocumentModel::ResolvedStyle& GetCurrentResolvedStyle()
				{
					return resolvedStyles[resolvedStyles.Count()-1];
				}

				// ---------------------------------------------------------

				template<typename T>
				void SetStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					if (style.Obj()->*dstField && (style.Obj()->*dstField).Value() != src.style.*srcField)
					{
						style.Obj()->*dstField = Nullable<T>();
					}
				}

				template<typename T>
				void SetStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T DocumentModel::ResolvedStyle::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					if (style.Obj()->*dstField && (style.Obj()->*dstField).Value() != src.*srcField)
					{
						style.Obj()->*dstField = Nullable<T>();
					}
				}

				void SetStyleItem(Nullable<DocumentFontSize> DocumentStyleProperties::* dstField, vint FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					if (style.Obj()->*dstField)
					{
						auto dfs = (style.Obj()->*dstField).Value();
						if (dfs.relative || dfs.size != src.style.*srcField)
						{
							style.Obj()->*dstField = Nullable<DocumentFontSize>();
						}
					}
				}

				// ---------------------------------------------------------

				template<typename T>
				void OverrideStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					style.Obj()->*dstField = src.style.*srcField;
				}

				template<typename T>
				void OverrideStyleItem(Nullable<T> DocumentStyleProperties::* dstField, T DocumentModel::ResolvedStyle::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					style.Obj()->*dstField = src.*srcField;
				}

				void OverrideStyleItem(Nullable<DocumentFontSize> DocumentStyleProperties::* dstField, vint FontProperties::* srcField)
				{
					const DocumentModel::ResolvedStyle& src = GetCurrentResolvedStyle();
					style.Obj()->*dstField = DocumentFontSize(src.style.*srcField, false);
				}

				// ---------------------------------------------------------

				void VisitContainer(DocumentContainerRun* run)
				{
					for(vint i=run->runs.Count()-1;i>=0;i--)
					{
						Ptr<DocumentRun> subRun=run->runs[i];
						RunRange range=runRanges[subRun.Obj()];
						if(range.start<end && start<range.end)
						{
							subRun->Accept(this);
						}
					}
				}

				void Visit(DocumentTextRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle=GetCurrentResolvedStyle();
					if (style)
					{
						SetStyleItem(&DocumentStyleProperties::face,					&FontProperties::fontFamily);
						SetStyleItem(&DocumentStyleProperties::size,					&FontProperties::size);
						SetStyleItem(&DocumentStyleProperties::color,					&DocumentModel::ResolvedStyle::color);
						SetStyleItem(&DocumentStyleProperties::backgroundColor,			&DocumentModel::ResolvedStyle::backgroundColor);
						SetStyleItem(&DocumentStyleProperties::bold,					&FontProperties::bold);
						SetStyleItem(&DocumentStyleProperties::italic,					&FontProperties::italic);
						SetStyleItem(&DocumentStyleProperties::underline,				&FontProperties::underline);
						SetStyleItem(&DocumentStyleProperties::strikeline,				&FontProperties::strikeline);
						SetStyleItem(&DocumentStyleProperties::antialias,				&FontProperties::antialias);
						SetStyleItem(&DocumentStyleProperties::verticalAntialias,		&FontProperties::verticalAntialias);
					}
					else
					{
						style=new DocumentStyleProperties;
						OverrideStyleItem(&DocumentStyleProperties::face,				&FontProperties::fontFamily);
						OverrideStyleItem(&DocumentStyleProperties::size,				&FontProperties::size);
						OverrideStyleItem(&DocumentStyleProperties::color,				&DocumentModel::ResolvedStyle::color);
						OverrideStyleItem(&DocumentStyleProperties::backgroundColor,	&DocumentModel::ResolvedStyle::backgroundColor);
						OverrideStyleItem(&DocumentStyleProperties::bold,				&FontProperties::bold);
						OverrideStyleItem(&DocumentStyleProperties::italic,				&FontProperties::italic);
						OverrideStyleItem(&DocumentStyleProperties::underline,			&FontProperties::underline);
						OverrideStyleItem(&DocumentStyleProperties::strikeline,			&FontProperties::strikeline);
						OverrideStyleItem(&DocumentStyleProperties::antialias,			&FontProperties::antialias);
						OverrideStyleItem(&DocumentStyleProperties::verticalAntialias,	&FontProperties::verticalAntialias);
					}
				}

				void Visit(DocumentStylePropertiesRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle=GetCurrentResolvedStyle();
					DocumentModel::ResolvedStyle resolvedStyle=model->GetStyle(run->style, currentResolvedStyle);
					resolvedStyles.Add(resolvedStyle);
					VisitContainer(run);
					resolvedStyles.RemoveAt(resolvedStyles.Count()-1);
				}

				void Visit(DocumentStyleApplicationRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle=GetCurrentResolvedStyle();
					DocumentModel::ResolvedStyle resolvedStyle=model->GetStyle(run->styleName, currentResolvedStyle);
					resolvedStyles.Add(resolvedStyle);
					VisitContainer(run);
					resolvedStyles.RemoveAt(resolvedStyles.Count()-1);
				}

				void Visit(DocumentHyperlinkRun* run)override
				{
					const DocumentModel::ResolvedStyle& currentResolvedStyle=GetCurrentResolvedStyle();
					DocumentModel::ResolvedStyle resolvedStyle=model->GetStyle(run->styleName, currentResolvedStyle);
					resolvedStyles.Add(resolvedStyle);
					VisitContainer(run);
					resolvedStyles.RemoveAt(resolvedStyles.Count()-1);
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

				static Ptr<DocumentStyleProperties> SummerizeStyle(DocumentParagraphRun* run, RunRangeMap& runRanges, DocumentModel* model, vint start, vint end)
				{
					SummerizeStyleVisitor visitor(runRanges, model, start, end);
					run->Accept(&visitor);
					return visitor.style;
				}

				template<typename T>
				static void AggregateStyleItem(Ptr<DocumentStyleProperties>& dst, Ptr<DocumentStyleProperties> src, Nullable<T> DocumentStyleProperties::* field)
				{
					if(dst.Obj()->*field && (!(src.Obj()->*field) || (dst.Obj()->*field).Value()!=(src.Obj()->*field).Value()))
					{
						dst.Obj()->*field=Nullable<T>();
					}
				}

				static void AggregateStyle(Ptr<DocumentStyleProperties>& dst, Ptr<DocumentStyleProperties> src)
				{
					AggregateStyleItem(dst, src, &DocumentStyleProperties::face);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::size);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::color);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::backgroundColor);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::bold);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::italic);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::underline);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::strikeline);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::antialias);
					AggregateStyleItem(dst, src, &DocumentStyleProperties::verticalAntialias);
				}
			};
		}
		using namespace document_operation_visitors;

/***********************************************************************
DocumentModel::EditRangeOperations
***********************************************************************/

		bool DocumentModel::CheckEditRange(TextPos begin, TextPos end, RunRangeMap& relatedRanges)
		{
			// check caret range
			if(begin>end) return false;
			if(begin.row<0 || begin.row>=paragraphs.Count()) return false;
			if(end.row<0 || end.row>=paragraphs.Count()) return false;

			// determine run ranges
			GetRunRangeVisitor::GetRunRange(paragraphs[begin.row].Obj(), relatedRanges);
			if(begin.row!=end.row)
			{
				GetRunRangeVisitor::GetRunRange(paragraphs[end.row].Obj(), relatedRanges);
			}
			
			// check caret range
			RunRange beginRange=relatedRanges[paragraphs[begin.row].Obj()];
			RunRange endRange=relatedRanges[paragraphs[end.row].Obj()];
			if(begin.column<0 || begin.column>beginRange.end) return false;
			if(end.column<0 || end.column>endRange.end) return false;

			return true;
		}

		Ptr<DocumentModel> DocumentModel::CopyDocument(TextPos begin, TextPos end, bool deepCopy)
		{
			// check caret range
			RunRangeMap runRanges;
			if(!CheckEditRange(begin, end, runRanges)) return nullptr;

			// get ranges
			for(vint i=begin.row+1;i<end.row;i++)
			{
				GetRunRangeVisitor::GetRunRange(paragraphs[i].Obj(), runRanges);
			}

			Ptr<DocumentModel> newDocument=new DocumentModel;

			// copy paragraphs
			if(begin.row==end.row)
			{
				newDocument->paragraphs.Add(CloneRunRecursivelyVisitor::CopyRun(paragraphs[begin.row].Obj(), runRanges, begin.column, end.column, deepCopy).Cast<DocumentParagraphRun>());
			}
			else
			{
				for(vint i=begin.row;i<=end.row;i++)
				{
					Ptr<DocumentParagraphRun> paragraph=paragraphs[i];
					RunRange range=runRanges[paragraph.Obj()];
					if(i==begin.row)
					{
						newDocument->paragraphs.Add(CloneRunRecursivelyVisitor::CopyRun(paragraph.Obj(), runRanges, begin.column, range.end, deepCopy).Cast<DocumentParagraphRun>());
					}
					else if(i==end.row)
					{
						newDocument->paragraphs.Add(CloneRunRecursivelyVisitor::CopyRun(paragraph.Obj(), runRanges, range.start, end.column, deepCopy).Cast<DocumentParagraphRun>());
					}
					else if(deepCopy)
					{
						newDocument->paragraphs.Add(CloneRunRecursivelyVisitor::CopyRun(paragraph.Obj(), runRanges, range.start, range.end, deepCopy).Cast<DocumentParagraphRun>());
					}
					else
					{
						newDocument->paragraphs.Add(paragraph);
					}
				}
			}

			// copy styles
			List<WString> styleNames;
			FOREACH(Ptr<DocumentParagraphRun>, paragraph, newDocument->paragraphs)
			{
				CollectStyleNameVisitor::CollectStyleName(paragraph.Obj(), styleNames);
			}

			for(vint i=0;i<styleNames.Count();i++)
			{
				WString styleName=styleNames[i];
				if(!newDocument->styles.Keys().Contains(styleName))
				{
					Ptr<DocumentStyle> style=styles[styleName];
					if(deepCopy)
					{
						Ptr<DocumentStyle> newStyle=new DocumentStyle;
						newStyle->parentStyleName=style->parentStyleName;
						newStyle->styles=CloneRunVisitor::CopyStyle(style->styles);
						newStyle->resolvedStyles=CloneRunVisitor::CopyStyle(style->resolvedStyles);
						newDocument->styles.Add(styleName, newStyle);
					}
					else
					{
						newDocument->styles.Add(styleName, style);
					}

					if(!styleNames.Contains(style->parentStyleName))
					{
						styleNames.Add(style->parentStyleName);
					}
				}
			}

			return newDocument;
		}

		Ptr<DocumentModel> DocumentModel::CopyDocument()
		{
			// determine run ranges
			RunRangeMap runRanges;
			vint lastParagraphIndex = paragraphs.Count() - 1;
			GetRunRangeVisitor::GetRunRange(paragraphs[lastParagraphIndex].Obj(), runRanges);
			
			TextPos begin(0, 0);
			TextPos end(lastParagraphIndex, runRanges[paragraphs[lastParagraphIndex].Obj()].end);
			return CopyDocument(begin, end, true);
		}

		bool DocumentModel::CutParagraph(TextPos position)
		{
			if(position.row<0 || position.row>=paragraphs.Count()) return false;

			Ptr<DocumentParagraphRun> paragraph=paragraphs[position.row];
			RunRangeMap runRanges;
			Ptr<DocumentRun> leftRun, rightRun;

			GetRunRangeVisitor::GetRunRange(paragraph.Obj(), runRanges);
			CutRunVisitor::CutRun(paragraph.Obj(), runRanges, position.column, leftRun, rightRun);

			CopyFrom(paragraph->runs, leftRun.Cast<DocumentParagraphRun>()->runs);
			CopyFrom(paragraph->runs, rightRun.Cast<DocumentParagraphRun>()->runs, true);
			
			return true;
		}

		bool DocumentModel::CutEditRange(TextPos begin, TextPos end)
		{
			// check caret range
			if(begin>end) return false;
			if(begin.row<0 || begin.row>=paragraphs.Count()) return false;
			if(end.row<0 || end.row>=paragraphs.Count()) return false;

			// cut paragraphs
			CutParagraph(begin);
			if(begin!=end)
			{
				CutParagraph(end);
			}
			return true;
		}

		bool DocumentModel::EditContainer(TextPos begin, TextPos end, const Func<void(DocumentParagraphRun*, RunRangeMap&, vint, vint)>& editor)
		{
			if(begin==end) return false;

			// cut paragraphs
			if(!CutEditRange(begin, end)) return false;

			// check caret range
			RunRangeMap runRanges;
			if(!CheckEditRange(begin, end, runRanges)) return false;

			// edit container
			if(begin.row==end.row)
			{
				editor(paragraphs[begin.row].Obj(), runRanges, begin.column, end.column);
			}
			else
			{
				for(vint i=begin.row;i<=end.row;i++)
				{
					Ptr<DocumentParagraphRun> paragraph=paragraphs[i];
					if(begin.row<i && i<end.row)
					{
						GetRunRangeVisitor::GetRunRange(paragraph.Obj(), runRanges);
					}
					RunRange range=runRanges[paragraph.Obj()];
					if(i==begin.row)
					{
						editor(paragraph.Obj(), runRanges, begin.column, range.end);
					}
					else if(i==end.row)
					{
						editor(paragraph.Obj(), runRanges, range.start, end.column);
					}
					else
					{
						editor(paragraph.Obj(), runRanges, range.start, range.end);
					}
				}
			}

			// clear paragraphs
			for(vint i=begin.row;i<=end.row;i++)
			{
				ClearRunVisitor::ClearRun(paragraphs[i].Obj());
			}

			return true;
		}

/***********************************************************************
DocumentModel::EditRun
***********************************************************************/

		vint DocumentModel::EditRun(TextPos begin, TextPos end, Ptr<DocumentModel> replaceToModel, bool copy)
		{
			// check caret range
			RunRangeMap runRanges;
			if(!CheckEditRange(begin, end, runRanges)) return -1;

			auto model = replaceToModel;
			if (copy)
			{
				model = replaceToModel->CopyDocument();
			}

			// calculate new names for the model's styles to prevent conflicting
			List<WString> oldNames, newNames;
			CopyFrom(oldNames, model->styles.Keys());
			CopyFrom(newNames, model->styles.Keys());
			for(vint i=0;i<newNames.Count();i++)
			{
				WString name=newNames[i];
				if((name.Length()==0 || name[0]!=L'#') && styles.Keys().Contains(name))
				{
					vint index=2;
					while(true)
					{
						WString newName=name+L"_"+itow(index++);
						if(!styles.Keys().Contains(newName) && !model->styles.Keys().Contains(newName))
						{
							newNames[i]=newName;
							break;
						}
					}
				}
			}

			// rename model's styles
			typedef Pair<WString, WString> NamePair;
			FOREACH(NamePair, name, From(oldNames).Pairwise(newNames))
			{
				model->RenameStyle(name.key, name.value);
			}
			FOREACH(WString, name, newNames)
			{
				if((name.Length()==0 || name[0]!=L'#') && !styles.Keys().Contains(name))
				{
					styles.Add(name, model->styles[name]);
				}
			}

			// edit runs
			Array<Ptr<DocumentParagraphRun>> runs;
			CopyFrom(runs, model->paragraphs);
			return EditRunNoCopy(begin, end, runs);
		}

		vint DocumentModel::EditRunNoCopy(TextPos begin, TextPos end, const collections::Array<Ptr<DocumentParagraphRun>>& runs)
		{
			// check caret range
			RunRangeMap runRanges;
			if(!CheckEditRange(begin, end, runRanges)) return -1;

			// remove unnecessary paragraphs
			if(begin.row!=end.row)
			{
				for(vint i=end.row-1;i>begin.row;i--)
				{
					paragraphs.RemoveAt(i);
				}
				end.row=begin.row+1;
			}

			// remove unnecessary runs and ensure begin.row!=end.row
			if(begin.row==end.row)
			{
				RemoveRunVisitor::RemoveRun(paragraphs[begin.row].Obj(), runRanges, begin.column, end.column);

				Ptr<DocumentRun> leftRun, rightRun;
				runRanges.Clear();
				GetRunRangeVisitor::GetRunRange(paragraphs[begin.row].Obj(), runRanges);
				CutRunVisitor::CutRun(paragraphs[begin.row].Obj(), runRanges, begin.column, leftRun, rightRun);

				paragraphs.RemoveAt(begin.row);
				paragraphs.Insert(begin.row, leftRun.Cast<DocumentParagraphRun>());
				paragraphs.Insert(begin.row+1, rightRun.Cast<DocumentParagraphRun>());
				end.row=begin.row+1;
			}
			else
			{
				RemoveRunVisitor::RemoveRun(paragraphs[begin.row].Obj(), runRanges, begin.column, runRanges[paragraphs[begin.row].Obj()].end);
				RemoveRunVisitor::RemoveRun(paragraphs[end.row].Obj(), runRanges, 0, end.column);
			}

			// insert new paragraphs
			Ptr<DocumentParagraphRun> beginParagraph=paragraphs[begin.row];
			Ptr<DocumentParagraphRun> endParagraph=paragraphs[end.row];
			if(runs.Count()==0)
			{
				CopyFrom(beginParagraph->runs, endParagraph->runs, true);
				paragraphs.RemoveAt(end.row);
			}
			else if(runs.Count()==1)
			{
				CopyFrom(beginParagraph->runs, runs[0]->runs, true);
				CopyFrom(beginParagraph->runs, endParagraph->runs, true);
				paragraphs.RemoveAt(end.row);
			}
			else
			{
				Ptr<DocumentParagraphRun> newBeginRuns=runs[0];
				CopyFrom(beginParagraph->runs, newBeginRuns->runs, true);
				
				Ptr<DocumentParagraphRun> newEndRuns=runs[runs.Count()-1];
				if (newEndRuns->alignment)
				{
					endParagraph->alignment = newEndRuns->alignment;
				}
				for(vint i=0;i<newEndRuns->runs.Count();i++)
				{
					endParagraph->runs.Insert(i, newEndRuns->runs[i]);
				}

				for(vint i=1;i<runs.Count()-1;i++)
				{
					paragraphs.Insert(begin.row+i, runs[i]);
				}
			}

			// clear unnecessary runs
			vint rows=runs.Count()==0?1:runs.Count();
			for(vint i=0;i<rows;i++)
			{
				ClearRunVisitor::ClearRun(paragraphs[begin.row+i].Obj());
			}
			return rows;
		}

/***********************************************************************
DocumentModel::EditText
***********************************************************************/

		vint DocumentModel::EditText(TextPos begin, TextPos end, bool frontSide, const collections::Array<WString>& text)
		{
			// check caret range
			RunRangeMap runRanges;
			if(!CheckEditRange(begin, end, runRanges)) return -1;

			// calcuate the position to get the text style
			TextPos stylePosition;
			if(frontSide)
			{
				stylePosition=begin;
				if(stylePosition.column==0)
				{
					frontSide=false;
				}
			}
			else
			{
				stylePosition=end;
				if(stylePosition.column==runRanges[paragraphs[end.row].Obj()].end)
				{
					frontSide=true;
				}
			}

			// copy runs that contains the target style for new text
			List<DocumentContainerRun*> styleRuns;
			LocateStyleVisitor::LocateStyle(paragraphs[stylePosition.row].Obj(), runRanges, stylePosition.column, frontSide, styleRuns);

			// create paragraphs
			Array<Ptr<DocumentParagraphRun>> runs(text.Count());
			for(vint i=0;i<text.Count();i++)
			{
				Ptr<DocumentRun> paragraph=CloneRunVisitor::CopyStyledText(styleRuns, text[i]);
				runs[i]=paragraph.Cast<DocumentParagraphRun>();
			}

			// replace the paragraphs
			return EditRunNoCopy(begin, end, runs);
		}

/***********************************************************************
DocumentModel::EditStyle
***********************************************************************/

		bool DocumentModel::EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style)
		{
			return EditContainer(begin, end, [=](DocumentParagraphRun* paragraph, RunRangeMap& runRanges, vint start, vint end)
			{
				AddStyleVisitor::AddStyle(paragraph, runRanges, start, end, style);
			});
		}

/***********************************************************************
DocumentModel::EditImage
***********************************************************************/

		Ptr<DocumentImageRun> DocumentModel::EditImage(TextPos begin, TextPos end, Ptr<GuiImageData> image)
		{
			Ptr<DocumentImageRun> imageRun=new DocumentImageRun;
			imageRun->size=image->GetImage()->GetFrame(image->GetFrameIndex())->GetSize();
			imageRun->baseline=imageRun->size.y;
			imageRun->image=image->GetImage();
			imageRun->frameIndex=image->GetFrameIndex();

			Ptr<DocumentParagraphRun> paragraph=new DocumentParagraphRun;
			paragraph->runs.Add(imageRun);

			Array<Ptr<DocumentParagraphRun>> runs(1);
			runs[0]=paragraph;
			if(EditRunNoCopy(begin, end, runs))
			{
				return imageRun;
			}
			else
			{
				return 0;
			}
		}

/***********************************************************************
DocumentModel::EditHyperlink
***********************************************************************/

		bool DocumentModel::EditHyperlink(vint paragraphIndex, vint begin, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName)
		{
			Ptr<DocumentHyperlinkRun> run=GetHyperlink(paragraphIndex, begin, end);
			if(run)
			{
				run->reference=reference;
				run->normalStyleName=normalStyleName;
				run->activeStyleName=activeStyleName;
				run->styleName=normalStyleName;
				return true;
			}
			else if(RemoveHyperlink(paragraphIndex, begin, end))
			{
				CutEditRange(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end));

				RunRangeMap runRanges;
				Ptr<DocumentParagraphRun> paragraph=paragraphs[paragraphIndex];
				GetRunRangeVisitor::GetRunRange(paragraph.Obj(), runRanges);
				AddHyperlinkVisitor::AddHyperlink(paragraph.Obj(), runRanges, begin, end, reference, normalStyleName, activeStyleName);

				ClearRunVisitor::ClearRun(paragraph.Obj());
				return true;
			}
			return false;
		}

		bool DocumentModel::RemoveHyperlink(vint paragraphIndex, vint begin, vint end)
		{
			RunRangeMap runRanges;
			if(!CheckEditRange(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), runRanges)) return false;

			Ptr<DocumentParagraphRun> paragraph=paragraphs[paragraphIndex];
			RemoveHyperlinkVisitor::RemoveHyperlink(paragraph.Obj(), runRanges, begin, end);
			ClearRunVisitor::ClearRun(paragraph.Obj());
			return true;
		}

		Ptr<DocumentHyperlinkRun> DocumentModel::GetHyperlink(vint paragraphIndex, vint begin, vint end)
		{
			RunRangeMap runRanges;
			if(!CheckEditRange(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), runRanges)) return 0;

			Ptr<DocumentParagraphRun> paragraph=paragraphs[paragraphIndex];
			return LocateHyperlinkVisitor::LocateHyperlink(paragraph.Obj(), runRanges, begin, end);
		}

/***********************************************************************
DocumentModel::EditStyleName
***********************************************************************/

		bool DocumentModel::EditStyleName(TextPos begin, TextPos end, const WString& styleName)
		{
			return EditContainer(begin, end, [=](DocumentParagraphRun* paragraph, RunRangeMap& runRanges, vint start, vint end)
			{
				AddStyleNameVisitor::AddStyleName(paragraph, runRanges, start, end, styleName);
			});
		}

		bool DocumentModel::RemoveStyleName(TextPos begin, TextPos end)
		{
			return EditContainer(begin, end, [=](DocumentParagraphRun* paragraph, RunRangeMap& runRanges, vint start, vint end)
			{
				RemoveStyleNameVisitor::RemoveStyleName(paragraph, runRanges, start, end);
			});
		}

		bool DocumentModel::RenameStyle(const WString& oldStyleName, const WString& newStyleName)
		{
			vint index=styles.Keys().IndexOf(oldStyleName);
			if(index==-1) return false;
			if(styles.Keys().Contains(newStyleName)) return false;

			Ptr<DocumentStyle> style=styles.Values()[index];
			styles.Remove(oldStyleName);
			styles.Add(newStyleName, style);

			FOREACH(Ptr<DocumentStyle>, subStyle, styles.Values())
			{
				if(subStyle->parentStyleName==oldStyleName)
				{
					subStyle->parentStyleName=newStyleName;
				}
			}

			FOREACH(Ptr<DocumentParagraphRun>, paragraph, paragraphs)
			{
				ReplaceStyleNameVisitor::ReplaceStyleName(paragraph.Obj(), oldStyleName, newStyleName);
			}
			return true;
		}

/***********************************************************************
DocumentModel::ClearStyle
***********************************************************************/

		bool DocumentModel::ClearStyle(TextPos begin, TextPos end)
		{
			return EditContainer(begin, end, [=](DocumentParagraphRun* paragraph, RunRangeMap& runRanges, vint start, vint end)
			{
				ClearStyleVisitor::ClearStyle(paragraph, runRanges, start, end);
			});
		}

/***********************************************************************
DocumentModel::ClearStyle
***********************************************************************/

		Ptr<DocumentStyleProperties> DocumentModel::SummarizeStyle(TextPos begin, TextPos end)
		{
			Ptr<DocumentStyleProperties> style;
			RunRangeMap runRanges;

			if(begin==end) goto END_OF_SUMMERIZING;

			// check caret range
			if(!CheckEditRange(begin, end, runRanges)) return nullptr;

			// summerize container
			if(begin.row==end.row)
			{
				style=SummerizeStyleVisitor::SummerizeStyle(paragraphs[begin.row].Obj(), runRanges, this, begin.column, end.column);
			}
			else
			{
				for(vint i=begin.row;i<=end.row;i++)
				{
					Ptr<DocumentParagraphRun> paragraph=paragraphs[i];
					if(begin.row<i && i<end.row)
					{
						GetRunRangeVisitor::GetRunRange(paragraph.Obj(), runRanges);
					}
					RunRange range=runRanges[paragraph.Obj()];
					Ptr<DocumentStyleProperties> paragraphStyle;
					if(i==begin.row)
					{
						paragraphStyle=SummerizeStyleVisitor::SummerizeStyle(paragraph.Obj(), runRanges, this, begin.column, range.end);
					}
					else if(i==end.row)
					{
						paragraphStyle=SummerizeStyleVisitor::SummerizeStyle(paragraph.Obj(), runRanges, this, range.start, end.column);
					}
					else
					{
						paragraphStyle=SummerizeStyleVisitor::SummerizeStyle(paragraph.Obj(), runRanges, this, range.start, range.end);
					}

					if(!style)
					{
						style=paragraphStyle;
					}
					else if(paragraphStyle)
					{
						SummerizeStyleVisitor::AggregateStyle(style, paragraphStyle);
					}
				}
			}

		END_OF_SUMMERIZING:
			if(!style)
			{
				style=new DocumentStyleProperties;
			}
			return style;
		}

		Nullable<Alignment> DocumentModel::SummarizeParagraphAlignment(TextPos begin, TextPos end)
		{
			bool left = false;
			bool center = false;
			bool right = false;

			RunRangeMap runRanges;
			if (!CheckEditRange(begin, end, runRanges)) return {};

			for (vint i = begin.row; i <= end.row; i++)
			{
				auto paragraph = paragraphs[i];
				if (paragraph->alignment)
				{
					switch (paragraph->alignment.Value())
					{
					case Alignment::Left:
						left = true;
						break;
					case Alignment::Center:
						center = true;
						break;
					case Alignment::Right:
						right = true;
						break;
					}
				}
				else
				{
					left = true;
				}
			}

			if (left && !center && !right) return Alignment::Left;
			if (!left && center && !right) return Alignment::Center;
			if (!left && !center && right) return Alignment::Right;
			return {};
		}
	}
}