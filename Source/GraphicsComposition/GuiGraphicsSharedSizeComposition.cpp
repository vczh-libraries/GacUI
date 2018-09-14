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
					Size minSize;
					if (sharedWidth)
					{
						vint index = parentRoot->itemWidths.Keys().IndexOf(group);
						if (index != -1)
						{
							minSize.x = parentRoot->itemWidths.Values()[index];
						}
					}
					if (sharedHeight)
					{
						vint index = parentRoot->itemHeights.Keys().IndexOf(group);
						if (index != -1)
						{
							minSize.y = parentRoot->itemHeights.Values()[index];
						}
					}
					SetPreferredMinSize(minSize);
				}
			}

			GuiSharedSizeItemComposition::GuiSharedSizeItemComposition()
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
		}
	}
}