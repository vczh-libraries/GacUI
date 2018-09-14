#include "GuiGraphicsHost.h"
#include "../Controls/GuiWindowControls.h"
#include "../Controls/GuiLabelControls.h"
#include "../Controls/Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace controls;
			using namespace theme;

			const wchar_t* const IGuiAltAction::Identifier = L"vl::presentation::compositions::IGuiAltAction";
			const wchar_t* const IGuiAltActionContainer::Identifier = L"vl::presentation::compositions::IGuiAltAction";
			const wchar_t* const IGuiAltActionHost::Identifier = L"vl::presentation::compositions::IGuiAltAction";

/***********************************************************************
IGuiAltAction
***********************************************************************/

			bool IGuiAltAction::IsLegalAlt(const WString& alt)
			{
				for (vint i = 0; i < alt.Length(); i++)
				{
					auto c = alt[i];
					if (('A' <= c && c <= 'Z') || ('0' <= c && c <= '9'))
					{
						continue;
					}
					return false;
				}
				return true;
			}

/***********************************************************************
IGuiAltActionHost
***********************************************************************/

			void IGuiAltActionHost::CollectAltActionsFromControl(controls::GuiControl* control, bool includeThisControl, collections::Group<WString, IGuiAltAction*>& actions)
			{
				List<GuiControl*> controls;
				controls.Add(control);
				vint index = 0;

				while (index < controls.Count())
				{
					auto current = controls[index++];

					if (current != control || includeThisControl)
					{
						if (auto container = current->QueryTypedService<IGuiAltActionContainer>())
						{
							vint count = container->GetAltActionCount();
							for (vint i = 0; i < count; i++)
							{
								auto action = container->GetAltAction(i);
								actions.Add(action->GetAlt(), action);
							}
							continue;
						}
						else if (auto action = current->QueryTypedService<IGuiAltAction>())
						{
							if (action->IsAltAvailable())
							{
								if (action->IsAltEnabled())
								{
									actions.Add(action->GetAlt(), action);
									continue;
								}
							}
						}
					}

					vint count = current->GetChildrenCount();
					for (vint i = 0; i < count; i++)
					{
						controls.Add(current->GetChild(i));
					}
				}
			}

/***********************************************************************
GuiAltActionHostBase
***********************************************************************/

			void GuiAltActionHostBase::SetAltComposition(GuiGraphicsComposition* _composition)
			{
				composition = _composition;
			}

			void GuiAltActionHostBase::SetAltControl(controls::GuiControl* _control, bool _includeControl)
			{
				control = _control;
				includeControl = _includeControl;
			}

			GuiGraphicsComposition* GuiAltActionHostBase::GetAltComposition()
			{
				CHECK_ERROR(composition, L"GuiAltActionHostBase::GetAltComposition()#Need to call SetAltComposition.");
				return composition;
			}

			IGuiAltActionHost* GuiAltActionHostBase::GetPreviousAltHost()
			{
				return previousHost;
			}

			void GuiAltActionHostBase::OnActivatedAltHost(IGuiAltActionHost* _previousHost)
			{
				previousHost = _previousHost;
			}

			void GuiAltActionHostBase::OnDeactivatedAltHost()
			{
				previousHost = nullptr;
			}

			void GuiAltActionHostBase::CollectAltActions(collections::Group<WString, IGuiAltAction*>& actions)
			{
				CHECK_ERROR(control, L"GuiAltActionHostBase::CollectAltActions(Group<WString, IGuiAltAction*>&)#Need to call SetAltControl.");
				CollectAltActionsFromControl(control, includeControl, actions);
			}

/***********************************************************************
GuiAltActionManager
***********************************************************************/

			void GuiAltActionManager::EnterAltHost(IGuiAltActionHost* host)
			{
				ClearAltHost();

				Group<WString, IGuiAltAction*> actions;
				host->CollectAltActions(actions);
				if (actions.Count() == 0)
				{
					CloseAltHost();
					return;
				}

				host->OnActivatedAltHost(currentAltHost);
				currentAltHost = host;
				CreateAltTitles(actions);
			}

			void GuiAltActionManager::LeaveAltHost()
			{
				if (currentAltHost)
				{
					ClearAltHost();
					auto previousHost = currentAltHost->GetPreviousAltHost();
					currentAltHost->OnDeactivatedAltHost();
					currentAltHost = previousHost;

					if (currentAltHost)
					{
						Group<WString, IGuiAltAction*> actions;
						currentAltHost->CollectAltActions(actions);
						CreateAltTitles(actions);
					}
				}
			}

			bool GuiAltActionManager::EnterAltKey(wchar_t key)
			{
				currentAltPrefix += key;
				vint index = currentActiveAltActions.Keys().IndexOf(currentAltPrefix);
				if (index == -1)
				{
					if (FilterTitles() == 0)
					{
						currentAltPrefix = currentAltPrefix.Left(currentAltPrefix.Length() - 1);
						FilterTitles();
					}
				}
				else
				{
					auto action = currentActiveAltActions.Values()[index];
					if (action->GetActivatingAltHost())
					{
						EnterAltHost(action->GetActivatingAltHost());
					}
					else
					{
						CloseAltHost();
					}
					action->OnActiveAlt();
					return true;
				}
				return false;
			}

			void GuiAltActionManager::LeaveAltKey()
			{
				if (currentAltPrefix.Length() >= 1)
				{
					currentAltPrefix = currentAltPrefix.Left(currentAltPrefix.Length() - 1);
				}
				FilterTitles();
			}

			void GuiAltActionManager::CreateAltTitles(const collections::Group<WString, IGuiAltAction*>& actions)
			{
				if (currentAltHost)
				{
					vint count = actions.Count();
					for (vint i = 0; i < count; i++)
					{
						WString key = actions.Keys()[i];
						const auto& values = actions.GetByIndex(i);
						vint numberLength = 0;
						if (values.Count() == 1 && key.Length() > 0)
						{
							numberLength = 0;
						}
						else if (values.Count() <= 10)
						{
							numberLength = 1;
						}
						else if (values.Count() <= 100)
						{
							numberLength = 2;
						}
						else if (values.Count() <= 1000)
						{
							numberLength = 3;
						}
						else
						{
							continue;
						}

						FOREACH_INDEXER(IGuiAltAction*, action, index, values)
						{
							WString key = actions.Keys()[i];
							if (numberLength > 0)
							{
								WString number = itow(index);
								while (number.Length() < numberLength)
								{
									number = L"0" + number;
								}
								key += number;
							}
							currentActiveAltActions.Add(key, action);
						}
					}

					count = currentActiveAltActions.Count();
					auto window = dynamic_cast<GuiWindow*>(currentAltHost->GetAltComposition()->GetRelatedControlHost());
					for (vint i = 0; i < count; i++)
					{
						auto key = currentActiveAltActions.Keys()[i];
						auto composition = currentActiveAltActions.Values()[i]->GetAltComposition();

						auto label = new GuiLabel(theme::ThemeName::ShortcutKey);
						if (auto labelStyle = window->GetControlTemplateObject(true)->GetShortcutKeyTemplate())
						{
							label->SetControlTemplate(labelStyle);
						}
						label->SetText(key);
						composition->AddChild(label->GetBoundsComposition());
						currentActiveAltTitles.Add(key, label);
					}

					FilterTitles();
				}
			}

			vint GuiAltActionManager::FilterTitles()
			{
				vint count = currentActiveAltTitles.Count();
				vint visibles = 0;
				for (vint i = 0; i < count; i++)
				{
					auto key = currentActiveAltTitles.Keys()[i];
					auto value = currentActiveAltTitles.Values()[i];
					if (key.Length() >= currentAltPrefix.Length() && key.Left(currentAltPrefix.Length()) == currentAltPrefix)
					{
						value->SetVisible(true);
						if (currentAltPrefix.Length() <= key.Length())
						{
							value->SetText(
								key
								.Insert(currentAltPrefix.Length(), L"[")
								.Insert(currentAltPrefix.Length() + 2, L"]")
								);
						}
						else
						{
							value->SetText(key);
						}
						visibles++;
					}
					else
					{
						value->SetVisible(false);
					}
				}
				return visibles;
			}

			void GuiAltActionManager::ClearAltHost()
			{
				FOREACH(GuiControl*, title, currentActiveAltTitles.Values())
				{
					SafeDeleteControl(title);
				}
				currentActiveAltActions.Clear();
				currentActiveAltTitles.Clear();
				currentAltPrefix = L"";
			}

			void GuiAltActionManager::CloseAltHost()
			{
				ClearAltHost();
				while (currentAltHost)
				{
					currentAltHost->OnDeactivatedAltHost();
					currentAltHost = currentAltHost->GetPreviousAltHost();
				}
			}

			GuiAltActionManager::GuiAltActionManager(controls::GuiControlHost* _controlHost)
				:controlHost(_controlHost)
			{
			}

			GuiAltActionManager::~GuiAltActionManager()
			{
			}

			bool GuiAltActionManager::KeyDown(const NativeWindowKeyInfo& info)
			{
				if (!info.ctrl && !info.shift && currentAltHost)
				{
					if (info.code == VKEY::_ESCAPE)
					{
						LeaveAltHost();
						return true;
					}
					else if (info.code == VKEY::_BACK)
					{
						LeaveAltKey();
					}
					else if (VKEY::_NUMPAD0 <= info.code && info.code <= VKEY::_NUMPAD9)
					{
						if (EnterAltKey((wchar_t)(L'0' + ((vint)info.code - (vint)VKEY::_NUMPAD0))))
						{
							supressAltKey = info.code;
							return true;
						}
					}
					else if ((VKEY::_0 <= info.code && info.code <= VKEY::_9) || (VKEY::_A <= info.code && info.code <= VKEY::_Z))
					{
						if (EnterAltKey((wchar_t)info.code))
						{
							supressAltKey = info.code;
							return true;
						}
					}
				}

				if (currentAltHost)
				{
					return true;
				}
				return false;
			}

			bool GuiAltActionManager::KeyUp(const NativeWindowKeyInfo& info)
			{
				if (!info.ctrl && !info.shift && info.code == supressAltKey)
				{
					supressAltKey = VKEY::_UNKNOWN;
					return true;
				}
				return false;
			}

			bool GuiAltActionManager::SysKeyDown(const NativeWindowKeyInfo& info)
			{
				if (!info.ctrl && !info.shift && info.code == VKEY::_MENU && !currentAltHost)
				{
					if (auto altHost = controlHost->QueryTypedService<IGuiAltActionHost>())
					{
						if (!altHost->GetPreviousAltHost())
						{
							EnterAltHost(altHost);
						}
					}
				}

				if (currentAltHost)
				{
					return true;
				}
				return false;
			}

			bool GuiAltActionManager::SysKeyUp(const NativeWindowKeyInfo& info)
			{
				return false;
			}

			bool GuiAltActionManager::Char(const NativeWindowCharInfo& info)
			{
				if (currentAltHost || supressAltKey != VKEY::_UNKNOWN)
				{
					return true;
				}
				return false;
			}
		}
	}
}