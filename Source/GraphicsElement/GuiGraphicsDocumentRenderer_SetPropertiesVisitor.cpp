#include "GuiGraphicsDocumentRenderer.h"

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
					typedef DocumentModel::ResolvedStyle					ResolvedStyle;
				public:
					vint							start = 0;
					vint							length = 0;
					vint							selectionBegin;
					vint							selectionEnd;
					vint							rangeBegin;
					vint							rangeEnd;
					List<ResolvedStyle>				styles;

					DocumentModel*					model;
					GuiDocumentParagraphCache*		paragraphCache;
					GuiDocumentImageCache*			imageCache;
					Ptr<pg::ParagraphCache>			cache;
					IGuiGraphicsParagraph*			paragraph;
					vint							paragraphIndex;

					SetPropertiesVisitor(
						DocumentModel* _model,
						GuiDocumentParagraphCache* _paragraphCache,
						GuiDocumentImageCache* _imageCache,
						Ptr<pg::ParagraphCache> _cache,
						vint _paragraphIndex,
						vint _selectionBegin,
						vint _selectionEnd,
						vint _rangeBegin,
						vint _rangeEnd
					)
						: model(_model)
						, paragraphCache(_paragraphCache)
						, imageCache(_imageCache)
						, cache(_cache)
						, paragraphIndex(_paragraphIndex)
						, paragraph(_cache->graphicsParagraph.Obj())
						, selectionBegin(_selectionBegin)
						, selectionEnd(_selectionEnd)
						, rangeBegin(_rangeBegin)
						, rangeEnd(_rangeEnd)
					{
						ResolvedStyle style;
						style = model->GetStyle(DocumentModel::DefaultStyleName, style);
						styles.Add(style);
					}

					void VisitContainer(DocumentContainerRun* run)
					{
						for (auto subRun : run->runs)
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
							((style.style.bold ? IGuiGraphicsParagraph::Bold : 0)
							| (style.style.italic ? IGuiGraphicsParagraph::Italic : 0)
							| (style.style.underline ? IGuiGraphicsParagraph::Underline : 0)
							| (style.style.strikeline ? IGuiGraphicsParagraph::Strikeline : 0)
							));
					}

					void ApplyColor(vint start, vint length, const ResolvedStyle& style)
					{
						paragraph->SetColor(start, length, style.color);
						paragraph->SetBackgroundColor(start, length, style.backgroundColor);
					}

					void Visit(DocumentTextRun* run)override
					{
						length = run->GetRepresentationText().Length();
						if (length > 0 && start < rangeEnd && rangeBegin < start + length)
						{
							ResolvedStyle style = styles[styles.Count() - 1];
							ApplyStyle(start, length, style);
							ApplyColor(start, length, style);

							vint styleStart = start;
							vint styleEnd = styleStart + length;
							if (styleStart < selectionEnd && selectionBegin < styleEnd)
							{
								vint s2 = styleStart > selectionBegin ? styleStart : selectionBegin;
								vint s3 = selectionEnd < styleEnd ? selectionEnd : styleEnd;

								if (s2 < s3)
								{
									ResolvedStyle selectionStyle = model->GetStyle(DocumentModel::SelectionStyleName, style);
									ApplyColor(s2, s3 - s2, selectionStyle);
								}
							}
						}
						start += length;
					}

					void Visit(DocumentStylePropertiesRun* run)override
					{
						ResolvedStyle style = styles[styles.Count() - 1];
						style = model->GetStyle(run->style, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count() - 1);
					}

					void Visit(DocumentStyleApplicationRun* run)override
					{
						ResolvedStyle style = styles[styles.Count() - 1];
						style = model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count() - 1);
					}

					void Visit(DocumentHyperlinkRun* run)override
					{
						ResolvedStyle style = styles[styles.Count() - 1];
						style = model->GetStyle(run->styleName, style);
						styles.Add(style);
						VisitContainer(run);
						styles.RemoveAt(styles.Count() - 1);
					}

					void Visit(DocumentImageRun* run)override
					{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::visitors::SetPropertiesVisitor::Visit(DocumentImageRun*)#"
						length = run->GetRepresentationText().Length();
						if (start < rangeEnd && rangeBegin < start + length)
						{
							IGuiGraphicsParagraph::InlineObjectProperties properties;
							properties.size = run->GetSize();
							properties.baseline = run->baseline;
							properties.breakCondition = IGuiGraphicsParagraph::Alone;
							properties.backgroundImage = imageCache->GetImageElement(run->image, run->frameIndex, paragraphIndex, start);

							bool result = paragraph->SetInlineObject(start, length, properties);
							CHECK_ERROR(result, ERROR_MESSAGE_PREFIX L"The specified range has already been occupied by another inline object.");

							if (start < selectionEnd && selectionBegin < start + length)
							{
								ResolvedStyle style = styles[styles.Count() - 1];
								ResolvedStyle selectionStyle = model->GetStyle(DocumentModel::SelectionStyleName, style);
								ApplyColor(start, length, selectionStyle);
							}
						}
						start += length;
#undef ERROR_MESSAGE_PREFIX
					}

					void Visit(DocumentEmbeddedObjectRun* run)override
					{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::elements::visitors::SetPropertiesVisitor::Visit(DocumentEmbeddedObjectRun*)#"
						length = run->GetRepresentationText().Length();
						if (start < rangeEnd && rangeBegin < start + length)
						{
							IGuiGraphicsParagraph::InlineObjectProperties properties;
							properties.breakCondition = IGuiGraphicsParagraph::Alone;

							if (run->name != L"")
							{
								vint index = paragraphCache->nameCallbackIdMap.Keys().IndexOf(run->name);
								if (index != -1)
								{
									auto id = paragraphCache->nameCallbackIdMap.Values()[index];
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
									auto eo = Ptr(new pg::EmbeddedObject);
									eo->name = run->name;
									eo->size = Size(0, 0);
									eo->start = start;

									vint id = -1;
									vint count = paragraphCache->freeCallbackIds.Count();
									if (count > 0)
									{
										id = paragraphCache->freeCallbackIds[count - 1];
										paragraphCache->freeCallbackIds.RemoveAt(count - 1);
									}
									else
									{
										id = paragraphCache->usedCallbackIds++;
									}

									paragraphCache->nameCallbackIdMap.Add(eo->name, id);
									cache->embeddedObjects.Add(id, eo);
									properties.callbackId = id;
								}
							}

							bool result = paragraph->SetInlineObject(start, length, properties);
							CHECK_ERROR(result, ERROR_MESSAGE_PREFIX L"The specified range has already been occupied by another inline object.");

							if (start < selectionEnd && selectionBegin < start + length)
							{
								ResolvedStyle style = styles[styles.Count() - 1];
								ResolvedStyle selectionStyle = model->GetStyle(DocumentModel::SelectionStyleName, style);
								ApplyColor(start, length, selectionStyle);
							}
						}
						start += length;
#undef ERROR_MESSAGE_PREFIX
					}

					void Visit(DocumentParagraphRun* run)override
					{
						VisitContainer(run);
					}
				};

				vint SetProperties(
					DocumentModel* model,
					GuiDocumentParagraphCache* paragraphCache,
					GuiDocumentImageCache* imageCache,
					Ptr<pg::ParagraphCache> cache,
					vint paragraphIndex,
					Ptr<DocumentParagraphRun> run,
					vint selectionBegin,
					vint selectionEnd,
					vint rangeBegin,
					vint rangeEnd
				)
				{
					SetPropertiesVisitor visitor(model, paragraphCache, imageCache, cache, paragraphIndex, selectionBegin, selectionEnd, rangeBegin, rangeEnd);
					run->Accept(&visitor);
					return visitor.length;
				}
			}
		}
	}
}