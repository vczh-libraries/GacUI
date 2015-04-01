#include "GuiApplication.h"
#include "Styles/GuiWin7Styles.h"
#include "Styles/GuiWin8Styles.h"

extern void GuiMain();

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace compositions;
			using namespace theme;
			using namespace description;

/***********************************************************************
GuiApplication
***********************************************************************/

			void GuiApplication::InvokeClipboardNotify(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments)
			{
				if(composition->HasEventReceiver())
				{
					composition->GetEventReceiver()->clipboardNotify.Execute(arguments);
				}
				FOREACH(GuiGraphicsComposition*, subComposition, composition->Children())
				{
					InvokeClipboardNotify(subComposition, arguments);
				}
			}

			void GuiApplication::LeftButtonDown(Point position)
			{
				OnMouseDown(position);
			}

			void GuiApplication::LeftButtonUp(Point position)
			{
			}

			void GuiApplication::RightButtonDown(Point position)
			{
				OnMouseDown(position);
			}

			void GuiApplication::RightButtonUp(Point position)
			{
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

			GuiApplication::GuiApplication()
				:mainWindow(0)
				,sharedTooltipOwnerWindow(0)
				,sharedTooltipOwner(0)
				,sharedTooltipControl(0)
				,sharedTooltipHovering(false)
				,sharedTooltipClosing(false)
			{
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

			void GuiApplication::RegisterWindow(GuiWindow* window)
			{
				windows.Add(window);
			}

			void GuiApplication::UnregisterWindow(GuiWindow* window)
			{
				windows.Remove(window);
			}

			void GuiApplication::RegisterPopupOpened(GuiPopup* popup)
			{
				vint index=openingPopups.IndexOf(popup);
				if(index==-1)
				{
					openingPopups.Add(popup);
					if(openingPopups.Count()==1)
					{
						GetCurrentController()->InputService()->StartHookMouse();
					}
				}
			}

			void GuiApplication::RegisterPopupClosed(GuiPopup* popup)
			{
				if(openingPopups.Remove(popup))
				{
					if(openingPopups.Count()==0)
					{
						GetCurrentController()->InputService()->StopHookMouse();
					}
				}
			}

			void GuiApplication::OnMouseDown(Point location)
			{
				GuiWindow* window=GetWindow(location);
				for(vint i=0;i<windows.Count();i++)
				{
					if(windows[i]!=window)
					{
						windows[i]->MouseClickedOnOtherWindow(window);
					}
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

			void GuiApplication::Run(GuiWindow* _mainWindow)
			{
				if(!mainWindow)
				{
					mainWindow=_mainWindow;
					GetCurrentController()->WindowService()->Run(mainWindow->GetNativeWindow());
					mainWindow=0;
				}
			}

			GuiWindow* GuiApplication::GetMainWindow()
			{
				return mainWindow;
			}

			const collections::List<GuiWindow*>& GuiApplication::GetWindows()
			{
				return windows;
			}

			GuiWindow* GuiApplication::GetWindow(Point location)
			{
				INativeWindow* nativeWindow=GetCurrentController()->WindowService()->GetWindow(location);
				if(nativeWindow)
				{
					for(vint i=0;i<windows.Count();i++)
					{
						GuiWindow* window=windows[i];
						if(window->GetNativeWindow()==nativeWindow)
						{
							return window;
						}
					}
				}
				return 0;
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
					GuiWindow::IStyleController* tooltipStyle = 0;
					if (ownerWindow)
					{
						tooltipStyle = dynamic_cast<GuiWindow::IStyleController*>(ownerWindow->GetStyleController())->CreateTooltipStyle();
					}
					if (!tooltipStyle)
					{
						tooltipStyle = GetCurrentTheme()->CreateTooltipStyle();
					}

					sharedTooltipControl=new GuiTooltip(tooltipStyle);
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

			bool GuiApplication::IsInMainThread()
			{
				return GetCurrentController()->AsyncService()->IsInMainThread();
			}

			void GuiApplication::InvokeAsync(const Func<void()>& proc)
			{
				GetCurrentController()->AsyncService()->InvokeAsync(proc);
			}

			void GuiApplication::InvokeInMainThread(const Func<void()>& proc)
			{
				GetCurrentController()->AsyncService()->InvokeInMainThread(proc);
			}

			bool GuiApplication::InvokeInMainThreadAndWait(const Func<void()>& proc, vint milliseconds)
			{
				return GetCurrentController()->AsyncService()->InvokeInMainThreadAndWait(proc, milliseconds);
			}

			Ptr<INativeDelay> GuiApplication::DelayExecute(const Func<void()>& proc, vint milliseconds)
			{
				return GetCurrentController()->AsyncService()->DelayExecute(proc, milliseconds);
			}

			Ptr<INativeDelay> GuiApplication::DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)
			{
				return GetCurrentController()->AsyncService()->DelayExecuteInMainThread(proc, milliseconds);
			}

			void GuiApplication::RunGuiTask(const Func<void()>& proc)
			{
				if(IsInMainThread())
				{
					return proc();
				}
				else
				{
					InvokeInMainThreadAndWait([&proc]()
					{
						proc();
					});
				}
			}

/***********************************************************************
GuiPluginManager
***********************************************************************/

			class GuiPluginManager : public Object, public IGuiPluginManager
			{
			protected:
				List<Ptr<IGuiPlugin>>				plugins;
				bool								loaded;
			public:
				GuiPluginManager()
					:loaded(false)
				{
				}

				~GuiPluginManager()
				{
					Unload();
				}

				void AddPlugin(Ptr<IGuiPlugin> plugin)override
				{
					plugins.Add(plugin);
					if(loaded)
					{
						plugin->Load();
					}
				}

				void Load()override
				{
					if(!loaded)
					{
						loaded=true;
						FOREACH(Ptr<IGuiPlugin>, plugin, plugins)
						{
							plugin->Load();
						}
						FOREACH(Ptr<IGuiPlugin>, plugin, plugins)
						{
							plugin->AfterLoad();
						}
					}
				}

				void Unload()override
				{
					if(loaded)
					{
						loaded=false;
						FOREACH(Ptr<IGuiPlugin>, plugin, plugins)
						{
							plugin->Unload();
						}
					}
				}

				bool IsLoaded()override
				{
					return loaded;
				}
			};

/***********************************************************************
Helpers
***********************************************************************/

			GuiApplication* application=0;
			IGuiPluginManager* pluginManager=0;

			GuiApplication* GetApplication()
			{
				return application;
			}

			IGuiPluginManager* GetPluginManager()
			{
				if(!pluginManager)
				{
					pluginManager=new GuiPluginManager;
				}
				return pluginManager;
			}

			void DestroyPluginManager()
			{
				if(pluginManager)
				{
					delete pluginManager;
					pluginManager=0;
				}
			}

/***********************************************************************
GuiApplicationMain
***********************************************************************/

			void GuiApplicationInitialize()
			{
				Ptr<theme::ITheme> theme;
				{
					WString osVersion=GetCurrentController()->GetOSVersion();
					vint index=osVersion.IndexOf(L';');
					if (index == -1)
					{
						theme=new win8::Win8Theme;
					}
					else
					{
						WString osMainVersion=osVersion.Sub(0, index);
						if(osMainVersion==L"Windows 8" || osMainVersion==L"Windows Server 2012")
						{
							theme=new win8::Win8Theme;
						}
						else
						{
							theme=new win7::Win7Theme;
						}
					}
				}

				GetCurrentController()->InputService()->StartTimer();
				GuiApplication app;
				application=&app;
				
				GetPluginManager()->Load();
				GetGlobalTypeManager()->Load();
				theme::SetCurrentTheme(theme.Obj());
				GuiMain();
				theme::SetCurrentTheme(0);
				DestroyGlobalTypeManager();
				DestroyPluginManager();
			}
		}
	}
}

void GuiApplicationMain()
{
	vl::presentation::controls::GuiApplicationInitialize();
}