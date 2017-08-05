/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows8 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8TOOLSTRIPSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8TOOLSTRIPSTYLES

#include "GuiWin8MenuStyles.h"
#include "GuiWin8ControlStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{

/***********************************************************************
Toolstrip Button
***********************************************************************/

			/// <summary>Toolstrip toolbar style (Windows 8). Using the Windows 8 window background color to fill the whold control</summary>
			class Win8ToolstripToolBarStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win8ToolstripToolBarStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			containerComposition;
			public:
				/// <summary>Create the style.</summary>
				Win8ToolstripToolBarStyle();
				~Win8ToolstripToolBarStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

			/// <summary>Toolstrip split button dropdown style (Windows 8).</summary>
			class Win8ToolstripButtonDropdownStyle : public Object, public virtual controls::GuiButton::IStyleController, public Description<Win8ToolstripButtonDropdownStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			splitterComposition;
				compositions::GuiBoundsComposition*			containerComposition;
				bool										isVisuallyEnabled;
				controls::ButtonState						controlState;

				virtual void								TransferInternal(controls::ButtonState value, bool enabled);
			public:
				/// <summary>Create the style.</summary>
				Win8ToolstripButtonDropdownStyle();
				~Win8ToolstripButtonDropdownStyle();
				
				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
				void										Transfer(controls::ButtonState value)override;
			};

			/// <summary>Toolstrip button style (Windows 8).</summary>
			class Win8ToolstripButtonStyle : public Object, public virtual controls::GuiMenuButton::IStyleController, public Description<Win8ToolstripButtonStyle>
			{
			public:
				/// <summary>Sub menu dropdown arrow style.</summary>
				enum ButtonStyle
				{
					/// <summary>No dropdown.</summary>
					CommandButton,
					/// <summary>The whole button is a dropdown.</summary>
					DropdownButton,
					/// <summary>The button and the dropdown is splitted.</summary>
					SplitButton,
				};
			protected:
				DEFINE_TRANSFERRING_ANIMATION(Win8ButtonColors, Win8ToolstripButtonStyle)

				Win8ButtonElements							elements;
				Ptr<TransferringAnimation>					transferringAnimation;
				controls::ButtonState						controlStyle;
				bool										isVisuallyEnabled;
				bool										isSelected;
				bool										isOpening;
				elements::GuiImageFrameElement*				imageElement;
				compositions::GuiBoundsComposition*			imageComposition;
				ButtonStyle									buttonStyle;
				controls::GuiButton*						subMenuHost;

				virtual void								TransferInternal(controls::ButtonState value, bool enabled, bool selected, bool menuOpening);
			public:
				/// <summary>Create the style.</summary>
				/// <param name="_buttonStyle">Defines the sub menu dropdown arrow style.</param>
				Win8ToolstripButtonStyle(ButtonStyle _buttonStyle);
				~Win8ToolstripButtonStyle();
				
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
				void														Transfer(controls::ButtonState value)override;
			};

			/// <summary>Toolstrip splitter style (Windows 8).</summary>
			class Win8ToolstripSplitterStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win8ToolstripSplitterStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
			public:
				/// <summary>Create the style.</summary>
				Win8ToolstripSplitterStyle();
				~Win8ToolstripSplitterStyle();

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