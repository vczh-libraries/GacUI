/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUICOMBOCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUICOMBOCONTROLS

#include "../GuiWindowControls.h"
#include "GuiTextListControls.h"
#include "GuiListViewControls.h"
#include "GuiTreeViewControls.h"

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
			public:
				/// <summary>A command executor for the combo box to change the control state.</summary>
				class ICommandExecutor : public virtual IDescriptable, public Description<ICommandExecutor>
				{
				public:
					/// <summary>Notify that an item is selected, the combo box should close the popup and show the text of the selected item.</summary>
					virtual void							SelectItem()=0;
				};
				
				/// <summary>Style controller interface for <see cref="GuiComboBoxBase"/>.</summary>
				class IStyleController : public virtual GuiMenuButton::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Called when the command executor is changed.</summary>
					/// <param name="value">The command executor.</param>
					virtual void							SetCommandExecutor(ICommandExecutor* value)=0;
					/// <summary>Notify that an item is selected.</summary>
					virtual void							OnItemSelected()=0;
				};
			protected:

				class CommandExecutor : public Object, public virtual ICommandExecutor
				{
				protected:
					GuiComboBoxBase*						combo;

				public:
					CommandExecutor(GuiComboBoxBase* _combo);
					~CommandExecutor();

					void									SelectItem()override;
				};

				Ptr<CommandExecutor>						commandExecutor;
				IStyleController*							styleController;
				
				bool										IsAltAvailable()override;
				IGuiMenuService::Direction					GetSubMenuDirection()override;
				virtual void								SelectItem();
				void										OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiComboBoxBase(IStyleController* _styleController);
				~GuiComboBoxBase();

				/// <summary>Item selected event.</summary>
				compositions::GuiNotifyEvent				ItemSelected;
			};

/***********************************************************************
ComboBox with GuiListControl
***********************************************************************/

			/// <summary>Combo box list control. This control is a combo box with a list control in its popup.</summary>
			class GuiComboBoxListControl : public GuiComboBoxBase, public Description<GuiComboBoxListControl>
			{
			public:
				/// <summary>Style controller interface for <see cref="GuiComboBoxListControl"/>.</summary>
				class IStyleController : public virtual GuiComboBoxBase::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Indicate that if the combo box need to display text.</summary>
					/// <param name="value">Set to true to display text.</param>
					virtual void							SetTextVisible(bool value) = 0;
				};
				
				/// <summary>Item style provider for a <see cref="GuiComboBoxListControl"/>.</summary>
				class IItemStyleProvider : public virtual IDescriptable, public Description<IItemStyleProvider>
				{
				public:
					/// <summary>Called when an item style provider in installed to a <see cref="GuiListControl"/>.</summary>
					/// <param name="value">The list control.</param>
					virtual void							AttachComboBox(GuiComboBoxListControl* value)=0;
					/// <summary>Called when an item style provider in uninstalled from a <see cref="GuiListControl"/>.</summary>
					virtual void							DetachComboBox()=0;
					/// <summary>Create an item style controller from an item.</summary>
					/// <returns>The created item style controller.</returns>
					/// <param name="item">The item.</param>
					virtual GuiControl::IStyleController*	CreateItemStyle(description::Value item)=0;
				};

			protected:
				IStyleController*							styleController;
				GuiSelectableListControl*					containedListControl;
				GuiListControl::IItemPrimaryTextView*		primaryTextView;
				GuiListControl::IItemBindingView*			itemBindingView;
				Ptr<IItemStyleProvider>						itemStyleProvider;
				Ptr<GuiControl::IStyleController>			itemStyleController;

				void										RemoveStyleController();
				void										InstallStyleController(vint itemIndex);
				virtual void								DisplaySelectedContent(vint itemIndex);
				void										OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnListControlAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void										OnListControlSelectionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style controller and a list control that will be put in the popup control to show all items.</summary>
				/// <param name="_styleController">The style controller.</param>
				/// <param name="_containedListControl">The list controller.</param>
				GuiComboBoxListControl(IStyleController* _styleController, GuiSelectableListControl* _containedListControl);
				~GuiComboBoxListControl();
				
				/// <summary>Style provider changed event.</summary>
				compositions::GuiNotifyEvent				StyleProviderChanged;
				/// <summary>Selected index changed event.</summary>
				compositions::GuiNotifyEvent				SelectedIndexChanged;
				
				/// <summary>Get the list control.</summary>
				/// <returns>The list control.</returns>
				GuiSelectableListControl*					GetContainedListControl();
				
				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				IItemStyleProvider*							GetStyleProvider();
				/// <summary>Set the item style provider</summary>
				/// <returns>The old item style provider</returns>
				/// <param name="value">The new item style provider</param>
				Ptr<IItemStyleProvider>						SetStyleProvider(Ptr<IItemStyleProvider> value);
				
				/// <summary>Get the selected index.</summary>
				/// <returns>The selected index.</returns>
				vint										GetSelectedIndex();
				/// <summary>Set the selected index.</summary>
				/// <param name="value">The selected index.</param>
				void										SetSelectedIndex(vint value);

				/// <summary>Get the selected item.</summary>
				description::Value							GetSelectedItem();
				/// <summary>Get the item provider in the list control.</summary>
				/// <returns>The item provider in the list control.</returns>
				GuiListControl::IItemProvider*				GetItemProvider();
			};
		}
	}
}

#endif