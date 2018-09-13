#include "GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiShortcutKeyItem
***********************************************************************/

			GuiShortcutKeyItem::GuiShortcutKeyItem(GuiShortcutKeyManager* _shortcutKeyManager, bool _ctrl, bool _shift, bool _alt, VKEY _key)
				:shortcutKeyManager(_shortcutKeyManager)
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
				if(ctrl) name+=L"Ctrl+";
				if(shift) name+=L"Shift+";
				if(alt) name+=L"Alt+";
				name+=GetCurrentController()->InputService()->GetKeyName(key);
				return name;
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

/***********************************************************************
GuiShortcutKeyManager
***********************************************************************/

			GuiShortcutKeyManager::GuiShortcutKeyManager()
			{
			}

			GuiShortcutKeyManager::~GuiShortcutKeyManager()
			{
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
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(info))
					{
						GuiEventArgs arguments;
						item->Executed.Execute(arguments);
						executed=true;
					}
				}
				return executed;
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::CreateShortcut(bool ctrl, bool shift, bool alt, VKEY key)
			{
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(ctrl, shift, alt, key))
					{
						return item.Obj();
					}
				}
				Ptr<GuiShortcutKeyItem> item=new GuiShortcutKeyItem(this, ctrl, shift, alt, key);
				shortcutKeyItems.Add(item);
				return item.Obj();
			}

			bool GuiShortcutKeyManager::DestroyShortcut(bool ctrl, bool shift, bool alt, VKEY key)
			{
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(ctrl, shift, alt, key))
					{
						shortcutKeyItems.Remove(item.Obj());
						return true;
					}
				}
				return false;
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::TryGetShortcut(bool ctrl, bool shift, bool alt, VKEY key)
			{
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(ctrl, shift, alt, key))
					{
						return item.Obj();
					}
				}
				return 0;
			}
		}
	}
}