/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUITEXTCONTROLS

#include "GuiTextCommonInterface.h"
#include "../GuiContainerControls.h"

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
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(MultilineTextBoxTemplate, GuiScrollView)
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
				public:
					TextElementOperatorCallback(GuiMultilineTextBox* _textControl);

					void									AfterModify(TextPos originalStart, TextPos originalEnd, const WString& originalText, TextPos inputStart, TextPos inputEnd, const WString& inputText)override;
					void									ScrollToView(Point point)override;
					vint									GetTextMargin()override;
				};

			protected:
				Ptr<TextElementOperatorCallback>			callback;
				Ptr<CommandExecutor>						commandExecutor;
				elements::GuiColorizedTextElement*			textElement = nullptr;
				compositions::GuiBoundsComposition*			textComposition = nullptr;

				void										UpdateVisuallyEnabled()override;
				void										UpdateDisplayFont()override;
				void										OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)override;
				Size										QueryFullSize()override;
				void										UpdateView(Rect viewBounds)override;
				void										CalculateViewAndSetScroll();
				void										OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiMultilineTextBox(theme::ThemeName themeName);
				~GuiMultilineTextBox();

				const WString&								GetText()override;
				void										SetText(const WString& value)override;
			};

/***********************************************************************
SinglelineTextBox
***********************************************************************/
			
			/// <summary>Single text box control.</summary>
			class GuiSinglelineTextBox : public GuiControl, public GuiTextBoxCommonInterface, public Description<GuiSinglelineTextBox>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(SinglelineTextBoxTemplate, GuiControl)
			public:
				static const vint							TextMargin=2;
				
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

			protected:
				Ptr<TextElementOperatorCallback>			callback;
				elements::GuiColorizedTextElement*			textElement = nullptr;
				compositions::GuiTableComposition*			textCompositionTable = nullptr;
				compositions::GuiCellComposition*			textComposition = nullptr;
				
				void										UpdateVisuallyEnabled()override;
				void										UpdateDisplayFont()override;
				void										RearrangeTextElement();
				void										OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)override;
				void										OnBoundsMouseButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiSinglelineTextBox(theme::ThemeName themeName);
				~GuiSinglelineTextBox();

				const WString&								GetText()override;
				void										SetText(const WString& value)override;

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