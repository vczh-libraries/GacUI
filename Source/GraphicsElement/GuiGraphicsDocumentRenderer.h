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
					Nullable<WString>					fullText;				// when fullText is null, all members below are invalid
					Ptr<IGuiGraphicsParagraph>			graphicsParagraph;
					IdEmbeddedObjectMap					embeddedObjects;
					vint								selectionBegin = -1;
					vint								selectionEnd = -1;
				};

				struct ParagraphSize
				{
					// invalid after GuiDocumentParagraphCache::validCachedTops-th ParagraphCache
					vint								cachedTopWithoutParagraphDistance = 0;
					Size								cachedSize;
				};

				typedef collections::Array<Ptr<ParagraphCache>>		ParagraphCacheArray;
				typedef collections::Array<ParagraphSize>			ParagraphSizeArray;
			}

			class GuiDocumentParagraphCache : public Object
			{
			protected:
				GuiDocumentElement*						element = nullptr;
				IGuiGraphicsRenderTarget*				renderTarget = nullptr;
				IGuiGraphicsLayoutProvider*				layoutProvider = nullptr;
				pg::ParagraphCacheArray					paragraphCaches;
				pg::ParagraphSizeArray					paragraphSizes;
				vint									validCachedTops = 0;

			public:
				GuiDocumentParagraphCache();
				~GuiDocumentParagraphCache();

				void									Initialize(GuiDocumentElement* _element);
				void									RenderTargetChanged(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget);

				Ptr<pg::ParagraphCache>					TryGetParagraphCache(vint paragraphIndex);
				Ptr<pg::ParagraphCache>					GetParagraphCache(vint paragraphIndex, bool requireParagraph);
				pg::ParagraphSize						GetParagraphSize(vint paragraphIndex, bool requireCachedTop);
				void									InvalidCachedTops(vint firstParagraphIndex);
			};

/***********************************************************************
GuiDocumentElementRenderer
***********************************************************************/

			class GuiDocumentElementRenderer
				: public GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget, IGuiDocumentElementRenderer>
				, private virtual IGuiGraphicsParagraphCallback
			{
				friend class visitors::SetPropertiesVisitor;
				friend class GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget, IGuiDocumentElementRenderer>;
			protected:

			private:

				Size									OnRenderInlineObject(vint callbackId, Rect location)override;
			protected:
				vint									paragraphDistance = 0;
				vint									lastMaxWidth = -1;
				Size									lastTotalSize{ -1,-1 };
				GuiDocumentParagraphCache				pgCache;

				TextPos									lastCaret{ -1,-1 };
				bool									lastCaretFrontSide = false;
				Color									lastCaretColor;

				pg::NameIdMap							nameCallbackIdMap;
				pg::FreeIdList							freeCallbackIds;
				vint									usedCallbackIds = 0;

				vint									renderingParagraph = -1;
				Point									renderingParagraphOffset;

				void									InitializeInternal();
				void									FinalizeInternal();
				void									RenderTargetChangedInternal(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget);
				Ptr<pg::ParagraphCache>					EnsureAndGetCache(vint paragraphIndex, bool createParagraph);
				bool									GetParagraphIndexFromPoint(Point point, vint& top, vint& index);
			public:
				GuiDocumentElementRenderer();

				void									Render(Rect bounds)override;
				void									OnElementStateChanged()override;
				void									NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText);
				Ptr<DocumentHyperlinkRun::Package>		GetHyperlinkFromPoint(Point point);

				void									OpenCaret(TextPos caret, Color color, bool frontSide);
				void									CloseCaret(TextPos caret);
				void									SetSelection(TextPos begin, TextPos end);
				TextPos									CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide);
				TextPos									CalculateCaretFromPoint(Point point);
				Rect									GetCaretBounds(TextPos caret, bool frontSide);
			};
		}
	}
}

#endif