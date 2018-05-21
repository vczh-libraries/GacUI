#include "GuiGraphicsComposition.h"
#include "../Controls/Templates/GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace reflection::description;
			using namespace collections;
			using namespace controls;
			using namespace elements;

/***********************************************************************
GuiSharedSizeItemComposition
***********************************************************************/

			void GuiSharedSizeItemComposition::Update()
			{
				if (parentRoot)
				{
					parentRoot->ForceCalculateSizeImmediately();
				}
				InvokeOnCompositionStateChanged();
			}

			void GuiSharedSizeItemComposition::OnParentLineChanged()
			{
				GuiBoundsComposition::OnParentLineChanged();
				if (parentRoot)
				{
					parentRoot->childItems.Remove(this);
					parentRoot = 0;
				}

				auto current = GetParent();
				while (current)
				{
					if (auto item = dynamic_cast<GuiSharedSizeItemComposition*>(current))
					{
						break;
					}
					else if (auto root = dynamic_cast<GuiSharedSizeRootComposition*>(current))
					{
						parentRoot = root;
						break;
					}
					current = current->GetParent();
				}

				if (parentRoot)
				{
					parentRoot->childItems.Add(this);
				}
			}

			GuiSharedSizeItemComposition::GuiSharedSizeItemComposition()
				:parentRoot(0)
				, sharedWidth(false)
				, sharedHeight(false)
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiSharedSizeItemComposition::~GuiSharedSizeItemComposition()
			{
			}

			const WString& GuiSharedSizeItemComposition::GetGroup()
			{
				return group;
			}

			void GuiSharedSizeItemComposition::SetGroup(const WString& value)
			{
				if (group != value)
				{
					group = value;
					Update();
				}
			}

			bool GuiSharedSizeItemComposition::GetSharedWidth()
			{
				return sharedWidth;
			}

			void GuiSharedSizeItemComposition::SetSharedWidth(bool value)
			{
				if (sharedWidth != value)
				{
					sharedWidth = value;
					Update();
				}
			}

			bool GuiSharedSizeItemComposition::GetSharedHeight()
			{
				return sharedHeight;
			}

			void GuiSharedSizeItemComposition::SetSharedHeight(bool value)
			{
				if (sharedHeight != value)
				{
					sharedHeight = value;
					Update();
				}
			}

/***********************************************************************
GuiSharedSizeRootComposition
***********************************************************************/

			void GuiSharedSizeRootComposition::AddSizeComponent(collections::Dictionary<WString, vint>& sizes, const WString& group, vint sizeComponent)
			{
				vint index = sizes.Keys().IndexOf(group);
				if (index == -1)
				{
					sizes.Add(group, sizeComponent);
				}
				else if (sizes.Values().Get(index) < sizeComponent)
				{
					sizes.Set(group, sizeComponent);
				}
			}

			void GuiSharedSizeRootComposition::CollectSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights)
			{
				FOREACH(GuiSharedSizeItemComposition*, item, childItems)
				{
					auto group = item->GetGroup();
					auto minSize = item->GetPreferredMinSize();
					item->SetPreferredMinSize(Size(0, 0));
					auto size = item->GetPreferredBounds().GetSize();

					if (item->GetSharedWidth())
					{
						AddSizeComponent(widths, group, size.x);
					}
					if (item->GetSharedHeight())
					{
						AddSizeComponent(heights, group, size.y);
					}

					item->SetPreferredMinSize(minSize);
				}
			}

			void GuiSharedSizeRootComposition::AlignSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights)
			{
				FOREACH(GuiSharedSizeItemComposition*, item, childItems)
				{
					auto group = item->GetGroup();
					auto size = item->GetPreferredMinSize();

					if (item->GetSharedWidth())
					{
						size.x = widths[group];
					}
					if (item->GetSharedHeight())
					{
						size.y = heights[group];
					}

					item->SetPreferredMinSize(size);
				}
			}

			void GuiSharedSizeRootComposition::UpdateBounds()
			{

			}

			GuiSharedSizeRootComposition::GuiSharedSizeRootComposition()
			{
			}

			GuiSharedSizeRootComposition::~GuiSharedSizeRootComposition()
			{
			}

			void GuiSharedSizeRootComposition::ForceCalculateSizeImmediately()
			{
				itemWidths.Clear();
				itemHeights.Clear();

				CollectSizes(itemWidths, itemHeights);
				AlignSizes(itemWidths, itemHeights);
				GuiBoundsComposition::ForceCalculateSizeImmediately();
			}

			Rect GuiSharedSizeRootComposition::GetBounds()
			{
				Dictionary<WString, vint> widths, heights;
				CollectSizes(widths, heights);
				bool minSizeModified = CompareEnumerable(itemWidths, widths) != 0 || CompareEnumerable(itemHeights, heights) != 0;

				if (minSizeModified)
				{
					CopyFrom(itemWidths, widths);
					CopyFrom(itemHeights, heights);
					AlignSizes(itemWidths, itemHeights);
					GuiBoundsComposition::ForceCalculateSizeImmediately();
				}
				return GuiBoundsComposition::GetBounds();
			}

/***********************************************************************
GuiRepeatCompositionBase
***********************************************************************/

			void GuiRepeatCompositionBase::OnItemChanged(vint index, vint oldCount, vint newCount)
			{
				if (itemTemplate && itemSource)
				{
					for (vint i = oldCount - 1; i >= 0; i--)
					{
						RemoveItem(index + i);
					}

					for (vint i = 0; i < newCount; i++)
					{
						InstallItem(index + i);
					}
				}
			}

			void GuiRepeatCompositionBase::RemoveItem(vint index)
			{
				GuiItemEventArgs arguments(dynamic_cast<GuiGraphicsComposition*>(this));
				arguments.itemIndex = index;
				ItemRemoved.Execute(arguments);

				auto item = RemoveRepeatComposition(index);
				SafeDeleteComposition(item);
			}

			void GuiRepeatCompositionBase::InstallItem(vint index)
			{
				auto source = itemSource->Get(index);
				auto templateItem = itemTemplate(source);
				auto item = InsertRepeatComposition(index);

				templateItem->SetAlignmentToParent(Margin(0, 0, 0, 0));
				item->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				item->AddChild(templateItem);

				GuiItemEventArgs arguments(dynamic_cast<GuiGraphicsComposition*>(this));
				arguments.itemIndex = index;
				ItemInserted.Execute(arguments);
			}

			void GuiRepeatCompositionBase::ClearItems()
			{
				for (vint i = GetRepeatCompositionCount() - 1; i >= 0; i--)
				{
					RemoveItem(i);
				}
			}

			void GuiRepeatCompositionBase::InstallItems()
			{
				if (itemTemplate && itemSource)
				{
					vint count = itemSource->GetCount();
					for (vint i = 0; i < count; i++)
					{
						InstallItem(i);
					}
				}
			}

			GuiRepeatCompositionBase::GuiRepeatCompositionBase()
			{
			}

			GuiRepeatCompositionBase::~GuiRepeatCompositionBase()
			{
			}

			GuiRepeatCompositionBase::ItemStyleProperty GuiRepeatCompositionBase::GetItemTemplate()
			{
				return itemTemplate;
			}

			void GuiRepeatCompositionBase::SetItemTemplate(ItemStyleProperty value)
			{
				ClearItems();
				itemTemplate = value;
				if (itemTemplate && itemSource)
				{
					InstallItems();
				}
			}

			Ptr<IValueEnumerable> GuiRepeatCompositionBase::GetItemSource()
			{
				return itemSource;
			}

			void GuiRepeatCompositionBase::SetItemSource(Ptr<IValueEnumerable> value)
			{
				if (value != itemSource)
				{
					if (itemChangedHandler)
					{
						itemSource.Cast<IValueObservableList>()->ItemChanged.Remove(itemChangedHandler);
					}

					ClearItems();
					itemSource = value.Cast<IValueList>();
					if (!itemSource && value)
					{
						itemSource = IValueList::Create(GetLazyList<Value>(value));
					}

					if (itemTemplate && itemSource)
					{
						InstallItems();
					}
					if (auto observable = itemSource.Cast<IValueObservableList>())
					{
						itemChangedHandler = observable->ItemChanged.Add(this, &GuiRepeatCompositionBase::OnItemChanged);
					}
				}
			}

/***********************************************************************
GuiRepeatStackComposition
***********************************************************************/

			vint GuiRepeatStackComposition::GetRepeatCompositionCount()
			{
				return stackItems.Count();
			}

			GuiGraphicsComposition* GuiRepeatStackComposition::GetRepeatComposition(vint index)
			{
				return stackItems[index];
			}

			GuiGraphicsComposition* GuiRepeatStackComposition::InsertRepeatComposition(vint index)
			{
				CHECK_ERROR(0 <= index && index <= stackItems.Count(), L"GuiRepeatStackComposition::InsertRepeatComposition(vint)#Index out of range.");
				auto item = new GuiStackItemComposition;
				InsertStackItem(index, item);
				return item;
			}

			GuiGraphicsComposition* GuiRepeatStackComposition::RemoveRepeatComposition(vint index)
			{
				auto item = stackItems[index];
				RemoveChild(item);
				return item;
			}

/***********************************************************************
GuiRepeatFlowComposition
***********************************************************************/

			vint GuiRepeatFlowComposition::GetRepeatCompositionCount()
			{
				return flowItems.Count();
			}

			GuiGraphicsComposition* GuiRepeatFlowComposition::GetRepeatComposition(vint index)
			{
				return flowItems[index];
			}

			GuiGraphicsComposition* GuiRepeatFlowComposition::InsertRepeatComposition(vint index)
			{
				CHECK_ERROR(0 <= index && index <= flowItems.Count(), L"GuiRepeatStackComposition::InsertRepeatComposition(vint)#Index out of range.");
				auto item = new GuiFlowItemComposition;
				InsertFlowItem(index, item);
				return item;
			}

			GuiGraphicsComposition* GuiRepeatFlowComposition::RemoveRepeatComposition(vint index)
			{
				auto item = flowItems[index];
				RemoveChild(item);
				return item;
			}
		}
	}
}