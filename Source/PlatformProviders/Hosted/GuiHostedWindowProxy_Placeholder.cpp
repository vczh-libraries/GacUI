#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
GuiPlaceholderHostedWindowProxy
***********************************************************************/

		class GuiPlaceholderHostedWindowProxy
			: public Object
			, public virtual IGuiHostedWindowProxy
		{
		protected:
			GuiHostedWindowData*			data = nullptr;

		public:

			GuiPlaceholderHostedWindowProxy(GuiHostedWindowData* _data)
				: data(_data)
			{
			}

			void CheckAndSyncProperties() override
			{
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
			}

			void UpdateMinimizedBox() override
			{
			}

			void UpdateBorderVisible() override
			{
			}

			void UpdateSizeBox() override
			{
			}

			void UpdateIconVisible() override
			{
			}

			void UpdateTitleBar() override
			{
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
			}

			/***********************************************************************
			Show/Hide/Focus
			***********************************************************************/

			void Show() override
			{
				CHECK_FAIL(L"vl::presentation::GuiPlaceholderHostedWindowProxy::Show()#This function should not be called.");
			}

			void ShowDeactivated() override
			{
				CHECK_FAIL(L"vl::presentation::GuiPlaceholderHostedWindowProxy::ShowDeactivated()#This function should not be called.");
			}

			void ShowRestored() override
			{
				CHECK_FAIL(L"vl::presentation::GuiPlaceholderHostedWindowProxy::ShowRestored()#This function should not be called.");
			}

			void ShowMaximized() override
			{
				CHECK_FAIL(L"vl::presentation::GuiPlaceholderHostedWindowProxy::ShowMaximized()#This function should not be called.");
			}

			void ShowMinimized() override
			{
				CHECK_FAIL(L"vl::presentation::GuiPlaceholderHostedWindowProxy::ShowMinimized()#This function should not be called.");
			}

			void Hide() override
			{
			}

			void Close() override
			{
			}

			void SetFocus() override
			{
			}
		};

/***********************************************************************
Helper
***********************************************************************/

		Ptr<IGuiHostedWindowProxy> CreatePlaceholderHostedWindowProxy(GuiHostedWindowData* data)
		{
			return Ptr(new GuiPlaceholderHostedWindowProxy(data));
		}
	}
}