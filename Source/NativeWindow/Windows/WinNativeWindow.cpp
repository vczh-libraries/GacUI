#include "..\..\Resources\GuiResource.h"
#include "WinNativeWindow.h"
#include "ServicesImpl\WindowsResourceService.h"
#include "ServicesImpl\WindowsClipboardService.h"
#include "ServicesImpl\WindowsImageService.h"
#include "ServicesImpl\WindowsAsyncService.h"
#include "ServicesImpl\WindowsScreenService.h"
#include "ServicesImpl\WindowsCallbackService.h"
#include "ServicesImpl\WindowsInputService.h"
#include "ServicesImpl\WindowsDialogService.h"
#include <CommCtrl.h>

#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Comctl32.lib")

namespace vl
{
	namespace presentation
	{
		namespace windows
		{
			using namespace collections;

			LPCWSTR defaultIconResourceName = nullptr;

			HICON CreateWindowDefaultIcon(vint size = 0)
			{
				if (!defaultIconResourceName) return NULL;
				return (HICON)LoadImage(GetModuleHandle(NULL), defaultIconResourceName, IMAGE_ICON, (int)size, (int)size, (size ? 0 : LR_DEFAULTSIZE) | LR_SHARED);
			}

			void SetWindowDefaultIcon(UINT resourceId)
			{
				CHECK_ERROR(defaultIconResourceName == nullptr, L"vl::presentation::windows::SetWindowDefaultIcon(UINT)#This function can only be called once.");
				defaultIconResourceName = MAKEINTRESOURCE(resourceId);
			}

			HWND GetHWNDFromNativeWindowHandle(INativeWindow* window)
			{
				if(!window) return NULL;
				IWindowsForm* form=GetWindowsForm(window);
				if(!form) return NULL;
				return form->GetWindowHandle();
			}

/***********************************************************************
WindowsClass
***********************************************************************/

			class WinClass : public Object
			{
			protected:
				WString									name;
				WNDCLASSEX								windowClass;
				ATOM									windowAtom;

			public:
				WinClass(WString _name, bool shadow, bool ownDC, WNDPROC procedure, HINSTANCE hInstance)
				{
					name=_name;
					ZeroMemory(&windowClass, sizeof(windowClass));
					windowClass.cbSize=sizeof(windowClass);
					windowClass.style=CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | (shadow?CS_DROPSHADOW:0) | (ownDC?CS_OWNDC:0);
					windowClass.lpfnWndProc=procedure;
					windowClass.cbClsExtra=0;
					windowClass.cbWndExtra=0;
					windowClass.hInstance=hInstance;
					if (defaultIconResourceName)
					{
						windowClass.hIcon = CreateWindowDefaultIcon();
					}
					windowClass.hCursor=NULL;//LoadCursor(NULL,IDC_ARROW);
					windowClass.hbrBackground=GetSysColorBrush(COLOR_BTNFACE);
					windowClass.lpszMenuName=NULL;
					windowClass.lpszClassName=name.Buffer();
					windowClass.hIconSm=NULL;
					windowAtom=RegisterClassEx(&windowClass);
				}

				bool IsAvailable()
				{
					return windowAtom!=0;
				}

				WString GetName()
				{
					return name;
				}

				ATOM GetClassAtom()
				{
					return windowAtom;
				}
			};

/***********************************************************************
WindowsForm
***********************************************************************/

			class WindowsForm : public Object, public INativeWindow, public IWindowsForm
			{
			protected:
				
				LONG_PTR InternalGetExStyle()
				{
					return GetWindowLongPtr(handle, GWL_EXSTYLE);
				}

				void InternalSetExStyle(LONG_PTR exStyle)
				{
					LONG_PTR result = SetWindowLongPtr(handle, GWL_EXSTYLE, exStyle);
					SetWindowPos(handle, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
				}

				bool GetExStyle(LONG_PTR exStyle)
				{
					LONG_PTR Long=InternalGetExStyle();
					return (Long & exStyle) != 0;
				}

				void SetExStyle(LONG_PTR exStyle, bool available)
				{
					LONG_PTR Long = InternalGetExStyle();
					if(available)
					{
						Long |= exStyle;
					}
					else
					{
						Long &= ~exStyle;
					}
					InternalSetExStyle(Long);
				}

				bool GetStyle(LONG_PTR style)
				{
					LONG_PTR Long = GetWindowLongPtr(handle, GWL_STYLE);
					return (Long & style) != 0;
				}

				void SetStyle(LONG_PTR style, bool available)
				{
					LONG_PTR Long = GetWindowLongPtr(handle, GWL_STYLE);
					if(available)
					{
						Long |= style;
					}
					else
					{
						Long &= ~style;
					}
					SetWindowLongPtr(handle, GWL_STYLE, Long);
					SetWindowPos(handle, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
				}
#pragma push_macro("_CONTROL")
#if defined _CONTROL
#undef _CONTROL
#endif
				NativeWindowMouseInfo ConvertMouse(WPARAM wParam, LPARAM lParam, bool wheelMessage, bool nonClient)
				{
					NativeWindowMouseInfo info;

					info.nonClient = false;
					if (nonClient)
					{
						switch (wParam)
						{
						case HTMINBUTTON:
						case HTMAXBUTTON:
						case HTCLOSE:
							break;
						default:
							info.nonClient = true;
							break;
						}
					}

					if(wheelMessage)
					{
						info.wheel=GET_WHEEL_DELTA_WPARAM(wParam);
						wParam=GET_KEYSTATE_WPARAM(wParam);
					}
					else
					{
						info.wheel=0;
					}

					if (nonClient)
					{
						info.ctrl = WinIsKeyPressing(VKEY::_CONTROL);
						info.shift = WinIsKeyPressing(VKEY::_SHIFT);
						info.left= WinIsKeyPressing(VKEY::_LBUTTON);
						info.middle= WinIsKeyPressing(VKEY::_MBUTTON);
						info.right = WinIsKeyPressing(VKEY::_RBUTTON);
						
						POINTS point = MAKEPOINTS(lParam);
						NativePoint offset = GetClientBoundsInScreen().LeftTop();
						info.x = point.x - offset.x.value;
						info.y = point.y - offset.y.value;
					}
					else
					{
						info.ctrl=((VKEY)wParam & VKEY::_CONTROL)!=(VKEY)0;
						info.shift=((VKEY)wParam & VKEY::_SHIFT)!= (VKEY)0;
						info.left=((VKEY)wParam & VKEY::_LBUTTON)!= (VKEY)0;
						info.middle=((VKEY)wParam & VKEY::_MBUTTON)!= (VKEY)0;
						info.right=((VKEY)wParam & VKEY::_RBUTTON)!= (VKEY)0;

						POINTS point = MAKEPOINTS(lParam);

						if (wheelMessage)
						{
							NativePoint offset = GetClientBoundsInScreen().LeftTop();
							info.x = point.x - offset.x.value;
							info.y = point.y - offset.y.value;
						}
						else
						{
							info.x = point.x;
							info.y = point.y;
						}
					}
					return info;
				}

				NativeWindowKeyInfo ConvertKey(WPARAM wParam, LPARAM lParam)
				{
					NativeWindowKeyInfo info;
					info.code=(VKEY)wParam;
					info.ctrl=WinIsKeyPressing(VKEY::_CONTROL);
					info.shift=WinIsKeyPressing(VKEY::_SHIFT);
					info.alt=WinIsKeyPressing(VKEY::_MENU);
					info.capslock=WinIsKeyToggled(VKEY::_CAPITAL);
					info.autoRepeatKeyDown = (((vuint32_t)lParam) >> 30) % 2 == 1;
					return info;
				}

				NativeWindowCharInfo ConvertChar(WPARAM wParam)
				{
					NativeWindowCharInfo info;
					info.code=(wchar_t)wParam;
					info.ctrl=WinIsKeyPressing(VKEY::_CONTROL);
					info.shift=WinIsKeyPressing(VKEY::_SHIFT);
					info.alt=WinIsKeyPressing(VKEY::_MENU);
					info.capslock=WinIsKeyToggled(VKEY::_CAPITAL);
					return info;
				}
#pragma pop_macro("_CONTROL")

				void TrackMouse(bool enable)
				{
					TRACKMOUSEEVENT trackMouseEvent;
					trackMouseEvent.cbSize=sizeof(trackMouseEvent);
					trackMouseEvent.hwndTrack=handle;
					trackMouseEvent.dwFlags=(enable?0:TME_CANCEL) | TME_HOVER | TME_LEAVE;
					trackMouseEvent.dwHoverTime=HOVER_DEFAULT;
					TrackMouseEvent(&trackMouseEvent);
				}

				void UpdateCompositionForContent()
				{
					HIMC imc = ImmGetContext(handle);
					COMPOSITIONFORM cf;
					cf.dwStyle = CFS_POINT;
					cf.ptCurrentPos.x = (int)caretPoint.x.value;
					cf.ptCurrentPos.y = (int)caretPoint.y.value;
					ImmSetCompositionWindow(imc, &cf);
					ImmReleaseContext(handle, imc);
				}

				bool HandleMessageInternal(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
				{
					bool transferFocusEvent = false;
					bool nonClient = false;

					switch(uMsg)
					{
					case WM_LBUTTONDOWN:
					case WM_LBUTTONUP:
					case WM_LBUTTONDBLCLK:
					case WM_RBUTTONDOWN:
					case WM_RBUTTONUP:
					case WM_RBUTTONDBLCLK:
					case WM_MBUTTONDOWN:
					case WM_MBUTTONUP:
					case WM_MBUTTONDBLCLK:
						transferFocusEvent=true;
					}
					switch(uMsg)
					{
					// ************************************** moving and sizing
					case WM_MOVING:case WM_SIZING:
						{
							LPRECT rawBounds=(LPRECT)lParam;
							NativeRect bounds(rawBounds->left, rawBounds->top, rawBounds->right, rawBounds->bottom);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->Moving(bounds, false);
							}
							if(		rawBounds->left!=bounds.Left().value
								||	rawBounds->top!=bounds.Top().value
								||	rawBounds->right!=bounds.Right().value
								||	rawBounds->bottom!=bounds.Bottom().value)
							{
								rawBounds->left=(int)bounds.Left().value;
								rawBounds->top=(int)bounds.Top().value;
								rawBounds->right=(int)bounds.Right().value;
								rawBounds->bottom=(int)bounds.Bottom().value;
								result=TRUE;
							}
						}
						break;
					case WM_MOVE:case WM_SIZE:
						{
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->Moved();
							}
						}
						break;
					case WM_DPICHANGED:
						{
							dpiX = LOWORD(wParam);
							dpiY = HIWORD(wParam);
							UpdateDpiAwaredFields(false);
							auto newRect = (RECT*)lParam;
							MoveWindow(handle, newRect->left, newRect->top, (newRect->right - newRect->left), (newRect->bottom - newRect->top), FALSE);
							for (vint i = 0; i < listeners.Count(); i++)
							{
								listeners[i]->DpiChanged();
							}
						}
						break;
					// ************************************** state
					case WM_ENABLE:
						{
							for(vint i=0;i<listeners.Count();i++)
							{
								if(wParam==TRUE)
								{
									listeners[i]->Enabled();
								}
								else
								{
									listeners[i]->Disabled();
								}
							}
						}
						break;
					case WM_SETFOCUS:
						{
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->GotFocus();
							}
						}
						break;
					case WM_KILLFOCUS:
						{
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->LostFocus();
							}
						}
						break;
					case WM_ACTIVATE:
						{
							for(vint i=0;i<listeners.Count();i++)
							{
								if(wParam==WA_ACTIVE || wParam==WA_CLICKACTIVE)
								{
									listeners[i]->Activated();
								}
								else
								{
									listeners[i]->Deactivated();
								}
							}
						}
						break;
					case WM_SHOWWINDOW:
						if (lParam == 0)
						{
							if (wParam == TRUE)
							{
								for (vint i = 0; i < listeners.Count(); i++)
								{
									listeners[i]->Opened();
								}
							}
							else
							{
								for (vint i = 0; i < listeners.Count(); i++)
								{
									listeners[i]->Closed();
								}
							}
						}
						break;
					case WM_CLOSE:
						{
							bool cancel=false;
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->Closing(cancel);
							}
							return cancel;
						}
						break;
					// ************************************** mouse
					case WM_NCLBUTTONDOWN:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_LBUTTONDOWN:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->LeftButtonDown(info);
							}
						}
						break;
					case WM_NCLBUTTONUP:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_LBUTTONUP:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->LeftButtonUp(info);
							}
						}
						break;
					case WM_NCLBUTTONDBLCLK:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_LBUTTONDBLCLK:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->LeftButtonDoubleClick(info);
							}
						}
						break;
					case WM_NCRBUTTONDOWN:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_RBUTTONDOWN:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->RightButtonDown(info);
							}
						}
						break;
					case WM_NCRBUTTONUP:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_RBUTTONUP:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->RightButtonUp(info);
							}
						}
						break;
					case WM_NCRBUTTONDBLCLK:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_RBUTTONDBLCLK:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->RightButtonDoubleClick(info);
							}
						}
						break;
					case WM_NCMBUTTONDOWN:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_MBUTTONDOWN:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->MiddleButtonDown(info);
							}
						}
						break;
					case WM_NCMBUTTONUP:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_MBUTTONUP:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->MiddleButtonUp(info);
							}
						}
						break;
					case WM_NCMBUTTONDBLCLK:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_MBUTTONDBLCLK:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->MiddleButtonDoubleClick(info);
							}
						}
						break;
					case WM_NCMOUSEMOVE:
						if (!customFrameMode) break;
						nonClient = true;
					case WM_MOUSEMOVE:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, false, nonClient);
							if(info.x!=mouseLastX || info.y!=mouseLastY)
							{
								if(!mouseHoving)
								{
									mouseHoving=true;
									for(vint i=0;i<listeners.Count();i++)
									{
										listeners[i]->MouseEntered();
									}
									TrackMouse(true);
								}
								for(vint i=0;i<listeners.Count();i++)
								{
									listeners[i]->MouseMoving(info);
								}
							}
						}
						break;
					// ************************************** wheel
					case WM_MOUSEHWHEEL:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, true, false);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->HorizontalWheel(info);
							}
						}
						break;
					case WM_MOUSEWHEEL:
						{
							NativeWindowMouseInfo info=ConvertMouse(wParam, lParam, true, false);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->VerticalWheel(info);
							}
						}
						break;
					// ************************************** mouse state
					case WM_NCMOUSELEAVE:
						nonClient = true;
					case WM_MOUSELEAVE:
						if (customFrameMode == nonClient)
						{
							mouseLastX=-1;
							mouseLastY=-1;
							mouseHoving=false;
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->MouseLeaved();
							}
						}
						break;
					case WM_NCMOUSEHOVER:
					case WM_MOUSEHOVER:
						{
							TrackMouse(true);
						}
						break;
					// ************************************** key
					case WM_KEYUP:
						{
							NativeWindowKeyInfo info=ConvertKey(wParam, lParam);
							info.autoRepeatKeyDown = false;
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->KeyUp(info);
							}
						}
						break;
					case WM_KEYDOWN:
						{
							NativeWindowKeyInfo info=ConvertKey(wParam, lParam);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->KeyDown(info);
							}
						}
						break;
					case WM_SYSKEYUP:
						{
							NativeWindowKeyInfo info=ConvertKey(wParam, lParam);
							info.autoRepeatKeyDown = false;
							if (supressingAlt && !info.ctrl && !info.shift && info.code == VKEY::_MENU)
							{
								supressingAlt = false;
								break;
							}
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->SysKeyUp(info);
							}
						}
						break;
					case WM_SYSKEYDOWN:
						{
							NativeWindowKeyInfo info=ConvertKey(wParam, lParam);
							if (supressingAlt && !info.ctrl && !info.shift && info.code == VKEY::_MENU)
							{
								break;
							}
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->SysKeyDown(info);
							}
						}
						break;
					case WM_CHAR:
						{
							NativeWindowCharInfo info=ConvertChar(wParam);
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->Char(info);
							}
						}
						break;
					// ************************************** painting
					case WM_PAINT:
						{
							for(vint i=0;i<listeners.Count();i++)
							{
								listeners[i]->Paint();
							}
						}
						break;
					case WM_ERASEBKGND:
						result = 0;
						return true;
					case WM_NCPAINT:
					case WM_SYNCPAINT:
						if(customFrameMode)
						{
							result=0;
							return true;
						}
						break;
					// ************************************** IME
					case WM_IME_SETCONTEXT:
						if(wParam==TRUE)
						{
							HIMC imc = ImmGetContext(handle);
							ImmAssociateContext(hwnd, imc);
							ImmReleaseContext(handle, imc);
						}
						break;
					case WM_IME_STARTCOMPOSITION:
						UpdateCompositionForContent();
						break;
					// ************************************** hit test
					case WM_NCHITTEST:
						{
							POINTS location=MAKEPOINTS(lParam);
							NativePoint windowLocation=GetBounds().LeftTop();
							location.x-=(SHORT)windowLocation.x.value;
							location.y-=(SHORT)windowLocation.y.value;
							for(vint i=0;i<listeners.Count();i++)
							{
								switch(listeners[i]->HitTest(NativePoint(location.x, location.y)))
								{
								case INativeWindowListener::BorderNoSizing:
									result=HTBORDER;
									return true;
								case INativeWindowListener::BorderLeft:
									result=HTLEFT;
									return true;
								case INativeWindowListener::BorderRight:
									result=HTRIGHT;
									return true;
								case INativeWindowListener::BorderTop:
									result=HTTOP;
									return true;
								case INativeWindowListener::BorderBottom:
									result=HTBOTTOM;
									return true;
								case INativeWindowListener::BorderLeftTop:
									result=HTTOPLEFT;
									return true;
								case INativeWindowListener::BorderRightTop:
									result=HTTOPRIGHT;
									return true;
								case INativeWindowListener::BorderLeftBottom:
									result=HTBOTTOMLEFT;
									return true;
								case INativeWindowListener::BorderRightBottom:
									result=HTBOTTOMRIGHT;
									return true;
								case INativeWindowListener::Title:
									result=HTCAPTION;
									return true;
								case INativeWindowListener::ButtonMinimum:
									result=HTMINBUTTON;
									return true;
								case INativeWindowListener::ButtonMaximum:
									result=HTMAXBUTTON;
									return true;
								case INativeWindowListener::ButtonClose:
									result=HTCLOSE;
									return true;
								case INativeWindowListener::Client:
									result=HTCLIENT;
									return true;
								case INativeWindowListener::Icon:
									result=HTSYSMENU;
									return true;
								}
							}
						}
						break;
					// ************************************** MISC
					case WM_SETCURSOR:
						{
							DWORD hitTestResult=LOWORD(lParam);
							if(hitTestResult==HTCLIENT)
							{
								HCURSOR cursorHandle=cursor->GetCursorHandle();
								if(GetCursor()!=cursorHandle)
								{
									SetCursor(cursorHandle);
								}
								result=TRUE;
								return true;
							}
						}
						break;
					case WM_NCCALCSIZE:
						if((BOOL)wParam && customFrameMode)
						{
							result=0;
							return true;
						}
						break;
					case WM_NCACTIVATE:
						if(customFrameMode)
						{
							if(wParam==TRUE)
							{
								result=FALSE;
							}
							else
							{
								result=TRUE;
							}
							return true;
						}
						break;
					}

					if(IsWindow(hwnd)!=0)
					{
						if(transferFocusEvent && IsFocused())
						{
							WindowsForm* window=this;
							while(window->parentWindow && window->alwaysPassFocusToParent)
							{
								window=window->parentWindow;
							}
							if(window!=this)
							{
								window->SetFocus();
							}
						}
					}

					if (customFrameMode)
					{
						switch (uMsg)
						{
						case WM_NCLBUTTONDOWN:
							switch (wParam)
							{
							case HTMINBUTTON:
							case HTMAXBUTTON:
							case HTCLOSE:
								result = 0;
								return true;
							}
							break;
						case WM_LBUTTONUP:
							{
								POINTS location = MAKEPOINTS(lParam);
								for(vint i=0;i<listeners.Count();i++)
								{
									switch(listeners[i]->HitTest(NativePoint(location.x, location.y)))
									{
									case INativeWindowListener::ButtonMinimum:
										ShowMinimized();
										return false;
									case INativeWindowListener::ButtonMaximum:
										if (GetSizeState() == INativeWindow::Maximized)
										{
											ShowRestored();
										}
										else
										{
											ShowMaximized();
										}
										return false;
									case INativeWindowListener::ButtonClose:
										Hide(true);
										return false;
									}
								}
							}
							break;
						}
					}
					return false;
				}
			protected:
				HWND								handle;
				WString								title;
				WindowsCursor*						cursor = nullptr;
				NativePoint							caretPoint;
				WindowsForm*						parentWindow = nullptr;
				bool								alwaysPassFocusToParent = false;
				List<INativeWindowListener*>		listeners;
				vint								mouseLastX = -1;
				vint								mouseLastY = -1;
				bool								mouseHoving = false;
				Interface*							graphicsHandler = nullptr;
				bool								customFrameMode = false;
				List<Ptr<INativeMessageHandler>>	messageHandlers;
				bool								supressingAlt = false;
				Ptr<bool>							flagDisposed = new bool(false);
				NativeMargin						customFramePadding;
				Ptr<GuiImageData>					defaultIcon;
				Ptr<GuiImageData>					replacementIcon;
				HICON								replacementHIcon = NULL;
				UINT								dpiX = 0;
				UINT								dpiY = 0;

				void UpdateDpiAwaredFields(bool refreshDpiXY)
				{
					if (refreshDpiXY)
					{
						DpiAwared_GetDpiForWindow(handle, &dpiX, &dpiY);
					}
					auto padding = (vint)(DpiAwared_GetSystemMetrics(SM_CXSIZEFRAME, dpiX) + DpiAwared_GetSystemMetrics(SM_CXPADDEDBORDER, dpiX));
					customFramePadding = NativeMargin(padding, padding, padding, padding);
				}
			public:
				WindowsForm(HWND parent, WString className, HINSTANCE hInstance)
				{
					DWORD exStyle = WS_EX_APPWINDOW | WS_EX_CONTROLPARENT;
					DWORD style = WS_BORDER | WS_CAPTION | WS_SIZEBOX | WS_SYSMENU | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
					handle = CreateWindowEx(exStyle, className.Buffer(), L"", style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, parent, NULL, hInstance, NULL);

					UpdateDpiAwaredFields(true);
				}

				~WindowsForm()
				{
					*flagDisposed.Obj() = true;
					List<INativeWindowListener*> copiedListeners;
					CopyFrom(copiedListeners, listeners);
					for (vint i = 0; i < copiedListeners.Count(); i++)
					{
						INativeWindowListener* listener = copiedListeners[i];
						if (listeners.Contains(listener))
						{
							listener->Destroyed();
						}
					}
					DestroyWindow(handle);
				}

				void InvokeDestroying()
				{
					for(vint i=0;i<listeners.Count();i++)
					{
						listeners[i]->Destroying();
					}
				}

				bool HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
				{
#define CHECK_DISPOSED if (*flag.Obj()) return skip
					auto flag = flagDisposed;
					bool skip = false;
					{
						FOREACH(Ptr<INativeMessageHandler>, handler, messageHandlers)
						{
							handler->BeforeHandle(hwnd, uMsg, wParam, lParam, skip);
							CHECK_DISPOSED;
						}
						if (skip)
						{
							return true;
						}
					}
					skip = HandleMessageInternal(hwnd, uMsg, wParam, lParam, result);
					CHECK_DISPOSED;
					if (GetWindowsFormFromHandle(hwnd))
					{
						FOREACH(Ptr<INativeMessageHandler>, handler, messageHandlers)
						{
							handler->AfterHandle(hwnd, uMsg, wParam, lParam, skip, result);
							CHECK_DISPOSED;
						}
					}
					return skip;
#undef CHECK_DISPOSED
				}

				HWND GetWindowHandle()override
				{
					return handle;
				}

				Interface* GetGraphicsHandler()override
				{
					return graphicsHandler;
				}

				void SetGraphicsHandler(Interface* handler)override
				{
					graphicsHandler=handler;
				}

				bool InstallMessageHandler(Ptr<INativeMessageHandler> handler)override
				{
					if (messageHandlers.Contains(handler.Obj()))
					{
						return false;
					}
					messageHandlers.Add(handler);
					return true;
				}

				bool UninstallMessageHandler(Ptr<INativeMessageHandler> handler)override
				{
					vint index = messageHandlers.IndexOf(handler.Obj());
					if (index == -1)return false;
					messageHandlers.RemoveAt(handler);
					return true;
				}

				Point Convert(NativePoint value)override
				{
					return Point((vint)value.x.value * 96 / dpiX, (vint)value.y.value * 96 / dpiY);
				}

				NativePoint Convert(Point value)override
				{
					return NativePoint(value.x * dpiX / 96, value.y * dpiY / 96);
				}

				Size Convert(NativeSize value)override
				{
					return Size((vint)value.x.value * 96 / dpiX, (vint)value.y.value * 96 / dpiY);
				}

				NativeSize Convert(Size value)override
				{
					return NativeSize(value.x * dpiX / 96, value.y * dpiY / 96);
				}

				Margin Convert(NativeMargin value)override
				{
					return Margin(
						(vint)value.left.value * 96 / dpiX,
						(vint)value.top.value * 96 / dpiY,
						(vint)value.right.value * 96 / dpiX,
						(vint)value.bottom.value * 96 / dpiY
					);
				}

				NativeMargin Convert(Margin value)override
				{
					return NativeMargin(
						(vint)value.left * dpiX / 96,
						(vint)value.top * dpiY / 96,
						(vint)value.right * dpiX / 96,
						(vint)value.bottom * dpiY / 96
					);
				}

				NativeRect GetBounds()override
				{
					RECT rect;
					GetWindowRect(handle, &rect);
					return NativeRect(rect.left, rect.top, rect.right, rect.bottom);
				}

				void SetBounds(const NativeRect& bounds)override
				{
					NativeRect newBounds=bounds;
					for(vint i=0;i<listeners.Count();i++)
					{
						listeners[i]->Moving(newBounds, true);
					}
					MoveWindow(handle, (int)newBounds.Left().value, (int)newBounds.Top().value, (int)newBounds.Width().value, (int)newBounds.Height().value, FALSE);
				}

				NativeSize GetClientSize()override
				{
					return GetClientBoundsInScreen().GetSize();
				}

				void SetClientSize(NativeSize size)override
				{
					RECT required={0,0,(int)size.x.value,(int)size.y.value };
					RECT bounds;
					GetWindowRect(handle, &bounds);
					DpiAwared_AdjustWindowRect(&required, handle, dpiX);
					SetBounds(NativeRect(NativePoint(bounds.left, bounds.top), NativeSize(required.right-required.left, required.bottom-required.top)));
				}

				NativeRect GetClientBoundsInScreen()override
				{
					if(customFrameMode)
					{
						return GetBounds();
					}
					else
					{
						RECT required={0,0,0,0};
						RECT bounds;
						GetWindowRect(handle, &bounds);
						DpiAwared_AdjustWindowRect(&required, handle, dpiX);
						return NativeRect(
							NativePoint(
								(bounds.left-required.left),
								(bounds.top-required.top)
								),
							NativeSize(
								(bounds.right-bounds.left)-(required.right-required.left),
								(bounds.bottom-bounds.top)-(required.bottom-required.top)
								)
							);
					}
				}

				WString GetTitle()override
				{
					return title;
				}

				void SetTitle(WString _title)override
				{
					title=_title;
					SetWindowText(handle, title.Buffer());
				}

				INativeCursor* GetWindowCursor()override
				{
					return cursor;
				}

				void SetWindowCursor(INativeCursor* _cursor)override
				{
					WindowsCursor* newCursor=dynamic_cast<WindowsCursor*>(_cursor);
					if(newCursor && cursor!=newCursor)
					{
						cursor=newCursor;
						if(mouseHoving && IsVisible())
						{
							SetCursor(cursor->GetCursorHandle());
						}
					}
				}
				
				NativePoint GetCaretPoint()override
				{
					return caretPoint;
				}

				void SetCaretPoint(NativePoint point)override
				{
					caretPoint=point;
					UpdateCompositionForContent();
				}

				INativeWindow* GetParent()override
				{
					return parentWindow;
				}

				void SetParent(INativeWindow* parent)override
				{
					parentWindow=dynamic_cast<WindowsForm*>(parent);
					if(parentWindow)
					{
						SetWindowLongPtr(handle, GWLP_HWNDPARENT, (LONG_PTR)parentWindow->handle);
					}
					else
					{
						SetWindowLongPtr(handle, GWLP_HWNDPARENT, NULL);
					}
				}

				bool GetAlwaysPassFocusToParent()override
				{
					return alwaysPassFocusToParent;
				}

				void SetAlwaysPassFocusToParent(bool value)override
				{
					alwaysPassFocusToParent=value;
				}

				void EnableCustomFrameMode()override
				{
					customFrameMode=true;
				}

				void DisableCustomFrameMode()override
				{
					customFrameMode=false;
				}

				bool IsCustomFrameModeEnabled()override
				{
					return customFrameMode;
				}

				NativeMargin GetCustomFramePadding()override
				{
					if (GetSizeBox() || GetTitleBar())
					{
						return customFramePadding;
					}
					else
					{
						return NativeMargin(0, 0, 0, 0);
					}
				}

				Ptr<GuiImageData> GetIcon()override
				{
					if (replacementIcon && replacementIcon->GetImage())
					{
						return replacementIcon;
					}
					else
					{
						if (!defaultIcon)
						{
							auto icon = CreateWindowDefaultIcon(16);
							if (icon == NULL)
							{
								icon = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 16, 16, LR_SHARED);
							}
							if (icon != NULL)
							{
								defaultIcon = new GuiImageData(CreateImageFromHICON(icon), 0);
							}
						}
						return defaultIcon;
					}
				}

				static double GetSizeScore(vint size)
				{
					if (size > 32)
					{
						return 32.0 / size;
					}
					else if (size < 32)
					{
						return size / 32.0 - 1;
					}
					else
					{
						return 1.0;
					}
				}

				static vint GetBppFromFormat(const WICPixelFormatGUID& format)
				{
					if (format == GUID_WICPixelFormat1bppIndexed)	return 1;
					if (format == GUID_WICPixelFormat2bppIndexed)	return 2;
					if (format == GUID_WICPixelFormat4bppIndexed)	return 4;
					if (format == GUID_WICPixelFormat8bppIndexed)	return 8;
					if (format == GUID_WICPixelFormatBlackWhite)	return 1;
					if (format == GUID_WICPixelFormat2bppGray)		return 2;
					if (format == GUID_WICPixelFormat4bppGray)		return 4;
					if (format == GUID_WICPixelFormat8bppGray)		return 8;
					if (format == GUID_WICPixelFormat8bppAlpha)		return 8;
					if (format == GUID_WICPixelFormat16bppBGR555)	return 16;
					if (format == GUID_WICPixelFormat16bppBGR565)	return 16;
					if (format == GUID_WICPixelFormat16bppBGRA5551)	return 16;
					if (format == GUID_WICPixelFormat16bppGray)		return 16;
					if (format == GUID_WICPixelFormat24bppBGR)		return 24;
					if (format == GUID_WICPixelFormat24bppRGB)		return 24;
					if (format == GUID_WICPixelFormat32bppBGR)		return 32;
					if (format == GUID_WICPixelFormat32bppBGRA)		return 32;
					if (format == GUID_WICPixelFormat32bppPBGRA)	return 32;
					return -1;
				}

				void SetIcon(Ptr<GuiImageData> icon)override
				{
					replacementIcon = icon;
					HICON newReplacementHIcon = NULL;
					if (replacementIcon && replacementIcon->GetImage())
					{
						stream::MemoryStream memoryStream;

						replacementIcon->GetImage()->SaveToStream(memoryStream, INativeImage::Icon);
						if (memoryStream.Size() > 0)
						{
							newReplacementHIcon = CreateIconFromResource((PBYTE)memoryStream.GetInternalBuffer(), (DWORD)memoryStream.Size(), TRUE, 0x00030000);
							if (newReplacementHIcon != NULL)
							{
								goto SKIP;
							}
						}

						INativeImageFrame* selectedFrame = nullptr;
						for (vint i = 0; i < replacementIcon->GetImage()->GetFrameCount(); i++)
						{
							auto frame = replacementIcon->GetImage()->GetFrame(i);
							auto size = frame->GetSize();
							if (size.x == size.y)
							{
								auto bitmap = GetWICBitmap(frame);
								WICPixelFormatGUID format;
								HRESULT hr = bitmap->GetPixelFormat(&format);
								if (hr != S_OK) continue;

								if (!selectedFrame)
								{
									selectedFrame = frame;
								}
								else
								{
									auto score = GetSizeScore(size.x);
									auto scoreSelected = GetSizeScore(selectedFrame->GetSize().x);
									if (score > scoreSelected)
									{
										selectedFrame = frame;
									}
									else if (score == scoreSelected)
									{
										WICPixelFormatGUID selectedFormat;
										auto selectedBitmap = GetWICBitmap(selectedFrame);
										hr = selectedBitmap->GetPixelFormat(&selectedFormat);

										auto bpp = GetBppFromFormat(format);
										auto bppSelected = GetBppFromFormat(selectedFormat);
										if (bpp > bppSelected)
										{
											selectedFrame = frame;
										}
									}
								}
							}
						}

						if (selectedFrame)
						{
							bool succeeded = false;
							WindowsBitmapImage newBitmap(replacementIcon->GetImage()->GetImageService(), GetWICBitmap(selectedFrame), replacementIcon->GetImage()->GetFormat());
							newBitmap.SaveToStream(memoryStream, INativeImage::Bmp);
							if (memoryStream.Size() > 0)
							{
								auto pBuffer = (char*)memoryStream.GetInternalBuffer();
								tagBITMAPFILEHEADER bfh = *(tagBITMAPFILEHEADER*)pBuffer;
								tagBITMAPINFOHEADER bih = *(tagBITMAPINFOHEADER*)(pBuffer + sizeof(tagBITMAPFILEHEADER));
								RGBQUAD rgb = *(RGBQUAD*)(pBuffer + sizeof(tagBITMAPFILEHEADER) + sizeof(tagBITMAPINFOHEADER));

								BITMAPINFO bi;
								bi.bmiColors[0] = rgb;
								bi.bmiHeader = bih;

								char* pPixels = (pBuffer + bfh.bfOffBits);
								char* ppvBits;
								auto hBitmap = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, (void**)&ppvBits, NULL, 0);
								if (hBitmap != NULL)
								{
									SetDIBits(NULL, hBitmap, 0, bih.biHeight, pPixels, &bi, DIB_RGB_COLORS);
									auto himl = ImageList_Create(32, 32, ILC_COLOR32, 1, 1);
									if (himl != NULL)
									{
										int addResult = ImageList_Add(himl, hBitmap, NULL);
										newReplacementHIcon = ImageList_GetIcon(himl, 0, ILD_NORMAL);
										ImageList_Destroy(himl);
									}
									DeleteObject(hBitmap);
								}
							}
						}
					}

				SKIP:
					{
						HICON hAppIcon = newReplacementHIcon;
						if (hAppIcon == NULL) hAppIcon = CreateWindowDefaultIcon();
						if (hAppIcon == NULL) hAppIcon = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
						bool isVisible = IsVisible();
						if (isVisible) SendMessage(handle, WM_SETREDRAW, (WPARAM)FALSE, NULL);
						SendMessage(handle, WM_SETICON, ICON_BIG, (LPARAM)newReplacementHIcon);
						SendMessage(handle, WM_SETICON, ICON_SMALL, (LPARAM)newReplacementHIcon);
						if (isVisible) SendMessage(handle, WM_SETREDRAW, (WPARAM)TRUE, NULL);

						if (this == GetCurrentController()->WindowService()->GetMainWindow())
						{
							SendMessage(GetWindow(handle, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
							SendMessage(GetWindow(handle, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hAppIcon);
						}
					}

					if (replacementHIcon != NULL)
					{
						DestroyIcon(replacementHIcon);
					}
					replacementHIcon = newReplacementHIcon;
				}

				WindowSizeState GetSizeState()override
				{
					if(IsIconic(handle))
					{
						return INativeWindow::Minimized;
					}
					else if(IsZoomed(handle))
					{
						return INativeWindow::Maximized;
					}
					else
					{
						return INativeWindow::Restored;
					}
				}

				void Show()override
				{
					ShowWindow(handle, SW_SHOWNORMAL);
				}

				void ShowDeactivated()override
				{
					ShowWindow(handle, SW_SHOWNOACTIVATE);
					SetWindowPos(handle,HWND_TOP,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
				}

				void ShowRestored()override
				{
					ShowWindow(handle, SW_RESTORE);
				}

				void ShowMaximized()override
				{
					ShowWindow(handle, SW_SHOWMAXIMIZED);
				}

				void ShowMinimized()override
				{
					ShowWindow(handle, SW_SHOWMINIMIZED);
				}

				void Hide(bool closeWindow)override
				{
					if (closeWindow)
					{
						PostMessage(handle, WM_CLOSE, NULL, NULL);
					}
					else
					{
						ShowWindow(handle, SW_HIDE);
					}
				}

				bool IsVisible()override
				{
					return IsWindowVisible(handle)!=0;
				}

				void Enable()override
				{
					EnableWindow(handle, TRUE);
				}

				void Disable()override
				{
					EnableWindow(handle, FALSE);
				}

				bool IsEnabled()override
				{
					return IsWindowEnabled(handle)!=0;
				}

				void SetFocus()override
				{
					::SetFocus(handle);
				}

				bool IsFocused()override
				{
					return GetFocus()==handle;
				}

				void SetActivate()override
				{
					SetActiveWindow(handle);
				}

				bool IsActivated()override
				{
					return GetActiveWindow()==handle;
				}

				void ShowInTaskBar()override
				{
					SetExStyle(WS_EX_APPWINDOW, true);
				}

				void HideInTaskBar()override
				{
					SetExStyle(WS_EX_APPWINDOW, false);
				}

				bool IsAppearedInTaskBar()override
				{
					return GetExStyle(WS_EX_APPWINDOW);
				}

				void EnableActivate()override
				{
					SetExStyle(WS_EX_NOACTIVATE, false);
				}

				void DisableActivate()override
				{
					SetExStyle(WS_EX_NOACTIVATE, true);
				}

				bool IsEnabledActivate()override
				{
					return !GetExStyle(WS_EX_NOACTIVATE);
				}

				bool RequireCapture()override
				{
					SetCapture(handle);
					return true;
				}

				bool ReleaseCapture()override
				{
					::ReleaseCapture();
					return true;
				}

				bool IsCapturing()override
				{
					return GetCapture()==handle;
				}

				bool GetMaximizedBox()override
				{
					return GetStyle(WS_MAXIMIZEBOX);
				}

				void SetMaximizedBox(bool visible)override
				{
					SetStyle(WS_MAXIMIZEBOX, visible);
				}

				bool GetMinimizedBox()override
				{
					return GetStyle(WS_MINIMIZEBOX);
				}

				void SetMinimizedBox(bool visible)override
				{
					SetStyle(WS_MINIMIZEBOX, visible);
				}

				bool GetBorder()override
				{
					return GetStyle(WS_BORDER);
				}

				void SetBorder(bool visible)override
				{
					SetStyle(WS_BORDER, visible);
				}

				bool GetSizeBox()override
				{
					return GetStyle(WS_SIZEBOX);
				}

				void SetSizeBox(bool visible)override
				{
					SetStyle(WS_SIZEBOX, visible);
				}

				bool GetIconVisible()override
				{
					return GetStyle(WS_SYSMENU);
				}

				void SetIconVisible(bool visible)override
				{
					SetStyle(WS_SYSMENU, visible);
				}

				bool GetTitleBar()override
				{
					return GetStyle(WS_CAPTION);
				}

				void SetTitleBar(bool visible)override
				{
					SetStyle(WS_CAPTION, visible);
				}

				bool GetTopMost()override
				{
					return GetExStyle(WS_EX_TOPMOST);
				}

				void SetTopMost(bool topmost)override
				{
					SetWindowPos(handle, (topmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
				}

				void SupressAlt()override
				{
					if (!supressingAlt)
					{
						supressingAlt = true;
						PostMessage(handle, WM_SYSKEYDOWN, VK_MENU, 0);
						PostMessage(handle, WM_SYSKEYUP, VK_MENU, 0);
					}
				}

				bool InstallListener(INativeWindowListener* listener)override
				{
					if(listeners.Contains(listener))
					{
						return false;
					}
					else
					{
						listeners.Add(listener);
						return true;
					}
				}

				bool UninstallListener(INativeWindowListener* listener)override
				{
					if(listeners.Contains(listener))
					{
						listeners.Remove(listener);
						return true;
					}
					else
					{
						return false;
					}
				}

				void RedrawContent()override
				{
					if(graphicsHandler)
					{
						SendMessage(this->handle, WM_PAINT, NULL, NULL);
					}
				}
			};

/***********************************************************************
WindowsController
***********************************************************************/

			LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT CALLBACK GodProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT CALLBACK MouseProc(int nCode , WPARAM wParam , LPARAM lParam);

			class WindowsController : public Object, public virtual INativeController, public virtual INativeWindowService
			{
			protected:
				WinClass							windowClass;
				WinClass							godClass;
				HINSTANCE							hInstance;
				HWND								godWindow;
				Dictionary<HWND, WindowsForm*>		windows;
				INativeWindow*						mainWindow;
				HWND								mainWindowHandle;

				WindowsCallbackService				callbackService;
				WindowsResourceService				resourceService;
				WindowsAsyncService					asyncService;
				WindowsClipboardService				clipboardService;
				WindowsImageService					imageService;
				WindowsScreenService				screenService;
				WindowsInputService					inputService;
				WindowsDialogService				dialogService;

			public:
				WindowsController(HINSTANCE _hInstance)
					:hInstance(_hInstance)
					,windowClass(L"VczhWindow", false, false, WndProc, _hInstance)
					,godClass(L"GodWindow", false, false, GodProc, _hInstance)
					,mainWindow(0)
					,mainWindowHandle(0)
					,screenService(&GetHWNDFromNativeWindowHandle)
					,inputService(&MouseProc)
					,dialogService(&GetHWNDFromNativeWindowHandle)
				{
					godWindow=CreateWindowEx(WS_EX_CONTROLPARENT, godClass.GetName().Buffer(), L"GodWindow", WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
					clipboardService.SetOwnerHandle(godWindow);
					inputService.SetOwnerHandle(godWindow);
				}

				~WindowsController()
				{
					inputService.StopTimer();
					inputService.StopHookMouse();
					clipboardService.SetOwnerHandle(NULL);
					DestroyWindow(godWindow);
				}

				WindowsForm* GetWindowsFormFromHandle(HWND hwnd)
				{
					vint index = windows.Keys().IndexOf(hwnd);
					if (index == -1)return 0;
					return windows.Values()[index];
				}

				bool HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result)
				{
					bool skipDefaultProcedure=false;
					{
						vint index = windows.Keys().IndexOf(hwnd);
						if (index != -1)
						{
							WindowsForm* window = windows.Values().Get(index);
							skipDefaultProcedure = window->HandleMessage(hwnd, uMsg, wParam, lParam, result);
							switch (uMsg)
							{
							case WM_CLOSE:
								if (!skipDefaultProcedure)
								{
									ShowWindow(window->GetWindowHandle(), SW_HIDE);
									if (window != mainWindow)
									{
										skipDefaultProcedure = true;
									}
								}
								break;
							case WM_DESTROY:
								DestroyNativeWindow(window);
								break;
							}
						}
					}
					{
						if (hwnd == mainWindowHandle && uMsg == WM_DESTROY)
						{
							for (vint i = 0; i < windows.Count(); i++)
							{
								if (windows.Values().Get(i)->IsVisible())
								{
									windows.Values().Get(i)->Hide(true);
								}
							}
							while (windows.Count())
							{
								DestroyNativeWindow(windows.Values().Get(0));
							}
							PostQuitMessage(0);
						}
					}
					return skipDefaultProcedure;
				}

				//=======================================================================

				INativeWindow* CreateNativeWindow()override
				{
					WindowsForm* window=new WindowsForm(godWindow, windowClass.GetName(), hInstance);
					windows.Add(window->GetWindowHandle(), window);
					callbackService.InvokeNativeWindowCreated(window);
					window->SetWindowCursor(resourceService.GetDefaultSystemCursor());
					return window;
				}

				void DestroyNativeWindow(INativeWindow* window)override
				{
					WindowsForm* windowsForm=dynamic_cast<WindowsForm*>(window);
					windowsForm->InvokeDestroying();
					if(windowsForm!=0 && windows.Keys().Contains(windowsForm->GetWindowHandle()))
					{
						callbackService.InvokeNativeWindowDestroyed(window);
						windows.Remove(windowsForm->GetWindowHandle());
						delete windowsForm;
					}
				}

				INativeWindow* GetMainWindow()override
				{
					return mainWindow;
				}

				void Run(INativeWindow* window)override
				{
					mainWindow=window;
					mainWindowHandle=GetWindowsForm(window)->GetWindowHandle();
					mainWindow->Show();
					MSG message;
					while(GetMessage(&message, NULL, 0, 0))
					{
						TranslateMessage(&message);
						DispatchMessage(&message);
						asyncService.ExecuteAsyncTasks();
					}
				}

				INativeWindow* GetWindow(NativePoint location)override
				{
					POINT p;
					p.x=(int)location.x.value;
					p.y=(int)location.y.value;
					HWND handle=WindowFromPoint(p);
					vint index=windows.Keys().IndexOf(handle);
					if(index==-1)
					{
						return 0;
					}
					else
					{
						return windows.Values().Get(index);
					}
				}

				//=======================================================================

				INativeCallbackService* CallbackService()
				{
					return &callbackService;
				}

				INativeResourceService* ResourceService()
				{
					return &resourceService;
				}
				
				INativeAsyncService* AsyncService()
				{
					return &asyncService;
				}

				INativeClipboardService* ClipboardService()
				{
					return &clipboardService;
				}

				INativeImageService* ImageService()
				{
					return &imageService;
				}

				INativeScreenService* ScreenService()
				{
					return &screenService;
				}

				INativeWindowService* WindowService()
				{
					return this;
				}

				INativeInputService* InputService()
				{
					return &inputService;
				}

				INativeDialogService* DialogService()
				{
					return &dialogService;
				}

				WString GetExecutablePath()
				{
					Array<wchar_t> buffer(65536);
					GetModuleFileName(NULL, &buffer[0], (DWORD)buffer.Count());
					return &buffer[0];
				}

				//=======================================================================

				void InvokeMouseHook(WPARAM message, NativePoint location)
				{
					callbackService.InvokeMouseHook(message, location);
				}

				void InvokeGlobalTimer()
				{
					callbackService.InvokeGlobalTimer();
				}

				void InvokeClipboardUpdated()
				{
					callbackService.InvokeClipboardUpdated();
				}
			};

/***********************************************************************
Windows Procedure
***********************************************************************/

			LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				WindowsController* controller=dynamic_cast<WindowsController*>(GetCurrentController());
				if(controller)
				{
					LRESULT result=0;
					if(controller->HandleMessage(hwnd, uMsg, wParam, lParam, result))
					{
						return result;
					}
				}
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}

			LRESULT CALLBACK GodProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				WindowsController* controller=dynamic_cast<WindowsController*>(GetCurrentController());
				if(controller)
				{
					switch(uMsg)
					{
					case WM_TIMER:
						controller->InvokeGlobalTimer();
						break;
					case WM_CLIPBOARDUPDATE:
						controller->InvokeClipboardUpdated();
						break;
					}
				}
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}

			LRESULT CALLBACK MouseProc(int nCode , WPARAM wParam , LPARAM lParam)
			{
				WindowsController* controller=dynamic_cast<WindowsController*>(GetCurrentController());
				if(controller)
				{
					MSLLHOOKSTRUCT* mouseHookStruct=(MSLLHOOKSTRUCT*)lParam;
					NativePoint location(mouseHookStruct->pt.x, mouseHookStruct->pt.y);
					controller->InvokeMouseHook(wParam, location);
				}
				return CallNextHookEx(NULL,nCode,wParam,lParam);
			}

/***********************************************************************
Windows Platform Native Controller
***********************************************************************/

			INativeController* CreateWindowsNativeController(HINSTANCE hInstance)
			{
				return new WindowsController(hInstance);
			}

			IWindowsForm* GetWindowsFormFromHandle(HWND hwnd)
			{
				auto controller = dynamic_cast<WindowsController*>(GetCurrentController());
				if (controller)
				{
					return controller->GetWindowsFormFromHandle(hwnd);
				}
				return 0;
			}

			IWindowsForm* GetWindowsForm(INativeWindow* window)
			{
				return dynamic_cast<WindowsForm*>(window);
			}

			void DestroyWindowsNativeController(INativeController* controller)
			{
				delete controller;
			}

			void EnableCrossKernelCrashing()
			{
				/*
				"HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options"
					DWORD DisableUserModeCallbackFilter = 1

				"HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\TestCppCodegen.exe"
					DWORD DisableUserModeCallbackFilter = 1
				*/
				typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags); 
				typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags); 
				const DWORD EXCEPTION_SWALLOWING = 0x1;
 
				HMODULE kernel32 = LoadLibrary(L"kernel32.dll"); 
				tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy"); 
				tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy"); 
				if (pGetPolicy && pSetPolicy) 
				{ 
					DWORD dwFlags; 
					if (pGetPolicy(&dwFlags)) 
					{ 
						// Turn off the filter 
						pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
					} 
				} 
			}
		}
	}
}