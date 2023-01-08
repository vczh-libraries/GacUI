#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiMainHostedWindowProxy
***********************************************************************/

		class GuiMainHostedWindowProxy
			: public Object
			, public virtual IGuiHostedWindowProxy
		{
		protected:
			GuiHostedWindowData*			data = nullptr;
			INativeWindow*					nativeWindow = nullptr;

		public:

			GuiMainHostedWindowProxy(GuiHostedWindowData* _data, INativeWindow* _nativeWindow)
				: data(_data)
				, nativeWindow(_nativeWindow)
			{
			}

			void CheckAndSyncProperties() override
			{
				UpdateBounds();
				UpdateTitle();
				UpdateIcon();
				UpdateEnabled();
				UpdateTopMost();

				UpdateMaximizedBox();
				UpdateMinimizedBox();
				UpdateBorderVisible();
				UpdateSizeBox();
				UpdateIconVisible();
				UpdateTitleBar();

				UpdateShowInTaskBar();
				UpdateEnabledActivate();
				UpdateCustomFrameMode();

				if (!data->wmWindow.visible)
				{
					data->wmWindow.Show();
				}
			}

			/***********************************************************************
			Visible Properties
			***********************************************************************/

			NativeRect FixBounds(const NativeRect& bounds) override
			{
				return { {},bounds.GetSize() };
			}

			void UpdateBounds() override
			{
				nativeWindow->SetClientSize(data->wmWindow.bounds.GetSize());
			}

			void UpdateTitle() override
			{
				nativeWindow->SetTitle(data->windowTitle);
			}

			void UpdateIcon() override
			{
				nativeWindow->SetIcon(data->windowIcon);
			}

			void UpdateEnabled() override
			{
				// Disabling the main window will not disable the native window
				// otherwise the whole application is disabled
			}

			void UpdateTopMost() override
			{
				nativeWindow->SetTopMost(data->wmWindow.topMost);
			}

			/***********************************************************************
			Border Properties
			***********************************************************************/

			void UpdateMaximizedBox() override
			{
				nativeWindow->SetMaximizedBox(data->windowMaximizedBox);
			}

			void UpdateMinimizedBox() override
			{
				nativeWindow->SetMinimizedBox(data->windowMinimizedBox);
			}

			void UpdateBorderVisible() override
			{
				nativeWindow->SetBorder(data->windowBorder);
			}

			void UpdateSizeBox() override
			{
				nativeWindow->SetSizeBox(data->windowSizeBox);
			}

			void UpdateIconVisible() override
			{
				nativeWindow->SetIconVisible(data->windowIconVisible);
			}

			void UpdateTitleBar() override
			{
				nativeWindow->SetTitleBar(data->windowTitleBar);
			}

			/***********************************************************************
			Behavior Properties
			***********************************************************************/

			void UpdateShowInTaskBar() override
			{
				if (data->windowShowInTaskBar)
				{
					nativeWindow->ShowInTaskBar();
				}
				else
				{
					nativeWindow->HideInTaskBar();
				}
			}

			void UpdateEnabledActivate() override
			{
				// In hosted mode, the native window is always activatable
			}

			void UpdateCustomFrameMode() override
			{
				if (data->windowCustomFrameMode)
				{
					nativeWindow->EnableCustomFrameMode();
				}
				else
				{
					nativeWindow->DisableCustomFrameMode();
				}
			}

			/***********************************************************************
			Show/Hide/Focus
			***********************************************************************/

			void Show() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void ShowDeactivated() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void ShowRestored() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void ShowMaximized() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void ShowMinimized() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void Hide() override
			{
				// In hosted mode, the main window is never closed
				// instead the native window is closed
				nativeWindow->Hide(false);
			}

			void Close() override
			{
				// In hosted mode, the main window is never closed
				// instead the native window is closed
				nativeWindow->Hide(true);
			}

			void SetFocus() override
			{
				data->wmWindow.Activate();
				nativeWindow->SetActivate();
			}
		};

/***********************************************************************
Helper
***********************************************************************/

		Ptr<IGuiHostedWindowProxy> CreateMainHostedWindowProxy(GuiHostedWindowData* data, INativeWindow* nativeWindow)
		{
			return Ptr(new GuiMainHostedWindowProxy(data, nativeWindow));
		}
	}
}