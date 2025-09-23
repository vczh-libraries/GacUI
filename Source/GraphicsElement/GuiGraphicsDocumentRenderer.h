/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Element System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSDOCUMENTRENDERER
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSDOCUMENTRENDERER

#include "GuiGraphicsDocumentElement.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			namespace visitors
			{
				class SetPropertiesVisitor;
			}

/***********************************************************************
GuiDocumentParagraphCache
***********************************************************************/

			namespace pg
			{
				struct EmbeddedObject
				{
					WString								name;
					Size								size;
					vint								start;
					bool								resized = false;
				};

				typedef collections::Dictionary<WString, vint>					NameIdMap;
				typedef collections::List<vint>									FreeIdList;
				typedef collections::Dictionary<vint, Ptr<EmbeddedObject>>		IdEmbeddedObjectMap;

				struct ParagraphCache
				{
					Ptr<IGuiGraphicsParagraph>			graphicsParagraph;
					bool								outdatedStyles = true;

					WString								fullText;
					IdEmbeddedObjectMap					embeddedObjects;
					vint								selectionBegin = -1;
					vint								selectionEnd = -1;
				};

				struct ParagraphSize
				{
					// cached tops are valid for indices < GuiDocumentParagraphCache::validCachedTops; invalid starting from validCachedTops (i.e., indices >= validCachedTops)
					vint								cachedTopWithoutParagraphDistance = 0;
					Size								cachedSize;
				};

				typedef collections::Array<Ptr<ParagraphCache>>		ParagraphCacheArray;
				typedef collections::Array<ParagraphSize>			ParagraphSizeArray;
			}

			class GuiDocumentParagraphCache : public Object
			{
				friend class visitors::SetPropertiesVisitor;
			protected:
				IGuiGraphicsParagraphCallback*			callback = nullptr;
				GuiDocumentElement*						element = nullptr;
				IGuiGraphicsRenderTarget*				renderTarget = nullptr;
				IGuiGraphicsLayoutProvider*				layoutProvider = nullptr;
				vint									defaultHeight = 0;

				pg::ParagraphCacheArray					paragraphCaches;
				pg::ParagraphSizeArray					paragraphSizes;
				vint									validCachedTops = 0;

				pg::NameIdMap							nameCallbackIdMap;
				pg::FreeIdList							freeCallbackIds;
				vint									usedCallbackIds = 0;

			public:
				GuiDocumentParagraphCache(IGuiGraphicsParagraphCallback* _callback);
				~GuiDocumentParagraphCache();

				void									Initialize(GuiDocumentElement* _element);
				void									RenderTargetChanged(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget);

				vint									GetParagraphCount();
				Ptr<pg::ParagraphCache>					TryGetParagraphCache(vint paragraphIndex);
				Ptr<pg::ParagraphCache>					GetParagraphCache(vint paragraphIndex, bool requireParagraph);
				Size									GetParagraphSize(vint paragraphIndex);
				vint									GetParagraphTopWithoutParagraphDistance(vint paragraphIndex);
				vint									GetParagraphTop(vint paragraphIndex, vint paragraphDistance);

				vint									ResetCache();																// returns total height
				vint									ResetCache(vint index, vint oldCount, vint newCount, bool updatedText);		// returns the diff of total height
				vint									EnsureParagraph(vint paragraphIndex, vint maxWidth);						// returns the diff of total height
				vint									GetParagraphFromY(vint y, vint paragraphDistance);
				void									ReleaseParagraphs(vint index, vint count);
			};

/***********************************************************************
GuiDocumentElementRenderer
***********************************************************************/

			class GuiDocumentElementRenderer
				: public GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget, IGuiDocumentElementRenderer>
				, private virtual IGuiGraphicsParagraphCallback
			{
				friend class GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget, IGuiDocumentElementRenderer>;
			protected:

			private:

				Size									OnRenderInlineObject(vint callbackId, Rect location)override;
			protected:
				vint									paragraphDistance = 0;
				vint									lastMaxWidth = -1;
				vint									lastTotalWidth = 0;
				vint									lastTotalHeightWithoutParagraphDistance = 0;
				GuiDocumentParagraphCache				pgCache;

				vint									previousRenderBegin = -1;	// -1 indicates invalid/uninitialized range
				vint									previousRenderCount = 0;	// Invalid when begin == -1

				TextPos									lastCaret{ -1,-1 };
				bool									lastCaretFrontSide = false;
				Color									lastCaretColor;

				vint									renderingParagraph = -1;
				Point									renderingParagraphOffset;

				void									InitializeInternal();
				void									FinalizeInternal();
				void									RenderTargetChangedInternal(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget);
				Ptr<pg::ParagraphCache>					EnsureParagraph(vint paragraphIndex);
				void									FixMinSize();
				void									UpdateRenderRange(vint index, vint oldCount, vint newCount);
				void									UpdateRenderRangeAndCleanUp(vint currentBegin, vint currentCount);

			public:
				GuiDocumentElementRenderer();

				void									Render(Rect bounds) override;
				void									OnElementStateChanged() override;
				void									NotifyParagraphPaddingUpdated(bool value) override;
				void									NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText) override;
				Ptr<DocumentHyperlinkRun::Package>		GetHyperlinkFromPoint(Point point) override;

				void									OpenCaret(TextPos caret, Color color, bool frontSide) override;
				void									CloseCaret(TextPos caret) override;
				void									SetSelection(TextPos begin, TextPos end) override;
				TextPos									CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide) override;
				TextPos									CalculateCaretFromPoint(Point point) override;
				Rect									GetCaretBounds(TextPos caret, bool frontSide) override;
			};
		}
	}
}

#endif