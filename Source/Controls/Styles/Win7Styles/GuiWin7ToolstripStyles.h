/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows7 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7TOOLSTRIPSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN7STYLES_GUIWIN7TOOLSTRIPSTYLES

#include "GuiWin7MenuStyles.h"
#include "GuiWin7ControlStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{

/***********************************************************************
Toolstrip Button
***********************************************************************/

			/// <summary>Toolstrip toolbar style (Windows 7). Using the Windows 7 window background color to fill the whold control</summary>
			class Win7ToolstripToolBarStyle : public Win7EmptyStyle, public Description<Win7ToolstripToolBarStyle>
			{
			public:
				/// <summary>Create the style.</summary>
				Win7ToolstripToolBarStyle();
				~Win7ToolstripToolBarStyle();
			};

			/// <summary>Toolstrip split button dropdown style (Windows 7).</summary>
			class Win7ToolstripButtonDropdownStyle : public Object, public virtual controls::GuiButton::IStyleController, public Description<Win7ToolstripButtonDropdownStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			splitterComposition;
				compositions::GuiBoundsComposition*			containerComposition;
				bool										isVisuallyEnabled;
				controls::GuiButton::ControlState			controlState;

				virtual void								TransferInternal(controls::GuiButton::ControlState value, bool enabled);
			public:
				/// <summary>Create the style.</summary>
				Win7ToolstripButtonDropdownStyle();
				~Win7ToolstripButtonDropdownStyle();
				
				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										Transfer(controls::GuiButton::ControlState value)override;
			};

			/// <summary>Toolstrip button style (Windows 7).</summary>
			class Win7ToolstripButtonStyle : public Object, public virtual controls::GuiMenuButton::IStyleController, public Description<Win7ToolstripButtonStyle>
			{
			public:
				/// <summary>Sub menu dropdown arrow style.</summary>
				enum ButtonStyle
				{
					/// <summary>[T:vl.presentation.win7.Win7ToolstripButtonStyle.ButtonStyle]No dropdown.</summary>
					CommandButton,
					/// <summary>[T:vl.presentation.win7.Win7ToolstripButtonStyle.ButtonStyle]The whole button is a dropdown.</summary>
					DropdownButton,
					/// <summary>[T:vl.presentation.win7.Win7ToolstripButtonStyle.ButtonStyle]The button and the dropdown is splitted.</summary>
					SplitButton,
				};
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Win7ButtonColors, Win7ToolstripButtonStyle)

				Win7ButtonElements							elements;
				Ptr<TransferringAnimation>					transferringAnimation;
				controls::GuiButton::ControlState			controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;
				bool										isOpening;
				elements::GuiImageFrameElement*				imageElement;
				compositions::GuiBoundsComposition*			imageComposition;
				ButtonStyle									buttonStyle;
				controls::GuiButton*						subMenuHost;

				virtual void								TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected, bool menuOpening);
			public:
				/// <summary>Create the style.</summary>
				/// <param name="_buttonStyle">Defines the sub menu dropdown arrow style.</param>
				Win7ToolstripButtonStyle(ButtonStyle _buttonStyle);
				~Win7ToolstripButtonStyle();
				
				compositions::GuiBoundsComposition*							GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*						GetContainerComposition()override;
				void														SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void														SetText(const WString& value)override;
				void														SetFont(const FontProperties& value)override;
				void														SetVisuallyEnabled(bool value)override;
				void														SetSelected(bool value)override;
				controls::GuiMenu::IStyleController*						CreateSubMenuStyleController()override;
				void														SetSubMenuExisting(bool value)override;
				void														SetSubMenuOpening(bool value)override;
				controls::GuiButton*										GetSubMenuHost()override;
				void														SetImage(Ptr<GuiImageData> value)override;
				void														SetShortcutText(const WString& value)override;
				void														Transfer(controls::GuiButton::ControlState value)override;
			};

			/// <summary>Toolstrip splitter style (Windows 7).</summary>
			class Win7ToolstripSplitterStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win7ToolstripSplitterStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
			public:
				/// <summary>Create the style.</summary>
				Win7ToolstripSplitterStyle();
				~Win7ToolstripSplitterStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};
		}
	}
}

#endif