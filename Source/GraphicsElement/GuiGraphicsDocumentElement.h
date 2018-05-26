/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Element System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSDOCUMENTELEMENT
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSDOCUMENTELEMENT

#include "GuiGraphicsElement.h"

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

			/// <summary>Defines a rich text document element for rendering complex styled document.</summary>
			class GuiDocumentElement : public GuiElementBase<GuiDocumentElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiDocumentElement, L"RichDocument");
			public:
				/// <summary>Callback interface for this element.</summary>
				class ICallback : public virtual IDescriptable, public Description<ICallback>
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

				class GuiDocumentElementRenderer : public Object, public IGuiGraphicsRenderer, private IGuiGraphicsParagraphCallback
				{
					friend class visitors::SetPropertiesVisitor;

					DEFINE_GUI_GRAPHICS_RENDERER(GuiDocumentElement, GuiDocumentElementRenderer, IGuiGraphicsRenderTarget)
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
					typedef collections::Array<vint>					ParagraphHeightArray;

				private:

					Size									OnRenderInlineObject(vint callbackId, Rect location)override;
				protected:
					vint									paragraphDistance;
					vint									lastMaxWidth;
					vint									cachedTotalHeight;
					IGuiGraphicsLayoutProvider*				layoutProvider;
					ParagraphCacheArray						paragraphCaches;
					ParagraphHeightArray					paragraphHeights;

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

			protected:
				Ptr<DocumentModel>							document;
				ICallback*									callback = nullptr;
				TextPos										caretBegin;
				TextPos										caretEnd;
				bool										caretVisible;
				bool										caretFrontSide;
				Color										caretColor;

				void										UpdateCaret();

				GuiDocumentElement();
			public:
				/// <summary>Get the callback.</summary>
				/// <returns>The callback.</returns>
				ICallback*									GetCallback();
				/// <summary>Set the callback.</summary>
				/// <param name="value">The callback.</param>
				void										SetCallback(ICallback* value);
				
				/// <summary>Get the document.</summary>
				/// <returns>The document.</returns>
				Ptr<DocumentModel>							GetDocument();
				/// <summary>Set the document. When a document is set to this element, modifying the document without invoking <see cref="NotifyParagraphUpdated"/> will lead to undefined behavior.</summary>
				/// <param name="value">The document.</param>
				void										SetDocument(Ptr<DocumentModel> value);
				/// <summary>
				/// Get the begin position of the selection area.
				/// </summary>
				/// <returns>The begin position of the selection area.</returns>
				TextPos										GetCaretBegin();
				/// <summary>
				/// Get the end position of the selection area.
				/// </summary>
				/// <returns>The end position of the selection area.</returns>
				TextPos										GetCaretEnd();
				/// <summary>
				/// Get the prefer side for the caret.
				/// </summary>
				/// <returns>Returns true if the caret is rendered for the front side.</returns>
				bool										IsCaretEndPreferFrontSide();
				/// <summary>
				/// Set the end position of the selection area.
				/// </summary>
				/// <param name="begin">The begin position of the selection area.</param>
				/// <param name="end">The end position of the selection area.</param>
				/// <param name="frontSide">Set to true to show the caret for the character before it. This argument is ignored if begin and end are the same.</param>
				void										SetCaret(TextPos begin, TextPos end, bool frontSide);
				/// <summary>
				/// Get the caret visibility.
				/// </summary>
				/// <returns>Returns true if the caret will be rendered.</returns>
				bool										GetCaretVisible();
				/// <summary>
				/// Set the caret visibility.
				/// </summary>
				/// <param name="value">True if the caret will be rendered.</param>
				void										SetCaretVisible(bool value);
				/// <summary>
				/// Get the color of the caret.
				/// </summary>
				/// <returns>The color of the caret.</returns>
				Color										GetCaretColor();
				/// <summary>
				/// Set the color of the caret.
				/// </summary>
				/// <param name="value">The color of the caret.</param>
				void										SetCaretColor(Color value);

				/// <summary>Calculate a caret using a specified comparing caret and a relative position.</summary>
				/// <returns>The calculated caret.</returns>
				/// <param name="comparingCaret">The comparing caret.</param>
				/// <param name="position">The relative position.</param>
				/// <param name="preferFrontSide">Specify the side for the comparingCaret. Retrive the suggested side for the new caret. If the return caret equals compareCaret, this output is ignored.</param>
				TextPos										CalculateCaret(TextPos comparingCaret, IGuiGraphicsParagraph::CaretRelativePosition position, bool& preferFrontSide);
				/// <summary>Calculate a caret using a specified point.</summary>
				/// <returns>The calculated caret.</returns>
				/// <param name="point">The specified point.</param>
				TextPos										CalculateCaretFromPoint(Point point);
				/// <summary>Get the bounds of a caret.</summary>
				/// <returns>The bounds.</returns>
				/// <param name="caret">The caret.</param>
				/// <param name="frontSide">Set to true to get the bounds for the character before it.</param>
				Rect										GetCaretBounds(TextPos caret, bool frontSide);

				/// <summary>Notify that some paragraphs are updated.</summary>
				/// <param name="index">The start paragraph index.</param>
				/// <param name="oldCount">The number of paragraphs to be updated.</param>
				/// <param name="newCount">The number of updated paragraphs.</param>
				/// <param name="updatedText">Set to true to notify that the text is updated.</param>
				void										NotifyParagraphUpdated(vint index, vint oldCount, vint newCount, bool updatedText);
				/// <summary>Edit run in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				/// <param name="model">The new run.</param>
				/// <param name="copy">Set to true to copy the model before editing. Otherwise, objects inside the model will be used directly</param>
				void										EditRun(TextPos begin, TextPos end, Ptr<DocumentModel> model, bool copy);
				/// <summary>Edit text in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				/// <param name="frontSide">Set to true to use the text style in front of the specified range.</param>
				/// <param name="text">The new text.</param>
				void										EditText(TextPos begin, TextPos end, bool frontSide, const collections::Array<WString>& text);
				/// <summary>Edit style in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				/// <param name="style">The new style.</param>
				void										EditStyle(TextPos begin, TextPos end, Ptr<DocumentStyleProperties> style);
				/// <summary>Edit image in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				/// <param name="image">The new image.</param>
				void										EditImage(TextPos begin, TextPos end, Ptr<GuiImageData> image);
				/// <summary>Set hyperlink in a specified range.</summary>
				/// <param name="paragraphIndex">The index of the paragraph to edit.</param>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				/// <param name="reference">The reference of the hyperlink.</param>
				/// <param name="normalStyleName">The normal style name of the hyperlink.</param>
				/// <param name="activeStyleName">The active style name of the hyperlink.</param>
				void										EditHyperlink(vint paragraphIndex, vint begin, vint end, const WString& reference, const WString& normalStyleName=DocumentModel::NormalLinkStyleName, const WString& activeStyleName=DocumentModel::ActiveLinkStyleName);
				/// <summary>Remove hyperlink in a specified range.</summary>
				/// <param name="paragraphIndex">The index of the paragraph to edit.</param>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				void										RemoveHyperlink(vint paragraphIndex, vint begin, vint end);
				/// <summary>Edit style name in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				/// <param name="styleName">The new style name.</param>
				void										EditStyleName(TextPos begin, TextPos end, const WString& styleName);
				/// <summary>Remove style name in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				void										RemoveStyleName(TextPos begin, TextPos end);
				/// <summary>Rename a style.</summary>
				/// <param name="oldStyleName">The name of the style.</param>
				/// <param name="newStyleName">The new name.</param>
				void										RenameStyle(const WString& oldStyleName, const WString& newStyleName);
				/// <summary>Clear all styles in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				void										ClearStyle(TextPos begin, TextPos end);
				/// <summary>Summarize the text style in a specified range.</summary>
				/// <returns>The text style summary.</returns>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				Ptr<DocumentStyleProperties>				SummarizeStyle(TextPos begin, TextPos end);
				/// <summary>Summarize the style name in a specified range.</summary>
				/// <returns>The style name summary.</returns>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				Nullable<WString>							SummarizeStyleName(TextPos begin, TextPos end);
				/// <summary>Set the alignment of paragraphs in a specified range.</summary>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				/// <param name="alignments">The alignment for each paragraph.</param>
				void										SetParagraphAlignment(TextPos begin, TextPos end, const collections::Array<Nullable<Alignment>>& alignments);
				/// <summary>Summarize the text alignment in a specified range.</summary>
				/// <returns>The text alignment summary.</returns>
				/// <param name="begin">The begin position of the range.</param>
				/// <param name="end">The end position of the range.</param>
				Nullable<Alignment>							SummarizeParagraphAlignment(TextPos begin, TextPos end);

				/// <summary>Get hyperlink from point.</summary>
				/// <returns>Corressponding hyperlink id. Returns -1 indicates that the point is not in a hyperlink.</returns>
				/// <param name="point">The point to get the hyperlink id.</param>
				Ptr<DocumentHyperlinkRun::Package>			GetHyperlinkFromPoint(Point point);
			};
		}
	}
}

#endif