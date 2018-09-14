#include "GuiGraphicsHost.h"
#include "../Controls/GuiWindowControls.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace controls;

			const wchar_t* const IGuiTabAction::Identifier = L"vl::presentation::compositions::IGuiTabAction";

/***********************************************************************
GuiTabActionManager
***********************************************************************/

			void GuiTabActionManager::BuildControlList()
			{

			}

			controls::GuiControl* GuiTabActionManager::GetNextFocusControl(controls::GuiControl* focusedControl)
			{
				if (!available)
				{
					BuildControlList();
					available = true;
				}

				if (controlsInOrder.Count() == 0) return nullptr;
				vint startIndex = controlsInOrder.IndexOf(focusedControl);
				startIndex =
					startIndex == -1 ? 0 :
					startIndex == controlsInOrder.Count() - 1 ? 0 :
					startIndex + 1;

				vint index = 0;
				do
				{
					auto control = controlsInOrder[index];
					if (auto tabAction = control->QueryTypedService<IGuiTabAction>())
					{
						if (tabAction->IsTabAvailable() && tabAction->IsTabEnabled())
						{
							return control;
						}
					}

					index = (index + 1) % controlsInOrder.Count();
				} while (index != startIndex);

				return nullptr;
			}

			GuiTabActionManager::GuiTabActionManager(controls::GuiControlHost* _controlHost)
				:controlHost(_controlHost)
			{
			}

			GuiTabActionManager::~GuiTabActionManager()
			{
			}

			void GuiTabActionManager::InvalidateTabOrderCache()
			{
				available = false;
				controlsInOrder.Clear();
			}

			bool GuiTabActionManager::Execute(const NativeWindowKeyInfo& info, GuiGraphicsComposition* focusedComposition)
			{
				if (info.code == VKEY::_TAB)
				{
					GuiControl* focusedControl = nullptr;
					if (focusedComposition)
					{
						focusedControl = focusedComposition->GetRelatedControl();
						if (focusedControl && focusedControl->GetAcceptTabInput())
						{
							return false;
						}
					}

					if (auto next = GetNextFocusControl(focusedControl))
					{
						next->SetFocus();
						return true;
					}
				}
				return false;
			}
		}
	}
}