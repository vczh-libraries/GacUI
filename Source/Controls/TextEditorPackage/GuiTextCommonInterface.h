/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTCOMMONINTERFACE
#define VCZH_PRESENTATION_CONTROLS_GUITEXTCOMMONINTERFACE

#include "EditorCallback/GuiTextGeneralOperations.h"
#include "EditorCallback/GuiTextColorizer.h"
#include "EditorCallback/GuiTextAutoComplete.h"
#include "EditorCallback/GuiTextUndoRedo.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class GuiShortcutKeyManager;
		}

		namespace controls
		{

/***********************************************************************
Common Interface
***********************************************************************/

			/// <summary>Common interface for text box controls.</summary>
			class GuiTextBoxCommonInterface abstract : public Description<GuiTextBoxCommonInterface>
			{
				typedef collections::Array<elements::text::ColorEntry>			ColorArray;
			protected:
				class ICallback : public virtual IDescriptable, public Description<ICallback>
				{
				public:
					virtual TextPos									GetLeftWord(TextPos pos)=0;
					virtual TextPos									GetRightWord(TextPos pos)=0;
					virtual void									GetWord(TextPos pos, TextPos& begin, TextPos& end)=0;
					virtual vint									GetPageRows()=0;
					virtual bool									BeforeModify(TextPos start, TextPos end, const WString& originalText, WString& inputText)=0;
					virtual void									AfterModify(TextPos originalStart, TextPos originalEnd, const WString& originalText, TextPos inputStart, TextPos inputEnd, const WString& inputText)=0;
					virtual void									ScrollToView(Point point)=0;
					virtual vint									GetTextMargin()=0;
				};

				class DefaultCallback : public Object, public ICallback, public Description<DefaultCallback>
				{
				protected:
					elements::GuiColorizedTextElement*				textElement;
					compositions::GuiGraphicsComposition*			textComposition;
					bool											readonly;
				public:
					DefaultCallback(elements::GuiColorizedTextElement* _textElement, compositions::GuiGraphicsComposition* _textComposition);
					~DefaultCallback();

					TextPos											GetLeftWord(TextPos pos)override;
					TextPos											GetRightWord(TextPos pos)override;
					void											GetWord(TextPos pos, TextPos& begin, TextPos& end)override;
					vint											GetPageRows()override;
					bool											BeforeModify(TextPos start, TextPos end, const WString& originalText, WString& inputText)override;
				};
			private:
				elements::GuiColorizedTextElement*					textElement;
				compositions::GuiGraphicsComposition*				textComposition;
				vuint												editVersion;
				GuiControl*											textControl;
				ICallback*											callback;
				bool												dragging;
				bool												readonly;
				Ptr<GuiTextBoxColorizerBase>						colorizer;
				Ptr<GuiTextBoxAutoCompleteBase>						autoComplete;
				Ptr<GuiTextBoxUndoRedoProcessor>					undoRedoProcessor;

				bool												filledDefaultColors = false;
				ColorArray											defaultColors;

				SpinLock											elementModifyLock;
				collections::List<Ptr<ICommonTextEditCallback>>		textEditCallbacks;
				Ptr<compositions::GuiShortcutKeyManager>			internalShortcutKeyManager;
				bool												preventEnterDueToAutoComplete;

				void												InvokeUndoRedoChanged();
				void												InvokeModifiedChanged();
				void												UpdateCaretPoint();
				void												Move(TextPos pos, bool shift);
				void												Modify(TextPos start, TextPos end, const WString& input, bool asKeyInput);
				bool												ProcessKey(VKEY code, bool shift, bool ctrl);
					
				void												OnGotFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void												OnLostFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void												OnCaretNotify(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				void												OnLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void												OnLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void												OnMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void												OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);
				void												OnCharInput(compositions::GuiGraphicsComposition* sender, compositions::GuiCharEventArgs& arguments);

			protected:

				void												Install(elements::GuiColorizedTextElement* _textElement, compositions::GuiGraphicsComposition* _textComposition, GuiControl* _textControl, compositions::GuiGraphicsComposition* eventComposition, compositions::GuiGraphicsComposition* focusableComposition);
				ICallback*											GetCallback();
				void												SetCallback(ICallback* value);
				bool												AttachTextEditCallback(Ptr<ICommonTextEditCallback> value);
				bool												DetachTextEditCallback(Ptr<ICommonTextEditCallback> value);
				void												AddShortcutCommand(VKEY key, const Func<void()>& eventHandler);
				elements::GuiColorizedTextElement*					GetTextElement();
				void												UnsafeSetText(const WString& value);

			public:
				GuiTextBoxCommonInterface();
				~GuiTextBoxCommonInterface();

				/// <summary>Selection changed event.</summary>
				compositions::GuiNotifyEvent						SelectionChanged;
				/// <summary>Undo redo status changed event.</summary>
				compositions::GuiNotifyEvent						UndoRedoChanged;
				/// <summary>Modified status changed event.</summary>
				compositions::GuiNotifyEvent						ModifiedChanged;

				//================ clipboard operations

				/// <summary>Test can the selection be cut.</summary>
				/// <returns>Returns true if the selection can be cut.</returns>
				bool												CanCut();
				/// <summary>Test can the selection be copied.</summary>
				/// <returns>Returns true if the selection can be cut.</returns>
				bool												CanCopy();
				/// <summary>Test can the content in the clipboard be pasted.</summary>
				/// <returns>Returns true if the content in the clipboard can be pasted.</returns>
				bool												CanPaste();
				/// <summary>Cut the selection text.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												Cut();
				/// <summary>Copy the selection text.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												Copy();
				/// <summary>Paste the content from the clipboard and replace the selected text.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												Paste();

				//================ editing control

				/// <summary>Get the readonly mode.</summary>
				/// <returns>Returns true if the text box is readonly.</returns>
				bool												GetReadonly();
				/// <summary>Set the readonly mode.</summary>
				/// <param name="value">Set to true to make the texg box readonly.</param>
				void												SetReadonly(bool value);

				//================ text operations

				/// <summary>Select all text.</summary>
				void												SelectAll();
				/// <summary>Select (highlight) a part of text.</summary>
				/// <param name="begin">The begin position.</param>
				/// <param name="end">The end position. This is also the caret position.</param>
				void												Select(TextPos begin, TextPos end);
				/// <summary>Get the selected text.</summary>
				/// <returns>The selected text.</returns>
				WString												GetSelectionText();
				/// <summary>Set the selected text.</summary>
				/// <param name="value">The selected text.</param>
				void												SetSelectionText(const WString& value);
				/// <summary>Set the selected text and let to text box treat this changing as input by the keyboard.</summary>
				/// <param name="value">The selected text.</param>
				void												SetSelectionTextAsKeyInput(const WString& value);
				
				/// <summary>Get the text from a specified row number.</summary>
				/// <returns>The text from a specified row number.</returns>
				/// <param name="row">The specified row number.</param>
				WString												GetRowText(vint row);
				/// <summary>Get the number of rows.</summary>
				/// <returns>The number of rows.</returns>
				vint												GetRowCount();
				/// <summary>Get the text from a specified range.</summary>
				/// <returns>The text from a specified range.</returns>
				/// <param name="start">The specified start position.</param>
				/// <param name="end">The specified end position.</param>
				WString												GetFragmentText(TextPos start, TextPos end);

				/// <summary>Get the begin text position of the selection.</summary>
				/// <returns>The begin text position of the selection.</returns>
				TextPos												GetCaretBegin();
				/// <summary>Get the end text position of the selection.</summary>
				/// <returns>The end text position of the selection.</returns>
				TextPos												GetCaretEnd();
				/// <summary>Get the left-top text position of the selection.</summary>
				/// <returns>The left-top text position of the selection.</returns>
				TextPos												GetCaretSmall();
				/// <summary>Get the right-bottom text position of the selection.</summary>
				/// <returns>The right-bottom text position of the selection.</returns>
				TextPos												GetCaretLarge();

				//================ position query

				/// <summary>Get the width of a row.</summary>
				/// <returns>The width of a row in pixel.</returns>
				/// <param name="row">The specified row number</param>
				vint												GetRowWidth(vint row);
				/// <summary>Get the height of a row.</summary>
				/// <returns>The height of a row in pixel.</returns>
				vint												GetRowHeight();
				/// <summary>Get the maximum width of all rows.</summary>
				/// <returns>The maximum width of all rows.</returns>
				vint												GetMaxWidth();
				/// <summary>Get the total height of all rows.</summary>
				/// <returns>The total height of all rows.</returns>
				vint												GetMaxHeight();
				/// <summary>Get the nearest position of a character from a specified display position.</summary>
				/// <returns>Get the nearest position of a character.</returns>
				/// <param name="point">The specified display position.</param>
				TextPos												GetTextPosFromPoint(Point point);
				/// <summary>Get the display position of a character from a specified text position.</summary>
				/// <returns>Get the display position of a character.</returns>
				/// <param name="pos">The specified text position.</param>
				Point												GetPointFromTextPos(TextPos pos);
				/// <summary>Get the display bounds of a character from a specified text position.</summary>
				/// <returns>Get the display bounds of a character.</returns>
				/// <param name="pos">The specified text position.</param>
				Rect												GetRectFromTextPos(TextPos pos);
				/// <summary>Get the nearest text position from a specified display position.</summary>
				/// <returns>Get the nearest text position.</returns>
				/// <param name="point">The specified display position.</param>
				TextPos												GetNearestTextPos(Point point);

				//================ colorizing

				/// <summary>Get the current colorizer.</summary>
				/// <returns>The current colorizer.</returns>
				Ptr<GuiTextBoxColorizerBase>						GetColorizer();
				/// <summary>Set the current colorizer.</summary>
				/// <param name="value">The current colorizer.</param>
				void												SetColorizer(Ptr<GuiTextBoxColorizerBase> value);

				//================ auto complete

				/// <summary>Get the current auto complete controller.</summary>
				/// <returns>The current auto complete controller.</returns>
				Ptr<GuiTextBoxAutoCompleteBase>						GetAutoComplete();
				/// <summary>Set the current auto complete controller.</summary>
				/// <param name="value">The current auto complete controller.</param>
				void												SetAutoComplete(Ptr<GuiTextBoxAutoCompleteBase> value);

				//================ undo redo control

				/// <summary>Get the current edit version. When the control is modified, the edit version increased. Calling <see cref="NotifyModificationSaved"/> will not reset the edit version.</summary>
				/// <returns>The current edit version.</returns>
				vuint												GetEditVersion();
				/// <summary>Test can undo.</summary>
				/// <returns>Returns true if this action can be performed.</returns>
				bool												CanUndo();
				/// <summary>Test can redo.</summary>
				/// <returns>Returns true if this action can be performed.</returns>
				bool												CanRedo();
				/// <summary>Clear all undo and redo information.</summary>
				void												ClearUndoRedo();
				/// <summary>Test is the text box modified.</summary>
				/// <returns>Returns true if the text box is modified.</returns>
				bool												GetModified();
				/// <summary>Notify the text box that the current status is considered saved.</summary>
				void												NotifyModificationSaved();
				/// <summary>Perform the undo action.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												Undo();
				/// <summary>Perform the redo action.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												Redo();
			};
		}
	}
}

#endif