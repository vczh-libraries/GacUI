/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows7 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7BUTTONSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7BUTTONSTYLES

#include "GuiWin7StylesCommon.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{

/***********************************************************************
Button
***********************************************************************/
			
			/// <summary>The base class of all button style implementations. (Windows 7)</summary>
			class Win7ButtonStyleBase : public Object, public virtual controls::GuiSelectableButton::IStyleController, public Description<Win7ButtonStyleBase>
			{
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Win7ButtonColors, Win7ButtonStyleBase)

				Win7ButtonElements							elements;
				Ptr<TransferringAnimation>					transferringAnimation;
				controls::GuiButton::ControlState			controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;
				bool										transparentWhenInactive;
				bool										transparentWhenDisabled;

				virtual void								TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)=0;
				virtual void								AfterApplyColors(const Win7ButtonColors& colors);
			public:
				/// <summary>Create the style.</summary>
				/// <param name="verticalGradient">Set to true to have a vertical gradient background.</param>
				/// <param name="roundBorder">Set to true to have a round border</param>
				/// <param name="initialColor">Set to true to set the initial colors of all components for a button.</param>
				/// <param name="horizontal">Horizontal alignment for text.</param>
				/// <param name="vertical">Vertical alignment for text.</param>
				Win7ButtonStyleBase(bool verticalGradient, bool roundBorder, const Win7ButtonColors& initialColor, Alignment horizontal, Alignment vertical);
				~Win7ButtonStyleBase();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										SetSelected(bool value)override;
				void										Transfer(controls::GuiButton::ControlState value)override;

				/// <summary>Get the transparent style for the inactive state.</summary>
				/// <returns>Returns true if the background is not transparent for the inactive state.</returns>
				bool										GetTransparentWhenInactive();
				/// <summary>Set the transparent style for the inactive state.</summary>
				/// <param name="value">Set to troe true to make the background not transparent for the inactive state.</param>
				void										SetTransparentWhenInactive(bool value);
				/// <summary>Get the transparent style for the disabled state.</summary>
				/// <returns>Returns true if the background is not transparent for the disabled state.</returns>
				bool										GetTransparentWhenDisabled();
				/// <summary>Set the transparent style for the inactive state.</summary>
				/// <param name="value">Set to troe true to make the background not transparent for the disabled state.</param>
				void										SetTransparentWhenDisabled(bool value);
				/// <summary>Get the automatically size changing state of the button.</summary>
				/// <returns>Returns true if the style automatically changes its size if the text changed.</returns>
				bool										GetAutoSizeForText();
				/// <summary>Set the automatically size changing state of the button.</summary>
				/// <param name="value">Set to true to make the style automatically changes its size if the text changed.</param>
				void										SetAutoSizeForText(bool value);
			};
			
			/// <summary>Button style (Windows 7).</summary>
			class Win7ButtonStyle : public Win7ButtonStyleBase, public Description<Win7ButtonStyle>
			{
			protected:
				void										TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected)override;
			public:
				/// <summary>Create the style.</summary>
				/// <param name="verticalGradient">Set to true to have a vertical gradient background.</param>
				Win7ButtonStyle(bool verticalGradient=true);
				~Win7ButtonStyle();
			};
			
			/// <summary>Check box style (Windows 7).</summary>
			class Win7CheckBoxStyle : public Object, public virtual controls::GuiSelectableButton::IStyleController, public Description<Win7CheckBoxStyle>
			{
			public:
				/// <summary>Bullet style.</summary>
				enum BulletStyle
				{
					/// <summary>[T:vl.presentation.win7.Win7CheckBoxStyle.BulletStyle]Check box bullet.</summary>
					CheckBox,
					/// <summary>[T:vl.presentation.win7.Win7CheckBoxStyle.BulletStyle]Radio button bullet.</summary>
					RadioButton,
				};
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Win7ButtonColors, Win7CheckBoxStyle)

				Win7CheckedButtonElements					elements;
				Ptr<TransferringAnimation>					transferringAnimation;
				controls::GuiButton::ControlState			controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;

				void										TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected);
			public:
				/// <summary>Create the style.</summary>
				/// <param name="bulletStyle">The bullet style.</param>
				/// <param name="backgroundVisible">Set to true to make the background visible.</param>
				Win7CheckBoxStyle(BulletStyle bulletStyle, bool backgroundVisible=false);
				~Win7CheckBoxStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										SetSelected(bool value)override;
				void										Transfer(controls::GuiButton::ControlState value)override;
			};
		}
	}
}

#endif