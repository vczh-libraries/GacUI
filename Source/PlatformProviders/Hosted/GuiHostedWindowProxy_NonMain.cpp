#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiNonMainHostedWindowProxy
***********************************************************************/

		class GuiNonMainHostedWindowProxy
			: public Object
			, public virtual IGuiHostedWindowProxy
		{
		protected:
			GuiHostedWindowData*			data = nullptr;

		public:

			GuiNonMainHostedWindowProxy(GuiHostedWindowData* _data)
				: data(_data)
			{
			}

			void EnsureNoSystemBorderWhenVisible()
			{
				if (!data->wmWindow.visible) return;
				if (data->windowCustomFrameMode) return;
				CHECK_ERROR(
					!data->windowBorder && !data->windowSizeBox && !data->windowTitleBar,
					L"vl::presentation::GuiNonMainHostedWindowProxy::EnsureNoSystemBorder()#"
					L"For non main window in hosted mode, when custom frame mode is disabled"
					L"the following window features should also be disabled: "
					L"Border, SizeBox, TitleBar.");
			}

			void CheckAndSyncProperties() override
			{
				if (data->windowMaximizedBox || data->windowMinimizedBox)
				{
					data->windowMaximizedBox = false;
					data->windowMinimizedBox = false;
					for (auto listener : data->listeners)
					{
						listener->BecomeNonMainHostedWindow();
					}
				}
				EnsureNoSystemBorderWhenVisible();
			}

			/***********************************************************************
			Visible Properties
			***********************************************************************/

			NativeRect FixBounds(const NativeRect& bounds) override
			{
				auto w = bounds.Width().value;
				auto h = bounds.Height().value;
				if (w < 1) w = 1;
				if (h < 1) h = 1;
				return { bounds.LeftTop(),{{w},{h}} };
			}

			void UpdateBounds() override
			{
			}

			void UpdateTitle() override
			{
			}

			void UpdateIcon() override
			{
			}

			void UpdateEnabled() override
			{
			}

			void UpdateTopMost() override
			{
			}

			/***********************************************************************
			Border Properties
			***********************************************************************/

			void UpdateMaximizedBox() override
			{
				if (data->windowMaximizedBox)
				{
					data->windowMaximizedBox = false;
					for (auto listener : data->listeners)
					{
						listener->BecomeNonMainHostedWindow();
					}
				}
			}

			void UpdateMinimizedBox() override
			{
				if (data->windowMinimizedBox)
				{
					data->windowMinimizedBox = false;
					for (auto listener : data->listeners)
					{
						listener->BecomeNonMainHostedWindow();
					}
				}
			}

			void UpdateBorderVisible() override
			{
				EnsureNoSystemBorderWhenVisible();
			}

			void UpdateSizeBox() override
			{
				EnsureNoSystemBorderWhenVisible();
			}

			void UpdateIconVisible() override
			{
			}

			void UpdateTitleBar() override
			{
				EnsureNoSystemBorderWhenVisible();
			}

			/***********************************************************************
			Behavior Properties
			***********************************************************************/

			void UpdateShowInTaskBar() override
			{
			}

			void UpdateEnabledActivate() override
			{
			}

			void UpdateCustomFrameMode() override
			{
				EnsureNoSystemBorderWhenVisible();
			}

			/***********************************************************************
			Show/Hide/Focus

			Maximized and Minimized are not available
			***********************************************************************/

			void Show() override
			{
				data->wmWindow.SetVisible(true);
				data->wmWindow.Activate();
				EnsureNoSystemBorderWhenVisible();
			}

			void ShowDeactivated() override
			{
				data->wmWindow.SetVisible(true);
				EnsureNoSystemBorderWhenVisible();
			}

			void ShowRestored() override
			{
				Show();
			}

			void ShowMaximized() override
			{
				Show();
			}

			void ShowMinimized() override
			{
				Show();
			}

			void Hide() override
			{
				data->wmWindow.SetVisible(false);
			}

			void Close() override
			{
				Hide();
			}

			void SetFocus() override
			{
				data->wmWindow.Activate();
			}
		};

/***********************************************************************
Helper
***********************************************************************/

		Ptr<IGuiHostedWindowProxy> CreateNonMainHostedWindowProxy(GuiHostedWindowData* data)
		{
			return Ptr(new GuiNonMainHostedWindowProxy(data));
		}
	}
}