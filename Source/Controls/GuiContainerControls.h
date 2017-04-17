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

			class GuiTab;

			/// <summary>Represents a page of a <see cref="GuiTab"/>. A tab page is not a control.</summary>
			class GuiTabPage : public Object, protected compositions::IGuiAltActionHost, public Description<GuiTabPage>
			{
				friend class GuiTab;
				friend class Ptr<GuiTabPage>;
			protected:
				GuiControl*										containerControl;
				GuiTab*											owner;
				WString											alt;
				WString											text;
				compositions::IGuiAltActionHost*				previousAltHost;

				bool											AssociateTab(GuiTab* _owner);
				bool											DeassociateTab(GuiTab* _owner);
				compositions::GuiGraphicsComposition*			GetAltComposition()override;
				compositions::IGuiAltActionHost*				GetPreviousAltHost()override;
				void											OnActivatedAltHost(compositions::IGuiAltActionHost* previousHost)override;
				void											OnDeactivatedAltHost()override;
				void											CollectAltActions(collections::Group<WString, compositions::IGuiAltAction*>& actions)override;
			public:
				/// <summary>Create a tab page.</summary>
				GuiTabPage();
				~GuiTabPage();
				
				/// <summary>Alt changed event.</summary>
				compositions::GuiNotifyEvent					AltChanged;
				/// <summary>Text changed event.</summary>
				compositions::GuiNotifyEvent					TextChanged;
				/// <summary>Page installed event.</summary>
				compositions::GuiNotifyEvent					PageInstalled;
				/// <summary>Page installed event.</summary>
				compositions::GuiNotifyEvent					PageUninstalled;

				/// <summary>Get the container control to store all sub controls.</summary>
				/// <returns>The container control to store all sub controls.</returns>
				compositions::GuiGraphicsComposition*			GetContainerComposition();
				/// <summary>Get the owner <see cref="GuiTab"/>.</summary>
				/// <returns>The owner <see cref="GuiTab"/>.</returns>
				GuiTab*											GetOwnerTab();
				/// <summary>Get the Alt-combined shortcut key associated with this control.</summary>
				/// <returns>The Alt-combined shortcut key associated with this control.</returns>
				const WString&									GetAlt();
				/// <summary>Associate a Alt-combined shortcut key with this control.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="value">The Alt-combined shortcut key to associate. Only zero, sigle or multiple upper case letters are legal.</param>
				bool											SetAlt(const WString& value);
				/// <summary>Get the text rendered as the name for this page.</summary>
				/// <returns>The text rendered as the name for this page.</returns>
				const WString&									GetText();
				/// <summary>Set the text rendered as the name for this page.</summary>
				/// <param name="value">The text rendered as the name for this page.</param>
				void											SetText(const WString& value);
				/// <summary>Test is this page selected.</summary>
				/// <returns>Returns true if this page is selected.</returns>
				bool											GetSelected();
			};

			/// <summary>Represents a container with multiple named tabs.</summary>
			class GuiTab : public GuiControl, protected compositions::IGuiAltActionContainer, public Description<GuiTab>
			{
				friend class GuiTabPage;
			public:
				/// <summary>A command executor for the style controller to change the control state.</summary>
				class ICommandExecutor : public virtual IDescriptable, public Description<ICommandExecutor>
				{
				public:
					/// <summary>Select a tab page.</summary>
					/// <param name="index">The specified position for the tab page.</param>
					virtual void								ShowTab(vint index)=0;
				};
				
				/// <summary>Style controller interface for <see cref="GuiTab"/>.</summary>
				class IStyleController : public virtual GuiControl::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Called when the command executor is changed.</summary>
					/// <param name="value">The command executor.</param>
					virtual void								SetCommandExecutor(ICommandExecutor* value)=0;
					/// <summary>Insert a tab header at the specified position.</summary>
					/// <param name="index">The specified position.</param>
					virtual void								InsertTab(vint index)=0;
					/// <summary>Set the text of a tab header at the specified position.</summary>
					/// <param name="index">The specified position.</param>
					/// <param name="value">The text.</param>
					virtual void								SetTabText(vint index, const WString& value)=0;
					/// <summary>Remove the tab header at the specified position.</summary>
					/// <param name="index">The specified position.</param>
					virtual void								RemoveTab(vint index)=0;
					/// <summary>Move a tab header from a position to another.</summary>
					/// <param name="oldIndex">The old position.</param>
					/// <param name="newIndex">The new position.</param>
					virtual void								MoveTab(vint oldIndex, vint newIndex)=0;
					/// <summary>Render a tab header at the specified position as selected.</summary>
					/// <param name="index">The specified position.</param>
					virtual void								SetSelectedTab(vint index)=0;
					/// <summary>Set the Alt-combined shortcut key of a tab header at the specified position.</summary>
					/// <param name="index">The specified position.</param>
					/// <param name="value">The Alt-combined shortcut key.</param>
					/// <param name="host">The alt action host object.</param>
					virtual void								SetTabAlt(vint index, const WString& value, compositions::IGuiAltActionHost* host)=0;
					/// <summary>Get the associated <see cref="compositions::IGuiAltAction"/> object of a tab header at the specified position.</summary>
					/// <returns>The associated <see cref="compositions::IGuiAltAction"/> object.</returns>
					/// <param name="index">The specified position.</param>
					virtual compositions::IGuiAltAction*		GetTabAltAction(vint index) = 0;
				};
			protected:
				class CommandExecutor : public Object, public ICommandExecutor
				{
				protected:
					GuiTab*										tab;
				public:
					CommandExecutor(GuiTab* _tab);
					~CommandExecutor();

					void										ShowTab(vint index)override;
				};

				Ptr<CommandExecutor>							commandExecutor;
				IStyleController*								styleController;
				collections::List<GuiTabPage*>					tabPages;
				GuiTabPage*										selectedPage;

				vint											GetAltActionCount()override;
				compositions::IGuiAltAction*					GetAltAction(vint index)override;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiTab(IStyleController* _styleController);
				~GuiTab();

				IDescriptable*									QueryService(const WString& identifier)override;

				/// <summary>Selected page changed event.</summary>
				compositions::GuiNotifyEvent					SelectedPageChanged;

				/// <summary>Create a tag page at the specified index.</summary>
				/// <returns>The created page.</returns>
				/// <param name="index">The specified index. Set to -1 to insert at the last position.</param>
				GuiTabPage*										CreatePage(vint index=-1);
				/// <summary>Insert a tag page at the specified index.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="page">The tab page to insert. This page should be a new page that has never been inserted to a <see cref="GuiTab"/>.</param>
				/// <param name="index">The specified index. Set to -1 to insert at the last position.</param>
				bool											CreatePage(GuiTabPage* page, vint index=-1);
				/// <summary>Remove the tag page at the specified index.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="page">The tab page to remove.</param>
				bool											RemovePage(GuiTabPage* page);
				/// <summary>Move a tag page at the specified index to a new position.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="page">The tab page to move.</param>
				/// <param name="newIndex">The new position.</param>
				bool											MovePage(GuiTabPage* page, vint newIndex);
				/// <summary>Get all pages.</summary>
				/// <returns>All pages.</returns>
				const collections::List<GuiTabPage*>&			GetPages();

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