/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control Styles::Windows8 Styles

Clases:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8MENUSTYLES
#define VCZH_PRESENTATION_CONTROLS_WIN8STYLES_GUIWIN8MENUSTYLES

#include "GuiWin8StylesCommon.h"

namespace vl
{
	namespace presentation
	{
		namespace win8
		{

/***********************************************************************
Menu Container
***********************************************************************/
			
			/// <summary>Menu style (Windows 8). For the background of a popup menu.</summary>
			class Win8MenuStyle : public Object, public virtual controls::GuiWindow::DefaultBehaviorStyleController, public Description<Win8MenuStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			containerComposition;
			public:
				/// <summary>Create the style.</summary>
				Win8MenuStyle();
				~Win8MenuStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};
			
			/// <summary>Menu bar style (Windows 8). For the background of a menu bar.</summary>
			class Win8MenuBarStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win8MenuBarStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
				compositions::GuiBoundsComposition*			containerComposition;
			public:
				/// <summary>Create the style.</summary>
				Win8MenuBarStyle();
				~Win8MenuBarStyle();

				compositions::GuiBoundsComposition*			GetBoundsComposition()override;
				compositions::GuiGraphicsComposition*		GetContainerComposition()override;
				void										SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
				void										SetText(const WString& value)override;
				void										SetFont(const FontProperties& value)override;
				void										SetVisuallyEnabled(bool value)override;
			};

/***********************************************************************
Menu Button
***********************************************************************/
			
			/// <summary>Menu bar button style (Windows 8). For menu buttons in a menu bar.</summary>
			class Win8MenuBarButtonStyle : public Object, public virtual controls::GuiMenuButton::IStyleController, public Description<Win8MenuBarButtonStyle>
			{
			protected:
				Win8ButtonElements							elements;
				controls::GuiButton::ControlState			controlStyle;
				bool										isVisuallyEnabled;
				bool										isOpening;

				void										TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool opening);
			public:
				/// <summary>Create the style.</summary>
				Win8MenuBarButtonStyle();
				~Win8MenuBarButtonStyle();

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
			
			/// <summary>Menu item button style (Windows 8). For menu buttons in a popup menu.</summary>
			class Win8MenuItemButtonStyle : public Object, public virtual controls::GuiMenuButton::IStyleController, public Description<Win8MenuItemButtonStyle>
			{
			protected:
				Win8MenuItemButtonElements									elements;
				controls::GuiButton::ControlState							controlStyle;
				bool														isVisuallyEnabled;
				bool														isSelected;
				bool														isOpening;

				void														TransferInternal(controls::GuiButton::ControlState value, bool enabled, bool selected, bool opening);
			public:
				/// <summary>Create the style.</summary>
				Win8MenuItemButtonStyle();
				~Win8MenuItemButtonStyle();

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
			
			/// <summary>Menu splitter style (Windows 8). For splitters in a popup menu.</summary>
			class Win8MenuSplitterStyle : public Object, public virtual controls::GuiControl::IStyleController, public Description<Win8MenuSplitterStyle>
			{
			protected:
				compositions::GuiBoundsComposition*			boundsComposition;
			public:
				/// <summary>Create the style.</summary>
				Win8MenuSplitterStyle();
				~Win8MenuSplitterStyle();

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