/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUITEXTCONTROLS

#include "GuiTextCommonInterface.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
MultilineTextBox
***********************************************************************/

			/// <summary>Multiline text box control.</summary>
			class GuiMultilineTextBox : public GuiScrollView, public GuiTextBoxCommonInterface, public Description<GuiMultilineTextBox>
			{
			public:
				static const vint							TextMargin=3;

				class CommandExecutor : public Object, public ITextBoxCommandExecutor
				{
				protected:
					GuiMultilineTextBox*					textBox;

				public:
					CommandExecutor(GuiMultilineTextBox* _textBox);
					~CommandExecutor();

					void									UnsafeSetText(const WString& value)override;
				};

			protected:
				class TextElementOperatorCallback : public GuiTextBoxCommonInterface::DefaultCallback, public Description<TextElementOperatorCallback>
				{
				protected:
					GuiMultilineTextBox*					textControl;
					IStyleController*						textController;
				public:
					TextElementOperatorCallback(GuiMultilineTextBox* _textControl);

					void									AfterModify(TextPos originalStart, TextPos originalEnd, const WString& originalText, TextPos inputStart, TextPos inputEnd, const WString& inputText)override;
					void									ScrollToView(Point point)override;
					vint									GetTextMargin()override;
				};

			protected:
				Ptr<TextElementOperatorCallback>			callback;
				Ptr<CommandExecutor>						commandExecutor;
				IStyleController*							styleController;

				void										CalculateViewAndSetScroll();
				void										OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)override;
				Size										QueryFullSize()override;
				void										UpdateView(Rect viewBounds)override;
				void										OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="styleController">The style controller.</param>
				GuiMultilineTextBox(IStyleController* _styleController);
				~GuiMultilineTextBox();

				const WString&								GetText()override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
			};

/***********************************************************************
SinglelineTextBox
***********************************************************************/
			
			/// <summary>Single text box control.</summary>
			class GuiSinglelineTextBox : public GuiControl, public GuiTextBoxCommonInterface, public Description<GuiSinglelineTextBox>
			{
			public:
				static const vint							TextMargin=3;
				
			protected:
				class TextElementOperatorCallback : public GuiTextBoxCommonInterface::DefaultCallback, public Description<TextElementOperatorCallback>
				{
				public:
					TextElementOperatorCallback(GuiSinglelineTextBox* _textControl);

					bool									BeforeModify(TextPos start, TextPos end, const WString& originalText, WString& inputText)override;
					void									AfterModify(TextPos originalStart, TextPos originalEnd, const WString& originalText, TextPos inputStart, TextPos inputEnd, const WString& inputText)override;
					void									ScrollToView(Point point)override;
					vint									GetTextMargin()override;
				};

				class CommandExecutor : public Object, public ITextBoxCommandExecutor
				{
				protected:
					GuiSinglelineTextBox*					textBox;

				public:
					CommandExecutor(GuiSinglelineTextBox* _textBox);
					~CommandExecutor();

					void									UnsafeSetText(const WString& value)override;
				};
			protected:
				Ptr<TextElementOperatorCallback>			callback;
				Ptr<CommandExecutor>						commandExecutor;
				IStyleController*							styleController;
				
				void										OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)override;
				void										OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="styleController">The style controller.</param>
				GuiSinglelineTextBox(GuiSinglelineTextBox::IStyleController* _styleController);
				~GuiSinglelineTextBox();

				const WString&								GetText()override;
				void										SetFont(const FontProperties& value)override;
				/// <summary>
				/// Get the password mode displaying character.
				/// </summary>
				/// <returns>The password mode displaying character. Returns L'\0' means the password mode is not activated.</returns>
				wchar_t										GetPasswordChar();
				/// <summary>
				/// Set the password mode displaying character.
				/// </summary>
				/// <param name="value">The password mode displaying character. Set to L'\0' to deactivate the password mode.</param>
				void										SetPasswordChar(wchar_t value);
			};
		}
	}
}

#endif