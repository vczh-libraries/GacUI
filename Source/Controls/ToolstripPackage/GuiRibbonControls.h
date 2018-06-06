/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIRIBBONCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIRIBBONCONTROLS

#include "GuiToolstripMenu.h"
#include "../GuiContainerControls.h"
#include "../ListControlPackage/GuiBindableListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Ribbon Containers
***********************************************************************/

			class GuiRibbonTabPage;
			class GuiRibbonGroup;

			/// <summary>Ribbon tab control, for displaying ribbon tab pages.</summary>
			class GuiRibbonTab : public GuiTab, public Description<GuiRibbonTab>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonTabTemplate, GuiTab)
			protected:
				compositions::GuiBoundsComposition*					beforeHeaders = nullptr;
				compositions::GuiBoundsComposition*					afterHeaders = nullptr;
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiRibbonTab(theme::ThemeName themeName);
				~GuiRibbonTab();

				/// <summary>Get the composition representing the space before tabs.</summary>
				/// <returns>The composition representing the space before tabs.</returns>
				compositions::GuiGraphicsComposition*				GetBeforeHeaders();
				/// <summary>Get the composition representing the space after tabs.</summary>
				/// <returns>The composition representing the space after tabs.</returns>
				compositions::GuiGraphicsComposition*				GetAfterHeaders();
			};

			class GuiRibbonGroupCollection : public collections::ObservableListBase<GuiRibbonGroup*>
			{
			protected:
				GuiRibbonTabPage*									tabPage = nullptr;

				bool												QueryInsert(vint index, GuiRibbonGroup* const& value)override;
				void												AfterInsert(vint index, GuiRibbonGroup* const& value)override;
				void												AfterRemove(vint index, vint count)override;

			public:
				GuiRibbonGroupCollection(GuiRibbonTabPage* _tabPage);
				~GuiRibbonGroupCollection();
			};

			/// <summary>Ribbon tab page control, adding to the Pages property of a <see cref="GuiRibbonTab"/>.</summary>
			class GuiRibbonTabPage : public GuiTabPage, public Description<GuiRibbonTabPage>
			{
				friend class GuiRibbonGroupCollection;
			protected:
				bool												highlighted = false;
				GuiRibbonGroupCollection							groups;
				compositions::GuiResponsiveStackComposition*		responsiveStack = nullptr;
				compositions::GuiResponsiveContainerComposition*	responsiveContainer = nullptr;
				compositions::GuiStackComposition*					stack = nullptr;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiRibbonTabPage(theme::ThemeName themeName);
				~GuiRibbonTabPage();
				
				/// <summary>Highlighted changed event.</summary>
				compositions::GuiNotifyEvent						HighlightedChanged;

				/// <summary>Test if this is a highlighted tab page.</summary>
				/// <returns>Returns true if this is a highlighted tab page.</returns>
				bool												GetHighlighted();
				/// <summary>Set if this is a highlighted tab page.</summary>
				/// <param name="value">Set to true to highlight the tab page.</param>
				void												SetHighlighted(bool value);

				/// <summary>Get the collection of ribbon groups.</summary>
				/// <returns>The collection of ribbon groups.</returns>
				collections::ObservableListBase<GuiRibbonGroup*>&	GetGroups();
			};

			class GuiRibbonGroupItemCollection : public collections::ObservableListBase<GuiControl*>
			{
			protected:
				GuiRibbonGroup*										group = nullptr;

				bool												QueryInsert(vint index, GuiControl* const& value)override;
				void												AfterInsert(vint index, GuiControl* const& value)override;
				void												AfterRemove(vint index, vint count)override;

			public:
				GuiRibbonGroupItemCollection(GuiRibbonGroup* _group);
				~GuiRibbonGroupItemCollection();
			};

			/// <summary>Ribbon group control, adding to the Groups property of a <see cref="GuiRibbonTabPage"/>.</summary>
			class GuiRibbonGroup : public GuiControl, public Description<GuiRibbonGroup>
			{
				friend class GuiRibbonGroupItemCollection;
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGroupTemplate, GuiControl)
			protected:

				class CommandExecutor : public Object, public IRibbonGroupCommandExecutor
				{
				protected:
					GuiRibbonGroup*									group;

				public:
					CommandExecutor(GuiRibbonGroup* _group);
					~CommandExecutor();

					void											NotifyExpandButtonClicked()override;
				};

				bool												expandable = false;
				Ptr<GuiImageData>									largeImage;
				GuiRibbonGroupItemCollection						items;
				compositions::GuiResponsiveStackComposition*		responsiveStack = nullptr;
				compositions::GuiStackComposition*					stack = nullptr;
				Ptr<CommandExecutor>								commandExecutor;

				compositions::GuiResponsiveViewComposition*			responsiveView = nullptr;
				compositions::GuiResponsiveFixedComposition*		responsiveFixedButton = nullptr;
				GuiToolstripButton*									dropdownButton = nullptr;
				GuiMenu*											dropdownMenu = nullptr;

				void												OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void												OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void												OnBeforeSwitchingView(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void												OnBeforeSubMenuOpening(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiRibbonGroup(theme::ThemeName themeName);
				~GuiRibbonGroup();

				/// <summary>Expandable changed event.</summary>
				compositions::GuiNotifyEvent						ExpandableChanged;
				/// <summary>Expand button clicked event.</summary>
				compositions::GuiNotifyEvent						ExpandButtonClicked;
				/// <summary>Large image changed event.</summary>
				compositions::GuiNotifyEvent						LargeImageChanged;

				/// <summary>Test if this group is expandable. An expandable group will display an extra small button, which raises <see cref="ExpandButtonClicked"/>.</summary>
				/// <returns>Returns true if this group is expandable.</returns>
				bool												GetExpandable();
				/// <summary>Set if this group is expandable.</summary>
				/// <param name="value">Set to true to make this group is expandable.</param>
				void												SetExpandable(bool value);

				/// <summary>Get the large image for the collapsed ribbon group.</summary>
				/// <returns>The large image for the collapsed ribbon group.</returns>
				Ptr<GuiImageData>									GetLargeImage();
				/// <summary>Set the large image for the collapsed ribbon group.</summary>
				/// <param name="value">The large image for the collapsed ribbon group.</param>
				void												SetLargeImage(Ptr<GuiImageData> value);

				/// <summary>Get the collection of controls in this group.</summary>
				/// <returns>The collection of controls.</returns>
				collections::ObservableListBase<GuiControl*>&		GetItems();
			};

/***********************************************************************
Ribbon Buttons
***********************************************************************/

			/// <summary>Auto resizable ribbon icon label.</summary>
			class GuiRibbonIconLabel : public GuiControl, public Description<GuiRibbonIconLabel>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonIconLabelTemplate, GuiControl)
			protected:
				Ptr<GuiImageData>						image;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiRibbonIconLabel(theme::ThemeName themeName);
				~GuiRibbonIconLabel();

				/// <summary>Image changed event.</summary>
				compositions::GuiNotifyEvent			ImageChanged;

				/// <summary>Get the image for the menu button.</summary>
				/// <returns>The image for the menu button.</returns>
				Ptr<GuiImageData>						GetImage();
				/// <summary>Set the image for the menu button.</summary>
				/// <param name="value">The image for the menu button.</param>
				void									SetImage(Ptr<GuiImageData> value);
			};

			/// <summary>Represents the size of a ribbon button in a <see cref="GuiRibbonButtons"/> control.</summary>
			enum class RibbonButtonSize
			{
				/// <summary>Large icon with text.</summary>
				Large = 0,
				/// <summary>Small icon with text.</summary>
				Small = 1,
				/// <summary>Small icon only.</summary>
				Icon = 2,
			};

			class GuiRibbonButtons;

			class GuiRibbonButtonsItemCollection : public collections::ObservableListBase<GuiControl*>
			{
			protected:
				GuiRibbonButtons*										buttons = nullptr;

				bool													QueryInsert(vint index, GuiControl* const& value)override;
				void													AfterInsert(vint index, GuiControl* const& value)override;
				void													BeforeRemove(vint index, GuiControl* const& value)override;

			public:
				GuiRibbonButtonsItemCollection(GuiRibbonButtons* _buttons);
				~GuiRibbonButtonsItemCollection();
			};

			/// <summary>Auto resizable ribbon buttons.</summary>
			class GuiRibbonButtons : public GuiControl, public Description<GuiRibbonButtons>
			{
				friend class GuiRibbonButtonsItemCollection;
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonButtonsTemplate, GuiControl)
			protected:
				RibbonButtonSize										minSize;
				RibbonButtonSize										maxSize;
				compositions::GuiResponsiveViewComposition*				responsiveView = nullptr;
				compositions::GuiResponsiveFixedComposition*			views[3] = { nullptr,nullptr,nullptr };
				GuiRibbonButtonsItemCollection							buttons;
																		
				void													OnBeforeSwitchingView(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void													SetButtonThemeName(compositions::GuiResponsiveCompositionBase* fixed, GuiControl* button);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_maxSize">Max allowed size.</param>
				/// <param name="_minSize">Min allowed size.</param>
				GuiRibbonButtons(theme::ThemeName themeName, RibbonButtonSize _maxSize, RibbonButtonSize _minSize);
				~GuiRibbonButtons();

				/// <summary>Get the collection of buttons. <see cref="GuiToolstripButton"/> is expected.</summary>
				/// <returns>The collection of buttons.</returns>
				collections::ObservableListBase<GuiControl*>&			GetButtons();
			};

/***********************************************************************
Ribbon Toolstrips
***********************************************************************/

			class GuiRibbonToolstrips;

			class GuiRibbonToolstripsGroupCollection : public collections::ObservableListBase<GuiToolstripGroup*>
			{
			protected:
				GuiRibbonToolstrips*									toolstrips = nullptr;

				bool													QueryInsert(vint index, GuiToolstripGroup* const& value)override;
				void													AfterInsert(vint index, GuiToolstripGroup* const& value)override;
				void													AfterRemove(vint index, vint count)override;

			public:
				GuiRibbonToolstripsGroupCollection(GuiRibbonToolstrips* _toolstrips);
				~GuiRibbonToolstripsGroupCollection();
			};

			/// <summary>Auto resizable ribbon toolstrips.</summary>
			class GuiRibbonToolstrips : public GuiControl, public Description<GuiRibbonToolstrips>
			{
				friend class GuiRibbonToolstripsGroupCollection;
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonToolstripsTemplate, GuiControl)
			protected:
				compositions::GuiResponsiveViewComposition*				responsiveView = nullptr;
				GuiToolstripToolBar*									toolbars[5] = { nullptr,nullptr,nullptr,nullptr,nullptr };
				GuiToolstripGroupContainer*								longContainers[2] = { nullptr,nullptr };
				GuiToolstripGroupContainer*								shortContainers[3] = { nullptr,nullptr,nullptr };
				compositions::GuiResponsiveFixedComposition*			views[2] = { nullptr,nullptr };
				GuiRibbonToolstripsGroupCollection						groups;

				void													OnBeforeSwitchingView(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void													RearrangeToolstripGroups(vint viewIndex = -1);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiRibbonToolstrips(theme::ThemeName themeName);
				~GuiRibbonToolstrips();

				/// <summary>Get the collection of toolstrip groups. <see cref="GuiRibbonToolstrips"/> will decide the order of these toolstrip groups.</summary>
				/// <returns>The collection of toolstrip groups.</returns>
				collections::ObservableListBase<GuiToolstripGroup*>&	GetGroups();
			};

/***********************************************************************
Ribbon Gallery
***********************************************************************/

			/// <summary>Ribbon gallery, with scroll up, scroll down, dropdown buttons.</summary>
			class GuiRibbonGallery : public GuiControl, public Description<GuiRibbonGallery>
			{
				using ItemStyle = templates::GuiListItemTemplate;
				using ItemStyleProperty = TemplateProperty<templates::GuiListItemTemplate>;

				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGalleryTemplate, GuiControl)
			protected:
				class CommandExecutor : public Object, public IRibbonGalleryCommandExecutor
				{
				protected:
					GuiRibbonGallery*									gallery;

				public:
					CommandExecutor(GuiRibbonGallery* _gallery);
					~CommandExecutor();

					void												NotifyScrollUp()override;
					void												NotifyScrollDown()override;
					void												NotifyDropdown()override;
				};

				bool													scrollUpEnabled = true;
				bool													scrollDownEnabled = true;
				Ptr<CommandExecutor>									commandExecutor;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiRibbonGallery(theme::ThemeName themeName);
				~GuiRibbonGallery();

				/// <summary>Scroll up enabled changed event.</summary>
				compositions::GuiNotifyEvent							ScrollUpEnabledChanged;
				/// <summary>Scroll down enabled changed event.</summary>
				compositions::GuiNotifyEvent							ScrollDownEnabledChanged;
				/// <summary>Scroll up button clicked event.</summary>
				compositions::GuiNotifyEvent							RequestedScrollUp;
				/// <summary>Scroll down button clicked event.</summary>
				compositions::GuiNotifyEvent							RequestedScrollDown;
				/// <summary>Dropdown button clicked event.</summary>
				compositions::GuiNotifyEvent							RequestedDropdown;

				/// <summary>Test if the scroll up button is enabled.</summary>
				/// <returns>Returns true if the scroll up button is enabled.</returns>
				bool													GetScrollUpEnabled();
				/// <summary>Set if the scroll up button is enabled.</summary>
				/// <param name="value">Set to true to enable the scroll up button.</param>
				void													SetScrollUpEnabled(bool value);

				/// <summary>Test if the scroll down button is enabled.</summary>
				/// <returns>Returns true if the scroll down button is enabled.</returns>
				bool													GetScrollDownEnabled();
				/// <summary>Set if the scroll down button is enabled.</summary>
				/// <param name="value">Set to true to enable the scroll down button.</param>
				void													SetScrollDownEnabled(bool value);
			};

			/// <summary>Resizable ribbon toolstrip menu with a space above of all menu items to display extra content.</summary>
			class GuiRibbonToolstripMenu : public GuiToolstripMenu, public Description<GuiRibbonToolstripMenu>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonToolstripMenuTemplate, GuiToolstripMenu)
			protected:
				compositions::GuiBoundsComposition*						contentComposition;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="owner">The owner menu item of the parent menu.</param>
				GuiRibbonToolstripMenu(theme::ThemeName themeName, GuiControl* owner);
				~GuiRibbonToolstripMenu();

				/// <summary>Get the composition representing the space above of menu items.</summary>
				/// <returns>The composition representing the space above of menu items.</returns>
				compositions::GuiGraphicsComposition*					GetContentComposition();
			};
		}
	}
}

#endif
