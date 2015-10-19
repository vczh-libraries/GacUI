#include "GuiGraphicsFlowComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiFlowComposition
***********************************************************************/

			void GuiFlowComposition::UpdateFlowItemBounds()
			{
			}

			void GuiFlowComposition::OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
			}

			void GuiFlowComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildInserted(child);
				auto item = dynamic_cast<GuiFlowItemComposition*>(child);
				if (item && !flowItems.Contains(item))
				{
					flowItems.Add(item);
					UpdateFlowItemBounds();
				}
			}

			void GuiFlowComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildRemoved(child);
				auto item=dynamic_cast<GuiFlowItemComposition*>(child);
				if(item)
				{
					flowItems.Remove(item);
					UpdateFlowItemBounds();
				}
			}

			GuiFlowComposition::GuiFlowComposition()
				:axis(new GuiDefaultAxis)
			{
			}

			GuiFlowComposition::~GuiFlowComposition()
			{
			}

			Margin GuiFlowComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiFlowComposition::SetExtraMargin(Margin value)
			{
				extraMargin = value;
				UpdateFlowItemBounds();
			}

			vint GuiFlowComposition::GetRowPadding()
			{
				return rowPadding;
			}

			void GuiFlowComposition::SetRowPadding(vint value)
			{
				rowPadding = value;
				UpdateFlowItemBounds();
			}

			vint GuiFlowComposition::GetColumnPadding()
			{
				return columnPadding;
			}

			void GuiFlowComposition::SetColumnPadding(vint value)
			{
				columnPadding = value;
				UpdateFlowItemBounds();
			}

			Ptr<IGuiAxis> GuiFlowComposition::GetAxis()
			{
				return axis;
			}

			void GuiFlowComposition::SetAxis(Ptr<IGuiAxis> value)
			{
				if (value)
				{
					axis = value;
					UpdateFlowItemBounds();
				}
			}

			FlowAlignment GuiFlowComposition::GetAlignment()
			{
				return alignment;
			}

			void GuiFlowComposition::SetAlignment(FlowAlignment value)
			{
				alignment = value;
				UpdateFlowItemBounds();
			}
			
			Size GuiFlowComposition::GetMinPreferredClientSize()
			{
				Size minSize = GuiBoundsComposition::GetMinPreferredClientSize();
				if (GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren)
				{
				}

				vint x = 0;
				vint y = 0;
				if (extraMargin.left > 0) x += extraMargin.left;
				if (extraMargin.right > 0) x += extraMargin.right;
				if (extraMargin.top > 0) y += extraMargin.top;
				if (extraMargin.bottom > 0) y += extraMargin.bottom;
				return minSize + Size(x, y);
			}

			Rect GuiFlowComposition::GetBounds()
			{
				for (vint i = 0; i < flowItems.Count(); i++)
				{
					if (flowItemBounds[i].GetSize() != flowItems[i]->GetMinSize())
					{
						UpdateFlowItemBounds();
						break;
					}
				}

				Rect bounds = GuiBoundsComposition::GetBounds();
				previousBounds = bounds;
				UpdatePreviousBounds(previousBounds);
				return bounds;
			}

/***********************************************************************
GuiFlowItemComposition
***********************************************************************/

			void GuiFlowItemComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsSite::OnParentChanged(oldParent, newParent);
				flowParent = newParent == 0 ? 0 : dynamic_cast<GuiFlowComposition*>(newParent);
			}

			Size GuiFlowItemComposition::GetMinSize()
			{
				return GetBoundsInternal(bounds).GetSize();
			}

			GuiFlowItemComposition::GuiFlowItemComposition()
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiFlowItemComposition::~GuiFlowItemComposition()
			{
			}
			
			bool GuiFlowItemComposition::IsSizeAffectParent()
			{
				return false;
			}

			Rect GuiFlowItemComposition::GetBounds()
			{
				Rect result = bounds;
				if(flowParent)
				{
				}
				UpdatePreviousBounds(result);
				return result;
			}

			void GuiFlowItemComposition::SetBounds(Rect value)
			{
				bounds = value;
			}

			Margin GuiFlowItemComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiFlowItemComposition::SetExtraMargin(Margin value)
			{
				extraMargin = value;
			}
		}
	}
}