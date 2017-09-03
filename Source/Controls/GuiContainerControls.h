/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUICONTAINERCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUICONTAINERCONTROLS

#include "GuiScrollControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
/***********************************************************************
Tab Control
***********************************************************************/

			class GuiTabPageList;
			class GuiTab;

			/// <summary>Represnets a tab page control.</summary>
			class GuiTabPage : public GuiCustomControl, public AggregatableDescription<GuiTabPage>
			{
				friend class GuiTabPageList;
				friend class GuiTab;
			protected:
				GuiTab*											tab = nullptr;

				bool											IsAltAvailable()override;
			public:
				/// <summary>Create a tab page control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiTabPage(IStyleController* _styleController);
				~GuiTabPage();
			};

			class GuiTabPageList : public collections::ObservableList<GuiTabPage*>
			{
			protected:
				GuiTab*											tab;

				bool											QueryInsert(vint index, GuiTabPage* const& value)override;
				void											AfterInsert(vint index, GuiTabPage* const& value)override;
				void											BeforeRemove(vint index, GuiTabPage* const& value)override;
			public:
				GuiTabPageList(GuiTab* _tab);
				~GuiTabPageList();
			};

			/// <summary>Represents a container with multiple named tabs.</summary>
			class GuiTab : public GuiControl, public Description<GuiTab>
			{
				friend class GuiTabPage;
				friend class GuiTabPageList;
			public:
				
				/// <summary>Style controller interface for <see cref="GuiTab"/>.</summary>
				class IStyleController : public virtual GuiControl::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Called when the command executor is changed.</summary>
					/// <param name="value">The command executor.</param>
					virtual void								SetCommandExecutor(ITabCommandExecutor* value) = 0;
					/// <summary>Called when the tab page list is changed.</summary>
					/// <param name="value">The tab page list.</param>
					virtual void								SetTabPages(Ptr<reflection::description::IValueObservableList> value) = 0;
					/// <summary>Render a tab header at the specified position as selected.</summary>
					/// <param name="index">The specified position.</param>
					virtual void								SetSelectedTabPage(GuiTabPage* value) = 0;
				};
			protected:
				class CommandExecutor : public Object, public ITabCommandExecutor
				{
				protected:
					GuiTab*										tab;
				public:
					CommandExecutor(GuiTab* _tab);
					~CommandExecutor();

					void										ShowTab(vint index)override;
				};

				Ptr<CommandExecutor>							commandExecutor;
				IStyleController*								styleController = nullptr;
				GuiTabPageList									tabPages;
				GuiTabPage*										selectedPage = nullptr;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiTab(IStyleController* _styleController);
				~GuiTab();

				/// <summary>Selected page changed event.</summary>
				compositions::GuiNotifyEvent					SelectedPageChanged;

				/// <summary>Get all pages.</summary>
				/// <returns>All pages.</returns>
				collections::ObservableList<GuiTabPage*>&		GetPages();

				/// <summary>Get the selected page.</summary>
				/// <returns>The selected page.</returns>
				GuiTabPage*										GetSelectedPage();
				/// <summary>Set the selected page.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="value">The selected page.</param>
				bool											SetSelectedPage(GuiTabPage* value);
			};

/***********************************************************************
Scroll View
***********************************************************************/

			/// <summary>A control with a vertical scroll bar and a horizontal scroll bar to perform partial viewing.</summary>
			class GuiScrollView : public GuiControl, public Description<GuiScrollView>
			{
			public:
				/// <summary>Style provider interface for <see cref="GuiScrollView"/>.</summary>
				class IStyleProvider : public virtual GuiControl::IStyleProvider, public Description<IStyleProvider>
				{
				public:
					/// <summary>Create a control style for the horizontal scroll bar.</summary>
					/// <returns>The created control style for the horizontal scroll bar.</returns>
					virtual GuiScroll::IStyleController*			CreateHorizontalScrollStyle()=0;
					/// <summary>Create a control style for the vertical scroll bar.</summary>
					/// <returns>The created control style for the vertical scroll bar.</returns>
					virtual GuiScroll::IStyleController*			CreateVerticalScrollStyle()=0;
					/// <summary>Get the default scroll size for scroll bars, width for vertical, height for horizontal.</summary>
					/// <returns>The default scroll size.</returns>
					virtual vint									GetDefaultScrollSize()=0;
					/// <summary>Called when the control begins to initialize. The control pass the bounds composition, and the style provider can put background compositions and elements on it, and return a container composition to contain content and scroll bars.</summary>
					/// <returns>A container composition to contain content and scroll bars</returns>
					/// <param name="boundsComposition">The bounds composition to install background.</param>
					virtual compositions::GuiGraphicsComposition*	InstallBackground(compositions::GuiBoundsComposition* boundsComposition)=0;
				};
				
				/// <summary>Style controller for <see cref="GuiScrollView"/>.</summary>
				class StyleController : public Object, public GuiControl::IStyleController, public Description<StyleController>
				{
				protected:
					Ptr<IStyleProvider>						styleProvider;
					GuiScrollView*							scrollView;
					GuiScroll*								horizontalScroll;
					GuiScroll*								verticalScroll;
					compositions::GuiBoundsComposition*		boundsComposition;
					compositions::GuiTableComposition*		tableComposition;
					compositions::GuiCellComposition*		containerCellComposition;
					compositions::GuiBoundsComposition*		containerComposition;
					bool									horizontalAlwaysVisible;
					bool									verticalAlwaysVisible;

					void									UpdateTable();
				public:
					/// <summary>Create a style controller with a specified style provider.</summary>
					/// <param name="_styleProvider">The style provider.</param>
					StyleController(IStyleProvider* _styleProvider);
					~StyleController();

					/// <summary>Called when the style controller is attched to a <see cref="GuiScrollView"/>.</summary>
					/// <param name="_scrollView">The scroll view control that attached to.</param>
					void									SetScrollView(GuiScrollView* _scrollView);
					/// <summary>Called when the view size of the scroll view changed. Scroll bars will be adjusted.</summary>
					/// <param name="fullSize">The view size.</param>
					void									AdjustView(Size fullSize);
					/// <summary>Get the attached style provider.</summary>
					/// <returns>The attached style provider.</returns>
					IStyleProvider*							GetStyleProvider();

					/// <summary>Get the horizontal scroll control.</summary>
					/// <returns>The horizontal scroll control.</returns>
					GuiScroll*								GetHorizontalScroll();
					/// <summary>Get the vertical scroll control.</summary>
					/// <returns>The vertical scroll control.</returns>
					GuiScroll*								GetVerticalScroll();

					compositions::GuiTableComposition*		GetInternalTableComposition();
					compositions::GuiBoundsComposition*		GetInternalContainerComposition();

					/// <summary>Test is the horizontal scroll bar always visible even the content doesn't exceed the view bounds.</summary>
					/// <returns>Returns true if the horizontal scroll bar always visible even the content doesn't exceed the view bounds</returns>
					bool									GetHorizontalAlwaysVisible();
					/// <summary>Set is the horizontal scroll bar always visible even the content doesn't exceed the view bounds.</summary>
					/// <param name="value">Set to true if the horizontal scroll bar always visible even the content doesn't exceed the view bounds</param>
					void									SetHorizontalAlwaysVisible(bool value);
					/// <summary>Test is the vertical scroll bar always visible even the content doesn't exceed the view bounds.</summary>
					/// <returns>Returns true if the vertical scroll bar always visible even the content doesn't exceed the view bounds</returns>
					bool									GetVerticalAlwaysVisible();
					/// <summary>Set is the vertical scroll bar always visible even the content doesn't exceed the view bounds.</summary>
					/// <param name="value">Set to true if the vertical scroll bar always visible even the content doesn't exceed the view bounds</param>
					void									SetVerticalAlwaysVisible(bool value);

					compositions::GuiBoundsComposition*		GetBoundsComposition()override;
					compositions::GuiGraphicsComposition*	GetContainerComposition()override;
					void									SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
					void									SetText(const WString& value)override;
					void									SetFont(const FontProperties& value)override;
					void									SetVisuallyEnabled(bool value)override;
				};
			protected:

				StyleController*						styleController;
				bool									supressScrolling;

				void									OnContainerBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnHorizontalScroll(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnVerticalScroll(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnHorizontalWheel(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void									OnVerticalWheel(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void									CallUpdateView();
				void									Initialize();

				/// <summary>Calculate the full size of the content.</summary>
				/// <returns>The full size of the content.</returns>
				virtual Size							QueryFullSize()=0;
				/// <summary>Update the visible content using a view bounds. The view bounds is in the space from (0,0) to full size.</summary>
				/// <param name="viewBounds">The view bounds.</param>
				virtual void							UpdateView(Rect viewBounds)=0;
				/// <summary>Calculate the small move of the scroll bar.</summary>
				/// <returns>The small move of the scroll bar.</returns>
				virtual vint							GetSmallMove();
				/// <summary>Calculate the big move of the scroll bar.</summary>
				/// <returns>The big move of the scroll bar.</returns>
				virtual Size							GetBigMove();
				
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiScrollView(StyleController* _styleController);
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="styleProvider">The style provider.</param>
				GuiScrollView(IStyleProvider* styleProvider);
				~GuiScrollView();

				virtual void							SetFont(const FontProperties& value);

				/// <summary>Force to update contents and scroll bars.</summary>
				void									CalculateView();
				/// <summary>Get the view size.</summary>
				/// <returns>The view size.</returns>
				Size									GetViewSize();
				/// <summary>Get the view bounds.</summary>
				/// <returns>The view bounds.</returns>
				Rect									GetViewBounds();
				
				/// <summary>Get the horizontal scroll control.</summary>
				/// <returns>The horizontal scroll control.</returns>
				GuiScroll*								GetHorizontalScroll();
				/// <summary>Get the vertical scroll control.</summary>
				/// <returns>The vertical scroll control.</returns>
				GuiScroll*								GetVerticalScroll();
				/// <summary>Test is the horizontal scroll bar always visible even the content doesn't exceed the view bounds.</summary>
				/// <returns>Returns true if the horizontal scroll bar always visible even the content doesn't exceed the view bounds</returns>
				bool									GetHorizontalAlwaysVisible();
				/// <summary>Set is the horizontal scroll bar always visible even the content doesn't exceed the view bounds.</summary>
				/// <param name="value">Set to true if the horizontal scroll bar always visible even the content doesn't exceed the view bounds</param>
				void									SetHorizontalAlwaysVisible(bool value);
				/// <summary>Test is the vertical scroll bar always visible even the content doesn't exceed the view bounds.</summary>
				/// <returns>Returns true if the vertical scroll bar always visible even the content doesn't exceed the view bounds</returns>
				bool									GetVerticalAlwaysVisible();
				/// <summary>Set is the vertical scroll bar always visible even the content doesn't exceed the view bounds.</summary>
				/// <param name="value">Set to true if the vertical scroll bar always visible even the content doesn't exceed the view bounds</param>
				void									SetVerticalAlwaysVisible(bool value);
			};
			
			/// <summary>A control container with a vertical scroll bar and a horizontal scroll bar to perform partial viewing. When controls are added, removed, moved or resized, the scroll bars will adjust automatically.</summary>
			class GuiScrollContainer : public GuiScrollView, public Description<GuiScrollContainer>
			{
			public:
				/// <summary>Style controller for <see cref="GuiScrollContainer"/>.</summary>
				class StyleController : public GuiScrollView::StyleController, public Description<StyleController>
				{
				protected:
					compositions::GuiBoundsComposition*		controlContainerComposition;
					bool									extendToFullWidth;
				public:
					/// <summary>Create a style controller with a specified style provider.</summary>
					/// <param name="styleProvider">The style provider.</param>
					StyleController(GuiScrollView::IStyleProvider* styleProvider);
					~StyleController();

					compositions::GuiGraphicsComposition*	GetContainerComposition()override;
					/// <summary>Update sub controls using a specified left-top position of the view bounds.</summary>
					/// <param name="leftTop">The specified left-top position of the view bounds.</param>
					void									MoveContainer(Point leftTop);

					/// <summary>Test does the content container always extend its width to fill the scroll container.</summary>
					/// <returns>Return true if the content container always extend its width to fill the scroll container.</returns>
					bool									GetExtendToFullWidth();
					/// <summary>Set does the content container always extend its width to fill the scroll container.</summary>
					/// <param name="value">Set to true if the content container always extend its width to fill the scroll container.</param>
					void									SetExtendToFullWidth(bool value);
				};

			protected:
				StyleController*						styleController;

				void									OnControlContainerBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				Size									QueryFullSize()override;
				void									UpdateView(Rect viewBounds)override;
			public:
				/// <summary>Create a control with a specified style provider.</summary>
				/// <param name="styleProvider">The style provider.</param>
				GuiScrollContainer(GuiScrollContainer::IStyleProvider* styleProvider);
				~GuiScrollContainer();
				
				/// <summary>Test does the content container always extend its width to fill the scroll container.</summary>
				/// <returns>Return true if the content container always extend its width to fill the scroll container.</returns>
				bool									GetExtendToFullWidth();
				/// <summary>Set does the content container always extend its width to fill the scroll container.</summary>
				/// <param name="value">Set to true if the content container always extend its width to fill the scroll container.</param>
				void									SetExtendToFullWidth(bool value);
			};
		}
	}
}

#endif