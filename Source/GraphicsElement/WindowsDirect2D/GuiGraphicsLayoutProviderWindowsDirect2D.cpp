#include "GuiGraphicsWindowsDirect2D.h"
#include "GuiGraphicsLayoutProviderWindowsDirect2D.h"

namespace vl
{
	namespace presentation
	{
		using namespace elements;
		using namespace collections;

		namespace elements_windows_d2d
		{

/***********************************************************************
WindowsDirect2DElementInlineObject
***********************************************************************/

			class WindowsDirect2DElementInlineObject : public IDWriteInlineObject
			{
			public:
				class IRendererCallback : public Interface
				{
				public:
					virtual Color									GetBackgroundColor(vint textPosition) = 0;
					virtual IWindowsDirect2DRenderTarget*			GetDirect2DRenderTarget() = 0;
					virtual Point									GetParagraphOffset() = 0;
					virtual IGuiGraphicsParagraphCallback*			GetParagraphCallback() = 0;
				};

			protected:
				vint												counter;
				IGuiGraphicsParagraph::InlineObjectProperties		properties;
				IRendererCallback*									rendererCallback;
				vint												start;
				vint												length;

			public:
				WindowsDirect2DElementInlineObject(
					const IGuiGraphicsParagraph::InlineObjectProperties& _properties,
					IRendererCallback* _rendererCallback,
					vint _start,
					vint _length
					)
					:counter(1)
					,properties(_properties)
					,rendererCallback(_rendererCallback)
					,start(_start)
					,length(_length)
				{
				}

				~WindowsDirect2DElementInlineObject()
				{
					if (properties.backgroundImage)
					{
						IGuiGraphicsRenderer* graphicsRenderer=properties.backgroundImage->GetRenderer();
						if(graphicsRenderer)
						{
							graphicsRenderer->SetRenderTarget(0);
						}
					}
				}

				vint GetStart()
				{
					return start;
				}

				vint GetLength()
				{
					return length;
				}

				const IGuiGraphicsParagraph::InlineObjectProperties& GetProperties()
				{
					return properties;
				}

				Ptr<IGuiGraphicsElement> GetElement()
				{
					return properties.backgroundImage;
				}

				HRESULT STDMETHODCALLTYPE QueryInterface( 
					REFIID riid,
					void __RPC_FAR *__RPC_FAR *ppvObject
					)
				{
					if(ppvObject)
					{
						*ppvObject=NULL;
					}
					return E_NOINTERFACE;
				}

				ULONG STDMETHODCALLTYPE AddRef(void)
				{
					++counter;
					return S_OK;
				}

				ULONG STDMETHODCALLTYPE Release(void)
				{
					if(--counter==0)
					{
						delete this;
					}
					return S_OK;
				}

				STDMETHOD(Draw)(
					void* clientDrawingContext,
					IDWriteTextRenderer* renderer,
					FLOAT originX,
					FLOAT originY,
					BOOL isSideways,
					BOOL isRightToLeft,
					IUnknown* clientDrawingEffect
					)override
				{
					Rect bounds(Point((vint)originX, (vint)originY), properties.size);
					if (properties.backgroundImage)
					{
						IGuiGraphicsRenderer* graphicsRenderer=properties.backgroundImage->GetRenderer();
						if(graphicsRenderer)
						{
							graphicsRenderer->Render(bounds);
						}
					}

					Color color=rendererCallback->GetBackgroundColor(start);
					if(color.a!=0)
					{
						color.a/=2;
						if(IWindowsDirect2DRenderTarget* renderTarget=rendererCallback->GetDirect2DRenderTarget())
						{
							ID2D1SolidColorBrush* brush=renderTarget->CreateDirect2DBrush(color);

							renderTarget->GetDirect2DRenderTarget()->FillRectangle(
								D2D1::RectF(bounds.x1-0.5f, bounds.y1-0.5f, bounds.x2+0.5f, bounds.y2+0.5f),
								brush
								);

							renderTarget->DestroyDirect2DBrush(color);
						}
					}

					if (properties.callbackId != -1)
					{
						if (auto callback = rendererCallback->GetParagraphCallback())
						{
							auto offset = rendererCallback->GetParagraphOffset();
							auto size = callback->OnRenderInlineObject(properties.callbackId, Rect(Point(bounds.x1 - offset.x, bounds.y1 - offset.y), bounds.GetSize()));
							properties.size = size;
						}
					}
					return S_OK;
				}

				STDMETHOD(GetMetrics)(
					DWRITE_INLINE_OBJECT_METRICS* metrics
					)override
				{
					metrics->width=(FLOAT)properties.size.x;
					metrics->height=(FLOAT)properties.size.y;
					metrics->baseline=(FLOAT)(properties.baseline==-1?properties.size.y:properties.baseline);
					metrics->supportsSideways=TRUE;
					return S_OK;
				}

				STDMETHOD(GetOverhangMetrics)(
					DWRITE_OVERHANG_METRICS* overhangs
					)override
				{
					overhangs->left=0;
					overhangs->right=0;
					overhangs->top=0;
					overhangs->bottom=0;
					return S_OK;
				}

				STDMETHOD(GetBreakConditions)(
					DWRITE_BREAK_CONDITION* breakConditionBefore,
					DWRITE_BREAK_CONDITION* breakConditionAfter
					)override
				{
					switch(properties.breakCondition)
					{
					case IGuiGraphicsParagraph::StickToPreviousRun:
						*breakConditionBefore=DWRITE_BREAK_CONDITION_MAY_NOT_BREAK;
						*breakConditionAfter=DWRITE_BREAK_CONDITION_CAN_BREAK;
						break;
					case IGuiGraphicsParagraph::StickToNextRun:
						*breakConditionBefore=DWRITE_BREAK_CONDITION_CAN_BREAK;
						*breakConditionAfter=DWRITE_BREAK_CONDITION_MAY_NOT_BREAK;
						break;
					default:
						*breakConditionBefore=DWRITE_BREAK_CONDITION_CAN_BREAK;
						*breakConditionAfter=DWRITE_BREAK_CONDITION_CAN_BREAK;
					}
					return S_OK;
				}
			};

/***********************************************************************
WindowsDirect2DParagraph
***********************************************************************/

			class WindowsDirect2DParagraph : public Object, public IGuiGraphicsParagraph, public WindowsDirect2DElementInlineObject::IRendererCallback
			{
			protected:
				struct TextRange
				{
					vint								start;
					vint								end;

					TextRange(){}
					TextRange(vint _start, vint _end):start(_start),end(_end){}

					bool operator==(const TextRange& range) const { return start==range.start; }
					bool operator!=(const TextRange& range) const { return start!=range.start; }
					bool operator<(const TextRange& range) const { return start<range.start; }
					bool operator<=(const TextRange& range) const { return start<=range.start; }
					bool operator>(const TextRange& range) const { return start>range.start; }
					bool operator>=(const TextRange& range) const { return start>=range.start; }
				};

				typedef Dictionary<IGuiGraphicsElement*, ComPtr<WindowsDirect2DElementInlineObject>>	InlineElementMap;
				typedef Dictionary<TextRange, Color>													ColorMap;
				typedef Dictionary<TextRange, IGuiGraphicsElement*>										GraphicsElementMap;
			protected:
				IGuiGraphicsLayoutProvider*				provider;
				ID2D1SolidColorBrush*					defaultTextColor;
				IDWriteFactory*							dwriteFactory;
				IWindowsDirect2DRenderTarget*			renderTarget;
				WString									paragraphText;
				ComPtr<IDWriteTextLayout>				textLayout;
				bool									wrapLine;
				vint									maxWidth;
				List<Color>								usedColors;

				InlineElementMap						inlineElements;
				GraphicsElementMap						graphicsElements;
				ColorMap								backgroundColors;

				vint									caret;
				Color									caretColor;
				bool									caretFrontSide;
				ID2D1SolidColorBrush*					caretBrush;

				bool									formatDataAvailable;
				Array<DWRITE_LINE_METRICS>				lineMetrics;
				Array<vint>								lineStarts;
				Array<FLOAT>							lineTops;
				Array<DWRITE_CLUSTER_METRICS>			clusterMetrics;
				Array<DWRITE_HIT_TEST_METRICS>			hitTestMetrics;
				Array<vint>								charHitTestMap;

				Point									paragraphOffset;
				IGuiGraphicsParagraphCallback*			paragraphCallback;

/***********************************************************************
WindowsDirect2DParagraph (Ranges)
***********************************************************************/

				template<typename T>
				void CutMap(Dictionary<TextRange, T>& map, vint start, vint length)
				{
					vint end=start+length;
					for(vint i=map.Count()-1;i>=0;i--)
					{
						TextRange key=map.Keys()[i];
						if(key.start<end && start<key.end)
						{
							T value=map.Values()[i];

							vint s1=key.start;
							vint s2=key.start>start?key.start:start;
							vint s3=key.end<end?key.end:end;
							vint s4=key.end;

							map.Remove(key);
							if(s1<s2)
							{
								map.Add(TextRange(s1, s2), value);
							}
							if(s2<s3)
							{
								map.Add(TextRange(s2, s3), value);
							}
							if(s3<s4)
							{
								map.Add(TextRange(s3, s4), value);
							}
						}
					}
				}
				
				template<typename T>
				void UpdateOverlappedMap(Dictionary<TextRange, T>& map, vint start, vint length, const T& value)
				{
					vint end=start+length;
					for(vint i=map.Count()-1;i>=0;i--)
					{
						TextRange key=map.Keys()[i];
						if(key.start<end && start<key.end)
						{
							map.Set(key, value);
						}
					}
				}
				
				template<typename T>
				void DefragmentMap(Dictionary<TextRange, T>& map)
				{
					vint lastIndex=map.Count()-1;
					T lastValue=map.Values()[lastIndex];
					for(vint i=map.Count()-2;i>=-1;i--)
					{
						if(i==-1 || map.Values()[i]!=lastValue)
						{
							if(lastIndex-i>0)
							{
								vint start=map.Keys()[i+1].start;
								vint end=map.Keys()[lastIndex].end;
								TextRange key(start, end);

								for(vint j=lastIndex;j>i;j--)
								{
									map.Remove(map.Keys()[j]);
								}
								map.Add(key, lastValue);
							}
							lastIndex=i;
							if(i!=-1)
							{
								lastValue=map.Values()[i];
							}
						}
					}
				}
				
				template<typename T>
				void SetMap(Dictionary<TextRange, T>& map, vint start, vint length, const T& value)
				{
					CutMap(map, start, length);
					UpdateOverlappedMap(map, start, length, value);
					DefragmentMap(map);
				}

				template<typename T>
				bool GetMap(Dictionary<TextRange, T>& map, vint textPosition, T& value)
				{
					vint start=0;
					vint end=map.Count()-1;
					while(start<=end)
					{
						vint middle=(start+end)/2;
						TextRange key=map.Keys()[middle];
						if(textPosition<key.start)
						{
							end=middle-1;
						}
						else if(textPosition>=key.end)
						{
							start=middle+1;
						}
						else
						{
							value=map.Values()[middle];
							return true;
						}
					}
					return false;
				}

/***********************************************************************
WindowsDirect2DParagraph (Layout Retriving)
***********************************************************************/

				void PrepareFormatData()
				{
					if(!formatDataAvailable)
					{
						formatDataAvailable=true;
						{
							UINT32 lineCount=0;
							textLayout->GetLineMetrics(NULL, 0, &lineCount);
							lineMetrics.Resize(lineCount);
							if(lineCount>0)
							{
								textLayout->GetLineMetrics(&lineMetrics[0], lineCount, &lineCount);
							}

							lineStarts.Resize(lineCount);
							lineTops.Resize(lineCount);
							vint start=0;
							FLOAT top=0;
							for(vint i=0;i<lineMetrics.Count();i++)
							{
								DWRITE_LINE_METRICS& metrics=lineMetrics[i];

								lineStarts[i]=start;
								start+=metrics.length;

								lineTops[i]=top;
								top+=metrics.height;
							}
						}
						{
							UINT32 clusterCount=0;
							textLayout->GetClusterMetrics(NULL, 0, &clusterCount);
							clusterMetrics.Resize(clusterCount);
							if(clusterCount>0)
							{
								textLayout->GetClusterMetrics(&clusterMetrics[0], clusterCount, &clusterCount);
							}
						}
						{
							vint textPos=0;
							hitTestMetrics.Resize(clusterMetrics.Count());
							for(vint i=0;i<hitTestMetrics.Count();i++)
							{
								FLOAT x=0;
								FLOAT y=0;
								DWRITE_HIT_TEST_METRICS& metrics=hitTestMetrics[i];
								textLayout->HitTestTextPosition((UINT32)textPos, FALSE, &x, &y, &metrics);
								textPos+=metrics.length;
							}
						}
						{
							charHitTestMap.Resize(paragraphText.Length());
							for(vint i=0;i<hitTestMetrics.Count();i++)
							{
								DWRITE_HIT_TEST_METRICS& metrics=hitTestMetrics[i];
								for(UINT32 j=0;j<metrics.length;j++)
								{
									charHitTestMap[metrics.textPosition+j]=i;
								}
							}
						}
					}
				}
			public:

/***********************************************************************
WindowsDirect2DParagraph (Initialization)
***********************************************************************/

				WindowsDirect2DParagraph(IGuiGraphicsLayoutProvider* _provider, const WString& _text, IGuiGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _paragraphCallback)
					:provider(_provider)
					,dwriteFactory(GetWindowsDirect2DObjectProvider()->GetDirectWriteFactory())
					,renderTarget(dynamic_cast<IWindowsDirect2DRenderTarget*>(_renderTarget))
					,paragraphText(_text)
					,textLayout(0)
					,wrapLine(true)
					,maxWidth(-1)
					,caret(-1)
					,caretFrontSide(false)
					,caretBrush(0)
					,formatDataAvailable(false)
					,paragraphCallback(_paragraphCallback)
				{
					FontProperties defaultFont=GetCurrentController()->ResourceService()->GetDefaultFont();
					Direct2DTextFormatPackage* package=GetWindowsDirect2DResourceManager()->CreateDirect2DTextFormat(defaultFont);
					defaultTextColor=renderTarget->CreateDirect2DBrush(Color(0, 0, 0));
					usedColors.Add(Color(0, 0, 0));

					IDWriteTextLayout* rawTextLayout=0;
					HRESULT hr=dwriteFactory->CreateTextLayout(
						_text.Buffer(),
						(int)_text.Length(),
						package->textFormat.Obj(),
						0,
						0,
						&rawTextLayout);
					if(!FAILED(hr))
					{
						textLayout=rawTextLayout;
						textLayout->SetMaxWidth(65536);
						textLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
					}
					graphicsElements.Add(TextRange(0, _text.Length()), 0);
					backgroundColors.Add(TextRange(0, _text.Length()), Color(0, 0, 0, 0));

					GetWindowsDirect2DResourceManager()->DestroyDirect2DTextFormat(defaultFont);
				}

				~WindowsDirect2DParagraph()
				{
					CloseCaret();
					FOREACH(Color, color, usedColors)
					{
						renderTarget->DestroyDirect2DBrush(color);
					}
				}

				IGuiGraphicsLayoutProvider* GetProvider()override
				{
					return provider;
				}

				IGuiGraphicsRenderTarget* GetRenderTarget()override
				{
					return renderTarget;
				}

				Point GetParagraphOffset()override
				{
					return paragraphOffset;
				}

				IGuiGraphicsParagraphCallback* GetParagraphCallback()override
				{
					return paragraphCallback;
				}

/***********************************************************************
WindowsDirect2DParagraph (Formatting)
***********************************************************************/

				bool GetWrapLine()override
				{
					return wrapLine;
				}

				void SetWrapLine(bool value)override
				{
					if(wrapLine!=value)
					{
						wrapLine=value;
						textLayout->SetWordWrapping(value?DWRITE_WORD_WRAPPING_WRAP:DWRITE_WORD_WRAPPING_NO_WRAP);
						formatDataAvailable=false;
					}
				}

				vint GetMaxWidth()override
				{
					return maxWidth;
				}

				void SetMaxWidth(vint value)override
				{
					if(maxWidth!=value)
					{
						maxWidth=value;
						textLayout->SetMaxWidth(value==-1?65536:(FLOAT)value);
						formatDataAvailable=false;
					}
				}

				Alignment GetParagraphAlignment()override
				{
					switch(textLayout->GetTextAlignment())
					{
					case DWRITE_TEXT_ALIGNMENT_LEADING:
						return Alignment::Left;
					case DWRITE_TEXT_ALIGNMENT_CENTER:
						return Alignment::Center;
					case DWRITE_TEXT_ALIGNMENT_TRAILING:
						return Alignment::Right;
					default:
						return Alignment::Left;
					}
				}

				void SetParagraphAlignment(Alignment value)override
				{
					formatDataAvailable=false;
					switch(value)
					{
					case Alignment::Left:
						textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
						break;
					case Alignment::Center:
						textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
						break;
					case Alignment::Right:
						textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
						break;
					}
				}

				bool SetFont(vint start, vint length, const WString& value)override
				{
					if(length==0) return true;
					formatDataAvailable=false;

					DWRITE_TEXT_RANGE range;
					range.startPosition=(int)start;
					range.length=(int)length;
					HRESULT hr=textLayout->SetFontFamilyName(value.Buffer(), range);
					return !FAILED(hr);
				}

				bool SetSize(vint start, vint length, vint value)override
				{
					if(length==0) return true;
					formatDataAvailable=false;

					DWRITE_TEXT_RANGE range;
					range.startPosition=(int)start;
					range.length=(int)length;
					HRESULT hr=textLayout->SetFontSize((FLOAT)value, range);
					return !FAILED(hr);
				}

				bool SetStyle(vint start, vint length, TextStyle value)override
				{
					if(length==0) return true;
					formatDataAvailable=false;

					DWRITE_TEXT_RANGE range;
					range.startPosition=(int)start;
					range.length=(int)length;
					HRESULT hr=S_OK;

					hr=textLayout->SetFontStyle(value&Italic?DWRITE_FONT_STYLE_ITALIC:DWRITE_FONT_STYLE_NORMAL, range);
					if(FAILED(hr)) return false;
					hr=textLayout->SetFontWeight(value&Bold?DWRITE_FONT_WEIGHT_BOLD:DWRITE_FONT_WEIGHT_NORMAL, range);
					if(FAILED(hr)) return false;
					hr=textLayout->SetUnderline(value&Underline?TRUE:FALSE, range);
					if(FAILED(hr)) return false;
					hr=textLayout->SetStrikethrough(value&Strikeline?TRUE:FALSE, range);
					if(FAILED(hr)) return false;

					return true;
				}

				bool SetColor(vint start, vint length, Color value)override
				{
					if(length==0) return true;
					formatDataAvailable=false;

					ID2D1SolidColorBrush* brush=renderTarget->CreateDirect2DBrush(value);
					usedColors.Add(value);

					DWRITE_TEXT_RANGE range;
					range.startPosition=(int)start;
					range.length=(int)length;
					HRESULT hr=textLayout->SetDrawingEffect(brush, range);
					return !FAILED(hr);
				}

				bool SetBackgroundColor(vint start, vint length, Color value)override
				{
					SetMap(backgroundColors, start, length, value);
					return true;
				}

				bool SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)override
				{
					if(inlineElements.Keys().Contains(properties.backgroundImage.Obj()))
					{
						return false;
					}
					for(vint i=0;i<inlineElements.Count();i++)
					{
						ComPtr<WindowsDirect2DElementInlineObject> inlineObject=inlineElements.Values().Get(i);
						if(start<inlineObject->GetStart()+inlineObject->GetLength() && inlineObject->GetStart()<start+length)
						{
							return false;
						}
					}
					formatDataAvailable=false;

					ComPtr<WindowsDirect2DElementInlineObject> inlineObject=new WindowsDirect2DElementInlineObject(properties, this, start, length);
					DWRITE_TEXT_RANGE range;
					range.startPosition=(int)start;
					range.length=(int)length;
					HRESULT hr=textLayout->SetInlineObject(inlineObject.Obj(), range);
					if(!FAILED(hr))
					{
						if (properties.backgroundImage)
						{
							IGuiGraphicsRenderer* renderer=properties.backgroundImage->GetRenderer();
							if(renderer)
							{
								renderer->SetRenderTarget(renderTarget);
							}
							inlineElements.Add(properties.backgroundImage.Obj(), inlineObject);
						}
						SetMap(graphicsElements, start, length, properties.backgroundImage.Obj());
						return true;
					}
					else
					{
						return false;
					}
				}

				bool ResetInlineObject(vint start, vint length)override
				{
					IGuiGraphicsElement* element=0;
					if(GetMap(graphicsElements, start, element) && element)
					{
						ComPtr<WindowsDirect2DElementInlineObject> inlineObject=inlineElements[element];
						DWRITE_TEXT_RANGE range;
						range.startPosition=(int)inlineObject->GetStart();
						range.length=(int)inlineObject->GetLength();
						HRESULT hr=textLayout->SetInlineObject(NULL, range);
						if(!FAILED(hr))
						{
							formatDataAvailable=false;
							inlineElements.Remove(element);
							SetMap(graphicsElements, inlineObject->GetStart(), inlineObject->GetLength(), (IGuiGraphicsElement*)0);
							return true;
						}
						else
						{
							return false;
						}
					}
					return false;
				}

				vint GetHeight()override
				{
					DWRITE_TEXT_METRICS metrics;
					textLayout->GetMetrics(&metrics);
					return (vint)metrics.height;
				}

/***********************************************************************
WindowsDirect2DParagraph (IRenderingCallback)
***********************************************************************/

				Color GetBackgroundColor(vint textPosition)override
				{
					Color color;
					if(GetMap(backgroundColors, textPosition, color))
					{
						return color;
					}
					else
					{
						return Color(0, 0, 0, 0);
					}
				}

				IWindowsDirect2DRenderTarget* GetDirect2DRenderTarget()override
				{
					return renderTarget;
				}

/***********************************************************************
WindowsDirect2DParagraph (Rendering)
***********************************************************************/

				bool OpenCaret(vint _caret, Color _color, bool _frontSide)override
				{
					if(!IsValidCaret(_caret)) return false;
					if(caret!=-1) CloseCaret();
					caret=_caret;
					caretColor=_color;
					caretFrontSide=_frontSide;
					caretBrush=renderTarget->CreateDirect2DBrush(caretColor);
					return true;
				}

				bool CloseCaret()override
				{
					if(caret==-1) return false;
					caret=-1;
					renderTarget->DestroyDirect2DBrush(caretColor);
					caretBrush=0;
					return true;
				}

				void Render(Rect bounds)override
				{
					paragraphOffset = bounds.LeftTop();
					PrepareFormatData();
					for(vint i=0;i<backgroundColors.Count();i++)
					{
						TextRange key=backgroundColors.Keys()[i];
						Color color=backgroundColors.Values()[i];
						if(color.a>0)
						{
							ID2D1SolidColorBrush* brush=renderTarget->CreateDirect2DBrush(color);

							vint start=key.start;
							if(start<0)
							{
								start=0;
							}

							while(start<charHitTestMap.Count() && start<key.end)
							{
								vint index=charHitTestMap[start];
								DWRITE_HIT_TEST_METRICS& hitTest=hitTestMetrics[index];

								FLOAT x1=hitTest.left+(FLOAT)bounds.x1;
								FLOAT y1=hitTest.top+(FLOAT)bounds.y1;
								FLOAT x2=x1+hitTest.width;
								FLOAT y2=y1+hitTest.height;

								x1-=0.5f;
								y1-=0.0f;
								x2+=0.5f;
								y2+=0.5f;

								renderTarget->GetDirect2DRenderTarget()->FillRectangle(
									D2D1::RectF(x1, y1, x2, y2),
									brush
									);
										
								start=hitTest.textPosition+hitTest.length;
							}

							renderTarget->DestroyDirect2DBrush(color);
						}
					}

					renderTarget->GetDirect2DRenderTarget()->DrawTextLayout(
						D2D1::Point2F((FLOAT)bounds.Left(), (FLOAT)bounds.Top()),
						textLayout.Obj(),
						defaultTextColor,
						D2D1_DRAW_TEXT_OPTIONS_NO_SNAP);

					if(caret!=-1)
					{
						Rect caretBounds=GetCaretBounds(caret, caretFrontSide);
						vint x=caretBounds.x1+bounds.x1;
						vint y1=caretBounds.y1+bounds.y1;
						vint y2=y1+caretBounds.Height();

						renderTarget->GetDirect2DRenderTarget()->DrawLine(
							D2D1::Point2F((FLOAT)x-0.5f, (FLOAT)y1+0.5f),
							D2D1::Point2F((FLOAT)x-0.5f, (FLOAT)y2+0.5f),
							caretBrush
							);
						renderTarget->GetDirect2DRenderTarget()->DrawLine(
							D2D1::Point2F((FLOAT)x+0.5f, (FLOAT)y1+0.5f),
							D2D1::Point2F((FLOAT)x+0.5f, (FLOAT)y2+0.5f),
							caretBrush
							);
					}
				}

/***********************************************************************
WindowsDirect2DParagraph (Caret Helper)
***********************************************************************/

				void GetLineIndexFromTextPos(vint textPos, vint& frontLineIndex, vint& backLineIndex)
				{
					frontLineIndex=-1;
					backLineIndex=-1;
					vint start=0;
					vint end=lineMetrics.Count()-1;
					while(start<=end)
					{
						vint middle=(start+end)/2;
						DWRITE_LINE_METRICS& metrics=lineMetrics[middle];
						vint lineStart=lineStarts[middle];
						vint lineEnd=lineStart+metrics.length-metrics.newlineLength;

						if(textPos<lineStart)
						{
							end=middle-1;
						}
						else if(textPos>lineEnd)
						{
							start=middle+1;
						}
						else if(textPos==lineStart && middle!=0)
						{
							DWRITE_LINE_METRICS& anotherLine=lineMetrics[middle-1];
							frontLineIndex=anotherLine.newlineLength==0?middle-1:middle;
							backLineIndex=middle;
							return;
						}
						else if(textPos==lineEnd && middle!=lineMetrics.Count()-1)
						{
							frontLineIndex=middle;
							backLineIndex=metrics.newlineLength==0?middle+1:middle;
							return;
						}
						else
						{
							frontLineIndex=middle;
							backLineIndex=middle;
							return;
						}
					}
				}

				Pair<FLOAT, FLOAT> GetLineYRange(vint lineIndex)
				{
					DWRITE_LINE_METRICS& line=lineMetrics[lineIndex];
					FLOAT top=lineTops[lineIndex];
					return Pair<FLOAT, FLOAT>(top, top+line.height);
				}

				vint GetLineIndexFromY(vint y)
				{
					if(paragraphText.Length()==0) return 0;
					FLOAT minY=0;
					FLOAT maxY=0;
					{
						minY=hitTestMetrics[0].top;
						DWRITE_HIT_TEST_METRICS& hitTest=hitTestMetrics[hitTestMetrics.Count()-1];
						maxY=hitTest.top+hitTest.height;
					}

					if(y<minY)
					{
						return 0;
					}
					else if(y>=maxY)
					{
						return lineMetrics.Count()-1;
					}

					vint start=0;
					vint end=lineMetrics.Count()-1;
					while(start<=end)
					{
						vint middle=(start+end)/2;
						Pair<FLOAT, FLOAT> yRange=GetLineYRange(middle);
						minY=yRange.key;
						maxY=yRange.value;

						if(y<minY)
						{
							end=middle-1;
						}
						else if(y>=maxY)
						{
							start=middle+1;
						}
						else
						{
							return middle;
						}
					}
					return -1;
				}

				vint GetCaretFromXWithLine(vint x, vint lineIndex)
				{
					DWRITE_LINE_METRICS& line=lineMetrics[lineIndex];
					vint lineStart=lineStarts[lineIndex];
					vint lineEnd=lineStart+line.length-line.newlineLength;

					FLOAT minLineX=0;
					FLOAT maxLineX=0;

					for(vint i=lineStart;i<lineEnd;)
					{
						vint index=charHitTestMap[i];
						DWRITE_HIT_TEST_METRICS& hitTest=hitTestMetrics[index];
						FLOAT minX=hitTest.left;
						FLOAT maxX=minX+hitTest.width;

						if(minLineX>minX) minLineX=minX;
						if(maxLineX<maxX) maxLineX=maxX;

						if(minX<=x && x<maxX)
						{
							DWRITE_CLUSTER_METRICS& cluster=clusterMetrics[index];
							FLOAT d1=x-minX;
							FLOAT d2=maxX-x;
							if(d1<=d2)
							{
								return cluster.isRightToLeft?i+hitTest.length:i;
							}
							else
							{
								return cluster.isRightToLeft?i:i+hitTest.length;
							}
						}
						i+=hitTest.length;
					}
					
					if(x<minLineX) return lineStart;
					if(x>=maxLineX) return lineEnd;
					return lineStart;
				}

/***********************************************************************
WindowsDirect2DParagraph (Caret)
***********************************************************************/

				vint GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)override
				{
					PrepareFormatData();
					if(position==CaretFirst) return 0;
					if(position==CaretLast) return paragraphText.Length();
					if(!IsValidCaret(comparingCaret)) return -1;

					vint frontLineIndex=-1;
					vint backLineIndex=-1;
					GetLineIndexFromTextPos(comparingCaret, frontLineIndex, backLineIndex);
					vint lineIndex=preferFrontSide?frontLineIndex:backLineIndex;
					DWRITE_LINE_METRICS& line=lineMetrics[lineIndex];
					vint lineStart=lineStarts[lineIndex];
					vint lineEnd=lineStart+line.length-line.newlineLength;

					switch(position)
					{
					case CaretLineFirst:
						return lineStarts[lineIndex];
					case CaretLineLast:
						return lineStarts[lineIndex]+line.length-line.newlineLength;
					case CaretMoveLeft:
						{
							if(comparingCaret==0)
							{
								return 0;
							}
							else if(comparingCaret==lineStart)
							{
								vint offset=lineMetrics[lineIndex-1].newlineLength;
								if(offset>0)
								{
									return lineStart-offset;
								}
							}
							return hitTestMetrics[charHitTestMap[comparingCaret-1]].textPosition;
						}
					case CaretMoveRight:
						{
							if(comparingCaret==paragraphText.Length())
							{
								return paragraphText.Length();
							}
							else if(comparingCaret==lineEnd && line.newlineLength!=0)
							{
								return lineEnd+line.newlineLength;
							}
							else
							{
								vint index=charHitTestMap[comparingCaret];
								if(index==hitTestMetrics.Count()-1) return paragraphText.Length();
								return hitTestMetrics[index+1].textPosition;
							}
						}
					case CaretMoveUp:
						{
							if(lineIndex==0)
							{
								return comparingCaret;
							}
							else
							{
								Rect bounds=GetCaretBounds(comparingCaret, preferFrontSide);
								preferFrontSide=true;
								return GetCaretFromXWithLine(bounds.x1, lineIndex-1);
							}
						}
					case CaretMoveDown:
						{
							if(lineIndex==lineMetrics.Count()-1)
							{
								return comparingCaret;
							}
							else
							{
								Rect bounds=GetCaretBounds(comparingCaret, preferFrontSide);
								preferFrontSide=false;
								return GetCaretFromXWithLine(bounds.x1, lineIndex+1);
							}
						}
					}
					return -1;
				}

				Rect GetCaretBounds(vint caret, bool frontSide)override
				{
					PrepareFormatData();
					if(!IsValidCaret(caret)) return Rect();
					if(paragraphText.Length()==0) return Rect(Point(0, 0), Size(0, GetHeight()));

					vint frontLineIndex=-1;
					vint backLineIndex=-1;
					GetLineIndexFromTextPos(caret, frontLineIndex, backLineIndex);
					vint lineIndex=frontSide?frontLineIndex:backLineIndex;

					Pair<FLOAT, FLOAT> lineYRange=GetLineYRange(lineIndex);
					DWRITE_LINE_METRICS& line=lineMetrics[lineIndex];
					if(line.length-line.newlineLength==0)
					{
						return Rect(0, (vint)lineYRange.key, 0, (vint)lineYRange.value);
					}
					else
					{
						vint lineStart=lineStarts[lineIndex];
						vint lineEnd=lineStart+line.length-line.newlineLength;
						if(caret==lineStart)
						{
							frontSide=false;
						}
						else if(caret==lineEnd)
						{
							frontSide=true;
						}
						if(frontSide)
						{
							caret--;
						}

						vint index=charHitTestMap[caret];
						DWRITE_HIT_TEST_METRICS& hitTest=hitTestMetrics[index];
						DWRITE_CLUSTER_METRICS& cluster=clusterMetrics[index];
						if(cluster.isRightToLeft)
						{
							frontSide=!frontSide;
						}

						if(frontSide)
						{
							return Rect(
								Point((vint)(hitTest.left+hitTest.width), (vint)hitTest.top),
								Size(0, (vint)hitTest.height)
								);
						}
						else
						{
							return Rect(
								Point((vint)hitTest.left, (vint)hitTest.top),
								Size(0, (vint)hitTest.height)
								);
						}
					}
				}

				vint GetCaretFromPoint(Point point)override
				{
					PrepareFormatData();
					vint lineIndex=GetLineIndexFromY(point.y);
					vint caret=GetCaretFromXWithLine(point.x, lineIndex);
					return caret;
				}

				Nullable<InlineObjectProperties> GetInlineObjectFromPoint(Point point, vint& start, vint& length)override
				{
					DWRITE_HIT_TEST_METRICS metrics={0};
					BOOL trailingHit=FALSE;
					BOOL inside=FALSE;
					start=-1;
					length=0;
					HRESULT hr=textLayout->HitTestPoint((FLOAT)point.x, (FLOAT)point.y, &trailingHit, &inside, &metrics);
					if(hr==S_OK)
					{
						IGuiGraphicsElement* element=0;
						if(GetMap(graphicsElements, metrics.textPosition, element) && element)
						{
							ComPtr<WindowsDirect2DElementInlineObject> inlineObject=inlineElements[element];
							start=inlineObject->GetStart();
							length=inlineObject->GetLength();
							return inlineObject->GetProperties();
						}
					}
					return Nullable<InlineObjectProperties>();
				}

				vint GetNearestCaretFromTextPos(vint textPos, bool frontSide)override
				{
					PrepareFormatData();
					if(!IsValidTextPos(textPos)) return -1;
					if(textPos==0 || textPos==paragraphText.Length()) return textPos;
				
					vint index=charHitTestMap[textPos];
					DWRITE_HIT_TEST_METRICS& hitTest=hitTestMetrics[index];
					if(hitTest.textPosition==textPos)
					{
						return textPos;
					}
					else if(frontSide)
					{
						return hitTest.textPosition;
					}
					else
					{
						return hitTest.textPosition+hitTest.length;
					}
				}

				bool IsValidCaret(vint caret)override
				{
					PrepareFormatData();
					if(!IsValidTextPos(caret)) return false;
					if(caret==0 || caret==paragraphText.Length()) return true;
					if(hitTestMetrics[charHitTestMap[caret]].textPosition==caret) return true;

					vint frontLineIndex=-1;
					vint backLineIndex=-1;
					GetLineIndexFromTextPos(caret, frontLineIndex, backLineIndex);
					if(frontLineIndex==-1 && backLineIndex==-1) return false;
					return false;
				}

				bool IsValidTextPos(vint textPos)
				{
					return 0<=textPos && textPos<=paragraphText.Length();
				}
			};

/***********************************************************************
WindowsDirect2DLayoutProvider
***********************************************************************/

			Ptr<IGuiGraphicsParagraph> WindowsDirect2DLayoutProvider::CreateParagraph(const WString& text, IGuiGraphicsRenderTarget* renderTarget, elements::IGuiGraphicsParagraphCallback* callback)
			{
				return new WindowsDirect2DParagraph(this, text, renderTarget, callback);
			}
		}
	}
}