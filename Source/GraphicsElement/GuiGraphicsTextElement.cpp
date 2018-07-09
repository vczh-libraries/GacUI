#include "GuiGraphicsTextElement.h"

namespace vl
{
	using namespace collections;

	namespace presentation
	{
		namespace elements
		{
			namespace text
			{

/***********************************************************************
text::TextLine
***********************************************************************/

				TextLine::TextLine()
					:text(0)
					,att(0)
					,availableOffsetCount(0)
					,bufferLength(0)
					,dataLength(0)
					,lexerFinalState(-1)
					,contextFinalState(-1)
				{
				}

				TextLine::~TextLine()
				{
				}

				vint TextLine::CalculateBufferLength(vint dataLength)
				{
					if(dataLength<1)dataLength=1;
					vint bufferLength=dataLength-dataLength%BlockSize;
					if(bufferLength<dataLength)
					{
						bufferLength+=BlockSize;
					}
					return bufferLength;
				}

				void TextLine::Initialize()
				{
					Finalize();
					text=new wchar_t[BlockSize];
					att=new CharAtt[BlockSize];
					bufferLength=BlockSize;

					memset(text, 0, sizeof(wchar_t)*bufferLength);
					memset(att, 0, sizeof(CharAtt)*bufferLength);
				}

				void TextLine::Finalize()
				{
					if(text)
					{
						delete[] text;
						text=0;
					}
					if(att)
					{
						delete[] att;
						att=0;
					}
					availableOffsetCount=0;
					bufferLength=0;
					dataLength=0;
				}

				bool TextLine::IsReady()
				{
					return text && att;
				}

				bool TextLine::Modify(vint start, vint count, const wchar_t* input, vint inputCount)
				{
					if(!text || !att || start<0 || count<0 || start+count>dataLength || inputCount<0) return false;

					vint newDataLength=dataLength-count+inputCount;
					vint newBufferLength=CalculateBufferLength(newDataLength);
					if(newBufferLength!=bufferLength)
					{
						wchar_t* newText=new wchar_t[newBufferLength];
						memcpy(newText, text, start*sizeof(wchar_t));
						memcpy(newText+start, input, inputCount*sizeof(wchar_t));
						memcpy(newText+start+inputCount, text+start+count, (dataLength-start-count)*sizeof(wchar_t));

						CharAtt* newAtt=new CharAtt[newBufferLength];
						memcpy(newAtt, att, start*sizeof(CharAtt));
						memset(newAtt+start, 0, inputCount*sizeof(CharAtt));
						memcpy(newAtt+start+inputCount, att+start+count, (dataLength-start-count)*sizeof(CharAtt));

						delete[] text;
						delete[] att;
						text=newText;
						att=newAtt;
					}
					else
					{
						memmove(text+start+inputCount, text+start+count, (dataLength-start-count)*sizeof(wchar_t));
						memmove(att+start+inputCount, att+start+count, (dataLength-start-count)*sizeof(CharAtt));
						memcpy(text+start, input, inputCount*sizeof(wchar_t));
						memset(att+start, 0, inputCount*sizeof(CharAtt));
					}
					dataLength=newDataLength;
					bufferLength=newBufferLength;
					if(availableOffsetCount>start)
					{
						availableOffsetCount=start;
					}

					return true;
				}

				TextLine TextLine::Split(vint index)
				{
					if(index<0 || index>dataLength) return TextLine();
					vint count=dataLength-index;
					TextLine line;
					line.Initialize();
					line.Modify(0, 0, text+index, count);
					memcpy(line.att, att+index, count*sizeof(CharAtt));
					Modify(index, count, L"", 0);
					return line;
				}

				void TextLine::AppendAndFinalize(TextLine& line)
				{
					vint oldDataLength=dataLength;
					Modify(oldDataLength, 0, line.text, line.dataLength);
					memcpy(att+oldDataLength, line.att, line.dataLength*sizeof(CharAtt));
					line.Finalize();
				}

/***********************************************************************
text::CharMeasurer
***********************************************************************/

				CharMeasurer::CharMeasurer(vint _rowHeight)
					:rowHeight(_rowHeight)
				{
					memset(widths, 0, sizeof(widths));
				}

				CharMeasurer::~CharMeasurer()
				{
				}

				void CharMeasurer::SetRenderTarget(IGuiGraphicsRenderTarget* value)
				{
					if(oldRenderTarget!=value)
					{
						oldRenderTarget=value;
						rowHeight=GetRowHeightInternal(oldRenderTarget);
						memset(widths, 0, sizeof(widths));
					}
				}

				vint CharMeasurer::MeasureWidth(UnicodeCodePoint codePoint)
				{
					vuint32_t index = codePoint.GetCodePoint();
					if (0 <= index && index < 65536)
					{
						vint w = widths[index];
						if (w == 0)
						{
							widths[index] = w = MeasureWidthInternal(codePoint, oldRenderTarget);
						}
						return w;
					}
					else if (index < 0x110000)
					{
						return MeasureWidthInternal(codePoint, oldRenderTarget);
					}
					else
					{
						return 0;
					}
				}

				vint CharMeasurer::GetRowHeight()
				{
					return rowHeight;
				}

/***********************************************************************
text::TextLines
***********************************************************************/

				TextLines::TextLines(GuiColorizedTextElement* _ownerElement)
					:ownerElement(_ownerElement)
					,charMeasurer(0)
					,renderTarget(0)
					,tabWidth(1)
					,tabSpaceCount(4)
					,passwordChar(L'\0')
				{
					TextLine line;
					line.Initialize();
					lines.Add(line);
				}

				TextLines::~TextLines()
				{
					RemoveLines(0, lines.Count());
				}

				//--------------------------------------------------------

				vint TextLines::GetCount()
				{
					return lines.Count();
				}

				TextLine& TextLines::GetLine(vint row)
				{
					return lines[row];
				}

				CharMeasurer* TextLines::GetCharMeasurer()
				{
					return charMeasurer;
				}

				void TextLines::SetCharMeasurer(CharMeasurer* value)
				{
					charMeasurer=value;
					if(charMeasurer) charMeasurer->SetRenderTarget(renderTarget);
					ClearMeasurement();
				}

				IGuiGraphicsRenderTarget* TextLines::GetRenderTarget()
				{
					return renderTarget;
				}

				void TextLines::SetRenderTarget(IGuiGraphicsRenderTarget* value)
				{
					renderTarget=value;
					if(charMeasurer) charMeasurer->SetRenderTarget(renderTarget);
					ClearMeasurement();
				}

				WString TextLines::GetText(TextPos start, TextPos end)
				{
					if(!IsAvailable(start) || !IsAvailable(end) || start>end) return L"";

					if(start.row==end.row)
					{
						return WString(lines[start.row].text+start.column, end.column-start.column);
					}

					vint count=0;
					for(vint i=start.row+1;i<end.row;i++)
					{
						count+=lines[i].dataLength;
					}
					count+=lines[start.row].dataLength-start.column;
					count+=end.column;

					Array<wchar_t> buffer;
					buffer.Resize(count+(end.row-start.row)*2);
					wchar_t* writing=&buffer[0];

					for(vint i=start.row;i<=end.row;i++)
					{
						wchar_t* text=lines[i].text;
						vint chars=0;
						if(i==start.row)
						{
							text+=start.column;
							chars=lines[i].dataLength-start.column;
						}
						else if(i==end.row)
						{
							chars=end.column;
						}
						else
						{
							chars=lines[i].dataLength;
						}

						if(i!=start.row)
						{
							*writing++=L'\r';
							*writing++=L'\n';
						}
						memcpy(writing, text, chars*sizeof(wchar_t));
						writing+=chars;
					}
					return WString(&buffer[0], buffer.Count());
				}

				WString TextLines::GetText()
				{
					return GetText(TextPos(0, 0), TextPos(lines.Count()-1, lines[lines.Count()-1].dataLength));
				}

				void TextLines::SetText(const WString& value)
				{
					Modify(TextPos(0, 0), TextPos(lines.Count()-1, lines[lines.Count()-1].dataLength), value);
				}

				//--------------------------------------------------------

				bool TextLines::RemoveLines(vint start, vint count)
				{
					if(start<0 || count<0 || start+count>lines.Count()) return false;
					for(vint i=start+count-1;i>=start;i--)
					{
						lines[i].Finalize();
					}
					lines.RemoveRange(start, count);
					return true;
				}

				bool TextLines::IsAvailable(TextPos pos)
				{
					return 0<=pos.row && pos.row<lines.Count() && 0<=pos.column && pos.column<=lines[pos.row].dataLength;
				}

				TextPos TextLines::Normalize(TextPos pos)
				{
					if(pos.row<0)
					{
						return TextPos(0, 0);
					}
					else if(pos.row>=lines.Count())
					{
						return TextPos(lines.Count()-1, lines[lines.Count()-1].dataLength);
					}
					else
					{
						TextLine& line=lines[pos.row];
						if(pos.column<0)
						{
							return TextPos(pos.row, 0);
						}
						else if(pos.column>line.dataLength)
						{
							return TextPos(pos.row, line.dataLength);
						}
						else
						{
							return pos;
						}
					}
				}

				TextPos TextLines::Modify(TextPos start, TextPos end, const wchar_t** inputs, vint* inputCounts, vint rows)
				{
					if(!IsAvailable(start) || !IsAvailable(end) || start>end) return TextPos(-1, -1);
					if (ownerElement)
					{
						ownerElement->InvokeOnElementStateChanged();
					}

					if(rows==1)
					{
						if(start.row==end.row)
						{
							lines[start.row].Modify(start.column, end.column-start.column, inputs[0], inputCounts[0]);
						}
						else
						{
							if(end.row-start.row>1)
							{
								RemoveLines(start.row+1, end.row-start.row-1);
							}
							vint modifyCount=lines[start.row].dataLength-start.column+end.column;
							lines[start.row].AppendAndFinalize(lines[start.row+1]);
							lines.RemoveAt(start.row+1);
							lines[start.row].Modify(start.column, modifyCount, inputs[0], inputCounts[0]);
						}
						return TextPos(start.row, start.column+inputCounts[0]);
					}

					if(start.row==end.row)
					{
						TextLine newLine=lines[start.row].Split(end.column);
						lines.Insert(start.row+1, newLine);
						end=TextPos(start.row+1, 0);
					}

					vint oldMiddleLines=end.row-start.row-1;
					vint newMiddleLines=rows-2;
					if(oldMiddleLines<newMiddleLines)
					{
						for(vint i=oldMiddleLines;i<newMiddleLines;i++)
						{
							TextLine line;
							line.Initialize();
							lines.Insert(end.row, line);
						}
					}
					else if(oldMiddleLines>newMiddleLines)
					{
						RemoveLines(start.row+newMiddleLines+1, oldMiddleLines-newMiddleLines);
					}
					end.row+=newMiddleLines-oldMiddleLines;

					lines[start.row].Modify(start.column, lines[start.row].dataLength-start.column, inputs[0], inputCounts[0]);
					lines[end.row].Modify(0, end.column, inputs[rows-1], inputCounts[rows-1]);
					for(vint i=1;i<rows-1;i++)
					{
						lines[start.row+i].Modify(0, lines[start.row+i].dataLength, inputs[i], inputCounts[i]);
					}
					return TextPos(end.row, inputCounts[rows-1]);
				}

				TextPos TextLines::Modify(TextPos start, TextPos end, const wchar_t* input, vint inputCount)
				{
					List<const wchar_t*> inputs;
					List<vint> inputCounts;
					const wchar_t* previous=input;
					const wchar_t* current=input;

					while(true)
					{
						if(current==input+inputCount)
						{
							inputs.Add(previous);
							inputCounts.Add(current-previous);
							break;
						}
						else if(*current==L'\r' || *current==L'\n')
						{
							inputs.Add(previous);
							inputCounts.Add(current-previous);
							previous=current+(current[1]==L'\n'?2:1);
							current=previous;
						}
						else
						{
							current++;
						}
					}

					return Modify(start, end, &inputs[0], &inputCounts[0], inputs.Count());
				}

				TextPos TextLines::Modify(TextPos start, TextPos end, const wchar_t* input)
				{
					return Modify(start, end, input, wcslen(input));
				}

				TextPos TextLines::Modify(TextPos start, TextPos end, const WString& input)
				{
					return Modify(start, end, input.Buffer(), input.Length());
				}

				void TextLines::Clear()
				{
					RemoveLines(0, lines.Count());
					TextLine line;
					line.Initialize();
					lines.Add(line);
					if (ownerElement)
					{
						ownerElement->InvokeOnElementStateChanged();
					}
				}

				//--------------------------------------------------------

				void TextLines::ClearMeasurement()
				{
					for (vint i = 0; i < lines.Count(); i++)
					{
						lines[i].availableOffsetCount = 0;
					}

					tabWidth = tabSpaceCount * (charMeasurer ? charMeasurer->MeasureWidth({ L' ' }) : 1);
					if (tabWidth == 0)
					{
						tabWidth = 1;
					}

					if (ownerElement)
					{
						ownerElement->InvokeOnElementStateChanged();
					}
				}

				vint TextLines::GetTabSpaceCount()
				{
					return tabSpaceCount;
				}

				void TextLines::SetTabSpaceCount(vint value)
				{
					if(value<1) value=1;
					if(tabSpaceCount!=value)
					{
						tabSpaceCount=value;
						ClearMeasurement();
					}
				}

				void TextLines::MeasureRow(vint row)
				{
					TextLine& line = lines[row];
					vint offset = 0;
					if (line.availableOffsetCount)
					{
						offset = line.att[line.availableOffsetCount - 1].rightOffset;
					}
					for (vint i = line.availableOffsetCount; i < line.dataLength; i++)
					{
						CharAtt& att = line.att[i];
						wchar_t c = line.text[i];
						vint width = 0;
						vint passwordWidth = 0;
						if (passwordChar)
						{
							passwordWidth = charMeasurer ? charMeasurer->MeasureWidth({ passwordChar }) : 1;
						}

						if (c == L'\t')
						{
							width = tabWidth - offset % tabWidth;
						}
#if defined VCZH_MSVC
						else if (UTF16SPFirst(c) && (i + 1 < line.dataLength) && UTF16SPSecond(line.text[i + 1]))
						{
							width = passwordChar ? passwordWidth : (charMeasurer ? charMeasurer->MeasureWidth({ c, line.text[i + 1] }) : 1);
							offset += width;
							att.rightOffset = (int)offset;
							line.att[i + 1].rightOffset = (int)offset;
							i++;
							continue;
						}
#endif
						else
						{
							width = passwordChar ? passwordWidth : (charMeasurer ? charMeasurer->MeasureWidth({ c }) : 1);
						}
						offset += width;
						att.rightOffset = (int)offset;
					}
					line.availableOffsetCount = line.dataLength;
				}

				vint TextLines::GetRowWidth(vint row)
				{
					if(row<0 || row>=lines.Count()) return -1;
					TextLine& line=lines[row];
					if(line.dataLength==0)
					{
						return 0;
					}
					else
					{
						MeasureRow(row);
						return line.att[line.dataLength-1].rightOffset;
					}
				}

				vint TextLines::GetRowHeight()
				{
					return charMeasurer ? charMeasurer->GetRowHeight() : 1;
				}

				vint TextLines::GetMaxWidth()
				{
					vint width=0;
					for(vint i=0;i<lines.Count();i++)
					{
						vint rowWidth=GetRowWidth(i);
						if(width<rowWidth)
						{
							width=rowWidth;
						}
					}
					return width;
				}

				vint TextLines::GetMaxHeight()
				{
					return lines.Count() * GetRowHeight();
				}

				TextPos TextLines::GetTextPosFromPoint(Point point)
				{
					vint h = GetRowHeight();
					if(point.y<0)
					{
						point.y=0;
					}
					else if(point.y>=h*lines.Count())
					{
						point.y=h*lines.Count()-1;
					}

					vint row=point.y/h;
					if(point.x<0)
					{
						return TextPos(row, 0);
					}
					else if(point.x>=GetRowWidth(row))
					{
						return TextPos(row, lines[row].dataLength);
					}
					TextLine& line=lines[row];

					vint i1=0, i2=line.dataLength;
					vint p1=0, p2=line.att[line.dataLength-1].rightOffset;
					while(i2-i1>1)
					{
						vint i=(i1+i2)/2;
						vint p=i==0?0:line.att[i-1].rightOffset;
						if(point.x<p)
						{
							i2=i;
							p2=p;
						}
						else
						{
							i1=i;
							p1=p;
						}
					}
#if defined VCZH_MSVC
					if (UTF16SPSecond(line.text[i1]) && i1 > 0 && UTF16SPFirst(line.text[i1 - 1]))
					{
						i1--;
					}
#endif
					return TextPos(row, i1);
				}

				Point TextLines::GetPointFromTextPos(TextPos pos)
				{
					if(IsAvailable(pos))
					{
						vint y = pos.row * GetRowHeight();
						if(pos.column==0)
						{
							return Point(0, y);
						}
						else
						{
							MeasureRow(pos.row);
							TextLine& line=lines[pos.row];
							return Point(line.att[pos.column-1].rightOffset, y);
						}
					}
					else
					{
						return Point(-1, -1);
					}
				}

				Rect TextLines::GetRectFromTextPos(TextPos pos)
				{
					Point point=GetPointFromTextPos(pos);
					if(point==Point(-1, -1))
					{
						return Rect(-1, -1, -1, -1);
					}
					else
					{
						vint h = GetRowHeight();
						TextLine& line=lines[pos.row];
						if(pos.column==line.dataLength)
						{
							return Rect(point, Size(h/2, h));
						}
						else
						{
							return Rect(point, Size(line.att[pos.column].rightOffset-point.x, h));
						}
					}
				}

				//--------------------------------------------------------

				wchar_t TextLines::GetPasswordChar()
				{
					return passwordChar;
				}

				void TextLines::SetPasswordChar(wchar_t value)
				{
					passwordChar=value;
					ClearMeasurement();
				}
			}

			using namespace text;

/***********************************************************************
GuiColorizedTextElement
***********************************************************************/

			GuiColorizedTextElement::GuiColorizedTextElement()
				:callback(0)
				,isVisuallyEnabled(true)
				,isFocused(false)
				,caretVisible(false)
				,lines(this)
			{
			}

			text::TextLines& GuiColorizedTextElement::GetLines()
			{
				return lines;
			}

			GuiColorizedTextElement::ICallback* GuiColorizedTextElement::GetCallback()
			{
				return callback;
			}

			void GuiColorizedTextElement::SetCallback(ICallback* value)
			{
				callback=value;
				if(!callback)
				{
					lines.SetCharMeasurer(0);
				}
			}

			const GuiColorizedTextElement::ColorArray& GuiColorizedTextElement::GetColors()
			{
				return colors;
			}

			void GuiColorizedTextElement::SetColors(const ColorArray& value)
			{
				CopyFrom(colors, value);
				if(callback) callback->ColorChanged();
				InvokeOnElementStateChanged();
			}

			void GuiColorizedTextElement::ResetTextColorIndex(vint index)
			{
				vint lineCount = lines.GetCount();
				for (vint i = 0; i < lineCount; i++)
				{
					auto& line = lines.GetLine(i);
					line.lexerFinalState = -1;
					line.contextFinalState = -1;
					for (vint j = 0; j < line.dataLength; j++)
					{
						line.att[j].colorIndex = (vuint32_t)index;
					}
				}
			}

			const FontProperties& GuiColorizedTextElement::GetFont()
			{
				return font;
			}

			void GuiColorizedTextElement::SetFont(const FontProperties& value)
			{
				if(font!=value)
				{
					font=value;
					if(callback)
					{
						callback->FontChanged();
					}
					InvokeOnElementStateChanged();
				}
			}

			wchar_t GuiColorizedTextElement::GetPasswordChar()
			{
				return lines.GetPasswordChar();
			}

			void GuiColorizedTextElement::SetPasswordChar(wchar_t value)
			{
				if(lines.GetPasswordChar()!=value)
				{
					lines.SetPasswordChar(value);
					InvokeOnElementStateChanged();
				}
			}

			Point GuiColorizedTextElement::GetViewPosition()
			{
				return viewPosition;
			}

			void GuiColorizedTextElement::SetViewPosition(Point value)
			{
				if(viewPosition!=value)
				{
					viewPosition=value;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiColorizedTextElement::GetVisuallyEnabled()
			{
				return isVisuallyEnabled;
			}

			void GuiColorizedTextElement::SetVisuallyEnabled(bool value)
			{
				if(isVisuallyEnabled!=value)
				{
					isVisuallyEnabled=value;
					InvokeOnElementStateChanged();
				}
			}

			bool GuiColorizedTextElement::GetFocused()
			{
				return isFocused;
			}

			void GuiColorizedTextElement::SetFocused(bool value)
			{
				if(isFocused!=value)
				{
					isFocused=value;
					InvokeOnElementStateChanged();
				}
			}

			TextPos GuiColorizedTextElement::GetCaretBegin()
			{
				return caretBegin;
			}

			void GuiColorizedTextElement::SetCaretBegin(TextPos value)
			{
				caretBegin=value;
				InvokeOnElementStateChanged();
			}

			TextPos GuiColorizedTextElement::GetCaretEnd()
			{
				return caretEnd;
			}

			void GuiColorizedTextElement::SetCaretEnd(TextPos value)
			{
				caretEnd=value;
				InvokeOnElementStateChanged();
			}

			bool GuiColorizedTextElement::GetCaretVisible()
			{
				return caretVisible;
			}

			void GuiColorizedTextElement::SetCaretVisible(bool value)
			{
				caretVisible=value;
				InvokeOnElementStateChanged();
			}

			Color GuiColorizedTextElement::GetCaretColor()
			{
				return caretColor;
			}

			void GuiColorizedTextElement::SetCaretColor(Color value)
			{
				if(caretColor!=value)
				{
					caretColor=value;
					InvokeOnElementStateChanged();
				}
			}
		}
	}
}
