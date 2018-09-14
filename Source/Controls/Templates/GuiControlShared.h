/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Template System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLSHARED
#define VCZH_PRESENTATION_CONTROLS_TEMPLATES_GUICONTROLSHARED

#include "../../GraphicsComposition/IncludeForward.h"
#include "../../Resources/GuiResource.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiControlHost;
			class GuiCustomControl;

			/// <summary>The visual state for button.</summary>
			enum class ButtonState
			{
				/// <summary>Normal state.</summary>
				Normal,
				/// <summary>Active state (when the cursor is hovering on a button).</summary>
				Active,
				/// <summary>Pressed state (when the buttin is being pressed).</summary>
				Pressed,
			};

			/// <summary>Represents the sorting state of list view items related to this column.</summary>
			enum class ColumnSortingState
			{
				/// <summary>Not sorted.</summary>
				NotSorted,
				/// <summary>Ascending.</summary>
				Ascending,
				/// <summary>Descending.</summary>
				Descending,
			};

			/// <summary>Represents the order of tab pages.</summary>
			enum class TabPageOrder
			{
				/// <summary>Unknown.</summary>
				Unknown,
				/// <summary>Left to right.</summary>
				LeftToRight,
				/// <summary>Right to left.</summary>
				RightToLeft,
				/// <summary>Top to bottom.</summary>
				TopToBottom,
				/// <summary>Bottom to top.</summary>
				BottomToTop,
			};

			/// <summary>A command executor for the combo box to change the control state.</summary>
			class ITextBoxCommandExecutor : public virtual IDescriptable, public Description<ITextBoxCommandExecutor>
			{
			public:
				/// <summary>Override the text content in the control.</summary>
				/// <param name="value">The new text content.</param>
				virtual void						UnsafeSetText(const WString& value) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IScrollCommandExecutor : public virtual IDescriptable, public Description<IScrollCommandExecutor>
			{
			public:
				/// <summary>Do small decrement.</summary>
				virtual void						SmallDecrease() = 0;
				/// <summary>Do small increment.</summary>
				virtual void						SmallIncrease() = 0;
				/// <summary>Do big decrement.</summary>
				virtual void						BigDecrease() = 0;
				/// <summary>Do big increment.</summary>
				virtual void						BigIncrease() = 0;

				/// <summary>Change to total size of the scroll.</summary>
				/// <param name="value">The total size.</param>
				virtual void						SetTotalSize(vint value) = 0;
				/// <summary>Change to page size of the scroll.</summary>
				/// <param name="value">The page size.</param>
				virtual void						SetPageSize(vint value) = 0;
				/// <summary>Change to position of the scroll.</summary>
				/// <param name="value">The position.</param>
				virtual void						SetPosition(vint value) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class ITabCommandExecutor : public virtual IDescriptable, public Description<ITabCommandExecutor>
			{
			public:
				/// <summary>Select a tab page.</summary>
				/// <param name="index">The specified position for the tab page.</param>
				/// <param name="setFocus">Set to true to set focus to the tab control.</param>
				virtual void						ShowTab(vint index, bool setFocus) = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IDatePickerCommandExecutor : public virtual IDescriptable, public Description<IDatePickerCommandExecutor>
			{
			public:
				/// <summary>Called when the date has been changed.</summary>
				virtual void						NotifyDateChanged() = 0;
				/// <summary>Called when navigated to a date.</summary>
				virtual void						NotifyDateNavigated() = 0;
				/// <summary>Called when selected a date.</summary>
				virtual void						NotifyDateSelected() = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IRibbonGroupCommandExecutor : public virtual IDescriptable, public Description<IRibbonGroupCommandExecutor>
			{
			public:
				/// <summary>Called when the expand button is clicked.</summary>
				virtual void						NotifyExpandButtonClicked() = 0;
			};

			/// <summary>A command executor for the style controller to change the control state.</summary>
			class IRibbonGalleryCommandExecutor : public virtual IDescriptable, public Description<IRibbonGalleryCommandExecutor>
			{
			public:
				/// <summary>Called when the scroll up button is clicked.</summary>
				virtual void						NotifyScrollUp() = 0;
				/// <summary>Called when the scroll down button is clicked.</summary>
				virtual void						NotifyScrollDown() = 0;
				/// <summary>Called when the dropdown button is clicked.</summary>
				virtual void						NotifyDropdown() = 0;
			};

			class GuiInstanceRootObject;

			/// <summary>
			/// Represnets a component.
			/// </summary>
			class GuiComponent : public Object, public Description<GuiComponent>
			{
			public:
				GuiComponent();
				~GuiComponent();

				virtual void							Attach(GuiInstanceRootObject* rootObject);
				virtual void							Detach(GuiInstanceRootObject* rootObject);
			};

/***********************************************************************
Animation
***********************************************************************/

			/// <summary>Animation.</summary>
			class IGuiAnimation abstract : public virtual IDescriptable, public Description<IGuiAnimation>
			{
			public:
				/// <summary>Called when the animation is about to play the first frame.</summary>
				virtual void							Start() = 0;

				/// <summary>Called when the animation is about to pause.</summary>
				virtual void							Pause() = 0;

				/// <summary>Called when the animation is about to resume.</summary>
				virtual void							Resume() = 0;

				/// <summary>Play the animation. The animation should calculate the time itself to determine the content of the current state of animating objects.</summary>
				virtual void							Run() = 0;

				/// <summary>Test if the animation has ended.</summary>
				/// <returns>Returns true if the animation has ended.</returns>
				virtual bool							GetStopped() = 0;

				/// <summary>Create a finite animation.</summary>
				/// <returns>Returns the created animation.</returns>
				/// <param name="run">The animation callback for each frame.</param>
				/// <param name="milliseconds">The length of the animation.</param>
				static Ptr<IGuiAnimation>				CreateAnimation(const Func<void(vuint64_t)>& run, vuint64_t milliseconds);

				/// <summary>Create an infinite animation.</summary>
				/// <returns>Returns the created animation.</returns>
				/// <param name="run">The animation callback for each frame.</param>
				static Ptr<IGuiAnimation>				CreateAnimation(const Func<void(vuint64_t)>& run);
			};

/***********************************************************************
Root Object
***********************************************************************/

			class RootObjectTimerCallback;

			/// <summary>Represnets a root GUI object.</summary>
			class GuiInstanceRootObject abstract : public Description<GuiInstanceRootObject>
			{
				friend class RootObjectTimerCallback;
				typedef collections::List<Ptr<description::IValueSubscription>>		SubscriptionList;
			protected:
				Ptr<GuiResourcePathResolver>					resourceResolver;
				SubscriptionList								subscriptions;
				collections::SortedList<GuiComponent*>			components;
				Ptr<RootObjectTimerCallback>					timerCallback;
				collections::SortedList<Ptr<IGuiAnimation>>		runningAnimations;
				collections::SortedList<Ptr<IGuiAnimation>>		pendingAnimations;
				bool											finalized = false;

				virtual controls::GuiControlHost*				GetControlHostForInstance() = 0;
				void											InstallTimerCallback(controls::GuiControlHost* controlHost);
				bool											UninstallTimerCallback(controls::GuiControlHost* controlHost);
				void											OnControlHostForInstanceChanged();
				void											StartPendingAnimations();
			public:
				GuiInstanceRootObject();
				~GuiInstanceRootObject();

				/// <summary>Clear all subscriptions and components.</summary>
				void											FinalizeInstance();

				/// <summary>Test has the object been finalized.</summary>
				/// <returns>Returns true if this object has been finalized.</returns>
				bool											IsFinalized();

				void											FinalizeInstanceRecursively(templates::GuiTemplate* thisObject);
				void											FinalizeInstanceRecursively(GuiCustomControl* thisObject);
				void											FinalizeInstanceRecursively(GuiControlHost* thisObject);
				void											FinalizeGeneralInstance(GuiInstanceRootObject* thisObject);

				/// <summary>Set the resource resolver to connect the current root object to the resource creating it.</summary>
				/// <param name="resolver">The resource resolver</param>
				void											SetResourceResolver(Ptr<GuiResourcePathResolver> resolver);
				/// <summary>Resolve a resource using the current resource resolver.</summary>
				/// <returns>The loaded resource. Returns null if failed to load.</returns>
				/// <param name="protocol">The protocol.</param>
				/// <param name="path">The path.</param>
				/// <param name="ensureExist">Set to true and it will throw an exception if the resource doesn't exist.</param>
				Ptr<DescriptableObject>							ResolveResource(const WString& protocol, const WString& path, bool ensureExist);

				/// <summary>Add a subscription. When this control host is disposing, all attached subscriptions will be deleted.</summary>
				/// <returns>Returns null if this operation failed.</returns>
				/// <param name="subscription">The subscription to test.</param>
				Ptr<description::IValueSubscription>			AddSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Clear all subscriptions.</summary>
				void											UpdateSubscriptions();

				/// <summary>Add a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to add.</param>
				bool											AddComponent(GuiComponent* component);

				/// <summary>Add a control host as a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="controlHost">The controlHost to add.</param>
				bool											AddControlHostComponent(GuiControlHost* controlHost);

				/// <summary>Add an animation. The animation will be paused if the root object is removed from a window.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="animation">The animation.</param>
				bool											AddAnimation(Ptr<IGuiAnimation> animation);

				/// <summary>Kill an animation.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="animation">The animation.</param>
				bool											KillAnimation(Ptr<IGuiAnimation> animation);
			};
		}
	}
}

#endif