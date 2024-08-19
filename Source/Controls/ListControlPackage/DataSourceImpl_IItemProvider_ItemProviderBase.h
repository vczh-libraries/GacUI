/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_DATASOURCEIMPL_IITEMPROVIDER_ITEMPROVIDERBASE
#define VCZH_PRESENTATION_CONTROLS_DATASOURCEIMPL_IITEMPROVIDER_ITEMPROVIDERBASE

#include "DataSource_IItemProvider.h"

namespace vl::presentation::controls::list
{
/***********************************************************************
ItemProviderBase
***********************************************************************/

	/// <summary>Item provider base. This class provider common functionalities for item providers.</summary>
	class ItemProviderBase : public Object, public virtual IItemProvider, public Description<ItemProviderBase>
	{
	protected:
		collections::List<IItemProviderCallback*>	callbacks;
		vint										editingCounter = 0;
		bool										callingOnItemModified = false;

		virtual void								InvokeOnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated);
	public:
		/// <summary>Create the item provider.</summary>
		ItemProviderBase();
		~ItemProviderBase();

		bool										AttachCallback(IItemProviderCallback* value)override;
		bool										DetachCallback(IItemProviderCallback* value)override;
		void										PushEditing()override;
		bool										PopEditing()override;
		bool										IsEditing()override;
	};

/***********************************************************************
ListProvider<T>
***********************************************************************/

	template<typename T>
	class ListProvider : public ItemProviderBase, public collections::ObservableListBase<T>
	{
	protected:
		void NotifyUpdateInternal(vint start, vint count, vint newCount)override
		{
			InvokeOnItemModified(start, count, newCount, true);
		}
	public:
		vint Count()override
		{
			return this->items.Count();
		}
	};
}

#endif
