/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Graphics Composition Host

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_SHORTCUTKEY
#define VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_SHORTCUTKEY

#include "../GraphicsCompositions/GuiGraphicsEventReceiver.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
Shortcut Key Manager
***********************************************************************/

			class IGuiShortcutKeyManager;

			/// <summary>Shortcut key item.</summary>
			class IGuiShortcutKeyItem : public virtual IDescriptable, public Description<IGuiShortcutKeyItem>
			{
			public:
				/// <summary>Shortcut key executed event.</summary>
				GuiNotifyEvent							Executed;

				/// <summary>Get the associated <see cref="IGuiShortcutKeyManager"/> object.</summary>
				/// <returns>The associated shortcut key manager.</returns>
				virtual IGuiShortcutKeyManager*			GetManager()=0;
				/// <summary>Get the name represents the shortcut key combination for this item.</summary>
				/// <returns>The name represents the shortcut key combination for this item.</returns>
				virtual WString							GetName()=0;
			};
			
			/// <summary>Shortcut key manager item.</summary>
			class IGuiShortcutKeyManager : public virtual IDescriptable, public Description<IGuiShortcutKeyManager>
			{
			public:
				/// <summary>Get the number of shortcut key items that already attached to the manager.</summary>
				/// <returns>T number of shortcut key items that already attached to the manager.</returns>
				virtual vint							GetItemCount()=0;
				/// <summary>Get the <see cref="IGuiShortcutKeyItem"/> associated with the index.</summary>
				/// <returns>The shortcut key item.</returns>
				/// <param name="index">The index.</param>
				virtual IGuiShortcutKeyItem*			GetItem(vint index)=0;
				/// <summary>Execute shortcut key items using a key event info.</summary>
				/// <returns>Returns true if at least one shortcut key item is executed.</returns>
				/// <param name="info">The key event info.</param>
				virtual bool							Execute(const NativeWindowKeyInfo& info)=0;
				
				/// <summary>Get a shortcut key item using a key combination. If the item for the key combination does not exist, this function returns null.</summary>
				/// <returns>The shortcut key item.</returns>
				/// <param name="ctrl">Set to true if the CTRL key is required.</param>
				/// <param name="shift">Set to true if the SHIFT key is required.</param>
				/// <param name="alt">Set to true if the ALT key is required.</param>
				/// <param name="key">The non-control key.</param>
				virtual IGuiShortcutKeyItem*			TryGetShortcut(bool ctrl, bool shift, bool alt, VKEY key)=0;
				/// <summary>Create a shortcut key item using a key combination. If the item for the key combination exists, this function crashes.</summary>
				/// <returns>The created shortcut key item.</returns>
				/// <param name="ctrl">Set to true if the CTRL key is required.</param>
				/// <param name="shift">Set to true if the SHIFT key is required.</param>
				/// <param name="alt">Set to true if the ALT key is required.</param>
				/// <param name="key">The non-control key.</param>
				virtual IGuiShortcutKeyItem*			CreateNewShortcut(bool ctrl, bool shift, bool alt, VKEY key)=0;
				/// <summary>Create a shortcut key item using a key combination. If the item for the key combination exists, this function returns the item that is created before.</summary>
				/// <returns>The created shortcut key item.</returns>
				/// <param name="ctrl">Set to true if the CTRL key is required.</param>
				/// <param name="shift">Set to true if the SHIFT key is required.</param>
				/// <param name="alt">Set to true if the ALT key is required.</param>
				/// <param name="key">The non-control key.</param>
				virtual IGuiShortcutKeyItem*			CreateShortcutIfNotExist(bool ctrl, bool shift, bool alt, VKEY key)=0;
				/// <summary>Destroy a shortcut key item using a key combination</summary>
				/// <returns>Returns true if the manager destroyed a existing shortcut key item.</returns>
				/// <param name="item">The shortcut key item.</param>
				virtual bool							DestroyShortcut(IGuiShortcutKeyItem* item)=0;
			};

/***********************************************************************
Shortcut Key Manager Helpers
***********************************************************************/

			class GuiShortcutKeyManager;

			class GuiShortcutKeyItem : public Object, public IGuiShortcutKeyItem
			{
			protected:
				GuiShortcutKeyManager*			shortcutKeyManager;
				bool							global;
				bool							ctrl;
				bool							shift;
				bool							alt;
				VKEY							key;

			public:
				GuiShortcutKeyItem(GuiShortcutKeyManager* _shortcutKeyManager, bool _global, bool _ctrl, bool _shift, bool _alt, VKEY _key);
				~GuiShortcutKeyItem();

				IGuiShortcutKeyManager*			GetManager()override;
				WString							GetName()override;

				void							ReadKeyConfig(bool& _ctrl, bool& _shift, bool& _alt, VKEY& _key);
				bool							CanActivate(const NativeWindowKeyInfo& info);
				bool							CanActivate(bool _ctrl, bool _shift, bool _alt, VKEY _key);
				void							Execute();
			};

			/// <summary>A default implementation for <see cref="IGuiShortcutKeyManager"/>.</summary>
			class GuiShortcutKeyManager : public Object, public IGuiShortcutKeyManager, public Description<GuiShortcutKeyManager>
			{
				typedef collections::List<Ptr<GuiShortcutKeyItem>>		ShortcutKeyItemList;
			protected:
				ShortcutKeyItemList				shortcutKeyItems;
				
				virtual bool					IsGlobal();
				virtual bool					OnCreatingShortcut(GuiShortcutKeyItem* item);
				virtual void					OnDestroyingShortcut(GuiShortcutKeyItem* item);
				IGuiShortcutKeyItem*			CreateShortcutInternal(bool ctrl, bool shift, bool alt, VKEY key);
			public:
				/// <summary>Create the shortcut key manager.</summary>
				GuiShortcutKeyManager();
				~GuiShortcutKeyManager();

				vint							GetItemCount()override;
				IGuiShortcutKeyItem*			GetItem(vint index)override;
				bool							Execute(const NativeWindowKeyInfo& info)override;
				
				IGuiShortcutKeyItem*			TryGetShortcut(bool ctrl, bool shift, bool alt, VKEY key)override;
				IGuiShortcutKeyItem*			CreateNewShortcut(bool ctrl, bool shift, bool alt, VKEY key)override;
				IGuiShortcutKeyItem*			CreateShortcutIfNotExist(bool ctrl, bool shift, bool alt, VKEY key)override;
				bool							DestroyShortcut(IGuiShortcutKeyItem* item)override;
			};
		}
	}
}

#endif