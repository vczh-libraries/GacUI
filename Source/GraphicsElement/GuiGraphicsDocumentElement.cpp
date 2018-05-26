#include "GuiGraphicsDocumentElement.h"

namespace vl
{
	using namespace collections;

	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
SetPropertiesVisitor
***********************************************************************/

			namespace visitors
			{
				class SetPropertiesVisitor : public Object, public DocumentRun::IVisitor
				{
					typedef GuiDocumentElement::GuiDocumentElementRenderer	Renderer;
					typedef DocumentModel::ResolvedStyle					ResolvedStyle;
				public:
					vint							start;
					vint							length;
					vint							selectionBegin;
					vint							selectionEnd;
					List<ResolvedStyle>				styles;

					DocumentModel*					model;
					Renderer*						renderer;
					Ptr<Renderer::ParagraphCache>	cache;
					IGuiGraphicsParagraph*			paragraph;

					SetPropertiesVisitor(DocumentModel* _model, Renderer* _renderer, Ptr<Renderer::ParagraphCache> _cache, vint _selectionBegin, vint _selectionEnd)
						:start(0)
						,length(0)
						,model(_model)
						,renderer(_renderer)
						,cache(_cache)
						,paragraph(_cache->graphicsParagraph.Obj())
						,selectionBegin(_selectionBegin)
						,selectionEnd(_selectionEnd)
					{
						ResolvedStyle style;
						style=model->GetStyle(DocumentModel::DefaultStyleName, style);
						styles.Add(style);
					}

					void VisitContainer(DocumentContainerRun* run)
					{
						FOREACH(Ptr<DocumentRun>, subRun, run->runs)
						{
							subRun->Accept(this);
						}
					}

					void ApplyStyle(vint start, vint length, const ResolvedStyle& style)
					{
						paragraph->SetFont(start, length, style.style.fontFamily);
						paragraph->SetSize(start, length, style.style.size);
						paragraph->SetStyle(start, length, 
							(IGuiGraphicsParagraph::TextStyle)
							( (style.style.bold?IGuiGraphicsParagraph::Bold:0)
							| (style.style.italic?IGuiGraphicsParagraph::Italic:0)
							| (style.style.underline?IGuiGraphicsParagraph::Underline:0)
							| (style.style.strikeline?IGuiGraphicsParagraph::Strikeline:0)
							));
					}

					void ApplyColor(vint start, vint length, const ResolvedStyle& style)
					{
						paragraph->SetColor(start, length, style.color);
						paragraph->SetBackgroundColor(start, length, style.backgroundColor);
					}

					void Visit(DocumentTextRun* run)override
					{
						length=run->GetRepresentationText().Length();
						if(length>0)
						{
							ResolvedStyle style=styles[styles.Count()-1];
							ApplyStyle(start, length, style);
							ApplyColor(start, length, style);

							vint styleStart=start;
							vint styleEnd=styleStart+length;
							if(styleStart<selectionEnd && selectionBegin<styleEnd)
							{
								vint s2=styleStart>selectionBegin?styleStart:selectionBegin;
								vint s3=selectionEnd<styleEnd?selectionEnd:styleEnd;

								if(s2<s3)
								{
									ResolvedStyle selectionStyle=model->GetStyle(DocumentModel::SelectionStyleName, style);
									ApplyColor(s2, s3-s2, selectionStyle);
								}
							}
						}
						start+=length;
					}

					void Visit(DocumentStylePropertiesRun* run)override
					{
						ResolvedStyle style=styles[styles.Count()-1];
						style=model->GetStyle(run->style, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count()-1);
					}

					void Visit(DocumentStyleApplicationRun* run)override
					{
						ResolvedStyle style=styles[styles.Count()-1];
						style=model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count()-1);
					}

					void Visit(DocumentHyperlinkRun* run)override
					{
						ResolvedStyle style=styles[styles.Count()-1];
						style=model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count()-1);
					}

					void Visit(DocumentImageRun* run)override
					{
						length=run->GetRepresentationText().Length();

						Ptr<GuiImageFrameElement> element=GuiImageFrameElement::Create();
						element->SetImage(run->image, run->frameIndex);
						element->SetStretch(true);

						IGuiGraphicsParagraph::InlineObjectProperties properties;
						properties.size=run->size;
						properties.baseline=run->baseline;
						properties.breakCondition=IGuiGraphicsParagraph::Alone;
						properties.backgroundImage = element;

						paragraph->SetInlineObject(start, length, properties);

						if(start<selectionEnd && selectionBegin<start+length)
						{
							ResolvedStyle style=styles[styles.Count()-1];
							ResolvedStyle selectionStyle=model->GetStyle(DocumentModel::SelectionStyleName, style);
							ApplyColor(start, length, selectionStyle);
						}
						start+=length;
					}

					void Visit(DocumentEmbeddedObjectRun* run)override
					{
						length=run->GetRepresentationText().Length();

						IGuiGraphicsParagraph::InlineObjectProperties properties;
						properties.breakCondition=IGuiGraphicsParagraph::Alone;

						if (run->name != L"")
						{
							vint index = renderer->nameCallbackIdMap.Keys().IndexOf(run->name);
							if (index != -1)
							{
								auto id = renderer->nameCallbackIdMap.Values()[index];
								index = cache->embeddedObjects.Keys().IndexOf(id);
								if (index != -1)
								{
									auto eo = cache->embeddedObjects.Values()[index];
									if (eo->start == start)
									{
										properties.size = eo->size;
										properties.callbackId = id;
									}
								}
							}
							else
							{
								auto eo = MakePtr<Renderer::EmbeddedObject>();
								eo->name = run->name;
								eo->size = Size(0, 0);
								eo->start = start;

								vint id = -1;
								vint count = renderer->freeCallbackIds.Count();
								if (count > 0)
								{
									id = renderer->freeCallbackIds[count - 1];
									renderer->freeCallbackIds.RemoveAt(count - 1);
								}
								else
								{
									id = renderer->usedCallbackIds++;
								}

								renderer->nameCallbackIdMap.Add(eo->name, id);
								cache->embeddedObjects.Add(id, eo);
								properties.callbackId = id;
							}
						}

						paragraph->SetInlineObject(start, length, properties);

						if(start<selectionEnd && selectionBegin<start+length)
						{
							ResolvedStyle style=styles[styles.Count()-1];
							ResolvedStyle selectionStyle=model->GetStyle(DocumentModel::SelectionStyleName, style);
							ApplyColor(start, length, selectionStyle);
						}
						start+=length;
					}

					void Visit(DocumentParagraphRun* run)override
					{
						VisitContainer(run);
					}

					static vint SetProperty(DocumentModel* model, Renderer* renderer, Ptr<Renderer::ParagraphCache> cache, Ptr<DocumentParagraphRun> run, vint selectionBegin, vint selectionEnd)
					{
						SetPropertiesVisitor visitor(model, renderer, cache, selectionBegin, selectionEnd);
						run->Accept(&visitor);
						return visitor.length;
					}
				};
			}
			using namespace visitors;

/***********************************************************************
GuiDocumentElement::GuiDocumentElementRenderer
***********************************************************************/

			Size GuiDocumentElement::GuiDocumentElementRenderer::OnRenderInlineObject(vint callbackId, Rect location)
			{
				if (element->callback)
				{
					auto cache = paragraphCaches[renderingParagraph];
					auto relativeLocation = Rect(Point(location.x1 + renderingParagraphOffset.x, location.y1 + renderingParagraphOffset.y), location.GetSize());
					auto eo = cache->embeddedObjects[callbackId];
					auto size = element->callback->OnRenderEmbeddedObject(eo->name, relativeLocation);
					eo->resized = eo->size != size;
					eo->size = size;
					return eo->size;
				}
				else
				{
					return Size();
				}
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::InitializeInternal()
			{
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::FinalizeInternal()
			{
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::RenderTargetChangedInternal(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget)
			{
				for(vint i=0;i<paragraphCaches.Count();i++)
				{
					ParagraphCache* cache=paragraphCaches[i].Obj();
					if(cache)
					{
						cache->graphicsParagraph=0;
					}
				}
			}

			Ptr<GuiDocumentElement::GuiDocumentElementRenderer::ParagraphCache> GuiDocumentElement::GuiDocumentElementRenderer::EnsureAndGetCache(vint paragraphIndex, bool createParagraph)
			{
				if(paragraphIndex<0 || paragraphIndex>=paragraphCaches.Count()) return 0;
				Ptr<DocumentParagraphRun> paragraph=element->document->paragraphs[paragraphIndex];
				Ptr<ParagraphCache> cache=paragraphCaches[paragraphIndex];
				if(!cache)
				{
					cache=new ParagraphCache;
					cache->fullText=paragraph->GetText(false);
					paragraphCaches[paragraphIndex]=cache;
				}

				if(createParagraph)
				{
					if(!cache->graphicsParagraph)
					{
						cache->graphicsParagraph=layoutProvider->CreateParagraph(cache->fullText, renderTarget, this);
						cache->graphicsParagraph->SetParagraphAlignment(paragraph->alignment ? paragraph->alignment.Value() : Alignment::Left);
						SetPropertiesVisitor::SetProperty(element->document.Obj(), this, cache, paragraph, cache->selectionBegin, cache->selectionEnd);
					}
					if(cache->graphicsParagraph->GetMaxWidth()!=lastMaxWidth)
					{
						cache->graphicsParagraph->SetMaxWidth(lastMaxWidth);
					}

					vint paragraphHeight=paragraphHeights[paragraphIndex];
					vint height=cache->graphicsParagraph->GetHeight();
					if(paragraphHeight!=height)
					{
						cachedTotalHeight+=height-paragraphHeight;
						paragraphHeight=height;
						paragraphHeights[paragraphIndex]=paragraphHeight;
						minSize=Size(0, cachedTotalHeight);
					}
				}

				return cache;
			}

			bool GuiDocumentElement::GuiDocumentElementRenderer::GetParagraphIndexFromPoint(Point point, vint& top, vint& index)
			{
				vint y=0;
				for(vint i=0;i<paragraphHeights.Count();i++)
				{
					vint paragraphHeight=paragraphHeights[i];
					vint nextY=y+paragraphHeight+paragraphDistance;
					top=y;
					index=i;

					if(nextY<=point.y)
					{
						y=nextY;
						continue;
					}
					else
					{
						break;
					}
				}
				return true;
			}

			GuiDocumentElement::GuiDocumentElementRenderer::GuiDocumentElementRenderer()
				:paragraphDistance(0)
				,lastMaxWidth(-1)
				,cachedTotalHeight(0)
				,layoutProvider(GetGuiGraphicsResourceManager()->GetLayoutProvider())
				,lastCaret(-1, -1)
				,lastCaretFrontSide(false)
			{
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::Render(Rect bounds)
			{
				if (element->callback)
				{
					element->callback->OnStartRender();
				}
				renderTarget->PushClipper(bounds);
				if(!renderTarget->IsClipperCoverWholeTarget())
				{
					vint maxWidth=bounds.Width();
					Rect clipper=renderTarget->GetClipper();
					vint cx=bounds.Left();
					vint cy=bounds.Top();
					vint y1=clipper.Top()-bounds.Top();
					vint y2=y1+clipper.Height();
					vint y=0;

					lastMaxWidth=maxWidth;

					for(vint i=0;i<paragraphHeights.Count();i++)
					{
						vint paragraphHeight=paragraphHeights[i];
						if(y+paragraphHeight<=y1)
						{
							y+=paragraphHeight+paragraphDistance;
							continue;
						}
						else if(y>=y2)
						{
							break;
						}
						else
						{
							Ptr<DocumentParagraphRun> paragraph=element->document->paragraphs[i];
							Ptr<ParagraphCache> cache=paragraphCaches[i];
							bool created=cache && cache->graphicsParagraph;
							cache=EnsureAndGetCache(i, true);
							if(!created && i==lastCaret.row && element->caretVisible)
							{
								cache->graphicsParagraph->OpenCaret(lastCaret.column, lastCaretColor, lastCaretFrontSide);
							}

							paragraphHeight=cache->graphicsParagraph->GetHeight();

							renderingParagraph = i;
							renderingParagraphOffset = Point(cx - bounds.x1, cy + y - bounds.y1);
							cache->graphicsParagraph->Render(Rect(Point(cx, cy+y), Size(maxWidth, paragraphHeight)));
							renderingParagraph = -1;

							bool resized = false;
							for (vint j = 0; j < cache->embeddedObjects.Count(); j++)
							{
								auto eo = cache->embeddedObjects.Values()[j];
								if (eo->resized)
								{
									eo->resized = false;
									resized = true;
								}
							}

							if (resized)
							{
								cache->graphicsParagraph = 0;
							}
						}

						y+=paragraphHeight+paragraphDistance;
					}
				}
				renderTarget->PopClipper();
				if (element->callback)
				{
					element->callback->OnFinishRender();
				}
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::OnElementStateChanged()
			{
				if (element->document && element->document->paragraphs.Count() > 0)
				{
					vint defaultSize = GetCurrentController()->ResourceService()->GetDefaultFont().size;
					paragraphDistance = defaultSize;
					vint defaultHeight = defaultSize;

					paragraphCaches.Resize(element->document->paragraphs.Count());
					paragraphHeights.Resize(element->document->paragraphs.Count());
					
					for (vint i = 0; i < paragraphCaches.Count(); i++)
					{
						paragraphCaches[i] = 0;
					}
					for (vint i = 0; i < paragraphHeights.Count(); i++)
					{
						paragraphHeights[i] = defaultHeight;
					}

					cachedTotalHeight = paragraphHeights.Count() * (defaultHeight + paragraphDistance);
					if (paragraphHeights.Count()>0)
					{
						cachedTotalHeight -= paragraphDistance;
					}
					minSize = Size(0, cachedTotalHeight);
				}
				else
				{
					paragraphCaches.Resize(0);
					paragraphHeights.Resize(0);
					cachedTotalHeight = 0;
					minSize = Size(0, 0);
				}

				nameCallbackIdMap.Clear();
				freeCallbackIds.Clear();
				usedCallbackIds = 0;
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText)
			{
				if (0 <= index && index < paragraphCaches.Count() && 0 <= oldCount && index + oldCount <= paragraphCaches.Count() && 0 <= newCount)
				{
					vint paragraphCount = element->document->paragraphs.Count();
					CHECK_ERROR(updatedText || oldCount == newCount, L"GuiDocumentlement::GuiDocumentElementRenderer::NotifyParagraphUpdated(vint, vint, vint, bool)#Illegal values of oldCount and newCount.");
					CHECK_ERROR(paragraphCount - paragraphCaches.Count() == newCount - oldCount, L"GuiDocumentElement::GuiDocumentElementRenderer::NotifyParagraphUpdated(vint, vint, vint, bool)#Illegal values of oldCount and newCount.");

					ParagraphCacheArray oldCaches;
					CopyFrom(oldCaches, paragraphCaches);
					paragraphCaches.Resize(paragraphCount);

					ParagraphHeightArray oldHeights;
					CopyFrom(oldHeights, paragraphHeights);
					paragraphHeights.Resize(paragraphCount);

					vint defaultHeight = GetCurrentController()->ResourceService()->GetDefaultFont().size;
					cachedTotalHeight = 0;

					for (vint i = 0; i < paragraphCount; i++)
					{
						if (i < index)
						{
							paragraphCaches[i] = oldCaches[i];
							paragraphHeights[i] = oldHeights[i];
						}
						else if (i < index + newCount)
						{
							paragraphCaches[i] = 0;
							paragraphHeights[i] = defaultHeight;
							if (!updatedText && i < index + oldCount)
							{
								auto cache = oldCaches[i];
								if(cache)
								{
									cache->graphicsParagraph = 0;
								}
								paragraphCaches[i] = cache;
								paragraphHeights[i] = oldHeights[i];
							}
						}
						else
						{
							paragraphCaches[i] = oldCaches[i - (newCount - oldCount)];
							paragraphHeights[i] = oldHeights[i - (newCount - oldCount)];
						}
						cachedTotalHeight += paragraphHeights[i] + paragraphDistance;
					}
					if (paragraphCount > 0)
					{
						cachedTotalHeight -= paragraphDistance;
					}

					if (updatedText)
					{
						vint count = oldCount < newCount ? oldCount : newCount;
						for (vint i = 0; i < count; i++)
						{
							if (auto cache = oldCaches[index + i])
							{
								for (vint j = 0; j < cache->embeddedObjects.Count(); j++)
								{
									auto id = cache->embeddedObjects.Keys()[j];
									auto name = cache->embeddedObjects.Values()[j]->name;
									nameCallbackIdMap.Remove(name);
									freeCallbackIds.Add(id);
								}
							}
						}
					}
				}
			}

			Ptr<DocumentHyperlinkRun::Package> GuiDocumentElement::GuiDocumentElementRenderer::GetHyperlinkFromPoint(Point point)
			{
				vint top=0;
				vint index=-1;
				if(GetParagraphIndexFromPoint(point, top, index))
				{
					Ptr<ParagraphCache> cache=EnsureAndGetCache(index, true);
					Point paragraphPoint(point.x, point.y-top);

					vint start=-1;
					vint length=0;
					if(cache->graphicsParagraph->GetInlineObjectFromPoint(paragraphPoint, start, length))
					{
						return element->document->GetHyperlink(index, start, start+length);
					}

					vint caret=cache->graphicsParagraph->GetCaretFromPoint(paragraphPoint);
					return element->document->GetHyperlink(index, caret, caret);
				}
				return 0;
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::OpenCaret(TextPos caret, Color color, bool frontSide)
			{
				CloseCaret(caret);
				lastCaret=caret;
				lastCaretColor=color;
				lastCaretFrontSide=frontSide;

				Ptr<ParagraphCache> cache=paragraphCaches[lastCaret.row];
				if(cache && cache->graphicsParagraph)
				{
					cache->graphicsParagraph->OpenCaret(lastCaret.column, lastCaretColor, lastCaretFrontSide);
				}
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::CloseCaret(TextPos caret)
			{
				if(lastCaret!=TextPos(-1, -1))
				{
					if(0<=lastCaret.row && lastCaret.row<paragraphCaches.Count())
					{
						Ptr<ParagraphCache> cache=paragraphCaches[lastCaret.row];
						if(cache && cache->graphicsParagraph)
						{
							cache->graphicsParagraph->CloseCaret();
						}
					}
				}
				lastCaret=caret;
			}

			void GuiDocumentElement::GuiDocumentElementRenderer::SetSelection(TextPos begin, TextPos end)
			{
				if(begin>end)
				{
					TextPos t=begin;
					begin=end;
					end=t;
				}
				if(begin==end)
				{
					begin=TextPos(-1, -1);
					end=TextPos(-1, -1);
				}

				for(vint i=0;i<paragraphCaches.Count();i++)
				{
					if(begin.row<=i && i<=end.row)
					{
						Ptr<ParagraphCache> cache=EnsureAndGetCache(i, false);
						vint newBegin=i==begin.row?begin.column:0;
						vint newEnd=i==end.row?end.column:cache->fullText.Length();

						if(cache->selectionBegin!=newBegin || cache->selectionEnd!=newEnd)
						{
							cache->selectionBegin=newBegin;
							cache->selectionEnd=newEnd;
							NotifyParagraphUpdated(i, 1, 1, false);
						}
					}
					else
					{
						Ptr<ParagraphCache> cache=paragraphCaches[i];
						if(cache)
						{
							if(cache->selectionBegin!=-1 || cache->selectionEnd!=-1)
							{
								cache->selectionBegin=-1;
								cache->selectionEnd=-1;
								NotifyParagraphUpdated(i, 1, 1, false);
							}
						}
					}
				}
			}

			TextPos GuiDocumentElement::GuiDocumentElementRenderer::CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide)
			{
				Ptr<ParagraphCache> cache=EnsureAndGetCache(comparingCaret.row, true);
				if(cache)
				{
					switch(position)
					{
					case IGuiGraphicsParagraph::CaretFirst:
						{
							preferFrontSide=false;
							vint caret=cache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretFirst, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretLast:
						{
							preferFrontSide=true;
							vint caret=cache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretLast, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretLineFirst:
						{
							preferFrontSide=false;
							vint caret=cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretLineFirst, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretLineLast:
						{
							preferFrontSide=true;
							vint caret=cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretLineLast, preferFrontSide);
							return TextPos(comparingCaret.row, caret);
						}
					case IGuiGraphicsParagraph::CaretMoveUp:
						{
							vint caret=cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveUp, preferFrontSide);
							if(caret==comparingCaret.column && comparingCaret.row>0)
							{
								Rect caretBounds=cache->graphicsParagraph->GetCaretBounds(comparingCaret.column, preferFrontSide);
								Ptr<ParagraphCache> anotherCache=EnsureAndGetCache(comparingCaret.row-1, true);
								vint height=anotherCache->graphicsParagraph->GetHeight();
								caret=anotherCache->graphicsParagraph->GetCaretFromPoint(Point(caretBounds.x1, height));
								return TextPos(comparingCaret.row-1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					case IGuiGraphicsParagraph::CaretMoveDown:
						{
							vint caret=cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveDown, preferFrontSide);
							if(caret==comparingCaret.column && comparingCaret.row<paragraphCaches.Count()-1)
							{
								Rect caretBounds=cache->graphicsParagraph->GetCaretBounds(comparingCaret.column, preferFrontSide);
								Ptr<ParagraphCache> anotherCache=EnsureAndGetCache(comparingCaret.row+1, true);
								caret=anotherCache->graphicsParagraph->GetCaretFromPoint(Point(caretBounds.x1, 0));
								return TextPos(comparingCaret.row+1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					case IGuiGraphicsParagraph::CaretMoveLeft:
						{
							preferFrontSide=false;
							vint caret=cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveLeft, preferFrontSide);
							if(caret==comparingCaret.column && comparingCaret.row>0)
							{
								Ptr<ParagraphCache> anotherCache=EnsureAndGetCache(comparingCaret.row-1, true);
								caret=anotherCache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretLast, preferFrontSide);
								return TextPos(comparingCaret.row-1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					case IGuiGraphicsParagraph::CaretMoveRight:
						{
							preferFrontSide=true;
							vint caret=cache->graphicsParagraph->GetCaret(comparingCaret.column, IGuiGraphicsParagraph::CaretMoveRight, preferFrontSide);
							if(caret==comparingCaret.column && comparingCaret.row<paragraphCaches.Count()-1)
							{
								Ptr<ParagraphCache> anotherCache=EnsureAndGetCache(comparingCaret.row+1, true);
								caret=anotherCache->graphicsParagraph->GetCaret(0, IGuiGraphicsParagraph::CaretFirst, preferFrontSide);
								return TextPos(comparingCaret.row+1, caret);
							}
							else
							{
								return TextPos(comparingCaret.row, caret);
							}
						}
					}
				}
				return comparingCaret;
			}

			TextPos GuiDocumentElement::GuiDocumentElementRenderer::CalculateCaretFromPoint(Point point)
			{
				vint top=0;
				vint index=-1;
				if(GetParagraphIndexFromPoint(point, top, index))
				{
					Ptr<ParagraphCache> cache=EnsureAndGetCache(index, true);
					Point paragraphPoint(point.x, point.y-top);
					vint caret=cache->graphicsParagraph->GetCaretFromPoint(paragraphPoint);
					return TextPos(index, caret);
				}
				return TextPos(-1, -1);
			}

			Rect GuiDocumentElement::GuiDocumentElementRenderer::GetCaretBounds(TextPos caret, bool frontSide)
			{
				Ptr<ParagraphCache> cache=EnsureAndGetCache(caret.row, true);
				if(cache)
				{
					Rect bounds=cache->graphicsParagraph->GetCaretBounds(caret.column, frontSide);
					if(bounds!=Rect())
					{
						vint y=0;
						for(vint i=0;i<caret.row;i++)
						{
							EnsureAndGetCache(i, true);
							y+=paragraphHeights[i]+paragraphDistance;
						}

						bounds.y1+=y;
						bounds.y2+=y;
						return bounds;
					}
				}
				return Rect();
			}

/***********************************************************************
GuiDocumentElement
***********************************************************************/

			void GuiDocumentElement::UpdateCaret()
			{
				auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>();
				if (elementRenderer)
				{
					elementRenderer->SetSelection(caretBegin, caretEnd);
					if (caretVisible)
					{
						elementRenderer->OpenCaret(caretEnd, caretColor, caretFrontSide);
					}
					else
					{
						elementRenderer->CloseCaret(caretEnd);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			GuiDocumentElement::GuiDocumentElement()
				:caretVisible(false)
				,caretFrontSide(false)
			{
			}

			GuiDocumentElement::ICallback* GuiDocumentElement::GetCallback()
			{
				return callback;
			}

			void GuiDocumentElement::SetCallback(ICallback* value)
			{
				callback = value;
			}

			Ptr<DocumentModel> GuiDocumentElement::GetDocument()
			{
				return document;
			}

			void GuiDocumentElement::SetDocument(Ptr<DocumentModel> value)
			{
				document=value;
				InvokeOnElementStateChanged();
				SetCaret(TextPos(), TextPos(), false);
			}

			TextPos GuiDocumentElement::GetCaretBegin()
			{
				return caretBegin;
			}

			TextPos GuiDocumentElement::GetCaretEnd()
			{
				return caretEnd;
			}

			bool GuiDocumentElement::IsCaretEndPreferFrontSide()
			{
				return caretFrontSide;
			}

			void GuiDocumentElement::SetCaret(TextPos begin, TextPos end, bool frontSide)
			{
				caretBegin=begin;
				caretEnd=end;
				if(caretBegin<caretEnd)
				{
					caretFrontSide=true;
				}
				else if(caretBegin>caretEnd)
				{
					caretFrontSide=false;
				}
				else
				{
					caretFrontSide=frontSide;
				}
				UpdateCaret();
			}

			bool GuiDocumentElement::GetCaretVisible()
			{
				return caretVisible;
			}

			void GuiDocumentElement::SetCaretVisible(bool value)
			{
				caretVisible=value;
				UpdateCaret();
			}

			Color GuiDocumentElement::GetCaretColor()
			{
				return caretColor;
			}

			void GuiDocumentElement::SetCaretColor(Color value)
			{
				caretColor=value;
				UpdateCaret();
			}

			TextPos GuiDocumentElement::CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					TextPos caret=elementRenderer->CalculateCaret(comparingCaret, position, preferFrontSide);
					return caret.column==-1?comparingCaret:caret;
				}
				else
				{
					return comparingCaret;
				}
			}

			TextPos GuiDocumentElement::CalculateCaretFromPoint(Point point)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					return elementRenderer->CalculateCaretFromPoint(point);
				}
				else
				{
					return TextPos(0, 0);
				}
			}

			Rect GuiDocumentElement::GetCaretBounds(TextPos caret, bool frontSide)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					return elementRenderer->GetCaretBounds(caret, frontSide);
				}
				else
				{
					return Rect();
				}
			}
			
			void GuiDocumentElement::NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					elementRenderer->NotifyParagraphUpdated(index, oldCount, newCount, updatedText);
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::EditRun(TextPos begin, TextPos end, Ptr<DocumentModel> model, bool copy)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					vint newRows = document->EditRun(begin, end, model, copy);
					if (newRows != -1)
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, newRows, true);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::EditText(TextPos begin, TextPos end, bool frontSide, const collections::Array<WString>& text)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					vint newRows = document->EditText(begin, end, frontSide, text);
					if (newRows != -1)
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, newRows, true);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					if (document->EditStyle(begin, end, style))
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::EditImage(TextPos begin, TextPos end, Ptr<GuiImageData> image)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					if (document->EditImage(begin, end, image))
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, 1, true);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::EditHyperlink(vint paragraphIndex, vint begin, vint end, const WString& reference, const WString& normalStyleName, const WString& activeStyleName)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						vint temp = begin;
						begin = end;
						end = temp;
					}

					if (document->EditHyperlink(paragraphIndex, begin, end, reference, normalStyleName, activeStyleName))
					{
						elementRenderer->NotifyParagraphUpdated(paragraphIndex, 1, 1, false);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::RemoveHyperlink(vint paragraphIndex, vint begin, vint end)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						vint temp = begin;
						begin = end;
						end = temp;
					}

					if (document->RemoveHyperlink(paragraphIndex, begin, end))
					{
						elementRenderer->NotifyParagraphUpdated(paragraphIndex, 1, 1, false);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::EditStyleName(TextPos begin, TextPos end, const WString& styleName)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					if (document->EditStyleName(begin, end, styleName))
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::RemoveStyleName(TextPos begin, TextPos end)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					if (document->RemoveStyleName(begin, end))
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			void GuiDocumentElement::RenameStyle(const WString& oldStyleName, const WString& newStyleName)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					document->RenameStyle(oldStyleName, newStyleName);
				}
			}

			void GuiDocumentElement::ClearStyle(TextPos begin, TextPos end)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					if (document->ClearStyle(begin, end))
					{
						elementRenderer->NotifyParagraphUpdated(begin.row, end.row - begin.row + 1, end.row - begin.row + 1, false);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			Ptr<DocumentStyleProperties> GuiDocumentElement::SummarizeStyle(TextPos begin, TextPos end)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					return document->SummarizeStyle(begin, end);
				}
				return nullptr;
			}

			Nullable<WString> GuiDocumentElement::SummarizeStyleName(TextPos begin, TextPos end)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					return document->SummarizeStyleName(begin, end);
				}
				return {};
			}

			void GuiDocumentElement::SetParagraphAlignment(TextPos begin, TextPos end, const collections::Array<Nullable<Alignment>>& alignments)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					vint first = begin.row;
					vint last = end.row;
					if (first > last)
					{
						vint temp = first;
						first = last;
						last = temp;
					}

					if (0 <= first && first < document->paragraphs.Count() && 0 <= last && last < document->paragraphs.Count() && last - first + 1 == alignments.Count())
					{
						for (vint i = first; i <= last; i++)
						{
							document->paragraphs[i]->alignment = alignments[i - first];
						}
						elementRenderer->NotifyParagraphUpdated(first, alignments.Count(), alignments.Count(), false);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			Nullable<Alignment> GuiDocumentElement::SummarizeParagraphAlignment(TextPos begin, TextPos end)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					if (begin > end)
					{
						TextPos temp = begin;
						begin = end;
						end = temp;
					}

					return document->SummarizeParagraphAlignment(begin, end);
				}
				return {};
			}

			Ptr<DocumentHyperlinkRun::Package> GuiDocumentElement::GetHyperlinkFromPoint(Point point)
			{
				if (auto elementRenderer = renderer.Cast<GuiDocumentElementRenderer>())
				{
					return elementRenderer->GetHyperlinkFromPoint(point);
				}
				return nullptr;
			}
		}
	}
}