/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows8 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWin8SCROLLABLESTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWin8SCROLLABLESTYLES

#include "GuiWin8StylesCommon.h"
#include "GuiWin8ButtonStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{

/***********************************************************************
Scroll
***********************************************************************/
			
			/// <summary>Scroll bar handle button style (Windows 8).</summary>
			class Win8ScrollHandleButtonStyle : public Win8ButtonStyleBase, public Description<Win8ScrollHandleButtonStyle>
			{
			protected:
				void										TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)override;
			public:
				/// <summary>Create the style.</summary>
				Win8ScrollHandleButtonStyle();
				~Win8ScrollHandleButtonStyle();
			};
			
			/// <summary>Scroll bar arrow button style (Windows 8).</summary>
			class Win8ScrollArrowButtonStyle : public Win8ButtonStyleBase, public Description<Win8ScrollArrowButtonStyle>
			{
			protected:
				elements::GuiPolygonElement*				arrowElement;

				void										TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)override;
				void										AfterApplyColors(const Win8ButtonColors& colors)override;
			public:
				/// <summary>Create the style.</summary>
				/// <param name="direction">The direction of the arrow.</param>
				/// <param name="increaseButton">Set to true to create the arrow for the increase button, otherwise for the decrease button.</param>
				Win8ScrollArrowButtonStyle(common_styles::CommonScrollStyle::Direction direction, bool increaseButton);
				~Win8ScrollArrowButtonStyle();
			};
			
			/// <summary>Scroll bar style (Windows 8).</summary>
			class Win8ScrollStyle : public common_styles::CommonScrollStyle, public Description<Win8ScrollStyle>
			{
			public:
				static const vint							DefaultSize=16;
				static const vint							ArrowSize=8;
			protected:
				controls::GuiButton::IStyleController*		CreateDecreaseButtonStyle(Direction direction)override;
				controls::GuiButton::IStyleController*		CreateIncreaseButtonStyle(Direction direction)override;
				controls::GuiButton::IStyleController*		CreateHandleButtonStyle(Direction direction)override;
				compositions::GuiBoundsComposition*			InstallBackground(compositions::GuiBoundsComposition* boundsComposition, Direction direction)override;
			public:
				/// <summary>Create the style using a specified direction.</summary>
				/// <param name="_direction">The specified direction</param>
				Win8ScrollStyle(Direction _direction);
				~Win8ScrollStyle();
			};
			
			/// <summary>Tracker (slide bar) style (Windows 8).</summary>
			class Win8TrackStyle : public common_styles::CommonTrackStyle, public Description<Win8TrackStyle>
			{
			public:
				static const vint							TrackThickness=4;
				static const vint							TrackPadding=6;
				static const vint							HandleLong=16;
				static const vint							HandleShort=10;

			protected:
				controls::GuiButton::IStyleController*		CreateHandleButtonStyle(Direction direction)override;
				void										InstallBackground(compositions::GuiGraphicsComposition* boundsComposition, Direction direction)override;
				void										InstallTrack(compositions::GuiGraphicsComposition* trackComposition, Direction direction)override;
			public:
				/// <summary>Create the style using a specified direction.</summary>
				/// <param name="_direction">The specified direction</param>
				Win8TrackStyle(Direction _direction);
				~Win8TrackStyle();
			};

			/// <summary>Progress bar style (Windows 8).</summary>
			class Win8ProgressBarStyle : public Object, public virtual controls::GuiScroll::IStyleController, public Description<Win8ProgressBarStyle>
			{
			protected:
				vint											totalSize;
				vint											pageSize;
				vint											position;
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			containerComposition;
				compositions::GuiPartialViewComposition*	progressComposition;

				void										UpdateProgressBar();
			public:
				Win8ProgressBarStyle();
				~Win8ProgressBarStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										SetCommandExecutor(controls::GuiScroll::ICommandExecutor* value)override;
				void										SetTotalSize(vint value)override;
				void										SetPageSize(vint value)override;
				void										SetPosition(vint value)override;
			};

/***********************************************************************
ScrollView
***********************************************************************/
			
			/// <summary>Scroll view style (Windows 8).</summary>
			class Win8ScrollViewProvider : public Object, public virtual controls::GuiScrollView::IStyleProvider, public Description<Win8ScrollViewProvider>
			{
			public:
				/// <summary>Create the style.</summary>
				Win8ScrollViewProvider();
				~Win8ScrollViewProvider();

				void										AssociateStyleController(controls::GuiControl::IStyleController* controller)override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;

				controls::GuiScroll::IStyleController*		CreateHorizontalScrollStyle()override;
				controls::GuiScroll::IStyleController*		CreateVerticalScrollStyle()override;
				vint										GetDefaultScrollSize()override;
				compositions::GuiGraphicsComposition*		InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override;
			};

/***********************************************************************
TextBox
***********************************************************************/
			
			class Win8TextBoxBackground : public Object, public Description<Win8TextBoxBackground>
			{
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Win8TextBoxColors, Win8TextBoxBackground)
					
				elements::GuiSolidBorderElement*			borderElement;
				elements::GuiSolidBackgroundElement*		backgroundElement;
				compositions::GuiGraphicsComposition*		focusableComposition;
				bool										isMouseEnter;
				bool										isFocused;
				bool										isVisuallyEnabled;
				Ptr<TransferringAnimation>					transferringAnimation;
				controls::GuiControl::IStyleController*		styleController;
				elements::GuiColorizedTextElement*			textElement;

				void										UpdateStyle();
				void										Apply(const Win8TextBoxColors& colors);

				void										OnBoundsMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnBoundsMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnBoundsGotFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnBoundsLostFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				Win8TextBoxBackground();
				~Win8TextBoxBackground();
				
				void										AssociateStyleController(controls::GuiControl::IStyleController* controller);
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value);
				void										SetVisuallyEnabled(bool value);
				compositions::GuiGraphicsComposition*		InstallBackground(compositions::GuiBoundsComposition* boundsComposition);
				void										InitializeTextElement(elements::GuiColorizedTextElement* _textElement);
			};
			
			/// <summary>Multiline text box style (Windows 8).</summary>
			class Win8MultilineTextBoxProvider : public Win8ScrollViewProvider, public Description<Win8MultilineTextBoxProvider>
			{
			protected:
				Win8TextBoxBackground						background;
				controls::GuiControl::IStyleController*		styleController;
			public:
				/// <summary>Create the style.</summary>
				Win8MultilineTextBoxProvider();
				~Win8MultilineTextBoxProvider();
				
				void										AssociateStyleController(controls::GuiControl::IStyleController* controller)override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetVisuallyEnabled(bool value)override;
				compositions::GuiGraphicsComposition*		InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override;
			};
			
			/// <summary>Singleline text box style (Windows 8).</summary>
			class Win8SinglelineTextBoxProvider : public Object, public virtual controls::GuiSinglelineTextBox::IStyleProvider, public Description<Win8SinglelineTextBoxProvider>
			{
			protected:
				Win8TextBoxBackground						background;
				controls::GuiControl::IStyleController*		styleController;
			public:
				/// <summary>Create the style.</summary>
				Win8SinglelineTextBoxProvider();
				~Win8SinglelineTextBoxProvider();

				void										AssociateStyleController(controls::GuiControl::IStyleController* controller)override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				compositions::GuiGraphicsComposition*		InstallBackground(compositions::GuiBoundsComposition* boundsComposition)override;
			};
			
#pragma warning(push)
#pragma warning(disable:4250)
			/// <summary>Document viewer style (Windows 8).</summary>
			class Win8DocumentViewerStyle : public Win8MultilineTextBoxProvider, public virtual controls::GuiDocumentViewer::IStyleProvider, public Description<Win8DocumentViewerStyle>
			{
			public:
				/// <summary>Create the style.</summary>
				Win8DocumentViewerStyle();
				~Win8DocumentViewerStyle();

				Ptr<DocumentModel>							GetBaselineDocument()override;
				Color										GetCaretColor()override;
			};

			/// <summary>Document label style (Windows 8).</summary>
			class Win8DocumentLabelStyle : public controls::GuiControl::EmptyStyleController, public virtual controls::GuiDocumentLabel::IStyleController, public Description<Win8DocumentLabelStyle>
			{
			public:
				/// <summary>Create the style.</summary>
				Win8DocumentLabelStyle();
				~Win8DocumentLabelStyle();

				Ptr<DocumentModel>							GetBaselineDocument()override;
			};

			/// <summary>Document label style (Windows 8).</summary>
			class Win8DocumentTextBoxStyle : public virtual controls::GuiDocumentLabel::IStyleController, public Description<Win8DocumentTextBoxStyle>
			{
			protected:
				Win8TextBoxBackground						background;
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiGraphicsComposition*		containerComposition;

			public:
				/// <summary>Create the style.</summary>
				Win8DocumentTextBoxStyle();
				~Win8DocumentTextBoxStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				Ptr<DocumentModel>							GetBaselineDocument()override;
			};
#pragma warning(pop)
		}
	}
}

#endif