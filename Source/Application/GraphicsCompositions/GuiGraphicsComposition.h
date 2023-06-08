/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Composition System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSCOMPOSITION
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSCOMPOSITION

#include "../../GraphicsElement/GuiGraphicsElementInterfaces.h"
#include "GuiGraphicsEventReceiver.h"

namespace vl
{
	namespace presentation
	{
		template<typename T>
		using ItemProperty = Func<T(const reflection::description::Value&)>;

		template<typename T>
		using WritableItemProperty = Func<T(const reflection::description::Value&, T, bool)>;

		template<typename T>
		using TemplateProperty = Func<T*(const reflection::description::Value&)>;

		namespace templates
		{
			class GuiTemplate;
		}

		namespace controls
		{
			class GuiControl;
			class GuiControlHost;
		}

		namespace compositions
		{
			class GuiWindowComposition;
			class GuiGraphicsHost;

/***********************************************************************
Basic Construction
***********************************************************************/

			/// <summary>
			/// Represents a composition for <see cref="elements::IGuiGraphicsElement"/>. A composition is a way to define the size and the position using the information from graphics elements and sub compositions.
			/// When a graphics composition is destroyed, all sub composition will be destroyed. The life cycle of the contained graphics element is partially controlled by the smart pointer to the graphics element inside the composition.
			/// </summary>
			class GuiGraphicsComposition : public Object, public Description<GuiGraphicsComposition>
			{
				typedef collections::List<GuiGraphicsComposition*> CompositionList;

				friend class controls::GuiControl;
				friend class GuiWindowComposition;
				friend class GuiGraphicsHost;
			public:
				/// <summary>
				/// Minimum size limitation.
				/// </summary>
				enum MinSizeLimitation
				{
					/// <summary>No limitation for the minimum size.</summary>
					NoLimit,
					/// <summary>Minimum size of this composition is the minimum size of the contained graphics element.</summary>
					LimitToElement,
					/// <summary>Minimum size of this composition is combiniation of sub compositions and the minimum size of the contained graphics element.</summary>
					LimitToElementAndChildren,
				};

			protected:

				struct GraphicsHostRecord
				{
					GuiGraphicsHost*						host = nullptr;
					elements::IGuiGraphicsRenderTarget*		renderTarget = nullptr;
					INativeWindow*							nativeWindow = nullptr;
				};

			private:
				bool										isRendering = false;
				bool										isTrivialComposition = true;

				CompositionList								children;
				GuiGraphicsComposition*						parent = nullptr;
				Ptr<elements::IGuiGraphicsElement>			ownedElement;
				bool										visible = true;
				bool										transparentToMouse = false;
				MinSizeLimitation							minSizeLimitation = MinSizeLimitation::NoLimit;

				Ptr<compositions::GuiGraphicsEventReceiver>	eventReceiver;
				GraphicsHostRecord*							relatedHostRecord = nullptr;
				controls::GuiControl*						associatedControl = nullptr;
				INativeCursor*								associatedCursor = nullptr;
				INativeWindowListener::HitTestResult		associatedHitTestResult = INativeWindowListener::NoDecision;

			protected:
				Margin										internalMargin;
				Size										preferredMinSize;

				virtual void								OnControlParentChanged(controls::GuiControl* control);
				virtual void								OnChildInserted(GuiGraphicsComposition* child);
				virtual void								OnChildRemoved(GuiGraphicsComposition* child);
				virtual void								OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent);
				virtual void								OnParentLineChanged();
				virtual void								OnRenderContextChanged();
				
				void										UpdateRelatedHostRecord(GraphicsHostRecord* record);
				void										SetAssociatedControl(controls::GuiControl* control);
				void										InvokeOnCompositionStateChanged();

			protected:
				static bool									SharedPtrDestructorProc(DescriptableObject* obj, bool forceDisposing);

			public:
				GuiGraphicsComposition(bool _isTrivialComposition);
				~GuiGraphicsComposition();

				bool										IsRendering();

				/// <summary>Test is this composition a trivial composition. A trivial composition means its parent doesn't do the layout for it.</summary>
				/// <returns>Returns true if it is a trivial composition.</returns>
				bool										IsTrivialComposition();

				/// <summary>Get the parent composition.</summary>
				/// <returns>The parent composition.</returns>
				GuiGraphicsComposition*						GetParent();
				/// <summary>Get all child compositions ordered by z-order from low to high.</summary>
				/// <returns>Child compositions.</returns>
				const CompositionList&						Children();
				/// <summary>Add a composition as a child.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="child">The child composition to add.</param>
				bool										AddChild(GuiGraphicsComposition* child);
				/// <summary>Add a composition as a child with a specified z-order.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="index">The z-order. 0 means the lowest position.</param>
				/// <param name="child">The child composition to add.</param>
				bool										InsertChild(vint index, GuiGraphicsComposition* child);
				/// <summary>Remove a child composition.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="child">The child composition to remove.</param>
				bool										RemoveChild(GuiGraphicsComposition* child);
				/// <summary>Move a child composition to a new z-order.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="child">The child composition to move.</param>
				/// <param name="newIndex">The new z-order. 0 means the lowest position.</param>
				bool										MoveChild(GuiGraphicsComposition* child, vint newIndex);

				/// <summary>Get the contained graphics element.</summary>
				/// <returns>The contained graphics element.</returns>
				Ptr<elements::IGuiGraphicsElement>			GetOwnedElement();
				/// <summary>Set the contained graphics element.</summary>
				/// <param name="element">The new graphics element to set.</param>
				void										SetOwnedElement(Ptr<elements::IGuiGraphicsElement> element);
				/// <summary>Get the visibility of the composition.</summary>
				/// <returns>Returns true if the composition is visible.</returns>
				bool										GetVisible();
				/// <summary>Set the visibility of the composition.</summary>
				/// <param name="value">Set to true to make the composition visible.</param>
				void										SetVisible(bool value);
				/// <summary>Get the visibility of the composition all the way to the root.</summary>
				/// <returns>Returns true if the composition and all ancestors are visible.</returns>
				bool										GetEventuallyVisible();
				/// <summary>Get the minimum size limitation of the composition.</summary>
				/// <returns>The minimum size limitation of the composition.</returns>
				MinSizeLimitation							GetMinSizeLimitation();
				/// <summary>Set the minimum size limitation of the composition.</summary>
				/// <param name="value">The minimum size limitation of the composition.</param>
				void										SetMinSizeLimitation(MinSizeLimitation value);
				/// <summary>Get the binded render target.</summary>
				/// <returns>The binded render target.</returns>
				elements::IGuiGraphicsRenderTarget*			GetRenderTarget();

				/// <summary>Render the composition using an offset.</summary>
				/// <param name="offset">The offset.</param>
				void										Render(Size offset);
				/// <summary>Get the event receiver object. All user input events can be found in this object. If an event receiver is never been requested from the composition, the event receiver will not be created, and all route events will not pass through this event receiver(performance will be better).</summary>
				/// <returns>The event receiver.</returns>
				compositions::GuiGraphicsEventReceiver*		GetEventReceiver();
				/// <summary>Test if any event receiver has already been requested.</summary>
				/// <returns>Returns true if any event receiver has already been requested.</returns>
				bool										HasEventReceiver();
				/// <summary>Find a deepest composition that under a specified location. If the location is inside a compsition but not hit any sub composition, this function will return this composition.</summary>
				/// <returns>The deepest composition that under a specified location.</returns>
				/// <param name="location">The specified location.</param>
				/// <param name="forMouseEvent">Find a composition for mouse event, it will ignore all compositions that are transparent to mouse events.</param>
				GuiGraphicsComposition*						FindComposition(Point location, bool forMouseEvent);
				/// <summary>Get is this composition transparent to mouse events.</summary>
				/// <returns>Returns true if this composition is transparent to mouse events, which means it just passes all mouse events to the composition under it.</returns>
				bool										GetTransparentToMouse();
				/// <summary>Set is the composition transparent to mouse events.</summary>
				/// <param name="value">Set to true to make this composition transparent to mouse events.</param>
				void										SetTransparentToMouse(bool value);
				/// <summary>Get the bounds in the top composition space.</summary>
				/// <returns>The bounds in the top composition space.</returns>
				Rect										GetGlobalBounds();

				/// <summary>Get the associated control. A control is associated to a composition only when the composition represents the bounds of this control. Such a composition usually comes from a control template.</summary>
				/// <returns>The associated control.</returns>
				controls::GuiControl*						GetAssociatedControl();
				/// <summary>Get the associated graphics host. A graphics host is associated to a composition only when the composition becomes the bounds of the graphics host.</summary>
				/// <returns>The associated graphics host.</returns>
				GuiGraphicsHost*							GetAssociatedHost();
				/// <summary>Get the associated cursor.</summary>
				/// <returns>The associated cursor.</returns>
				INativeCursor*								GetAssociatedCursor();
				/// <summary>Set the associated cursor.</summary>
				/// <param name="cursor">The associated cursor.</param>
				void										SetAssociatedCursor(INativeCursor* cursor);
				/// <summary>Get the associated hit test result.</summary>
				/// <returns>The associated hit test result.</returns>
				INativeWindowListener::HitTestResult		GetAssociatedHitTestResult();
				/// <summary>Set the associated hit test result.</summary>
				/// <param name="value">The associated hit test result.</param>
				void										SetAssociatedHitTestResult(INativeWindowListener::HitTestResult value);
				
				/// <summary>Get the related control. A related control is the deepest control that contains this composition.</summary>
				/// <returns>The related control.</returns>
				controls::GuiControl*						GetRelatedControl();
				/// <summary>Get the related graphics host. A related graphics host is the graphics host that contains this composition.</summary>
				/// <returns>The related graphics host.</returns>
				GuiGraphicsHost*							GetRelatedGraphicsHost();
				/// <summary>Get the related control host. A related control host is the control host that contains this composition.</summary>
				/// <returns>The related control host.</returns>
				controls::GuiControlHost*					GetRelatedControlHost();
				/// <summary>Get the related cursor. A related cursor is from the deepest composition that contains this composition and associated with a cursor.</summary>
				/// <returns>The related cursor.</returns>
				INativeCursor*								GetRelatedCursor();
				
				/// <summary>Get the internal margin.</summary>
				/// <returns>The internal margin.</returns>
				Margin										GetInternalMargin();
				/// <summary>Set the internal margin.</summary>
				/// <param name="value">The internal margin.</param>
				void										SetInternalMargin(Margin value);
				/// <summary>Get the preferred minimum size.</summary>
				/// <returns>The preferred minimum size.</returns>
				Size										GetPreferredMinSize();
				/// <summary>Set the preferred minimum size.</summary>
				/// <param name="value">The preferred minimum size.</param>
				void										SetPreferredMinSize(Size value);

			protected:
				Size										cachedMinSize;
				Rect										cachedBounds;

				virtual Size								Layout_CalculateMinSize();
				virtual Size								Layout_CalculateMinClientSizeForParent(Margin parentInternalMargin);
				virtual Rect								Layout_CalculateBounds(Rect parentBounds);
				void										Layout_SetCachedMinSize(Size value);
				void										Layout_SetCachedBounds(Rect value);
				void										Layout_UpdateMinSize();
				void										Layout_UpdateBounds(Rect parentBounds);
			public:

				/// <summary>Event that will be raised when the minimum size is updated.</summary>
				compositions::GuiNotifyEvent				CachedMinSizeChanged;

				/// <summary>Event that will be raised when the bounds is updated.</summary>
				compositions::GuiNotifyEvent				CachedBoundsChanged;

				/// <summary>Get the updated minimum size.</summary>
				/// <returns>The updated minimum size.</returns>
				Size										GetCachedMinSize();

				/// <summary>Get the updated minimum client size. It is the minimum size removing the internal margin.</summary>
				/// <returns>The updated minimum client size.</returns>
				Size										GetCachedMinClientSize();

				/// <summary>Get the updated bounds.</summary>
				/// <returns>The updated bounds.</returns>
				Rect										GetCachedBounds();

				/// <summary>Get the updated client bounds. It is the bounds removing the internal margin.</summary>
				/// <returns>The updated client bounds.</returns>
				Rect										GetCachedClientArea();
			};

/***********************************************************************
Helper Functions
***********************************************************************/

			/// <summary>Call [M:vl.presentation.controls.GuiInstanceRootObject.FinalizeInstance] in all child root objects.</summary>
			/// <param name="value">The container control to notify.</param>
			extern void								NotifyFinalizeInstance(controls::GuiControl* value);

			/// <summary>Call [M:vl.presentation.controls.GuiInstanceRootObject.FinalizeInstance] in all child root objects.</summary>
			/// <param name="value">The container composition to notify.</param>
			extern void								NotifyFinalizeInstance(GuiGraphicsComposition* value);

			/// <summary>Safely remove and delete a control.</summary>
			/// <param name="value">The control to delete.</param>
			extern void								SafeDeleteControl(controls::GuiControl* value);

			/// <summary>Safely remove and delete a composition. If some sub compositions are controls, those controls will be deleted too.</summary>
			/// <param name="value">The composition to delete.</param>
			extern void								SafeDeleteComposition(GuiGraphicsComposition* value);
		}
	}
}

#endif