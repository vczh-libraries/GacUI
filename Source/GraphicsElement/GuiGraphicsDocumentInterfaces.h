/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Element System and Infrastructure Interfaces

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSDOCUMENTINTERFACES
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSDOCUMENTINTERFACES

#include "GuiGraphicsElementInterfaces.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
Layout Engine
***********************************************************************/

			class IGuiGraphicsParagraph;
			class IGuiGraphicsLayoutProvider;

			/// <summary>Represents a paragraph of a layouted rich text content.</summary>
			class IGuiGraphicsParagraph : public IDescriptable, public Description<IGuiGraphicsParagraph>
			{
			public:
				static const vint		NullInteractionId = -1;

				/// <summary>Text style. Items in this enumeration type can be combined.</summary>
				enum TextStyle
				{
					/// <summary>Bold.</summary>
					Bold=1,
					/// <summary>Italic.</summary>
					Italic=2,
					/// <summary>Underline.</summary>
					Underline=4,
					/// <summary>Strikeline.</summary>
					Strikeline=8,
				};

				/// <summary>Inline object break condition.</summary>
				enum BreakCondition
				{
					/// <summary>Stay together with the previous run if possible.</summary>
					StickToPreviousRun,
					/// <summary>Stay together with the next run if possible.</summary>
					StickToNextRun,
					/// <summary>Treat as a single run.</summary>
					Alone,
				};

				/// <summary>Caret relative position.</summary>
				enum CaretRelativePosition
				{
					/// <summary>The first caret position.</summary>
					CaretFirst,
					/// <summary>The last caret position.</summary>
					CaretLast,
					/// <summary>The first caret position of the current line.</summary>
					CaretLineFirst,
					/// <summary>The last caret position of the current line.</summary>
					CaretLineLast,
					/// <summary>The relative left caret position.</summary>
					CaretMoveLeft,
					/// <summary>The relative right caret position.</summary>
					CaretMoveRight,
					/// <summary>The relative up caret position.</summary>
					CaretMoveUp,
					/// <summary>The relative down caret position.</summary>
					CaretMoveDown,
				};

				/// <summary>Inline object properties.</summary>
				struct InlineObjectProperties
				{
					/// <summary>The size of the inline object.</summary>
					Size						size;
					/// <summary>The baseline of the inline object.If the baseline is at the bottom, then set the baseline to -1.</summary>
					vint						baseline = -1;
					/// <summary>The break condition of the inline object.</summary>
					BreakCondition				breakCondition;
					/// <summary>The background image, nullable.</summary>
					Ptr<IGuiGraphicsElement>	backgroundImage;
					/// <summary>The id for callback. If the value is -1, then no callback will be received .</summary>
					vint						callbackId = -1;

					InlineObjectProperties()
						:baseline(-1)
					{
					}
				};

				/// <summary>Get the <see cref="IGuiGraphicsLayoutProvider"/> object that created this paragraph.</summary>
				/// <returns>The layout provider object.</returns>
				virtual IGuiGraphicsLayoutProvider*			GetProvider()=0;
				/// <summary>Get the associated <see cref="IGuiGraphicsRenderTarget"/> to this paragraph.</summary>
				/// <returns>The associated render target.</returns>
				virtual IGuiGraphicsRenderTarget*			GetRenderTarget()=0;
				/// <summary>Get if line auto-wrapping is enabled for this paragraph.</summary>
				/// <returns>Return true if line auto-wrapping is enabled for this paragraph.</returns>
				virtual bool								GetWrapLine()=0;
				/// <summary>Set if line auto-wrapping is enabled for this paragraph.</summary>
				/// <param name="value">True if line auto-wrapping is enabled for this paragraph.</param>
				virtual void								SetWrapLine(bool value)=0;
				/// <summary>Get the max width for this paragraph. If there is no max width limitation, it returns -1.</summary>
				/// <returns>The max width for this paragraph.</returns>
				virtual vint								GetMaxWidth()=0;
				/// <summary>Set the max width for this paragraph. If the max width is set to -1, the max width limitation will be removed.</summary>
				/// <param name="value">The max width.</param>
				virtual void								SetMaxWidth(vint value)=0;
				/// <summary>Get the horizontal alignment for this paragraph.</summary>
				/// <returns>The alignment.</returns>
				virtual Alignment							GetParagraphAlignment()=0;
				/// <summary>Set the horizontal alignment for this paragraph.</summary>
				/// <param name="value">The alignment.</param>
				virtual void								SetParagraphAlignment(Alignment value)=0;

				/// <summary>Replace the font within the specified range.</summary>
				/// <param name="start">The position of the first character of the specified range.</param>
				/// <param name="length">The length of the specified range by character.</param>
				/// <param name="value">The font.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								SetFont(vint start, vint length, const WString& value)=0;
				/// <summary>Replace the size within the specified range.</summary>
				/// <param name="start">The position of the first character of the specified range.</param>
				/// <param name="length">The length of the specified range by character.</param>
				/// <param name="value">The size.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								SetSize(vint start, vint length, vint value)=0;
				/// <summary>Replace the text style within the specified range.</summary>
				/// <param name="start">The position of the first character of the specified range.</param>
				/// <param name="length">The length of the specified range by character.</param>
				/// <param name="value">The text style.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								SetStyle(vint start, vint length, TextStyle value)=0;
				/// <summary>Replace the color within the specified range.</summary>
				/// <param name="start">The position of the first character of the specified range.</param>
				/// <param name="length">The length of the specified range by character.</param>
				/// <param name="value">The color.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								SetColor(vint start, vint length, Color value)=0;
				/// <summary>Replace the background color within the specified range.</summary>
				/// <param name="start">The position of the first character of the specified range.</param>
				/// <param name="length">The length of the specified range by character.</param>
				/// <param name="value">The background color.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								SetBackgroundColor(vint start, vint length, Color value)=0;
				/// <summary>Bind an <see cref="IGuiGraphicsElement"/> to a range of text.</summary>
				/// <param name="start">The position of the first character of the specified range.</param>
				/// <param name="length">The length of the specified range by character.</param>
				/// <param name="properties">The properties for the inline object.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)=0;
				/// <summary>Unbind all inline objects to a range of text.</summary>
				/// <param name="start">The position of the first character of the specified range.</param>
				/// <param name="length">The length of the specified range by character.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								ResetInlineObject(vint start, vint length)=0;

				/// <summary>Get the layouted height of the text. The result depends on rich styled text and the two important properties that can be set using <see cref="SetWrapLine"/> and <see cref="SetMaxWidth"/>.</summary>
				/// <returns>The layouted height.</returns>
				virtual vint								GetHeight()=0;
				/// <summary>Make the caret visible so that it will be rendered in the paragraph.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="caret">The caret.</param>
				/// <param name="color">The color of the caret.</param>
				/// <param name="frontSide">Set to true to display the caret for the character before it.</param>
				virtual bool								OpenCaret(vint caret, Color color, bool frontSide)=0;
				/// <summary>Make the caret invisible.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				virtual bool								CloseCaret()=0;
				/// <summary>Render the graphics element using a specified bounds.</summary>
				/// <param name="bounds">Bounds to decide the size and position of the binded graphics element.</param>
				virtual void								Render(Rect bounds)=0;

				/// <summary>Get a new caret from the old caret with a relative position.</summary>
				/// <returns>The new caret. Returns -1 if failed.</returns>
				/// <param name="comparingCaret">The caret to compare. If the position is CaretFirst or CaretLast, this argument is ignored.</param>
				/// <param name="position">The relative position.</param>
				/// <param name="preferFrontSide">Only for CaretMoveUp and CaretMoveDown. Set to true to make the caret prefer to get closer to the character before it. After this function is called, this argument stored the suggested side for displaying the new caret.</param>
				virtual vint								GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)=0;
				/// <summary>Get the bounds of the caret.</summary>
				/// <returns>The bounds whose width is 0. Returns an empty Rect value if failed.</returns>
				/// <param name="caret">The caret.</param>
				/// <param name="frontSide">Set to true to get the bounds of the front side, otherwise the back side. If only one side is valid, this argument is ignored.</param>
				virtual Rect								GetCaretBounds(vint caret, bool frontSide)=0;
				/// <summary>Get the caret from a specified position.</summary>
				/// <returns>The caret. Returns -1 if failed.</returns>
				/// <param name="point">The point.</param>
				virtual vint								GetCaretFromPoint(Point point)=0;
				/// <summary>Get the inline object from a specified position.</summary>
				/// <returns>The inline object. Returns null if failed.</returns>
				/// <param name="point">The point.</param>
				/// <param name="start">Get the start position of this element.</param>
				/// <param name="length">Get the length of this element.</param>
				virtual Nullable<InlineObjectProperties>	GetInlineObjectFromPoint(Point point, vint& start, vint& length)=0;
				/// <summary>Get the nearest caret from a text position.</summary>
				/// <returns>The caret. Returns -1 if failed. If the text position is a caret, then the result will be the text position itself without considering the frontSide argument.</returns>
				/// <param name="textPos">The caret to compare. If the position is CaretFirst or CaretLast, this argument is ignored.</param>
				/// <param name="frontSide">Set to true to search in front of the text position, otherwise the opposite position.</param>
				virtual vint								GetNearestCaretFromTextPos(vint textPos, bool frontSide)=0;
				/// <summary>Test is the caret valid.</summary>
				/// <returns>Returns true if the caret is valid.</returns>
				/// <param name="caret">The caret to test.</param>
				virtual bool								IsValidCaret(vint caret)=0;
				/// <summary>Test is the text position valid.</summary>
				/// <returns>Returns true if the text position is valid.</returns>
				/// <param name="textPos">The text position to test.</param>
				virtual bool								IsValidTextPos(vint textPos)=0;
			};

			/// <summary>Paragraph callback</summary>
			class IGuiGraphicsParagraphCallback : public IDescriptable, public Description<IGuiGraphicsParagraphCallback>
			{
			public:
				/// <summary>Called when an inline object with a valid callback id is being rendered.</summary>
				/// <returns>Returns the new size of the rendered inline object.</returns>
				/// <param name="callbackId">The callback id of the inline object</param>
				/// <param name="location">The location of the inline object, relative to the left-top corner of this paragraph.</param>
				virtual Size								OnRenderInlineObject(vint callbackId, Rect location) = 0;
			};

			/// <summary>Renderer awared rich text document layout engine provider interface.</summary>
			class IGuiGraphicsLayoutProvider : public IDescriptable, public Description<IGuiGraphicsLayoutProvider>
			{
			public:
				/// <summary>Create a paragraph with internal renderer device dependent objects initialized.</summary>
				/// <param name="text">The text used to fill the paragraph.</param>
				/// <param name="renderTarget">The render target that the created paragraph will render to.</param>
				/// <param name="callback">A callback to receive necessary information when the paragraph is being rendered.</param>
				/// <returns>The created paragraph object.</returns>
				virtual Ptr<IGuiGraphicsParagraph>			CreateParagraph(const WString& text, IGuiGraphicsRenderTarget* renderTarget, IGuiGraphicsParagraphCallback* callback)=0;
			};
		}
	}
}

#endif