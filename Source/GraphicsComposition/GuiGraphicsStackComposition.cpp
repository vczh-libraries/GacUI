#include "GuiGraphicsStackComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiStackComposition
***********************************************************************/

			void GuiStackComposition::UpdateStackItemBounds()
			{
				if(stackItemBounds.Count()!=stackItems.Count())
				{
					stackItemBounds.Resize(stackItems.Count());
				}

				stackItemTotalSize=Size(0, 0);
				vint x=extraMargin.left?extraMargin.left:0;
				vint y=extraMargin.top?extraMargin.top:0;
				switch(direction)
				{
				case GuiStackComposition::Horizontal:
					{
						for(vint i=0;i<stackItems.Count();i++)
						{
							Size itemSize=stackItems[i]->GetMinSize();
							if(i>0) stackItemTotalSize.x+=padding;
							if(stackItemTotalSize.y<itemSize.y) stackItemTotalSize.y=itemSize.y;
							stackItemBounds[i]=Rect(Point(stackItemTotalSize.x+x, y), Size(itemSize.x, 0));
							stackItemTotalSize.x+=itemSize.x;
						}
					}
					break;
				case GuiStackComposition::Vertical:
					{
						for(vint i=0;i<stackItems.Count();i++)
						{
							Size itemSize=stackItems[i]->GetMinSize();
							if(i>0) stackItemTotalSize.y+=padding;
							if(stackItemTotalSize.x<itemSize.x) stackItemTotalSize.x=itemSize.x;
							stackItemBounds[i]=Rect(Point(x, stackItemTotalSize.y+y), Size(0, itemSize.y));
							stackItemTotalSize.y+=itemSize.y;
						}
					}
					break;
				}

				FixStackItemSizes();
			}

			void GuiStackComposition::FixStackItemSizes()
			{
				switch(direction)
				{
				case Horizontal:
					{
						vint y=0;
						if(extraMargin.top>0) y+=extraMargin.top;
						if(extraMargin.bottom>0) y+=extraMargin.bottom;

						for(vint i=0;i<stackItemBounds.Count();i++)
						{
							stackItemBounds[i].y2=stackItemBounds[i].y1+previousBounds.Height()-y;
						}
					}
					break;
				case Vertical:
					{
						vint x=0;
						if(extraMargin.left>0) x+=extraMargin.left;
						if(extraMargin.right>0) x+=extraMargin.right;

						for(vint i=0;i<stackItemBounds.Count();i++)
						{
							stackItemBounds[i].x2=stackItemBounds[i].x1+previousBounds.Width()-x;
						}
					}
					break;
				}

				vint index=stackItems.IndexOf(ensuringVisibleStackItem);
				if(index!=-1)
				{
					Rect itemBounds=stackItemBounds[index];
					Size size=previousBounds.GetSize();
					Size offset;
					switch(direction)
					{
					case Horizontal:
						{
							if(itemBounds.Left()<=0)
							{
								offset.x=-itemBounds.Left();
							}
							else if(itemBounds.Right()>=size.x)
							{
								offset.x=size.x-itemBounds.Right();
							}
						}
						break;
					case Vertical:
						{
							if(itemBounds.Top()<=0)
							{
								offset.y=-itemBounds.Top();
							}
							else if(itemBounds.Bottom()>=size.y)
							{
								offset.y=size.y-itemBounds.Bottom();
							}
						}
						break;
					}
					for(vint i=0;i<stackItemBounds.Count();i++)
					{
						stackItemBounds[i].x1+=offset.x;
						stackItemBounds[i].y1+=offset.y;
						stackItemBounds[i].x2+=offset.x;
						stackItemBounds[i].y2+=offset.y;
					}
				}
			}

			void GuiStackComposition::OnBoundsChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
			{
				UpdateStackItemBounds();
			}

			void GuiStackComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildInserted(child);
				GuiStackItemComposition* item=dynamic_cast<GuiStackItemComposition*>(child);
				if(item && !stackItems.Contains(item))
				{
					stackItems.Add(item);
				}
			}

			void GuiStackComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				GuiBoundsComposition::OnChildRemoved(child);
				GuiStackItemComposition* item=dynamic_cast<GuiStackItemComposition*>(child);
				if(item)
				{
					stackItems.Remove(item);
					if(item==ensuringVisibleStackItem)
					{
						ensuringVisibleStackItem=0;
						UpdateStackItemBounds();
					}
				}
			}

			GuiStackComposition::GuiStackComposition()
				:direction(Horizontal)
				,padding(0)
				,ensuringVisibleStackItem(0)
			{
				BoundsChanged.AttachMethod(this, &GuiStackComposition::OnBoundsChanged);
			}

			GuiStackComposition::~GuiStackComposition()
			{
			}

			const GuiStackComposition::ItemCompositionList& GuiStackComposition::GetStackItems()
			{
				return stackItems;
			}

			bool GuiStackComposition::InsertStackItem(vint index, GuiStackItemComposition* item)
			{
				index=stackItems.Insert(index, item);
				if(!AddChild(item))
				{
					stackItems.RemoveAt(index);
					return false;
				}
				else
				{
					return true;
				}
			}

			GuiStackComposition::Direction GuiStackComposition::GetDirection()
			{
				return direction;
			}

			void GuiStackComposition::SetDirection(Direction value)
			{
				direction=value;
			}

			vint GuiStackComposition::GetPadding()
			{
				return padding;
			}

			void GuiStackComposition::SetPadding(vint value)
			{
				padding=value;
			}
			
			Size GuiStackComposition::GetMinPreferredClientSize()
			{
				Size minSize=GuiBoundsComposition::GetMinPreferredClientSize();
				UpdateStackItemBounds();
				if(GetMinSizeLimitation()==GuiGraphicsComposition::LimitToElementAndChildren)
				{
					if (!ensuringVisibleStackItem || direction == Vertical)
					{
						if(minSize.x<stackItemTotalSize.x) minSize.x=stackItemTotalSize.x;
					}
					if (!ensuringVisibleStackItem || direction == Horizontal)
					{
						if(minSize.y<stackItemTotalSize.y) minSize.y=stackItemTotalSize.y;
					}
				}
				vint x=0;
				vint y=0;
				if(extraMargin.left>0) x+=extraMargin.left;
				if(extraMargin.right>0) x+=extraMargin.right;
				if(extraMargin.top>0) y+=extraMargin.top;
				if(extraMargin.bottom>0) y+=extraMargin.bottom;
				return minSize+Size(x, y);
			}

			Rect GuiStackComposition::GetBounds()
			{
				Rect bounds=GuiBoundsComposition::GetBounds();
				previousBounds=bounds;
				FixStackItemSizes();
				return bounds;
			}

			Margin GuiStackComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiStackComposition::SetExtraMargin(Margin value)
			{
				extraMargin=value;
			}

			bool GuiStackComposition::IsStackItemClipped()
			{
				Rect clientArea=GetClientArea();
				for(vint i=0;i<stackItems.Count();i++)
				{
					Rect stackItemBounds=stackItems[i]->GetBounds();
					switch(direction)
					{
					case Horizontal:
						{
							if(stackItemBounds.Left()<0 || stackItemBounds.Right()>=clientArea.Width())
							{
								return true;
							}
						}
						break;
					case Vertical:
						{
							if(stackItemBounds.Top()<0 || stackItemBounds.Bottom()>=clientArea.Height())
							{
								return true;
							}
						}
						break;
					}
				}
				return false;
			}

			bool GuiStackComposition::EnsureVisible(vint index)
			{
				if(0<=index && index<stackItems.Count())
				{
					ensuringVisibleStackItem = stackItems[index];
				}
				else
				{
					ensuringVisibleStackItem = 0;
				}
				UpdateStackItemBounds();
				return ensuringVisibleStackItem != 0;
			}

/***********************************************************************
GuiStackItemComposition
***********************************************************************/

			void GuiStackItemComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				GuiGraphicsSite::OnParentChanged(oldParent, newParent);
				stackParent=newParent==0?0:dynamic_cast<GuiStackComposition*>(newParent);
			}

			Size GuiStackItemComposition::GetMinSize()
			{
				return GetBoundsInternal(bounds).GetSize();
			}

			GuiStackItemComposition::GuiStackItemComposition()
				:stackParent(0)
			{
				SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiStackItemComposition::~GuiStackItemComposition()
			{
			}

			bool GuiStackItemComposition::IsSizeAffectParent()
			{
				return false;
			}

			Rect GuiStackItemComposition::GetBounds()
			{
				Rect result=bounds;
				if(stackParent)
				{
					vint index=stackParent->stackItems.IndexOf(this);
					if(index!=-1)
					{
						if(stackParent->stackItemBounds.Count()!=stackParent->stackItems.Count())
						{
							stackParent->UpdateStackItemBounds();
						}
						result=stackParent->stackItemBounds[index];
					}
				}
				result.x1-=extraMargin.left;
				result.y1-=extraMargin.top;
				result.x2+=extraMargin.right;
				result.y2+=extraMargin.bottom;
				UpdatePreviousBounds(result);
				return result;
			}

			void GuiStackItemComposition::SetBounds(Rect value)
			{
				bounds=value;
			}

			Margin GuiStackItemComposition::GetExtraMargin()
			{
				return extraMargin;
			}

			void GuiStackItemComposition::SetExtraMargin(Margin value)
			{
				extraMargin=value;
			}
		}
	}
}
