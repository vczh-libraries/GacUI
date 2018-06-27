#include "GuiDocumentEditor.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace document_editor;

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
			GetRunRange(paragraphs[begin.row].Obj(), relatedRanges);
			if(begin.row!=end.row)
			{
				GetRunRange(paragraphs[end.row].Obj(), relatedRanges);
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
				GetRunRange(paragraphs[i].Obj(), runRanges);
			}

			Ptr<DocumentModel> newDocument=new DocumentModel;

			// copy paragraphs
			if(begin.row==end.row)
			{
				newDocument->paragraphs.Add(CopyRunRecursively(paragraphs[begin.row].Obj(), runRanges, begin.column, end.column, deepCopy).Cast<DocumentParagraphRun>());
			}
			else
			{
				for(vint i=begin.row;i<=end.row;i++)
				{
					Ptr<DocumentParagraphRun> paragraph=paragraphs[i];
					RunRange range=runRanges[paragraph.Obj()];
					if(i==begin.row)
					{
						newDocument->paragraphs.Add(CopyRunRecursively(paragraph.Obj(), runRanges, begin.column, range.end, deepCopy).Cast<DocumentParagraphRun>());
					}
					else if(i==end.row)
					{
						newDocument->paragraphs.Add(CopyRunRecursively(paragraph.Obj(), runRanges, range.start, end.column, deepCopy).Cast<DocumentParagraphRun>());
					}
					else if(deepCopy)
					{
						newDocument->paragraphs.Add(CopyRunRecursively(paragraph.Obj(), runRanges, range.start, range.end, deepCopy).Cast<DocumentParagraphRun>());
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
				CollectStyleName(paragraph.Obj(), styleNames);
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
						newStyle->styles=CopyStyle(style->styles);
						newStyle->resolvedStyles=CopyStyle(style->resolvedStyles);
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
			GetRunRange(paragraphs[lastParagraphIndex].Obj(), runRanges);
			
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

			GetRunRange(paragraph.Obj(), runRanges);
			CutRun(paragraph.Obj(), runRanges, position.column, leftRun, rightRun);

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
						GetRunRange(paragraph.Obj(), runRanges);
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
				ClearUnnecessaryRun(paragraphs[i].Obj(), this);
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
				RemoveRun(paragraphs[begin.row].Obj(), runRanges, begin.column, end.column);

				Ptr<DocumentRun> leftRun, rightRun;
				runRanges.Clear();
				GetRunRange(paragraphs[begin.row].Obj(), runRanges);
				CutRun(paragraphs[begin.row].Obj(), runRanges, begin.column, leftRun, rightRun);

				paragraphs.RemoveAt(begin.row);
				paragraphs.Insert(begin.row, leftRun.Cast<DocumentParagraphRun>());
				paragraphs.Insert(begin.row+1, rightRun.Cast<DocumentParagraphRun>());
				end.row=begin.row+1;
			}
			else
			{
				RemoveRun(paragraphs[begin.row].Obj(), runRanges, begin.column, runRanges[paragraphs[begin.row].Obj()].end);
				RemoveRun(paragraphs[end.row].Obj(), runRanges, 0, end.column);
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
				ClearUnnecessaryRun(paragraphs[begin.row+i].Obj(), this);
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
			LocateStyle(paragraphs[stylePosition.row].Obj(), runRanges, stylePosition.column, frontSide, styleRuns);

			// create paragraphs
			Array<Ptr<DocumentParagraphRun>> runs(text.Count());
			for(vint i=0;i<text.Count();i++)
			{
				Ptr<DocumentRun> paragraph=CopyStyledText(styleRuns, text[i]);
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
				AddStyle(paragraph, runRanges, start, end, style);
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
			auto package = GetHyperlink(paragraphIndex, begin, end);
			if (package->hyperlinks.Count() > 0)
			{
				FOREACH(Ptr<DocumentHyperlinkRun>, run, package->hyperlinks)
				{
					run->reference = reference;
					run->normalStyleName = normalStyleName;
					run->activeStyleName = activeStyleName;
					run->styleName = normalStyleName;
				}
				return true;
			}
			else if (RemoveHyperlink(paragraphIndex, begin, end))
			{
				CutEditRange(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end));

				RunRangeMap runRanges;
				Ptr<DocumentParagraphRun> paragraph = paragraphs[paragraphIndex];
				GetRunRange(paragraph.Obj(), runRanges);
				AddHyperlink(paragraph.Obj(), runRanges, begin, end, reference, normalStyleName, activeStyleName);

				ClearUnnecessaryRun(paragraph.Obj(), this);
				return true;
			}
			return false;
		}

		bool DocumentModel::RemoveHyperlink(vint paragraphIndex, vint begin, vint end)
		{
			RunRangeMap runRanges;
			if (!CheckEditRange(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), runRanges)) return 0;

			auto paragraph = paragraphs[paragraphIndex];
			auto package = LocateHyperlink(paragraph.Obj(), runRanges, paragraphIndex, begin, end);
			document_editor::RemoveHyperlink(paragraph.Obj(), runRanges, package->start, package->end);
			ClearUnnecessaryRun(paragraph.Obj(), this);
			return true;
		}

		Ptr<DocumentHyperlinkRun::Package> DocumentModel::GetHyperlink(vint paragraphIndex, vint begin, vint end)
		{
			RunRangeMap runRanges;
			if (!CheckEditRange(TextPos(paragraphIndex, begin), TextPos(paragraphIndex, end), runRanges)) return 0;

			auto paragraph = paragraphs[paragraphIndex];
			return LocateHyperlink(paragraph.Obj(), runRanges, paragraphIndex, begin, end);
		}

/***********************************************************************
DocumentModel::EditStyleName
***********************************************************************/

		bool DocumentModel::EditStyleName(TextPos begin, TextPos end, const WString& styleName)
		{
			return EditContainer(begin, end, [=](DocumentParagraphRun* paragraph, RunRangeMap& runRanges, vint start, vint end)
			{
				AddStyleName(paragraph, runRanges, start, end, styleName);
			});
		}

		bool DocumentModel::RemoveStyleName(TextPos begin, TextPos end)
		{
			return EditContainer(begin, end, [=](DocumentParagraphRun* paragraph, RunRangeMap& runRanges, vint start, vint end)
			{
				document_editor::RemoveStyleName(paragraph, runRanges, start, end);
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
				ReplaceStyleName(paragraph.Obj(), oldStyleName, newStyleName);
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
				document_editor::ClearStyle(paragraph, runRanges, start, end);
			});
		}

/***********************************************************************
DocumentModel::ClearStyle
***********************************************************************/

		Ptr<DocumentStyleProperties> DocumentModel::SummarizeStyle(TextPos begin, TextPos end)
		{
			Ptr<DocumentStyleProperties> style;
			RunRangeMap runRanges;

			if (begin == end) goto END_OF_SUMMERIZING;

			// check caret range
			if (!CheckEditRange(begin, end, runRanges)) return nullptr;

			// Summarize container
			if (begin.row == end.row)
			{
				style = document_editor::SummarizeStyle(paragraphs[begin.row].Obj(), runRanges, this, begin.column, end.column);
			}
			else
			{
				for (vint i = begin.row; i <= end.row; i++)
				{
					Ptr<DocumentParagraphRun> paragraph = paragraphs[i];
					if (begin.row < i && i < end.row)
					{
						GetRunRange(paragraph.Obj(), runRanges);
					}
					RunRange range = runRanges[paragraph.Obj()];
					Ptr<DocumentStyleProperties> paragraphStyle;
					if (i == begin.row)
					{
						paragraphStyle = document_editor::SummarizeStyle(paragraph.Obj(), runRanges, this, begin.column, range.end);
					}
					else if (i == end.row)
					{
						paragraphStyle = document_editor::SummarizeStyle(paragraph.Obj(), runRanges, this, range.start, end.column);
					}
					else
					{
						paragraphStyle = document_editor::SummarizeStyle(paragraph.Obj(), runRanges, this, range.start, range.end);
					}

					if (!style)
					{
						style = paragraphStyle;
					}
					else if (paragraphStyle)
					{
						AggregateStyle(style, paragraphStyle);
					}
				}
			}

		END_OF_SUMMERIZING:
			if (!style)
			{
				style = new DocumentStyleProperties;
			}
			return style;
		}

		Nullable<WString> DocumentModel::SummarizeStyleName(TextPos begin, TextPos end)
		{
			if (begin == end) return {};

			// check caret range
			RunRangeMap runRanges;
			if (!CheckEditRange(begin, end, runRanges)) return {};

			// Summarize container
			Nullable<WString> styleName;

			if (begin.row == end.row)
			{
				styleName = document_editor::SummarizeStyleName(paragraphs[begin.row].Obj(), runRanges, this, begin.column, end.column);
			}
			else
			{
				for (vint i = begin.row; i <= end.row; i++)
				{
					Ptr<DocumentParagraphRun> paragraph = paragraphs[i];
					if (begin.row < i && i < end.row)
					{
						GetRunRange(paragraph.Obj(), runRanges);
					}
					RunRange range = runRanges[paragraph.Obj()];
					Nullable<WString> newStyleName;
					if (i == begin.row)
					{
						newStyleName = document_editor::SummarizeStyleName(paragraph.Obj(), runRanges, this, begin.column, range.end);
					}
					else if (i == end.row)
					{
						newStyleName = document_editor::SummarizeStyleName(paragraph.Obj(), runRanges, this, range.start, end.column);
					}
					else
					{
						newStyleName = document_editor::SummarizeStyleName(paragraph.Obj(), runRanges, this, range.start, end.column);
					}

					if (i == begin.row)
					{
						styleName = newStyleName;
					}
					else if (!styleName || !newStyleName || styleName.Value() != newStyleName.Value())
					{
						styleName = Nullable<WString>();
					}
				}
			}
			return styleName;
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
