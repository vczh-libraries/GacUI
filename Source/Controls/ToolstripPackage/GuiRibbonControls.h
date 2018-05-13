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

			class GuiRibbonTab : public GuiTab, public Description<GuiRibbonTab>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonTabTemplate, GuiTab)
			protected:
				compositions::GuiBoundsComposition*					beforeHeaders = nullptr;
				compositions::GuiBoundsComposition*					afterHeaders = nullptr;
			public:
				GuiRibbonTab(theme::ThemeName themeName);
				~GuiRibbonTab();

				compositions::GuiGraphicsComposition*				GetBeforeHeaders();
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
				GuiRibbonTabPage(theme::ThemeName themeName);
				~GuiRibbonTabPage();

				compositions::GuiNotifyEvent						HighlightedChanged;

				bool												GetHighlighted();
				void												SetHighlighted(bool value);

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

			class GuiRibbonGroup : public GuiControl, public Description<GuiRibbonGroup>
			{
				friend class GuiRibbonGroupItemCollection;
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGroupTemplate, GuiControl)
			protected:
				bool												expandable = false;
				GuiRibbonGroupItemCollection						items;
				compositions::GuiResponsiveStackComposition*		responsiveStack = nullptr;
				compositions::GuiStackComposition*					stack = nullptr;

			public:
				GuiRibbonGroup(theme::ThemeName themeName);
				~GuiRibbonGroup();

				compositions::GuiNotifyEvent						ExpandableChanged;
				compositions::GuiNotifyEvent						ExpandButtonClicked;

				bool												GetExpandable();
				void												SetExpandable(bool value);

				collections::ObservableListBase<GuiControl*>&		GetItems();
			};

/***********************************************************************
Ribbon Buttons
***********************************************************************/

			enum class RibbonButtonSize
			{
				Large = 0,
				Small = 1,
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
				GuiRibbonButtons(theme::ThemeName themeName, RibbonButtonSize _maxSize, RibbonButtonSize _minSize);
				~GuiRibbonButtons();

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
				GuiRibbonToolstrips(theme::ThemeName themeName);
				~GuiRibbonToolstrips();

				collections::ObservableListBase<GuiToolstripGroup*>&	GetGroups();
			};

/***********************************************************************
Ribbon Gallery
***********************************************************************/

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
				GuiRibbonGallery(theme::ThemeName themeName);
				~GuiRibbonGallery();

				compositions::GuiNotifyEvent							ScrollUpEnabledChanged;
				compositions::GuiNotifyEvent							ScrollDownEnabledChanged;
				compositions::GuiNotifyEvent							RequestedScrollUp;
				compositions::GuiNotifyEvent							RequestedScrollDown;
				compositions::GuiNotifyEvent							RequestedDropdown;

				bool													GetScrollUpEnabled();
				void													SetScrollUpEnabled(bool value);

				bool													GetScrollDownEnabled();
				void													SetScrollDownEnabled(bool value);
			};

			class GuiRibbonToolstripMenu : public GuiToolstripMenu, public Description<GuiRibbonToolstripMenu>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonToolstripMenuTemplate, GuiToolstripMenu)
			protected:
				compositions::GuiBoundsComposition*						contentComposition;

			public:
				GuiRibbonToolstripMenu(theme::ThemeName themeName, GuiControl* owner);
				~GuiRibbonToolstripMenu();

				compositions::GuiGraphicsComposition*					GetContentComposition();
			};

/***********************************************************************
Ribbon Gallery List
***********************************************************************/

			struct GalleryPos
			{
				vint			group;
				vint			item;

				GalleryPos()
					:group(-1), item(-1)
				{
				}

				GalleryPos(vint _group, vint _item)
					:group(_group), item(_item)
				{
				}

				vint Compare(GalleryPos value)const
				{
					vint result = group - value.group;
					if (result != 0) return result;
					return item - value.item;
				}

				bool operator==(const GalleryPos& value)const { return Compare(value) == 0; }
				bool operator!=(const GalleryPos& value)const { return Compare(value) != 0; }
				bool operator<(const GalleryPos& value)const { return Compare(value)<0; }
				bool operator<=(const GalleryPos& value)const { return Compare(value) <= 0; }
				bool operator>(const GalleryPos& value)const { return Compare(value)>0; }
				bool operator>=(const GalleryPos& value)const { return Compare(value) >= 0; }
			};

			class GuiBindableRibbonGalleryList : public GuiRibbonGallery, public Description<GuiBindableRibbonGalleryList>
			{
				using IValueEnumerable = reflection::description::IValueEnumerable;
			protected:
				GuiRibbonToolstripMenu*									subMenu;
				ItemProperty<WString>									titleProperty;
				ItemProperty<Ptr<IValueEnumerable>>						childrenProperty;

				void													OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnRequestedDropdown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				GuiBindableRibbonGalleryList(theme::ThemeName themeName);
				~GuiBindableRibbonGalleryList();

				compositions::GuiNotifyEvent							GroupEnabledChanged;
				compositions::GuiNotifyEvent							GroupTitlePropertyChanged;
				compositions::GuiNotifyEvent							GroupChildrenPropertyChanged;
				compositions::GuiNotifyEvent							SelectionChanged;

				Ptr<IValueEnumerable>									GetItemSource();
				void													SetItemSource(Ptr<IValueEnumerable> value);

				bool													GetGroupEnabled();
				ItemProperty<WString>									GetGroupTitleProperty();
				void													SetGroupTitleProperty(const ItemProperty<WString>& value);
				ItemProperty<Ptr<IValueEnumerable>>						GetGroupChildrenProperty();
				void													SetGroupChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value);

				GalleryPos												GetSelection();
				void													SetSelection(GalleryPos value);

				description::Value										GetGroupValue(vint groupIndex);
				description::Value										GetItemValue(GalleryPos pos);

				GuiToolstripMenu*										GetSubMenu();
			};
		}
	}
}

#endif
