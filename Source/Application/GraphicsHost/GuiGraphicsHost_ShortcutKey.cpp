#include "GuiGraphicsHost_ShortcutKey.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiShortcutKeyItem
***********************************************************************/

			GuiShortcutKeyItem::GuiShortcutKeyItem(GuiShortcutKeyManager* _shortcutKeyManager, bool _global, bool _ctrl, bool _shift, bool _alt, VKEY _key)
				:shortcutKeyManager(_shortcutKeyManager)
				,global(_global)
				,ctrl(_ctrl)
				,shift(_shift)
				,alt(_alt)
				,key(_key)
			{
			}

			GuiShortcutKeyItem::~GuiShortcutKeyItem()
			{
			}

			IGuiShortcutKeyManager* GuiShortcutKeyItem::GetManager()
			{
				return shortcutKeyManager;
			}

			WString GuiShortcutKeyItem::GetName()
			{
				WString name;
				if (global) name += L"{";
				if (ctrl) name += L"Ctrl+";
				if (shift) name += L"Shift+";
				if (alt) name += L"Alt+";
				name += GetCurrentController()->InputService()->GetKeyName(key);
				if (global) name += L"}";
				return name;
			}

			void GuiShortcutKeyItem::ReadKeyConfig(bool& _ctrl, bool& _shift, bool& _alt, VKEY& _key)
			{
				_ctrl = ctrl;
				_shift = shift;
				_alt = alt;
				_key = key;
			}

			bool GuiShortcutKeyItem::CanActivate(const NativeWindowKeyInfo& info)
			{
				return
					info.ctrl==ctrl &&
					info.shift==shift &&
					info.alt==alt &&
					info.code==key;
			}

			bool GuiShortcutKeyItem::CanActivate(bool _ctrl, bool _shift, bool _alt, VKEY _key)
			{
				return
					_ctrl==ctrl &&
					_shift==shift &&
					_alt==alt &&
					_key==key;
			}

			void GuiShortcutKeyItem::Execute()
			{
				GuiEventArgs arguments;
				Executed.Execute(arguments);
			}

/***********************************************************************
GuiShortcutKeyManager
***********************************************************************/

			bool GuiShortcutKeyManager::IsGlobal()
			{
				return false;
			}

			bool GuiShortcutKeyManager::OnCreatingShortcut(GuiShortcutKeyItem* item)
			{
				return true;
			}

			void GuiShortcutKeyManager::OnDestroyingShortcut(GuiShortcutKeyItem* item)
			{
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::CreateShortcutInternal(bool ctrl, bool shift, bool alt, VKEY key)
			{
				auto item = Ptr(new GuiShortcutKeyItem(this, IsGlobal(), ctrl, shift, alt, key));
				if (!OnCreatingShortcut(item.Obj())) return nullptr;
				shortcutKeyItems.Add(item);
				return item.Obj();
			}

			GuiShortcutKeyManager::GuiShortcutKeyManager()
			{
			}

			GuiShortcutKeyManager::~GuiShortcutKeyManager()
			{
				for (auto item : shortcutKeyItems)
				{
					OnDestroyingShortcut(item.Obj());
				}
			}

			vint GuiShortcutKeyManager::GetItemCount()
			{
				return shortcutKeyItems.Count();
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::GetItem(vint index)
			{
				return shortcutKeyItems[index].Obj();
			}

			bool GuiShortcutKeyManager::Execute(const NativeWindowKeyInfo& info)
			{
				bool executed=false;
				for (auto item : shortcutKeyItems)
				{
					if(item->CanActivate(info))
					{
						item->Execute();
						executed=true;
					}
				}
				return executed;
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::TryGetShortcut(bool ctrl, bool shift, bool alt, VKEY key)
			{
				for (auto item : shortcutKeyItems)
				{
					if (item->CanActivate(ctrl, shift, alt, key))
					{
						return item.Obj();
					}
				}
				return nullptr;
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::CreateNewShortcut(bool ctrl, bool shift, bool alt, VKEY key)
			{
				CHECK_ERROR(
					TryGetShortcut(ctrl, shift, alt, key) == nullptr,
					L"vl::presentation::compositions::GuiShortcutKeyManager::CreateNewShortcut(bool, bool, bool, VKEY)#The shortcut key exists."
					);
				return CreateShortcutInternal(ctrl, shift, alt, key);
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::CreateShortcutIfNotExist(bool ctrl, bool shift, bool alt, VKEY key)
			{
				if (TryGetShortcut(ctrl, shift, alt, key))
				{
					return nullptr;
				}
				return CreateShortcutInternal(ctrl, shift, alt, key);
			}

			bool GuiShortcutKeyManager::DestroyShortcut(IGuiShortcutKeyItem* item)
			{
				if (!item) return false;
				if (item->GetManager() != this) return false;

				auto skItem = dynamic_cast<GuiShortcutKeyItem*>(item);
				if (!skItem) return false;

				vint index = shortcutKeyItems.IndexOf(skItem);
				if (index == -1) return false;
				OnDestroyingShortcut(skItem);
				return shortcutKeyItems.RemoveAt(index);
			}
		}
	}
}