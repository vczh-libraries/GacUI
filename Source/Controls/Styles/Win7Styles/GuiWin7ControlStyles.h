/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows7 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7CONTROLSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7CONTROLSTYLES

#include "GuiWin7StylesCommon.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{

/***********************************************************************
Container
***********************************************************************/

			/// <summary>Empty style. Nothing but a color filled the whole control.</summary>
			class Win7EmptyStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win7EmptyStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
			public:
				/// <summary>Create the style with a specified color.</summary>
				/// <param name="color">The specified color.</param>
				Win7EmptyStyle(Color color);
				~Win7EmptyStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Window style (Windows 7). Using the Windows 7 window background color to fill the whole control.</summary>
			class Win7WindowStyle : public virtual controls::GuiWindow::DefaultBehaviorStyleController, public Description<Win7WindowStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
			public:
				/// <summary>Create the style.</summary>
				Win7WindowStyle();
				~Win7WindowStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Tooltip style (Windows 7). Using the Windows 7 tooltip background color to fill the whole control.</summary>
			class Win7TooltipStyle : public virtual controls::GuiWindow::DefaultBehaviorStyleController, public Description<Win7TooltipStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			containerComposition;
			public:
				/// <summary>Create the style.</summary>
				Win7TooltipStyle();
				~Win7TooltipStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Label style (Windows 7). A label contains a text, and the label will automatically resize to fit the text.</summary>
			class Win7LabelStyle : public Object, public virtual controls::GuiLabel::IStyleController, public Description<Win7LabelStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				elements::GuiSolidLabelElement*				textElement;
			public:
				/// <summary>Create the style.</summary>
				/// <param name="forShortcutKey">Set to true to create a style for displaying shortcut key.</param>
				Win7LabelStyle(bool forShortcutKey);
				~Win7LabelStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				Color										GetDefaultTextColor()override;
				void										SetTextColor(Color value)override;
			};
			
			/// <summary>Group box style (Windows 7).</summary>
			class Win7GroupBoxStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win7GroupBoxStyle>
			{
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Color, Win7GroupBoxStyle)

				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			sinkBorderComposition;
				compositions::GuiBoundsComposition*			raisedBorderComposition;
				compositions::GuiBoundsComposition*			textComposition;
				compositions::GuiBoundsComposition*			textBackgroundComposition;
				compositions::GuiBoundsComposition*			containerComposition;
				elements::GuiSolidLabelElement*				textElement;
				Ptr<TransferringAnimation>					transferringAnimation;

				void										SetMargins(vint fontSize);
			public:
				/// <summary>Create the style.</summary>
				Win7GroupBoxStyle();
				~Win7GroupBoxStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Date Picker (Windows 7)</summary>
			class Win7DatePickerStyle : public Object, public virtual controls::GuiDatePicker::IStyleProvider, public Description<Win7DatePickerStyle>
			{
			public:
				/// <summary>Create the style.</summary>
				Win7DatePickerStyle();
				~Win7DatePickerStyle();

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