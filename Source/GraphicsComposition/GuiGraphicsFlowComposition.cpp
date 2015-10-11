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
				throw 0;
			}

			void GuiFlowComposition::OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				UpdateFlowItemBounds();
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

			Alignment GuiFlowComposition::GetAlignment()
			{
				return alignment;
			}

			void GuiFlowComposition::SetAlignment(Alignment value)
			{
				alignment = value;
				UpdateFlowItemBounds();
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

			Rect GuiFlowItemComposition::GetBounds()
			{
				Rect result = bounds;
				if(flowParent)
				{
					throw 0;
				}
				UpdatePreviousBounds(result);
				return result;
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