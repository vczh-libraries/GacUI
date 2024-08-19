#include "GuiListControlItemArrangers.h"
#include "../Templates/GuiControlTemplates.h"
#include "../Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;

			namespace list
			{

/***********************************************************************
RangedItemArrangerBase (ItemSource)
***********************************************************************/

				class ArrangerItemSource : public Object, public virtual description::IValueObservableList
				{
				protected:
					list::IItemProvider*				itemProvider = nullptr;

				public:
					ArrangerItemSource(list::IItemProvider* _itemProvider)
						: itemProvider(_itemProvider)
					{
					}

					vint GetCount() override
					{
						return itemProvider->Count();
					}

					description::Value Get(vint index) override
					{
						return itemProvider->GetBindingValue(index);
					}

					Ptr<description::IValueEnumerator>	CreateEnumerator()										override { CHECK_FAIL(L"ArrangerItemSource::CreateEnumerator should not be called."); }
					bool								Contains(const description::Value& value)				override { CHECK_FAIL(L"ArrangerItemSource::Contains should not be called."); }
					vint								IndexOf(const description::Value& value)				override { CHECK_FAIL(L"ArrangerItemSource::IndexOf should not be called."); }

					void								Set(vint index, const description::Value& value)		override { CHECK_FAIL(L"ArrangerItemSource::Set should not be called."); }
					vint								Add(const description::Value& value)					override { CHECK_FAIL(L"ArrangerItemSource::Add should not be called."); }
					vint								Insert(vint index, const description::Value& value)		override { CHECK_FAIL(L"ArrangerItemSource::Insert should not be called."); }
					bool								Remove(const description::Value& value)					override { CHECK_FAIL(L"ArrangerItemSource::Remove should not be called."); }
					bool								RemoveAt(vint index)									override { CHECK_FAIL(L"ArrangerItemSource::RemoveAt should not be called."); }
					void								Clear()													override { CHECK_FAIL(L"ArrangerItemSource::Clear should not be called."); }
				};

/***********************************************************************
RangedItemArrangerBase
***********************************************************************/

				void RangedItemArrangerBase::OnViewLocationChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments)
				{
					if (callback)
					{
						callback->SetViewLocation(repeat->GetViewLocation());
					}
				}

				void RangedItemArrangerBase::OnTotalSizeChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments)
				{
					if (callback)
					{
						callback->OnTotalSizeChanged();
					}
				}

				void RangedItemArrangerBase::OnAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments)
				{
					if (callback)
					{
						callback->OnAdoptedSizeChanged();
					}
				}

				RangedItemArrangerBase::RangedItemArrangerBase(compositions::GuiVirtualRepeatCompositionBase* _repeat)
					: repeat(_repeat)
				{
					repeat->ViewLocationChanged.AttachMethod(this, &RangedItemArrangerBase::OnViewLocationChanged);
					repeat->TotalSizeChanged.AttachMethod(this, &RangedItemArrangerBase::OnTotalSizeChanged);
					repeat->AdoptedSizeInvalidated.AttachMethod(this, &RangedItemArrangerBase::OnAdoptedSizeInvalidated);
				}

				RangedItemArrangerBase::~RangedItemArrangerBase()
				{
					SafeDeleteComposition(repeat);
				}

				void RangedItemArrangerBase::OnAttached(list::IItemProvider* provider)
				{
					itemProvider = provider;
					if (provider)
					{
						itemSource = Ptr(new ArrangerItemSource(provider));
						repeat->SetItemSource(itemSource);
					}
					else
					{
						repeat->SetItemSource(nullptr);
						itemSource = nullptr;
					}
				}

				void RangedItemArrangerBase::OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
				{
					if (itemSource && itemReferenceUpdated)
					{
						itemSource->ItemChanged(start, count, newCount);
					}
				}

				void RangedItemArrangerBase::AttachListControl(GuiListControl* value)
				{
					listControl = value;
					repeat->SetAxis(Ptr(listControl->GetAxis()));
				}

				void RangedItemArrangerBase::DetachListControl()
				{
					repeat->SetAxis(nullptr);
					listControl = nullptr;
				}

				GuiListControl::IItemArrangerCallback* RangedItemArrangerBase::GetCallback()
				{
					return callback;
				}

				void RangedItemArrangerBase::SetCallback(GuiListControl::IItemArrangerCallback* value)
				{
					if (callback != value)
					{
						if (callback)
						{
							repeat->GetParent()->RemoveChild(repeat);
							repeat->SetItemTemplate({});
						}
						callback = value;
						if (callback)
						{
							callback->GetContainerComposition()->AddChild(repeat);
							repeat->SetItemTemplate([](const description::Value&)->templates::GuiTemplate*
							{
								CHECK_FAIL(L"This function should not be called, it is used to enable the virtual repeat composition.");
							});
						}
					}
				}

				Size RangedItemArrangerBase::GetTotalSize()
				{
					if (callback && repeat)
					{
						return repeat->GetTotalSize();
					}
					return Size(0, 0);
				}

				GuiListControl::ItemStyle* RangedItemArrangerBase::GetVisibleStyle(vint itemIndex)
				{
					auto bounds = repeat->GetVisibleStyle(itemIndex);
					return bounds ? callback->GetItem(bounds) : nullptr;
				}

				vint RangedItemArrangerBase::GetVisibleIndex(GuiListControl::ItemStyle* style)
				{
					auto bounds = callback->GetItemBounds(style);
					return repeat->GetVisibleIndex(bounds);
				}

				void RangedItemArrangerBase::ReloadVisibleStyles()
				{
					if (repeat) repeat->ResetLayout(true);
				}

				void RangedItemArrangerBase::OnViewChanged(Rect bounds)
				{
					repeat->SetViewLocation(bounds.LeftTop());
					repeat->SetExpectedBounds(Rect({ 0,0 }, bounds.GetSize()));
				}

				vint RangedItemArrangerBase::FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)
				{
					return repeat->FindItemByVirtualKeyDirection(itemIndex, key);
				}

				GuiListControl::EnsureItemVisibleResult RangedItemArrangerBase::EnsureItemVisible(vint itemIndex)
				{
					switch (repeat->EnsureItemVisible(itemIndex))
					{
					case VirtualRepeatEnsureItemVisibleResult::Moved:
						return GuiListControl::EnsureItemVisibleResult::Moved;
					case VirtualRepeatEnsureItemVisibleResult::NotMoved:
						return GuiListControl::EnsureItemVisibleResult::NotMoved;
					default:
						return GuiListControl::EnsureItemVisibleResult::ItemNotExists;
					}
				}

				Size RangedItemArrangerBase::GetAdoptedSize(Size expectedSize)
				{
					return repeat->GetAdoptedSize(expectedSize);
				}

/***********************************************************************
VirtualRepeatRangedItemArrangerBase
***********************************************************************/

				FreeHeightItemArranger::FreeHeightItemArranger()
				{
				}

				FreeHeightItemArranger::~FreeHeightItemArranger()
				{
				}

				FixedHeightItemArranger::FixedHeightItemArranger()
				{
				}

				FixedHeightItemArranger::~FixedHeightItemArranger()
				{
				}

				FixedSizeMultiColumnItemArranger::FixedSizeMultiColumnItemArranger()
				{
				}

				FixedSizeMultiColumnItemArranger::~FixedSizeMultiColumnItemArranger()
				{
				}

				FixedHeightMultiColumnItemArranger::FixedHeightMultiColumnItemArranger()
				{
				}

				FixedHeightMultiColumnItemArranger::~FixedHeightMultiColumnItemArranger()
				{
				}
			}
		}
	}
}