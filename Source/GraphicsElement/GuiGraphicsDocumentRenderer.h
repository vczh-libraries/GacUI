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
Rich Content Document (element)
***********************************************************************/

			/// <summary>Callback interface for this element.</summary>
			class IGuiDocumentElementCallback : public virtual IDescriptable, public Description<IGuiDocumentElementCallback>
			{
			public:
				/// <summary>Called when the rendering is started.</summary>
				virtual void							OnStartRender() = 0;

				/// <summary>Called when the rendering is finished.</summary>
				virtual void							OnFinishRender() = 0;

				/// <summary>Called when an embedded object is being rendered.</summary>
				/// <returns>Returns the new size of the rendered embedded object.</returns>
				/// <param name="name">The name of the embedded object</param>
				/// <param name="location">The location of the embedded object, relative to the left-top corner of this element.</param>
				virtual Size							OnRenderEmbeddedObject(const WString& name, const Rect& location) = 0;
			};

			class GuiDocumentElementRenderer : public GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget>, private IGuiGraphicsParagraphCallback
			{
				friend class visitors::SetPropertiesVisitor;
				friend class GuiElementRendererBase<GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget>;
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