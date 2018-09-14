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

			namespace tab_focus
			{
				void CollectControls(GuiControl* current, bool includeCurrent, Group<vuint64_t, GuiControl*>& prioritized)
				{
					if (includeCurrent)
					{
						auto tabAction = current->QueryTypedService<IGuiTabAction>();
						if (tabAction && (tabAction->IsTabAvailable() || tabAction->GetTabPriority() != -1))
						{
							vint priority = tabAction->GetTabPriority();
							vuint64_t normalized = priority < 0 ? ~(vuint64_t)0 : (vuint64_t)priority;
							prioritized.Add(normalized, current);
							return;
						}
					}

					vint count = current->GetChildrenCount();
					for (vint i = 0; i < count; i++)
					{
						CollectControls(current->GetChild(i), true, prioritized);
					}
				}

				void InsertPrioritized(List<GuiControl*>& controls, vint index, Group<vuint64_t, GuiControl*>& prioritized)
				{
					vint count = prioritized.Count();
					for (vint i = 0; i < count; i++)
					{
						auto& values = prioritized.GetByIndex(i);
						for (vint j = 0; j < values.Count(); j++)
						{
							controls.Insert(index++, values[j]);
						}
					}
				}
			}
			using namespace tab_focus;

			void GuiTabActionManager::BuildControlList()
			{
				controlsInOrder.Clear();
				{
					Group<vuint64_t, GuiControl*> prioritized;
					CollectControls(controlHost, false, prioritized);
					InsertPrioritized(controlsInOrder, 0, prioritized);
				}

				for (vint i = 0; i < controlsInOrder.Count(); i++)
				{
					Group<vuint64_t, GuiControl*> prioritized;
					CollectControls(controlsInOrder[i], false, prioritized);
					InsertPrioritized(controlsInOrder, i + 1, prioritized);
				}
			}

			controls::GuiControl* GuiTabActionManager::GetNextFocusControl(controls::GuiControl* focusedControl, vint offset)
			{
				if (!available)
				{
					BuildControlList();
					available = true;
				}
#define STEP_AND_NORMALIZE(INDEX) (((INDEX) + offset + controlsInOrder.Count()) % controlsInOrder.Count())

				if (controlsInOrder.Count() == 0) return nullptr;
				vint startIndex = controlsInOrder.IndexOf(focusedControl);
				startIndex =
					startIndex == -1 ? 0 :
					STEP_AND_NORMALIZE(startIndex);

				vint index = startIndex;
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

					index = STEP_AND_NORMALIZE(index);
				} while (index != startIndex);

#undef STEP_AND_NORMALIZE

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

			bool GuiTabActionManager::KeyDown(const NativeWindowKeyInfo& info, GuiGraphicsComposition* focusedComposition)
			{
				if (!info.ctrl && !info.alt && info.code == VKEY::_TAB)
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

					if (auto next = GetNextFocusControl(focusedControl, (info.shift ? -1 : 1)))
					{
						next->SetFocus();
						supressTabOnce = true;
						return true;
					}
				}
				return false;
			}

			bool GuiTabActionManager::Char(const NativeWindowCharInfo& info)
			{
				bool supress = supressTabOnce;
				supressTabOnce = false;
				return supress && info.code == L'\t';
			}
		}
	}
}