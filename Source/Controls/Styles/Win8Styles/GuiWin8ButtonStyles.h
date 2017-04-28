/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows8 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN8BUTTONSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN8BUTTONSTYLES

#include "GuiWin8StylesCommon.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{

/***********************************************************************
Button
***********************************************************************/
			
			/// <summary>The base class of all button style implementations. (Windows 8)</summary>
			class Win8ButtonStyleBase : public Object, public virtual controls::GuiSelectableButton::IStyleController, public Description<Win8ButtonStyleBase>
			{
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Win8ButtonColors, Win8ButtonStyleBase)

				Win8ButtonElements							elements;
				Ptr<TransferringAnimation>					transferringAnimation;
				controls::ButtonState						controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;
				bool										transparentWhenInactive;
				bool										transparentWhenDisabled;

				virtual void								TransferInternal(controls::ButtonState value, bool enabled, bool selected)=0;
				virtual void								AfterApplyColors(const Win8ButtonColors& colors);
			public:
				/// <summary>Create the style.</summary>
				/// <param name="initialColor">Set to true to set the initial colors of all components for a button.</param>
				/// <param name="horizontal">Horizontal alignment for text.</param>
				/// <param name="vertical">Vertical alignment for text.</param>
				Win8ButtonStyleBase(const Win8ButtonColors& initialColor, Alignment horizontal, Alignment vertical);
				~Win8ButtonStyleBase();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										SetSelected(bool value)override;
				void										Transfer(controls::ButtonState value)override;

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
			
			/// <summary>Button style (Windows 8).</summary>
			class Win8ButtonStyle : public Win8ButtonStyleBase, public Description<Win8ButtonStyle>
			{
			protected:
				void										TransferInternal(controls::ButtonState value, bool enabled, bool selected)override;
			public:
				/// <summary>Create the style.</summary>
				Win8ButtonStyle();
				~Win8ButtonStyle();
			};
			
			/// <summary>Check box style (Windows 8).</summary>
			class Win8CheckBoxStyle : public Object, public virtual controls::GuiSelectableButton::IStyleController, public Description<Win8CheckBoxStyle>
			{
			public:
				/// <summary>Bullet style.</summary>
				enum BulletStyle
				{
					/// <summary>Check box bullet.</summary>
					CheckBox,
					/// <summary>Radio button bullet.</summary>
					RadioButton,
				};
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Win8ButtonColors, Win8CheckBoxStyle)

				Win8CheckedButtonElements					elements;
				Ptr<TransferringAnimation>					transferringAnimation;
				controls::ButtonState						controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;

				void										TransferInternal(controls::ButtonState value, bool enabled, bool selected);
			public:
				/// <summary>Create the style.</summary>
				/// <param name="bulletStyle">The bullet style.</param>
				/// <param name="backgroundVisible">Set to true to make the background visible.</param>
				Win8CheckBoxStyle(BulletStyle bulletStyle, bool backgroundVisible=false);
				~Win8CheckBoxStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										SetSelected(bool value)override;
				void										Transfer(controls::ButtonState value)override;
			};
		}
	}
}

#endif