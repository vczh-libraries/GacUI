/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Graphics Composition Host

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSHOST
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSHOST

#include "../GraphicsComposition/GuiGraphicsComposition.h"

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
Shortcut Key Manager
***********************************************************************/

			class IGuiShortcutKeyManager;

			/// <summary>Shortcut key item.</summary>
			class IGuiShortcutKeyItem : public virtual IDescriptable, public Description<IGuiShortcutKeyItem>
			{
			public:
				/// <summary>Shortcut key executed event.</summary>
				GuiNotifyEvent							Executed;

				/// <summary>Get the associated <see cref="IGuiShortcutKeyManager"/> object.</summary>
				/// <returns>The associated shortcut key manager.</returns>
				virtual IGuiShortcutKeyManager*			GetManager()=0;
				/// <summary>Get the name represents the shortcut key combination for this item.</summary>
				/// <returns>The name represents the shortcut key combination for this item.</returns>
				virtual WString							GetName()=0;
			};
			
			/// <summary>Shortcut key manager item.</summary>
			class IGuiShortcutKeyManager : public virtual IDescriptable, public Description<IGuiShortcutKeyManager>
			{
			public:
				/// <summary>Get the number of shortcut key items that already attached to the manager.</summary>
				/// <returns>T number of shortcut key items that already attached to the manager.</returns>
				virtual vint							GetItemCount()=0;
				/// <summary>Get the <see cref="IGuiShortcutKeyItem"/> associated with the index.</summary>
				/// <returns>The shortcut key item.</returns>
				/// <param name="index">The index.</param>
				virtual IGuiShortcutKeyItem*			GetItem(vint index)=0;
				/// <summary>Execute shortcut key items using a key event info.</summary>
				/// <returns>Returns true if at least one shortcut key item is executed.</returns>
				/// <param name="info">The key event info.</param>
				virtual bool							Execute(const NativeWindowKeyInfo& info)=0;
			};

/***********************************************************************
Alt-Combined Shortcut Key Interfaces
***********************************************************************/

			class IGuiAltActionHost;
			
			/// <summary>IGuiAltAction is the handler when an alt-combined shortcut key is activated.</summary>
			class IGuiAltAction : public virtual IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				static bool								IsLegalAlt(const WString& alt);

				virtual const WString&					GetAlt() = 0;
				virtual bool							IsAltEnabled() = 0;
				virtual bool							IsAltAvailable() = 0;
				virtual GuiGraphicsComposition*			GetAltComposition() = 0;
				virtual IGuiAltActionHost*				GetActivatingAltHost() = 0;
				virtual void							OnActiveAlt() = 0;
			};
			
			/// <summary>IGuiAltActionContainer enumerates multiple <see cref="IGuiAltAction"/>.</summary>
			class IGuiAltActionContainer : public virtual IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				virtual vint							GetAltActionCount() = 0;
				virtual IGuiAltAction*					GetAltAction(vint index) = 0;
			};
			
			/// <summary>IGuiAltActionHost is an alt-combined shortcut key host. A host can also be entered or leaved, with multiple sub actions enabled or disabled.</summary>
			class IGuiAltActionHost : public virtual IDescriptable
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				static void								CollectAltActionsFromControl(controls::GuiControl* control, bool includeThisControl, collections::Group<WString, IGuiAltAction*>& actions);
				
				virtual GuiGraphicsComposition*			GetAltComposition() = 0;
				virtual IGuiAltActionHost*				GetPreviousAltHost() = 0;
				virtual void							OnActivatedAltHost(IGuiAltActionHost* previousHost) = 0;
				virtual void							OnDeactivatedAltHost() = 0;
				virtual void							CollectAltActions(collections::Group<WString, IGuiAltAction*>& actions) = 0;
			};

			/// <summary>Default implementation for <see cref="IGuiAltActionHost"/></summary>
			class GuiAltActionHostBase : public virtual IGuiAltActionHost
			{
			private:
				GuiGraphicsComposition*					composition = nullptr;
				controls::GuiControl*					control = nullptr;
				bool									includeControl = true;
				IGuiAltActionHost*						previousHost = nullptr;

			protected:
				void									SetAltComposition(GuiGraphicsComposition* _composition);
				void									SetAltControl(controls::GuiControl* _control, bool _includeControl);

			public:
				GuiGraphicsComposition*					GetAltComposition()override;
				IGuiAltActionHost*						GetPreviousAltHost()override;
				void									OnActivatedAltHost(IGuiAltActionHost* _previousHost)override;
				void									OnDeactivatedAltHost()override;
				void									CollectAltActions(collections::Group<WString, IGuiAltAction*>& actions)override;
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
				typedef collections::Dictionary<WString, IGuiAltAction*>					AltActionMap;
				typedef collections::Dictionary<WString, controls::GuiControl*>				AltControlMap;
				typedef GuiGraphicsComposition::GraphicsHostRecord							HostRecord;
			public:
				static const vuint64_t					CaretInterval = 500;
			protected:
				HostRecord								hostRecord;
				bool									supressPaint = false;
				bool									needRender = true;

				IGuiShortcutKeyManager*					shortcutKeyManager = nullptr;
				controls::GuiControlHost*				controlHost = nullptr;
				GuiWindowComposition*					windowComposition = nullptr;
				GuiGraphicsComposition*					focusedComposition = nullptr;
				Size									previousClientSize;
				Size									minSize;
				Point									caretPoint;
				vuint64_t								lastCaretTime = 0;

				GuiGraphicsTimerManager					timerManager;
				GuiGraphicsComposition*					mouseCaptureComposition = nullptr;
				CompositionList							mouseEnterCompositions;

				IGuiAltActionHost*						currentAltHost = nullptr;
				AltActionMap							currentActiveAltActions;
				AltControlMap							currentActiveAltTitles;
				WString									currentAltPrefix;
				VKEY									supressAltKey = VKEY::_UNKNOWN;

				void									EnterAltHost(IGuiAltActionHost* host);
				void									LeaveAltHost();
				bool									EnterAltKey(wchar_t key);
				void									LeaveAltKey();
				void									CreateAltTitles(const collections::Group<WString, IGuiAltAction*>& actions);
				vint									FilterTitles();
				void									ClearAltHost();
				void									CloseAltHost();
				void									RefreshRelatedHostRecord(INativeWindow* nativeWindow);

				void									DisconnectCompositionInternal(GuiGraphicsComposition* composition);
				void									MouseCapture(const NativeWindowMouseInfo& info);
				void									MouseUncapture(const NativeWindowMouseInfo& info);
				void									OnCharInput(const NativeWindowCharInfo& info, GuiGraphicsComposition* composition, GuiCharEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				void									OnKeyInput(const NativeWindowKeyInfo& info, GuiGraphicsComposition* composition, GuiKeyEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				void									RaiseMouseEvent(GuiMouseEventArgs& arguments, GuiGraphicsComposition* composition, GuiMouseEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				void									OnMouseInput(const NativeWindowMouseInfo& info, GuiMouseEvent GuiGraphicsEventReceiver::* eventReceiverEvent);
				
			private:
				INativeWindowListener::HitTestResult	HitTest(Point location)override;
				void									Moving(Rect& bounds, bool fixSizeOnly)override;
				void									Moved()override;
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

/***********************************************************************
Shortcut Key Manager Helpers
***********************************************************************/

			class GuiShortcutKeyManager;

			class GuiShortcutKeyItem : public Object, public IGuiShortcutKeyItem
			{
			protected:
				GuiShortcutKeyManager*			shortcutKeyManager;
				bool							ctrl;
				bool							shift;
				bool							alt;
				VKEY							key;

				void							AttachManager(GuiShortcutKeyManager* manager);
				void							DetachManager(GuiShortcutKeyManager* manager);
			public:
				GuiShortcutKeyItem(GuiShortcutKeyManager* _shortcutKeyManager, bool _ctrl, bool _shift, bool _alt, VKEY _key);
				~GuiShortcutKeyItem();

				IGuiShortcutKeyManager*			GetManager()override;
				WString							GetName()override;
				bool							CanActivate(const NativeWindowKeyInfo& info);
				bool							CanActivate(bool _ctrl, bool _shift, bool _alt, VKEY _key);
			};

			/// <summary>A default implementation for <see cref="IGuiShortcutKeyManager"/>.</summary>
			class GuiShortcutKeyManager : public Object, public IGuiShortcutKeyManager, public Description<GuiShortcutKeyManager>
			{
				typedef collections::List<Ptr<GuiShortcutKeyItem>>		ShortcutKeyItemList;
			protected:
				ShortcutKeyItemList				shortcutKeyItems;

			public:
				/// <summary>Create the shortcut key manager.</summary>
				GuiShortcutKeyManager();
				~GuiShortcutKeyManager();

				vint							GetItemCount()override;
				IGuiShortcutKeyItem*			GetItem(vint index)override;
				bool							Execute(const NativeWindowKeyInfo& info)override;

				/// <summary>Create a shortcut key item using a key combination. If the item for the key combination exists, this function returns the item that is created before.</summary>
				/// <returns>The created shortcut key item.</returns>
				/// <param name="ctrl">Set to true if the CTRL key is required.</param>
				/// <param name="shift">Set to true if the SHIFT key is required.</param>
				/// <param name="alt">Set to true if the ALT key is required.</param>
				/// <param name="key">The non-control key.</param>
				IGuiShortcutKeyItem*			CreateShortcut(bool ctrl, bool shift, bool alt, VKEY key);
				/// <summary>Destroy a shortcut key item using a key combination</summary>
				/// <returns>Returns true if the manager destroyed a existing shortcut key item.</returns>
				/// <param name="ctrl">Set to true if the CTRL key is required.</param>
				/// <param name="shift">Set to true if the SHIFT key is required.</param>
				/// <param name="alt">Set to true if the ALT key is required.</param>
				/// <param name="key">The non-control key.</param>
				bool							DestroyShortcut(bool ctrl, bool shift, bool alt, VKEY key);
				/// <summary>Get a shortcut key item using a key combination. If the item for the key combination does not exist, this function returns null.</summary>
				/// <returns>The shortcut key item.</returns>
				/// <param name="ctrl">Set to true if the CTRL key is required.</param>
				/// <param name="shift">Set to true if the SHIFT key is required.</param>
				/// <param name="alt">Set to true if the ALT key is required.</param>
				/// <param name="key">The non-control key.</param>
				IGuiShortcutKeyItem*			TryGetShortcut(bool ctrl, bool shift, bool alt, VKEY key);
			};
		}
	}
}

#endif