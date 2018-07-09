/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Element System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSTEXTELEMENT
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSTEXTELEMENT

#include "GuiGraphicsElement.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			class GuiColorizedTextElement;

/***********************************************************************
Colorized Plain Text (model)
***********************************************************************/

			namespace text
			{
				/// <summary>
				/// Represents the extra information of a character to display.
				/// </summary>
				struct CharAtt
				{
					/// <summary>
					/// The distance from the head of the line to the right side of this character in pixel.
					/// </summary>
					vuint32_t						rightOffset;
					/// <summary>
					/// The color index of the character. Use [M:vl.presentation.elements.GuiColorizedTextElement.GetColors] and [M:vl.presentation.elements.GuiColorizedTextElement.SetColors] to access the color table.
					/// </summary>
					vuint32_t						colorIndex;
				};
				
				/// <summary>
				/// Represents a line of characters.
				/// </summary>
				struct TextLine
				{
					static const vint				BlockSize=32;
					static const vint				MaxWidth=0xFFFF;
					
					/// <summary>
					/// A character buffer starts from the first character of this line.
					/// </summary>
					wchar_t*						text;
					/// <summary>
					/// A extra information buffer starts from the first character of this line.
					/// </summary>
					CharAtt*						att;
					/// <summary>
					/// The number of available <see cref="CharAtt::rightOffset"/> in the buffer.
					/// </summary>
					vint								availableOffsetCount;
					/// <summary>
					/// The number of elements in the allocated buffer memory.
					/// </summary>
					vint								bufferLength;
					/// <summary>
					/// The number of available characters in the buffer.
					/// </summary>
					vint								dataLength;
					/// <summary>
					/// The internal lexical analyzer state of a colorizer when it parses to the end of this line. -1 means that this state is not available.
					/// </summary>
					vint								lexerFinalState;
					/// <summary>
					/// The internal context sensitive state of a colorizer when it parses to the end of this line. -1 means that this state is not available.
					/// </summary>
					vint								contextFinalState;

					TextLine();
					~TextLine();

					static vint						CalculateBufferLength(vint dataLength);
					bool							operator==(const TextLine& value)const{return false;}
					bool							operator!=(const TextLine& value)const{return true;}

					/// <summary>
					/// Initialize the <see cref="TextLine"/> instance to be an empty line.
					/// </summary>
					void							Initialize();
					/// <summary>
					/// Release all resources used in this line.
					/// </summary>
					void							Finalize();
					/// <summary>
					/// Test is the line initialized.
					/// </summary>
					/// <returns>Returns true if the line is initialized.</returns>
					bool							IsReady();
					/// <summary>
					/// Modify the characters in the line by replacing characters.
					/// </summary>
					/// <returns>Returns true if the modification succeeded.</returns>
					/// <param name="start">The position of the first character to be replaced.</param>
					/// <param name="count">The number of characters to be replaced.</param>
					/// <param name="input">The buffer to the characters to write into this line.</param>
					/// <param name="inputCount">The numbers of the characters to write into this line.</param>
					bool							Modify(vint start, vint count, const wchar_t* input, vint inputCount);
					/// <summary>
					/// Split a text line into two by the position. The current line contains characters before this position. This function returns a new text line contains characters after this position.
					/// </summary>
					/// <returns>The new text line.</returns>
					/// <param name="index">.</param>
					TextLine						Split(vint index);
					/// <summary>
					/// Append a text line after the this text line, and finalize the input text line.
					/// </summary>
					/// <param name="line">The text line that contains all characters and color indices to append and be finalized.</param>
					void							AppendAndFinalize(TextLine& line);
				};

#if defined VCZH_MSVC
				/// <summary>Test if a wchar_t is the first character of a surrogate pair.</summary>
				/// <param name="c">The character to test.</param>
				/// <returns>Returns true if it is the first character of a surrogate pair.</returns>
				inline bool UTF16SPFirst(wchar_t c)
				{
					return 0xD800 <= c && c < 0xDC00;
				}

				/// <summary>Test if a wchar_t is the second character of a surrogate pair.</summary>
				/// <param name="c">The character to test.</param>
				/// <returns>Returns true if it is the second character of a surrogate pair.</returns>
				inline bool UTF16SPSecond(wchar_t c)
				{
					return 0xDC00 <= c && c < 0xDFFF;
				}
#endif

				/// <summary>
				/// A unicode code point.
				/// In Windows, when the first character is not the leading character of a surrogate pair, the second character is ignored.
				/// In other platforms which treat wchar_t as a UTF-32 character, the second character is ignored.
				/// </summary>
				struct UnicodeCodePoint
				{
#if defined VCZH_MSVC
					wchar_t							characters[2];

					UnicodeCodePoint(wchar_t c) :characters{ c,0 } {}
					UnicodeCodePoint(wchar_t c1, wchar_t c2) :characters{ c1,c2 } {}
#elif defined VCZH_GCC
					wchar_t							character;

					UnicodeCodePoint(wchar_t c) :character(c) {}
#endif

					vuint32_t GetCodePoint()const
					{
#if defined VCZH_MSVC
						if (UTF16SPFirst(characters[0]) && UTF16SPSecond(characters[1]))
						{
							return (wchar_t)(characters[0] - 0xD800) * 0x400 + (wchar_t)(characters[1] - 0xDC00) + 0x10000;
						}
						else
						{
							return (vuint32_t)characters[0];
						}
#elif defined VCZH_GCC
						return (vuint32_t)character;
#endif
					}
				};

				/// <summary>
				/// An abstract class for character size measuring in differect rendering technology.
				/// </summary>
				class CharMeasurer : public virtual IDescriptable
				{
				protected:
					IGuiGraphicsRenderTarget*		oldRenderTarget = nullptr;
					vint							rowHeight;
					vint							widths[65536];
					
					/// <summary>
					/// Measure the width of a character.
					/// </summary>
					/// <returns>The width in pixel.</returns>
					/// <param name="codePoint">The unicode code point to measure.</param>
					/// <param name="renderTarget">The render target which the character is going to be rendered. This is a pure virtual member function to be overrided.</param>
					virtual vint					MeasureWidthInternal(UnicodeCodePoint codePoint, IGuiGraphicsRenderTarget* renderTarget)=0;
					/// <summary>
					/// Measure the height of a character.
					/// </summary>
					/// <returns>The height in pixel.</returns>
					/// <param name="renderTarget">The render target which the character is going to be rendered.</param>
					virtual vint					GetRowHeightInternal(IGuiGraphicsRenderTarget* renderTarget)=0;
				public:

					/// <summary>
					/// Initialize a character measurer.
					/// </summary>
					/// <param name="_rowHeight">The default character height in pixel before the character measurer is binded to a render target.</param>
					CharMeasurer(vint _rowHeight);
					~CharMeasurer();

					/// <summary>
					/// Bind a render target to this character measurer.
					/// </summary>
					/// <param name="value">The render target to bind.</param>
					void							SetRenderTarget(IGuiGraphicsRenderTarget* value);
					/// <summary>
					/// Measure the width of a character using the binded render target.
					/// </summary>
					/// <returns>The width of a character, in pixel.</returns>
					/// <param name="codePoint">The unicode code point to measure.</param>
					vint							MeasureWidth(UnicodeCodePoint codePoint);
					/// <summary>
					/// Measure the height of a character.
					/// </summary>
					/// <returns>The height of a character, in pixel.</returns>
					vint							GetRowHeight();
				};

				/// <summary>
				/// A class to maintain multiple lines of text buffer.
				/// </summary>
				class TextLines : public Object, public Description<TextLines>
				{
					typedef collections::List<TextLine>		TextLineList;
				protected:
					GuiColorizedTextElement*		ownerElement;
					TextLineList					lines;
					CharMeasurer*					charMeasurer;
					IGuiGraphicsRenderTarget*		renderTarget;
					vint							tabWidth;
					vint							tabSpaceCount;
					wchar_t							passwordChar;
				public:
					TextLines(GuiColorizedTextElement* _ownerElement);
					~TextLines();

					/// <summary>
					/// Returns the number of text lines.
					/// </summary>
					/// <returns>The number of text lines.</returns>
					vint							GetCount();
					/// <summary>
					/// Returns the text line of a specified row number.
					/// </summary>
					/// <returns>The related text line object.</returns>
					/// <param name="row">The specified row number.</param>
					TextLine&						GetLine(vint row);
					/// <summary>
					/// Returns the binded <see cref="CharMeasurer"/>.
					/// </summary>
					/// <returns>The binded <see cref="CharMeasurer"/>.</returns>
					CharMeasurer*					GetCharMeasurer();
					/// <summary>
					/// Binded a <see cref="CharMeasurer"/>.
					/// </summary>
					/// <param name="value">The <see cref="CharMeasurer"/> to bind.</param>
					void							SetCharMeasurer(CharMeasurer* value);
					/// <summary>
					/// Returns the binded <see cref="IGuiGraphicsRenderTarget"/>.
					/// </summary>
					/// <returns>The binded <see cref="IGuiGraphicsRenderTarget"/>.</returns>
					IGuiGraphicsRenderTarget*		GetRenderTarget();
					/// <summary>
					/// Binded a <see cref="IGuiGraphicsRenderTarget"/>.
					/// </summary>
					/// <param name="value">The <see cref="IGuiGraphicsRenderTarget"/> to bind.</param>
					void							SetRenderTarget(IGuiGraphicsRenderTarget* value);
					/// <summary>
					/// Returns a string from a specified range of the text lines.
					/// </summary>
					/// <returns>The string.</returns>
					/// <param name="start">The start position.</param>
					/// <param name="end">The end position.</param>
					WString							GetText(TextPos start, TextPos end);
					/// <summary>
					/// Returns the whole string in the text lines.
					/// </summary>
					/// <returns>The string.</returns>
					WString							GetText();
					/// <summary>
					/// Set the string to the text lines. This operation will modified every <see cref="TextLine"/> objects.
					/// </summary>
					/// <param name="value">The string to set into the text lines.</param>
					void							SetText(const WString& value);
					
					/// <summary>
					/// Remove text lines in a specified range.
					/// </summary>
					/// <returns>Returns true if this operation succeeded.</returns>
					/// <param name="start">The first row number.</param>
					/// <param name="count">The number of text lines to be removed.</param>
					bool							RemoveLines(vint start, vint count);
					/// <summary>
					/// Test is a text position available in the text lines.
					/// </summary>
					/// <returns>Returns true if this position is available.</returns>
					/// <param name="pos">The text position to test.</param>
					bool							IsAvailable(TextPos pos);
					/// <summary>
					/// Normalize a text position to be available.
					/// </summary>
					/// <returns>The normalized text position.</returns>
					/// <param name="pos">The text position to normalize.</param>
					TextPos							Normalize(TextPos pos);
					/// <summary>
					/// Modify some text lines by replacing characters.
					/// </summary>
					/// <returns>The new end position.</returns>
					/// <param name="start">The start position of the range of characters to be replaced.</param>
					/// <param name="end">The end position of the range of characters to be replaced.</param>
					/// <param name="inputs">The buffer to the string buffers to replace into the text lines.</param>
					/// <param name="inputCounts">The numbers of characters for each string buffer.</param>
					/// <param name="rows">The number of string buffers.</param>
					TextPos							Modify(TextPos start, TextPos end, const wchar_t** inputs, vint* inputCounts, vint rows);
					/// <summary>
					/// Modify some text lines by replacing characters.
					/// </summary>
					/// <returns>The new end position.</returns>
					/// <param name="start">The start position of the range of characters to be replaced.</param>
					/// <param name="end">The end position of the range of characters to be replaced.</param>
					/// <param name="input">The buffer to the string to replace into the text lines.</param>
					/// <param name="inputCount">The number of characters to replace into the text lines.</param>
					TextPos							Modify(TextPos start, TextPos end, const wchar_t* input, vint inputCount);
					/// <summary>
					/// Modify some text lines by replacing characters.
					/// </summary>
					/// <returns>The new end position.</returns>
					/// <param name="start">The start position of the range of characters to be replaced.</param>
					/// <param name="end">The end position of the range of characters to be replaced.</param>
					/// <param name="input">The string to replace into the text lines.</param>
					TextPos							Modify(TextPos start, TextPos end, const wchar_t* input);
					/// <summary>
					/// Modify some text lines by replacing characters.
					/// </summary>
					/// <returns>The new end position.</returns>
					/// <param name="start">The start position of the range of characters to be replaced.</param>
					/// <param name="end">The end position of the range of characters to be replaced.</param>
					/// <param name="input">The string to replace into the text lines.</param>
					TextPos							Modify(TextPos start, TextPos end, const WString& input);
					/// <summary>
					/// Remove every text lines.
					/// </summary>
					void							Clear();
					
					/// <summary>
					/// Clear all cached <see cref="CharAtt::rightOffset"/>.
					/// </summary>
					void							ClearMeasurement();
					/// <summary>
					/// Returns the number of spaces to replace a tab character for rendering.
					/// </summary>
					/// <returns>The number of spaces to replace a tab character for rendering.</returns>
					vint							GetTabSpaceCount();
					/// <summary>
					/// Set the number of spaces to replace a tab character for rendering.
					/// </summary>
					/// <param name="value">The number of spaces to replace a tab character for rendering.</param>
					void							SetTabSpaceCount(vint value);
					/// <summary>
					/// Measure all characters in a specified row.
					/// </summary>
					/// <param name="row">The specified row number.</param>
					void							MeasureRow(vint row);
					/// <summary>
					/// Returns the width of a specified row.
					/// </summary>
					/// <returns>The width of a specified row, in pixel.</returns>
					/// <param name="row">The specified row number.</param>
					vint							GetRowWidth(vint row);
					/// <summary>
					/// Returns the height of a row.
					/// </summary>
					/// <returns>The height of a row, in pixel.</returns>
					vint							GetRowHeight();
					/// <summary>
					/// Returns the total width of the text lines.
					/// </summary>
					/// <returns>The width of the text lines, in pixel.</returns>
					vint							GetMaxWidth();
					/// <summary>
					/// Returns the total height of the text lines.
					/// </summary>
					/// <returns>The height of the text lines, in pixel.</returns>
					vint							GetMaxHeight();
					/// <summary>
					/// Get the text position near to specified point.
					/// </summary>
					/// <returns>The text position.</returns>
					/// <param name="point">The specified point, in pixel.</param>
					TextPos							GetTextPosFromPoint(Point point);
					/// <summary>
					/// Get the point of a specified text position.
					/// </summary>
					/// <returns>The point, in pixel. Returns (-1, -1) if the text position is not available.</returns>
					/// <param name="pos">The specified text position.</param>
					Point							GetPointFromTextPos(TextPos pos);
					/// <summary>
					/// Get the bounds of a specified text position.
					/// </summary>
					/// <returns>The bounds, in pixel. Returns (-1, -1, -1, -1) if the text position is not available.</returns>
					/// <param name="pos">The specified text position.</param>
					Rect							GetRectFromTextPos(TextPos pos);
					/// <summary>
					/// Get the password mode displaying character.
					/// </summary>
					/// <returns>The password mode displaying character. Returns L'\0' means the password mode is not activated.</returns>
					wchar_t							GetPasswordChar();
					/// <summary>
					/// Set the password mode displaying character.
					/// </summary>
					/// <param name="value">The password mode displaying character. Set to L'\0' to deactivate the password mode.</param>
					void							SetPasswordChar(wchar_t value);
				};
				
				/// <summary>
				/// Represents colors of a character.
				/// </summary>
				struct ColorItem
				{
					/// <summary>
					/// Text color.
					/// </summary>
					Color							text;
					/// <summary>
					/// Background color.
					/// </summary>
					Color							background;

					bool							operator==(const ColorItem& value)const { return text == value.text && background == value.background; }
					bool							operator!=(const ColorItem& value)const { return !(*this == value); }
				};
				
				/// <summary>
				/// Represents color entry in a color table. Use [M:vl.presentation.elements.GuiColorizedTextElement.GetColors] and [M:vl.presentation.elements.GuiColorizedTextElement.SetColors] to access the color table.
				/// </summary>
				struct ColorEntry
				{
					/// <summary>
					/// Colors in normal state.
					/// </summary>
					ColorItem						normal;
					/// <summary>
					/// Colors in focused and selected state.
					/// </summary>
					ColorItem						selectedFocused;
					/// <summary>
					/// Colors in not focused and selected state.
					/// </summary>
					ColorItem						selectedUnfocused;

					bool							operator==(const ColorEntry& value)const {return normal == value.normal && selectedFocused == value.selectedFocused && selectedUnfocused == value.selectedUnfocused;}
					bool							operator!=(const ColorEntry& value)const {return !(*this == value);}
				};
			}

/***********************************************************************
Colorized Plain Text (element)
***********************************************************************/
			
			/// <summary>
			/// Defines a text element with separate color configuration for each character.
			/// </summary>
			class GuiColorizedTextElement : public GuiElementBase<GuiColorizedTextElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiColorizedTextElement, L"ColorizedText");

				friend class text::TextLines;
				typedef collections::Array<text::ColorEntry>			ColorArray;
			public:
				/// <summary>
				/// An callback interface. Member functions will be called when colors or fonts of a <see cref="GuiColorizedTextElement"/> changed.
				/// </summary>
				class ICallback : public virtual IDescriptable, public Description<ICallback>
				{
				public:
					/// <summary>
					/// Called when the color table of a <see cref="GuiColorizedTextElement"/> changed.
					/// </summary>
					virtual void					ColorChanged()=0;
					/// <summary>
					/// Called when the font configuration of a <see cref="GuiColorizedTextElement"/> changed.
					/// </summary>
					virtual void					FontChanged()=0;
				};
			protected:
				ICallback*							callback;
				ColorArray							colors;
				FontProperties						font;
				Point								viewPosition;
				bool								isVisuallyEnabled;
				bool								isFocused;

				TextPos								caretBegin;
				TextPos								caretEnd;
				bool								caretVisible;
				Color								caretColor;

				text::TextLines						lines;

				GuiColorizedTextElement();
			public:
				/// <summary>
				/// Get the internal <see cref="text::TextLines"/> object that stores all characters and colors.
				/// </summary>
				/// <returns>The internal <see cref="text::TextLines"/> object.</returns>
				text::TextLines&					GetLines();
				/// <summary>
				/// Get the binded callback object.
				/// </summary>
				/// <returns>The binded callback object.</returns>
				ICallback*							GetCallback();
				/// <summary>
				/// Bind a callback object.
				/// </summary>
				/// <param name="value">The callback object to bind.</param>
				void								SetCallback(ICallback* value);
				
				/// <summary>
				/// Get the binded color table. Use <see cref="text::CharAtt::colorIndex"/> to use colors in this color table.
				/// </summary>
				/// <returns>The binded color table.</returns>
				const ColorArray&					GetColors();
				/// <summary>
				/// Bind a color table. Use <see cref="text::CharAtt::colorIndex"/> to use colors in this color table. <see cref="ICallback::ColorChanged"/> will be called.
				/// </summary>
				/// <param name="value">The color table to bind.</param>
				void								SetColors(const ColorArray& value);
				/// <summary>
				/// Reset color of all characters
				/// </summary>
				/// <param name="index">Color index of all characters.</param>
				void								ResetTextColorIndex(vint index);
				/// <summary>
				/// Get the font configuration for all characters.
				/// </summary>
				/// <returns>The font configuration.</returns>
				const FontProperties&				GetFont();
				/// <summary>
				/// Set the font configuration for all characters. <see cref="ICallback::FontChanged"/> will be called.
				/// </summary>
				/// <param name="value">The font configuration.</param>
				void								SetFont(const FontProperties& value);
				/// <summary>
				/// Get the password mode displaying character.
				/// </summary>
				/// <returns>The password mode displaying character. Returns L'\0' means the password mode is not activated.</returns>
				wchar_t								GetPasswordChar();
				/// <summary>
				/// Set the password mode displaying character.
				/// </summary>
				/// <param name="value">The password mode displaying character. Set to L'\0' to deactivate the password mode.</param>
				void								SetPasswordChar(wchar_t value);
				/// <summary>
				/// Get the left-top position of the visible bounds of characters.
				/// </summary>
				/// <returns>The left-top position of the visible bounds of characters.</returns>
				Point								GetViewPosition();
				/// <summary>
				/// Set the left-top position of the visible bounds of characters.
				/// </summary>
				/// <param name="value">The left-top position of the visible bounds of characters.</param>
				void								SetViewPosition(Point value);
				/// <summary>
				/// Get the enabling state.
				/// </summary>
				/// <returns>Returns true if the element will be rendered as an enabled element.</returns>
				bool								GetVisuallyEnabled();
				/// <summary>
				/// Set the enabling state.
				/// </summary>
				/// <param name="value">True if the element will be rendered as an enabled element.</param>
				void								SetVisuallyEnabled(bool value);
				/// <summary>
				/// Get the focused state.
				/// </summary>
				/// <returns>Returns true if the element will be rendered as a focused element.</returns>
				bool								GetFocused();
				/// <summary>
				/// Set the focused state.
				/// </summary>
				/// <param name="value">True if the element will be rendered as a focused element.</param>
				void								SetFocused(bool value);
				
				/// <summary>
				/// Get the begin position of the selection area.
				/// </summary>
				/// <returns>The begin position of the selection area.</returns>
				TextPos								GetCaretBegin();
				/// <summary>
				/// Set the begin position of the selection area.
				/// </summary>
				/// <param name="value">The begin position of the selection area.</param>
				void								SetCaretBegin(TextPos value);
				/// <summary>
				/// Get the end position of the selection area.
				/// </summary>
				/// <returns>The end position of the selection area.</returns>
				TextPos								GetCaretEnd();
				/// <summary>
				/// Set the end position of the selection area.
				/// </summary>
				/// <param name="value">The end position of the selection area.</param>
				void								SetCaretEnd(TextPos value);
				/// <summary>
				/// Get the caret visibility.
				/// </summary>
				/// <returns>Returns true if the caret will be rendered.</returns>
				bool								GetCaretVisible();
				/// <summary>
				/// Set the caret visibility.
				/// </summary>
				/// <param name="value">True if the caret will be rendered.</param>
				void								SetCaretVisible(bool value);
				/// <summary>
				/// Get the color of the caret.
				/// </summary>
				/// <returns>The color of the caret.</returns>
				Color								GetCaretColor();
				/// <summary>
				/// Set the color of the caret.
				/// </summary>
				/// <param name="value">The color of the caret.</param>
				void								SetCaretColor(Color value);
			};
		}
	}
}

#endif