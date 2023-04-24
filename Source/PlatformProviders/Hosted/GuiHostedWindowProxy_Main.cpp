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
				for (auto listener : data->listeners)
				{
					listener->AssignFrameConfig(data->controller->WindowService()->GetMainWindowFrameConfig());
				}

				if (!data->wmWindow.visible)
				{
					data->wmWindow.Show();
				}
				data->wmWindow.SetBounds(FixBounds(data->wmWindow.bounds));

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

			In hosted mode, the main window is never closed.
			Closing the main window causes the native window to be closed.
			***********************************************************************/

			void Show() override
			{
				data->wmWindow.Activate();
				nativeWindow->Show();
			}

			void ShowDeactivated() override
			{
				data->wmWindow.Deactivate();
				nativeWindow->ShowDeactivated();
			}

			void ShowRestored() override
			{
				nativeWindow->ShowRestored();
			}

			void ShowMaximized() override
			{
				nativeWindow->ShowMaximized();
			}

			void ShowMinimized() override
			{
				nativeWindow->ShowMinimized();
			}

			void Hide() override
			{
				nativeWindow->Hide(false);
			}

			void Close() override
			{
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