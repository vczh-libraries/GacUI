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

			controls::GuiControl* GuiTabActionManager::GetNextFocusControl(controls::GuiControl* focusedControl)
			{
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