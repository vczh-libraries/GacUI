/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Graphics Composition Host

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST
#define VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST

#include "GuiGraphicsHost_ShortcutKey.h"
#include "GuiGraphicsHost_Alt.h"
#include "GuiGraphicsHost_Tab.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;
		}

		namespace compositions
		{

/***********************************************************************
Animation
***********************************************************************/

			/// <summary>
			/// Represents a timer callback object.
			/// </summary>
			class IGuiGraphicsTimerCallback : public virtual IDescriptable, public Description<IGuiGraphicsTimerCallback>
			{
			public:
				/// <summary>Called periodically.</summary>
				/// <returns>Returns false to indicate that this callback need to be removed.</returns>
				virtual bool					Play() = 0;
			};

			/// <summary>
			/// Timer callback manager.
			/// </summary>
			class GuiGraphicsTimerManager : public Object, public Description<GuiGraphicsTimerManager>
			{
				typedef collections::List<Ptr<IGuiGraphicsTimerCallback>>		CallbackList;
			protected:
				CallbackList					callbacks;

			public:
				GuiGraphicsTimerManager();
				~GuiGraphicsTimerManager();

				/// <summary>Add a new callback.</summary>
				/// <param name="callback">The new callback to add.</param>
				void							AddCallback(Ptr<IGuiGraphicsTimerCallback> callback);
				/// <summary>Called periodically.</summary>
				void							Play();
			};

/***********************************************************************
Host
***********************************************************************/

			/// <summary>
			/// GuiGraphicsHost hosts an <see cref="GuiWindowComposition"/> in an <see cref="INativeWindow"/>. The composition will fill the whole window.
			/// </summary>
			class GuiGraphicsHost : public Object, private INativeWindowListener, private INativeControllerListener, public Description<GuiGraphicsHost>
			{
				typedef collections::List<GuiGraphicsComposition*>							CompositionList;
				typedef GuiGraphicsComposition::GraphicsHostRecord							HostRecord;
				typedef collections::Pair<DescriptableObject*, vint>						ProcKey;
				typedef collections::List<Func<void()>>										ProcList;
				typedef collections::Dictionary<ProcKey, Func<void()>>						ProcMap;
			public:
				static const vuint64_t					CaretInterval = 500;

			protected:
				HostRecord								hostRecord;
				bool									supressPaint = false;
				bool									needRender = true;
				ProcList								afterRenderProcs;
				ProcMap									afterRenderKeyedProcs;

				GuiAltActionManager*					altActionManager = nullptr;
				GuiTabActionManager*					tabActionManager = nullptr;
				IGuiShortcutKeyManager*					shortcutKeyManager = nullptr;

				controls::GuiControlHost*				controlHost = nullptr;
				GuiWindowComposition*					windowComposition = nullptr;
				GuiGraphicsComposition*					focusedComposition = nullptr;
				NativeSize								previousClientSize;
				Size									minSize;
				Point									caretPoint;
				vuint64_t								lastCaretTime = 0;

				GuiGraphicsTimerManager					timerManager;
				GuiGraphicsComposition*					mouseCaptureComposition = nullptr;
				CompositionList							mouseEnterCompositions;
				void									RefreshRelatedHostRecord(INativeWindow* nativeWindow);

				void									DisconnectCompositionInternal(GuiGraphicsComposition* composition);
				void									MouseCapture(const NativeWindowMouseInfo& info);
				void									MouseUncapture(const NativeWindowMouseInfo& info);
				void									OnCharInput(const NativeWindowCharInfo& info, GuiGraphicsComposition* composition, GuiCharEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				void									OnKeyInput(const NativeWindowKeyInfo& info, GuiGraphicsComposition* composition, GuiKeyEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				void									RaiseMouseEvent(GuiMouseEventArgs& arguments, GuiGraphicsComposition* composition, GuiMouseEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				void									OnMouseInput(const NativeWindowMouseInfo& info, GuiMouseEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				void									RecreateRenderTarget();
				
			private:
				INativeWindowListener::HitTestResult	HitTest(NativePoint location)override;
				void									Moving(NativeRect& bounds, bool fixSizeOnly)override;
				void									Moved()override;
				void									DpiChanged()override;
				void									Paint()override;

				void									LeftButtonDown(const NativeWindowMouseInfo& info)override;
				void									LeftButtonUp(const NativeWindowMouseInfo& info)override;
				void									LeftButtonDoubleClick(const NativeWindowMouseInfo& info)override;
				void									RightButtonDown(const NativeWindowMouseInfo& info)override;
				void									RightButtonUp(const NativeWindowMouseInfo& info)override;
				void									RightButtonDoubleClick(const NativeWindowMouseInfo& info)override;
				void									MiddleButtonDown(const NativeWindowMouseInfo& info)override;
				void									MiddleButtonUp(const NativeWindowMouseInfo& info)override;
				void									MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)override;
				void									HorizontalWheel(const NativeWindowMouseInfo& info)override;
				void									VerticalWheel(const NativeWindowMouseInfo& info)override;
				void									MouseMoving(const NativeWindowMouseInfo& info)override;
				void									MouseEntered()override;
				void									MouseLeaved()override;

				void									KeyDown(const NativeWindowKeyInfo& info)override;
				void									KeyUp(const NativeWindowKeyInfo& info)override;
				void									SysKeyDown(const NativeWindowKeyInfo& info)override;
				void									SysKeyUp(const NativeWindowKeyInfo& info)override;
				void									Char(const NativeWindowCharInfo& info)override;

				void									GlobalTimer()override;
			public:
				GuiGraphicsHost(controls::GuiControlHost* _controlHost, GuiGraphicsComposition* boundsComposition);
				~GuiGraphicsHost();

				/// <summary>Get the associated window.</summary>
				/// <returns>The associated window.</returns>
				INativeWindow*							GetNativeWindow();
				/// <summary>Associate a window. A <see cref="GuiWindowComposition"/> will fill and appear in the window.</summary>
				/// <param name="_nativeWindow">The window to associated.</param>
				void									SetNativeWindow(INativeWindow* _nativeWindow);
				/// <summary>Get the main <see cref="GuiWindowComposition"/>. If a window is associated, everything that put into the main composition will be shown in the window.</summary>
				/// <returns>The main compositoin.</returns>
				GuiGraphicsComposition*					GetMainComposition();
				/// <summary>Render the main composition and all content to the associated window.</summary>
				/// <param name="forceUpdate">Set to true to force updating layout and then render.</param>
				void									Render(bool forceUpdate);
				/// <summary>Request a rendering</summary>
				void									RequestRender();
				/// <summary>Invoke a specified function after rendering.</summary>
				/// <param name="proc">The specified function.</param>
				/// <param name="key">A key to cancel a previous binded key if not null.</param>
				void									InvokeAfterRendering(const Func<void()>& proc, ProcKey key = { nullptr,-1 });

				/// <summary>Invalidte the internal tab order control list. Next time when TAB is pressed it will be rebuilt.</summary>
				void									InvalidateTabOrderCache();
				/// <summary>Get the <see cref="IGuiShortcutKeyManager"/> attached with this graphics host.</summary>
				/// <returns>The shortcut key manager.</returns>
				IGuiShortcutKeyManager*					GetShortcutKeyManager();
				/// <summary>Attach or detach the <see cref="IGuiShortcutKeyManager"/> associated with this graphics host. When this graphics host is disposing, the associated shortcut key manager will be deleted if exists.</summary>
				/// <param name="value">The shortcut key manager. Set to null to detach the previous shortcut key manager from this graphics host.</param>
				void									SetShortcutKeyManager(IGuiShortcutKeyManager* value);

				/// <summary>Set the focus composition. A focused composition will receive keyboard messages.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="composition">The composition to set focus. This composition should be or in the main composition.</param>
				bool									SetFocus(GuiGraphicsComposition* composition);
				/// <summary>Get the focus composition. A focused composition will receive keyboard messages.</summary>
				/// <returns>The focus composition.</returns>
				GuiGraphicsComposition*					GetFocusedComposition();
				/// <summary>Get the caret point. A caret point is the position to place the edit box of the activated input method editor.</summary>
				/// <returns>The caret point.</returns>
				Point									GetCaretPoint();
				/// <summary>Set the caret point. A caret point is the position to place the edit box of the activated input method editor.</summary>
				/// <param name="value">The caret point.</param>
				/// <param name="referenceComposition">The point space. If this argument is null, the "value" argument will use the point space of the client area in the main composition.</param>
				void									SetCaretPoint(Point value, GuiGraphicsComposition* referenceComposition=0);

				/// <summary>Get the timer manager.</summary>
				/// <returns>The timer manager.</returns>
				GuiGraphicsTimerManager*				GetTimerManager();
				/// <summary>Notify that a composition is going to disconnect from this graphics host. Generally this happens when a composition's parent line changes.</summary>
				/// <param name="composition">The composition to disconnect</param>
				void									DisconnectComposition(GuiGraphicsComposition* composition);
			};
		}
	}
}

#endif