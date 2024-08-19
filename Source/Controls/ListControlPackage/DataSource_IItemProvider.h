/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_DATASOURCE_IITEMPROVIDER
#define VCZH_PRESENTATION_CONTROLS_DATASOURCE_IITEMPROVIDER

#include "../../GuiTypes.h"

namespace vl::presentation::controls::list
{
	class IItemProvider;

/***********************************************************************
IItemProviderCallback
***********************************************************************/

	/// <summary>Item provider callback. Item providers use this interface to notify item modification.</summary>
	class IItemProviderCallback : public virtual IDescriptable, public Description<IItemProviderCallback>
	{
	public:
		/// <summary>Called when an item provider callback object is attached to an item provider.</summary>
		/// <param name="provider">The item provider.</param>
		virtual void								OnAttached(IItemProvider* provider)=0;
		/// <summary>Called when items in the item provider is modified.</summary>
		/// <param name="start">The index of the first modified item.</param>
		/// <param name="count">The number of all modified items.</param>
		/// <param name="newCount">The number of new items. If items are inserted or removed, newCount may not equals to count.</param>
		/// <param name="itemReferenceUpdated">True when items are replaced, false when only content in items are updated.</param>
		virtual void								OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)=0;
	};

/***********************************************************************
IItemProviderCallback
***********************************************************************/

	/// <summary>Item provider for a <see cref="GuiListControl"/>.</summary>
	class IItemProvider : public virtual IDescriptable, public Description<IItemProvider>
	{
	public:
		/// <summary>Attach an item provider callback to this item provider.</summary>
		/// <returns>Returns true if this operation succeeded.</returns>
		/// <param name="value">The item provider callback.</param>
		virtual bool								AttachCallback(IItemProviderCallback* value) = 0;
		/// <summary>Detach an item provider callback from this item provider.</summary>
		/// <returns>Returns true if this operation succeeded.</returns>
		/// <param name="value">The item provider callback.</param>
		virtual bool								DetachCallback(IItemProviderCallback* value) = 0;
		/// <summary>Increase the editing counter indicating that an [T:vl.presentation.templates.GuiListItemTemplate] is editing an item.</summary>
		virtual void								PushEditing() = 0;
		/// <summary>Decrease the editing counter indicating that an [T:vl.presentation.templates.GuiListItemTemplate] has stopped editing an item.</summary>
		/// <returns>Returns false if there is no supression before calling this function.</returns>
		virtual bool								PopEditing() = 0;
		/// <summary>Test if an [T:vl.presentation.templates.GuiListItemTemplate] is editing an item.</summary>
		/// <returns>Returns false if there is no editing.</returns>
		virtual bool								IsEditing() = 0;
		/// <summary>Get the number of items in this item proivder.</summary>
		/// <returns>The number of items in this item proivder.</returns>
		virtual vint								Count() = 0;

		/// <summary>Get the text representation of an item.</summary>
		/// <returns>The text representation of an item.</returns>
		/// <param name="itemIndex">The index of the item.</param>
		virtual WString								GetTextValue(vint itemIndex) = 0;
		/// <summary>Get the binding value of an item.</summary>
		/// <returns>The binding value of an item.</returns>
		/// <param name="itemIndex">The index of the item.</param>
		virtual description::Value					GetBindingValue(vint itemIndex) = 0;

		/// <summary>Request a view for this item provider. If the specified view is not supported, it returns null. If you want to get a view of type IXXX, use IXXX::Identifier as the identifier.</summary>
		/// <returns>The view object.</returns>
		/// <param name="identifier">The identifier for the requested view.</param>
		virtual IDescriptable*						RequestView(const WString& identifier) = 0;
	};
}

#endif
