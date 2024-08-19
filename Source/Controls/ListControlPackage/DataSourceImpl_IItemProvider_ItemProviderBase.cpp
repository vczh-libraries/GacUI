#include "DataSourceImpl_IItemProvider_ItemProviderBase.h"

namespace vl::presentation::controls::list
{
/***********************************************************************
ItemProviderBase
***********************************************************************/

	void ItemProviderBase::InvokeOnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
	{
		CHECK_ERROR(!callingOnItemModified, L"ItemProviderBase::InvokeOnItemModified(vint, vint, vint)#Canning modify the observable data source during its item modified event, which will cause this event to be executed recursively.");
		callingOnItemModified = true;
		// TODO: (enumerable) foreach
		for (vint i = 0; i < callbacks.Count(); i++)
		{
			callbacks[i]->OnItemModified(start, count, newCount, itemReferenceUpdated);
		}
		callingOnItemModified = false;
	}

	ItemProviderBase::ItemProviderBase()
	{
	}

	ItemProviderBase::~ItemProviderBase()
	{
		// TODO: (enumerable) foreach
		for(vint i=0;i<callbacks.Count();i++)
		{
			callbacks[i]->OnAttached(0);
		}
	}

	bool ItemProviderBase::AttachCallback(IItemProviderCallback* value)
	{
		if(callbacks.Contains(value))
		{
			return false;
		}
		else
		{
			callbacks.Add(value);
			value->OnAttached(this);
			return true;
		}
	}

	bool ItemProviderBase::DetachCallback(IItemProviderCallback* value)
	{
		vint index=callbacks.IndexOf(value);
		if(index==-1)
		{
			return false;
		}
		else
		{
			value->OnAttached(0);
			callbacks.Remove(value);
			return true;
		}
	}

	void ItemProviderBase::PushEditing()
	{
		editingCounter++;
	}

	bool ItemProviderBase::PopEditing()
	{
		if (editingCounter == 0)return false;
		editingCounter--;
		return true;
	}

	bool ItemProviderBase::IsEditing()
	{
		return editingCounter > 0;
	}
}