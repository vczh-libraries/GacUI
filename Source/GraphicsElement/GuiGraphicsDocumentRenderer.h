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
GuiDocumentElementRenderer
***********************************************************************/

			class GuiDocumentElementRenderer
				: public GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget, IGuiDocumentElementRenderer>
				, private virtual IGuiGraphicsParagraphCallback
			{
				friend class visitors::SetPropertiesVisitor;
				friend class GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget, IGuiDocumentElementRenderer>;
			protected:
				struct EmbeddedObject
				{
					WString								name;
					Size								size;
					vint								start;
					bool								resized = false;
				};

				typedef collections::Dictionary<vint, Ptr<EmbeddedObject>>		IdEmbeddedObjectMap;
				typedef collections::Dictionary<WString, vint>					NameIdMap;
				typedef collections::List<vint>									FreeIdList;

				struct ParagraphCache
				{
					WString								fullText;
					Ptr<IGuiGraphicsParagraph>			graphicsParagraph;
					IdEmbeddedObjectMap					embeddedObjects;
					vint								selectionBegin;
					vint								selectionEnd;

					ParagraphCache()
						:selectionBegin(-1)
						,selectionEnd(-1)
					{
					}
				};

				typedef collections::Array<Ptr<ParagraphCache>>		ParagraphCacheArray;
				typedef collections::Array<Size>					ParagraphSizeArray;

			private:

				Size									OnRenderInlineObject(vint callbackId, Rect location)override;
			protected:
				vint									paragraphDistance;
				vint									lastMaxWidth;
				Size									cachedTotalSize;
				IGuiGraphicsLayoutProvider*				layoutProvider;
				ParagraphCacheArray						paragraphCaches;
				ParagraphSizeArray						paragraphSizes;

				TextPos									lastCaret;
				Color									lastCaretColor;
				bool									lastCaretFrontSide;

				NameIdMap								nameCallbackIdMap;
				FreeIdList								freeCallbackIds;
				vint									usedCallbackIds = 0;

				vint									renderingParagraph = -1;
				Point									renderingParagraphOffset;

				void									InitializeInternal();
				void									FinalizeInternal();
				void									RenderTargetChangedInternal(IGuiGraphicsRenderTarget* oldRenderTarget, IGuiGraphicsRenderTarget* newRenderTarget);
				Ptr<ParagraphCache>						EnsureAndGetCache(vint paragraphIndex, bool createParagraph);
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