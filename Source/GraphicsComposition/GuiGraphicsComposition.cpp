#include "GuiGraphicsComposition.h"

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
		}
	}
}