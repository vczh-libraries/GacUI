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
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateBounds() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateTitle() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateIcon() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateEnabled() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateTopMost() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}


			/***********************************************************************
			Border Properties
			***********************************************************************/

			void UpdateMaximizedBox() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateMinimizedBox() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateBorderVisible() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateSizeBox() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateIconVisible() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateTitleBar() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}


			/***********************************************************************
			Behavior Properties
			***********************************************************************/

			void UpdateShowInTaskBar() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void UpdateEnabledActivate() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void EnableCustomFrameMode() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void DisableCustomFrameMode() override
			{
				CHECK_FAIL(L"Not Implemented!");
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
				CHECK_FAIL(L"Not Implemented!");
			}

			void Close() override
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			void SetFocus() override
			{
				CHECK_FAIL(L"Not Implemented!");
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