#include "GuiApplication.h"

extern void GuiMain();

namespace vl
{
	namespace presentation
	{
		extern void GuiInitializeUtilities();
		extern void GuiFinalizeUtilities();

		namespace controls
		{
			using namespace collections;
			using namespace compositions;
			using namespace theme;
			using namespace description;

/***********************************************************************
GuiGlobalShortcutKeyManager
***********************************************************************/

			class GuiGlobalShortcutKeyManager : public GuiShortcutKeyManager
			{
			protected:
				Dictionary<vint, GuiShortcutKeyItem*>		idToItemsMap;
				Dictionary<GuiShortcutKeyItem*, vint>		itemToIdsMap;

				bool IsGlobal() override
				{
					return true;
				}

				bool OnCreatingShortcut(GuiShortcutKeyItem* item) override
				{
					bool ctrl, shift, alt;
					VKEY key;
					item->ReadKeyConfig(ctrl, shift, alt, key);

					vint id = GetCurrentController()->InputService()->RegisterGlobalShortcutKey(ctrl, shift, alt, key);
					if (id < (vint)NativeGlobalShortcutKeyResult::ValidIdBegins) return false;

					idToItemsMap.Add(id, item);
					itemToIdsMap.Add(item, id);
					return true;
				}

				void OnDestroyingShortcut(GuiShortcutKeyItem* item) override
				{
					vint id = itemToIdsMap[item];
					idToItemsMap.Remove(id);
					itemToIdsMap.Remove(item);
					GetCurrentController()->InputService()->UnregisterGlobalShortcutKey(id);
				}

			public:

				GuiShortcutKeyItem* TryGetItemFromId(vint id)
				{
					vint index = idToItemsMap.Keys().IndexOf(id);
					return index == -1 ? nullptr : idToItemsMap.Values()[index];
				}
			};

/***********************************************************************
GuiApplication
***********************************************************************/

			void GuiApplication::InvokeClipboardNotify(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments)
			{
				if(composition->HasEventReceiver())
				{
					composition->GetEventReceiver()->clipboardNotify.Execute(arguments);
				}
				for (auto subComposition : composition->Children())
				{
					InvokeClipboardNotify(subComposition, arguments);
				}
			}

			void GuiApplication::ClipboardUpdated()
			{
				for(vint i=0;i<windows.Count();i++)
				{
					GuiEventArgs arguments=windows[i]->GetNotifyEventArguments();
					windows[i]->ClipboardUpdated.Execute(arguments);
					InvokeClipboardNotify(windows[i]->GetBoundsComposition(), arguments);
				}
			}

			void GuiApplication::GlobalShortcutKeyActivated(vint id)
			{
				auto manager = dynamic_cast<GuiGlobalShortcutKeyManager*>(globalShortcutKeyManager.Obj());
				if (auto item = manager->TryGetItemFromId(id))
				{
					item->Execute();
				}
			}

			GuiApplication::GuiApplication()
				:locale(Locale::UserDefault())
			{
				globalShortcutKeyManager = Ptr(new GuiGlobalShortcutKeyManager);
				GetCurrentController()->CallbackService()->InstallListener(this);
			}

			GuiApplication::~GuiApplication()
			{
				if(sharedTooltipControl)
				{
					delete sharedTooltipControl;
					sharedTooltipControl=0;
				}
				GetCurrentController()->CallbackService()->UninstallListener(this);
			}

			INativeWindow* GuiApplication::GetThreadContextNativeWindow(GuiControlHost* controlHost)
			{
				return nullptr;
			}

			void GuiApplication::RegisterWindow(GuiWindow* window)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::GuiApplication::RegisterWindow(GuiWindow*)#"
				CHECK_ERROR(!window->registeredInApplication, ERROR_MESSAGE_PREFIX L"The window has been registered");
				window->registeredInApplication = true;
				windows.Add(window);
				if (auto nativeWindow = window->GetNativeWindow())
				{
					windowMap.Add(nativeWindow, window);
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiApplication::UnregisterWindow(GuiWindow* window)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::GuiApplication::UnregisterWindow(GuiWindow*)#"
				CHECK_ERROR(window->registeredInApplication, ERROR_MESSAGE_PREFIX L"The window has not been registered");
				window->registeredInApplication = false;
				if (auto nativeWindow = window->GetNativeWindow())
				{
					windowMap.Remove(nativeWindow);
				}
				windows.Remove(window);
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiApplication::NotifyNativeWindowChanged(GuiControlHost* controlHost, INativeWindow* previousNativeWindow)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::GuiApplication::NotifyNativeWindowChanged(GuiControlsHost*, INativeWindow*)#"
				if (auto window = dynamic_cast<GuiWindow*>(controlHost))
				{
					if (!window->registeredInApplication) return;
					if (previousNativeWindow)
					{
						CHECK_ERROR(windowMap[previousNativeWindow] == window, ERROR_MESSAGE_PREFIX L"Unpaired arguments.");
						windowMap.Remove(previousNativeWindow);
					}
					if (auto nativeWindow = window->GetNativeWindow())
					{
						windowMap.Add(nativeWindow, window);
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiApplication::RegisterPopupOpened(GuiPopup* popup)
			{
				vint index=openingPopups.IndexOf(popup);
				if(index==-1)
				{
					openingPopups.Add(popup);
				}
			}

			void GuiApplication::RegisterPopupClosed(GuiPopup* popup)
			{
				if(openingPopups.Remove(popup))
				{
				}
			}

			void GuiApplication::TooltipMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				sharedTooltipHovering=true;
			}

			void GuiApplication::TooltipMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				sharedTooltipHovering=false;
				if(sharedTooltipClosing)
				{
					CloseTooltip();
				}
			}

			Locale GuiApplication::GetLocale()
			{
				return locale;
			}

			void GuiApplication::SetLocale(Locale value)
			{
				if (locale != value)
				{
					locale = value;
					LocaleChanged();
				}
			}

			void GuiApplication::Run(GuiWindow* _mainWindow)
			{
				if (!mainWindow)
				{
					mainWindow = _mainWindow;
					GetCurrentController()->WindowService()->Run(mainWindow->GetNativeWindow());
					mainWindow = nullptr;
				}
			}

			bool GuiApplication::RunOneCycle()
			{
				return GetCurrentController()->WindowService()->RunOneCycle();
			}

			GuiWindow* GuiApplication::GetMainWindow()
			{
				return mainWindow;
			}

			const collections::List<GuiWindow*>& GuiApplication::GetWindows()
			{
				return windows;
			}

			GuiWindow* GuiApplication::GetWindow(NativePoint location)
			{
				INativeWindow* nativeWindow = GetCurrentController()->WindowService()->GetWindow(location);
				if (nativeWindow)
				{
					// TODO: (enumerable) foreach
					for (vint i = 0; i < windows.Count(); i++)
					{
						GuiWindow* window = windows[i];
						if (window->GetNativeWindow() == nativeWindow)
						{
							return window;
						}
					}
				}
				return 0;
			}

			GuiWindow* GuiApplication::GetWindowFromNative(INativeWindow* nativeWindow)
			{
				vint index = windowMap.Keys().IndexOf(nativeWindow);
				return index == -1 ? nullptr : windowMap.Values()[index];
			}

			void GuiApplication::ShowTooltip(GuiControl* owner, GuiControl* tooltip, vint preferredContentWidth, Point location)
			{
				GuiWindow* ownerWindow = dynamic_cast<GuiWindow*>(owner->GetRelatedControlHost());
				if (sharedTooltipOwnerWindow != ownerWindow)
				{
					delete sharedTooltipControl;
					sharedTooltipControl = 0;
				}

				if(!sharedTooltipControl)
				{
					sharedTooltipControl = new GuiTooltip(theme::ThemeName::Tooltip);
					if (ownerWindow)
					{
						if (auto tooltipStyle = ownerWindow->TypedControlTemplateObject(true)->GetTooltipTemplate())
						{
							sharedTooltipControl->SetControlTemplate(tooltipStyle);
						}
					}
					sharedTooltipControl->GetBoundsComposition()->GetEventReceiver()->mouseEnter.AttachMethod(this, &GuiApplication::TooltipMouseEnter);
					sharedTooltipControl->GetBoundsComposition()->GetEventReceiver()->mouseLeave.AttachMethod(this, &GuiApplication::TooltipMouseLeave);
				}

				sharedTooltipHovering=false;
				sharedTooltipClosing=false;
				sharedTooltipOwnerWindow = ownerWindow;
				sharedTooltipOwner=owner;
				sharedTooltipControl->SetTemporaryContentControl(tooltip);
				sharedTooltipControl->SetPreferredContentWidth(preferredContentWidth);
				sharedTooltipControl->SetClientSize(Size(10, 10));
				sharedTooltipControl->ShowPopup(owner, location);
			}

			void GuiApplication::CloseTooltip()
			{
				if(sharedTooltipControl)
				{
					if(sharedTooltipHovering)
					{
						sharedTooltipClosing=true;
					}
					else
					{
						sharedTooltipClosing=false;
						sharedTooltipControl->Close();
					}
				}
			}

			GuiControl* GuiApplication::GetTooltipOwner()
			{
				if(!sharedTooltipControl) return 0;
				if(!sharedTooltipControl->GetTemporaryContentControl()) return 0;
				return sharedTooltipOwner;
			}

			compositions::IGuiShortcutKeyManager* GuiApplication::GetGlobalShortcutKeyManager()
			{
				return globalShortcutKeyManager.Obj();
			}

			WString GuiApplication::GetExecutablePath()
			{
				return GetCurrentController()->GetExecutablePath();
			}

			WString GuiApplication::GetExecutableFolder()
			{
				WString path=GetExecutablePath();
				for(vint i=path.Length()-1;i>=0;i--)
				{
					if(path[i]==L'\\' || path[i]==L'/')
					{
						return path.Sub(0, i+1);
					}
				}
				return L"";
			}

			bool GuiApplication::IsInMainThread(GuiControlHost* controlHost)
			{
				return GetCurrentController()->AsyncService()->IsInMainThread(GetThreadContextNativeWindow(controlHost));
			}

			void GuiApplication::InvokeAsync(const Func<void()>& proc)
			{
				GetCurrentController()->AsyncService()->InvokeAsync(proc);
			}

			void GuiApplication::InvokeInMainThread(GuiControlHost* controlHost, const Func<void()>& proc)
			{
				GetCurrentController()->AsyncService()->InvokeInMainThread(GetThreadContextNativeWindow(controlHost), proc);
			}

			bool GuiApplication::InvokeInMainThreadAndWait(GuiControlHost* controlHost, const Func<void()>& proc, vint milliseconds)
			{
				CHECK_ERROR(!IsInMainThread(controlHost), L"GuiApplication::InvokeInMainThreadAndWait(GuiControlHost*, const Func<void()>&, vint)#This function cannot be called in UI thread.");
				return GetCurrentController()->AsyncService()->InvokeInMainThreadAndWait(GetThreadContextNativeWindow(controlHost), proc, milliseconds);
			}

			Ptr<INativeDelay> GuiApplication::DelayExecute(const Func<void()>& proc, vint milliseconds)
			{
				return GetCurrentController()->AsyncService()->DelayExecute(proc, milliseconds);
			}

			Ptr<INativeDelay> GuiApplication::DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)
			{
				return GetCurrentController()->AsyncService()->DelayExecuteInMainThread(proc, milliseconds);
			}

			void GuiApplication::RunGuiTask(GuiControlHost* controlHost, const Func<void()>& proc)
			{
				if (IsInMainThread(controlHost))
				{
					proc();
				}
				else
				{
					InvokeInMainThreadAndWait(controlHost, [&proc]()
					{
						proc();
					});
				}
			}

/***********************************************************************
Helpers
***********************************************************************/

			GuiApplication* application = nullptr;
			bool GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES = false;
			bool GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD = false;

			GuiApplication* GetApplication()
			{
				return application;
			}

/***********************************************************************
GuiApplicationMain
***********************************************************************/

			class UIThreadAsyncScheduler :public Object, public IAsyncScheduler, public Description<UIThreadAsyncScheduler>
			{
			public:
				void Execute(const Func<void()>& callback)override
				{
					GetApplication()->InvokeInMainThread(GetApplication()->GetMainWindow(), callback);
				}

				void ExecuteInBackground(const Func<void()>& callback)override
				{
					GetApplication()->InvokeAsync(callback);
				}

				void DelayExecute(const Func<void()>& callback, vint milliseconds)override
				{
					GetApplication()->DelayExecuteInMainThread(callback, milliseconds);
				}
			};

			class OtherThreadAsyncScheduler :public Object, public IAsyncScheduler, public Description<UIThreadAsyncScheduler>
			{
			public:
				void Execute(const Func<void()>& callback)override
				{
					GetApplication()->InvokeAsync(callback);
				}

				void ExecuteInBackground(const Func<void()>& callback)override
				{
					GetApplication()->InvokeAsync(callback);
				}

				void DelayExecute(const Func<void()>& callback, vint milliseconds)override
				{
					GetApplication()->DelayExecute(callback, milliseconds);
				}
			};

			void GuiApplicationInitialize()
			{
				theme::InitializeTheme();
				if (!GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD)
				{
#ifndef VCZH_DEBUG_NO_REFLECTION
					GetGlobalTypeManager()->Load();
#endif
					GetPluginManager()->Load(true, true);
				}
				else
				{
					GetPluginManager()->Load(false, true);
				}

				GetCurrentController()->InputService()->StartTimer();
				{
					GuiApplication app;
					application = &app;
					IAsyncScheduler::RegisterSchedulerForCurrentThread(Ptr(new UIThreadAsyncScheduler));
					IAsyncScheduler::RegisterDefaultScheduler(Ptr(new OtherThreadAsyncScheduler));
					GuiInitializeUtilities();
					GuiMain();
					GuiFinalizeUtilities();
					IAsyncScheduler::UnregisterDefaultScheduler();
					IAsyncScheduler::UnregisterSchedulerForCurrentThread();
					application = nullptr;
				}
				GetCurrentController()->InputService()->StopTimer();
				theme::FinalizeTheme();
				FinalizeGlobalStorage();

				if (!GACUI_UNITTEST_ONLY_SKIP_TYPE_AND_PLUGIN_LOAD_UNLOAD)
				{
					GetPluginManager()->Unload(true, true);
					DestroyPluginManager();
#ifndef VCZH_DEBUG_NO_REFLECTION
					ResetGlobalTypeManager();
#endif
				}
				else
				{
					GetPluginManager()->Unload(false, true);
				}

				if (!GACUI_UNITTEST_ONLY_SKIP_THREAD_LOCAL_STORAGE_DISPOSE_STORAGES)
				{
					ThreadLocalStorage::DisposeStorages();
				}
			}
		}
	}
}

void GuiApplicationMain()
{
	vl::presentation::controls::GuiApplicationInitialize();
}