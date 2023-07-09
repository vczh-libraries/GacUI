#include "GuiGraphicsSharedSizeComposition.h"

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

			void GuiSharedSizeItemComposition::OnParentLineChanged()
			{
				GuiBoundsComposition::OnParentLineChanged();
				if (parentRoot)
				{
					parentRoot->childItems.Remove(this);
					parentRoot = nullptr;
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

			Size GuiSharedSizeItemComposition::Layout_CalculateMinSize()
			{
				if (parentRoot)
				{
					return cachedMinSize;
				}
				else
				{
					return GuiBoundsComposition::Layout_CalculateMinSize();
				}
			}

			Size GuiSharedSizeItemComposition::Layout_CalculateOriginalMinSize()
			{
				return GuiBoundsComposition::Layout_CalculateMinSize();
			}

			GuiSharedSizeItemComposition::GuiSharedSizeItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
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
					InvokeOnCompositionStateChanged();
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
					InvokeOnCompositionStateChanged();
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
					InvokeOnCompositionStateChanged();
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

			void GuiSharedSizeRootComposition::CalculateOriginalMinSizes()
			{
				for (auto item : childItems)
				{
					item->originalMinSize = item->Layout_CalculateOriginalMinSize();
				}
			}

			void GuiSharedSizeRootComposition::CollectSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights)
			{
				for (auto item : childItems)
				{
					auto group = item->GetGroup();
					if (item->GetSharedWidth())
					{
						AddSizeComponent(widths, group, item->originalMinSize.x);
					}
					if (item->GetSharedHeight())
					{
						AddSizeComponent(heights, group, item->originalMinSize.y);
					}
				}
			}

			void GuiSharedSizeRootComposition::AlignSizes(collections::Dictionary<WString, vint>& widths, collections::Dictionary<WString, vint>& heights)
			{
				for (auto item : childItems)
				{
					auto group = item->GetGroup();
					auto size = item->originalMinSize;

					if (item->GetSharedWidth())
					{
						size.x = widths[group];
					}
					if (item->GetSharedHeight())
					{
						size.y = heights[group];
					}

					item->Layout_SetCachedMinSize(size);
				}
			}

			Size GuiSharedSizeRootComposition::Layout_CalculateMinSize()
			{
				itemWidths.Clear();
				itemHeights.Clear();
				CalculateOriginalMinSizes();
				CollectSizes(itemWidths, itemHeights);
				AlignSizes(itemWidths, itemHeights);
				return GuiBoundsComposition::Layout_CalculateMinSize();
			}
		}
	}
}