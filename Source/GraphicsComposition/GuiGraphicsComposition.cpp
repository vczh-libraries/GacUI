#include "GuiGraphicsComposition.h"
#include "../Controls/Templates/GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
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

			GuiSharedSizeRootComposition::GuiSharedSizeRootComposition()
			{
			}

			GuiSharedSizeRootComposition::~GuiSharedSizeRootComposition()
			{
			}

			void AddSizeComponent(Dictionary<WString, vint>& sizes, const WString& group, vint sizeComponent)
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

			void GuiSharedSizeRootComposition::ForceCalculateSizeImmediately()
			{
				Dictionary<WString, vint> widths, heights;

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

				GuiBoundsComposition::ForceCalculateSizeImmediately();
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
				auto item = RemoveRepeatComposition(index);
				SafeDeleteComposition(item);
			}

			void GuiRepeatCompositionBase::InstallItem(vint index)
			{
				auto source = itemSource->Get(index);
				auto templateItem = itemTemplate(source);
				auto item = InsertRepeatComposition(index);

				templateItem->SetMargin(Margin(0, 0, 0, 0));
				item->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				item->AddChild(templateItem);
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

			GuiRepeatCompositionBase::ItemSourceType GuiRepeatCompositionBase::GetItemSource()
			{
				return itemSource;
			}

			void GuiRepeatCompositionBase::SetItemSource(ItemSourceType value)
			{
				if (itemSource != value)
				{
					if (itemSource)
					{
						itemSource->ItemChanged.Remove(itemChangedHandler);
					}
					ClearItems();
					itemSource = value;
					if (itemTemplate && itemSource)
					{
						InstallItems();
					}
					if (itemSource)
					{
						itemChangedHandler = itemSource->ItemChanged.Add(this, &GuiRepeatCompositionBase::OnItemChanged);
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