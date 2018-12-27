/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUICOMBOCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUICOMBOCONTROLS

#include "../GuiWindowControls.h"
#include "GuiListControls.h"
#include "../ToolstripPackage/GuiMenuControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
ComboBox Base
***********************************************************************/

			/// <summary>The base class of combo box control.</summary>
			class GuiComboBoxBase : public GuiMenuButton, public Description<GuiComboBoxBase>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ComboBoxTemplate, GuiMenuButton)
			protected:
				
				IGuiMenuService::Direction					GetSubMenuDirection()override;
				void										OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiComboBoxBase(theme::ThemeName themeName);
				~GuiComboBoxBase();
			};

/***********************************************************************
ComboBox with GuiListControl
***********************************************************************/

			/// <summary>Combo box list control. This control is a combo box with a list control in its popup.</summary>
			class GuiComboBoxListControl
				: public GuiComboBoxBase
				, private GuiListControl::IItemProviderCallback
				, public Description<GuiComboBoxListControl>
			{
			public:
				using ItemStyleProperty = TemplateProperty<templates::GuiTemplate>;

			protected:
				GuiSelectableListControl*					containedListControl = nullptr;
				vint										selectedIndex = -1;
				ItemStyleProperty							itemStyleProperty;
				templates::GuiTemplate*						itemStyleController = nullptr;
				Ptr<compositions::IGuiGraphicsEventHandler>	boundsChangedHandler;

				void										UpdateDisplayFont()override;
				void										BeforeControlTemplateUninstalled()override;
				void										AfterControlTemplateInstalled(bool initialize)override;
				void										RemoveStyleController();
				void										InstallStyleController(vint itemIndex);
				virtual void								DisplaySelectedContent(vint itemIndex);
				void										AdoptSubMenuSize();
				void										OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnContextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnAfterSubMenuOpening(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnListControlAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnListControlBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnListControlItemMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments);
				void										OnListControlKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);

			private:
				// ===================== GuiListControl::IItemProviderCallback =====================

				void										OnAttached(GuiListControl::IItemProvider* provider)override;
				void										OnItemModified(vint start, vint count, vint newCount)override;
			public:
				/// <summary>Create a control with a specified default theme and a list control that will be put in the popup control to show all items.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_containedListControl">The list controller.</param>
				GuiComboBoxListControl(theme::ThemeName themeName, GuiSelectableListControl* _containedListControl);
				~GuiComboBoxListControl();
				
				/// <summary>Style provider changed event.</summary>
				compositions::GuiNotifyEvent				ItemTemplateChanged;
				/// <summary>Selected index changed event.</summary>
				compositions::GuiNotifyEvent				SelectedIndexChanged;
				
				/// <summary>Get the list control.</summary>
				/// <returns>The list control.</returns>
				GuiSelectableListControl*					GetContainedListControl();

				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				virtual ItemStyleProperty					GetItemTemplate();
				/// <summary>Set the item style provider</summary>
				/// <param name="value">The new item style provider</param>
				virtual void								SetItemTemplate(ItemStyleProperty value);
				
				/// <summary>Get the selected index.</summary>
				/// <returns>The selected index.</returns>
				vint										GetSelectedIndex();
				/// <summary>Set the selected index.</summary>
				/// <param name="value">The selected index.</param>
				void										SetSelectedIndex(vint value);

				/// <summary>Get the selected item.</summary>
				/// <returns>The selected item.</returns>
				description::Value							GetSelectedItem();
				/// <summary>Get the item provider in the list control.</summary>
				/// <returns>The item provider in the list control.</returns>
				GuiListControl::IItemProvider*				GetItemProvider();
			};
		}
	}
}

#endif