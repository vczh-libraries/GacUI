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

			void EnsureNoSystemBorder()
			{
				if (!data->windowCustomFrameMode)
				{
					CHECK_ERROR(
						!data->windowBorder && !data->windowSizeBox && !data->windowTitleBar,
						L"vl::presentation::GuiNonMainHostedWindowProxy::EnsureNoSystemBorder()#"
						L"For non main window in hosted mode, when custom frame mode is disabled"
						L"the following window features should also be disabled: "
						L"Border, SizeBox, TitleBar.");
				}
			}

			void CheckAndSyncProperties() override
			{
				EnsureNoSystemBorder();
			}

			/***********************************************************************
			Visible Properties
			***********************************************************************/

			NativeRect FixBounds(const NativeRect& bounds) override
			{
				return bounds;
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
				EnsureNoSystemBorder();
			}

			void UpdateMinimizedBox() override
			{
				EnsureNoSystemBorder();
			}

			void UpdateBorderVisible() override
			{
				EnsureNoSystemBorder();
			}

			void UpdateSizeBox() override
			{
				EnsureNoSystemBorder();
			}

			void UpdateIconVisible() override
			{
				EnsureNoSystemBorder();
			}

			void UpdateTitleBar() override
			{
				EnsureNoSystemBorder();
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
				EnsureNoSystemBorder();
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
				data->wmWindow.SetVisible(false);
			}

			void Close() override
			{
				data->wmWindow.SetVisible(false);
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