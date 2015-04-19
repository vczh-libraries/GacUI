/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows8 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWin8CONTROLSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWin8CONTROLSTYLES

#include "GuiWin8StylesCommon.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{

/***********************************************************************
Container
***********************************************************************/

			/// <summary>Empty style. Nothing but a color filled the whole control.</summary>
			class Win8EmptyStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win8EmptyStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
			public:
				/// <summary>Create the style with a specified color.</summary>
				/// <param name="color">The specified color.</param>
				Win8EmptyStyle(Color color);
				~Win8EmptyStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Window style (Windows 8). Using the Windows 8 window background color to fill the whold control</summary>
			class Win8WindowStyle : public virtual controls::GuiWindow::DefaultBehaviorStyleController, public Description<Win8WindowStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
			public:
				/// <summary>Create the style.</summary>
				Win8WindowStyle();
				~Win8WindowStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Tooltip style (Windows 8). Using the Windows 8 tooltip background color to fill the whole control.</summary>
			class Win8TooltipStyle : public virtual controls::GuiWindow::DefaultBehaviorStyleController, public Description<Win8TooltipStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			containerComposition;
			public:
				/// <summary>Create the style.</summary>
				Win8TooltipStyle();
				~Win8TooltipStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Label style (Windows 8). A label contains a text, and the label will automatically resize to fit the text.</summary>
			class Win8LabelStyle : public Object, public virtual controls::GuiLabel::IStyleController, public Description<Win8LabelStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				elements::GuiSolidLabelElement*				textElement;
			public:
				/// <summary>Create the style.</summary>
				/// <param name="forShortcutKey">Set to true to create a style for displaying shortcut key.</param>
				Win8LabelStyle(bool forShortcutKey);
				~Win8LabelStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				Color										GetDefaultTextColor()override;
				void										SetTextColor(Color value)override;
			};
			
			/// <summary>Group box style (Windows 8).</summary>
			class Win8GroupBoxStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win8GroupBoxStyle>
			{
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Color, Win8GroupBoxStyle)

				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			borderComposition;
				compositions::GuiBoundsComposition*			textComposition;
				compositions::GuiBoundsComposition*			textBackgroundComposition;
				compositions::GuiBoundsComposition*			containerComposition;
				elements::GuiSolidLabelElement*				textElement;
				Ptr<TransferringAnimation>					transferringAnimation;

				void										SetMargins(vint fontSize);
			public:
				/// <summary>Create the style.</summary>
				Win8GroupBoxStyle();
				~Win8GroupBoxStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Date Picker (Windows 8)</summary>
			class Win8DatePickerStyle : public Object, public virtual controls::GuiDatePicker::IStyleProvider, public Description<Win8DatePickerStyle>
			{
			public:
				/// <summary>Create the style.</summary>
				Win8DatePickerStyle();
				~Win8DatePickerStyle();

				void													AssociateStyleController(controls::GuiControl::IStyleController* controller)override;
				void													SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void													SetText(const WString& value)override;
				void													SetFont(const FontProperties& value)override;
				void													SetVisuallyEnabled(bool value)override;

				controls::GuiSelectableButton::IStyleController*		CreateDateButtonStyle()override;
				controls::GuiTextList*									CreateTextList()override;
				controls::GuiComboBoxListControl::IStyleController*		CreateComboBoxStyle()override;

				Color													GetBackgroundColor()override;
				Color													GetPrimaryTextColor()override;
				Color													GetSecondaryTextColor()override;
			};
		}
	}
}

#endif