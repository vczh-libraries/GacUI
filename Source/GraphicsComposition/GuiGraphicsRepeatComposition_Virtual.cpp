#include "GuiGraphicsRepeatComposition.h"
#include "../Controls/Templates/GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;

/***********************************************************************
GuiVirtualRepeatCompositionBase
***********************************************************************/

			void GuiVirtualRepeatCompositionBase::Layout_UpdateIndex(ItemStyleRecord style, vint index)
			{
			}

			void GuiVirtualRepeatCompositionBase::Layout_UpdateViewBounds(Rect value)
			{
				auto old = GetViewLocation();
				Rect oldBounds = viewBounds;
				viewBounds = value;
				OnViewChangedInternal(oldBounds, value);
				RearrangeItemBounds();
				if (old != GetViewLocation())
				{
					ViewLocationChanged.Execute(GuiEventArgs(this));
				}
			}

			Rect GuiVirtualRepeatCompositionBase::Layout_CalculateBounds(Size parentSize)
			{
				auto bounds = GuiBoundsComposition::Layout_CalculateBounds(parentSize);
				auto size = axis->RealSizeToVirtualSize(bounds.GetSize());
				if (size != viewBounds.GetSize())
				{
					Layout_UpdateViewBounds(Rect(viewBounds.LeftTop(), size));
				}
				return bounds;
			}

			void GuiVirtualRepeatCompositionBase::OnItemChanged(vint start, vint oldCount, vint newCount)
			{
				vint visibleCount = visibleStyles.Count();
				vint itemCount = itemSource->GetCount();
				SortedList<ItemStyleRecord> reusedStyles;
				for (vint i = 0; i < visibleCount; i++)
				{
					vint index = startIndex + i;
					if (index >= itemCount)
					{
						break;
					}

					vint oldIndex = -1;
					if (index < start)
					{
						oldIndex = index;
					}
					else if (index >= start + newCount)
					{
						oldIndex = index - newCount + oldCount;
					}

					if (oldIndex != -1)
					{
						if (oldIndex >= startIndex && oldIndex < startIndex + visibleCount)
						{
							auto style = visibleStyles[oldIndex - startIndex];
							reusedStyles.Add(style);
							visibleStyles.Add(style);
						}
						else
						{
							oldIndex = -1;
						}
					}
					if (oldIndex == -1)
					{
						visibleStyles.Add(CreateStyle(index));
					}
				}

				for (vint i = 0; i < visibleCount; i++)
				{
					auto style = visibleStyles[i];
					if (!reusedStyles.Contains(style))
					{
						DeleteStyle(style);
					}
				}

				visibleStyles.RemoveRange(0, visibleCount);
				for (auto [style, i] : indexed(visibleStyles))
				{
					Layout_UpdateIndex(style, startIndex + 1);
				}

				realFullSize = axis->VirtualSizeToRealSize(Layout_CalculateTotalSize());
				TotalSizeChanged.Execute(GuiEventArgs(this));
				AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
			}

			void GuiVirtualRepeatCompositionBase::ClearItems()
			{
				startIndex = 0;
				for (auto style : visibleStyles)
				{
					DeleteStyle(style);
				}
				visibleStyles.Clear();
				viewBounds = Rect(0, 0, 0, 0);
				Layout_InvalidateItemSizeCache();
				AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
			}

			void GuiVirtualRepeatCompositionBase::InstallItems()
			{
				OnViewChangedInternal(viewBounds, viewBounds);
				RearrangeItemBounds();
			}

			void GuiVirtualRepeatCompositionBase::UpdateContext()
			{
				for (auto style : visibleStyles)
				{
					style->SetContext(itemContext);
				}
			}

			vint GuiVirtualRepeatCompositionBase::CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize)
			{
				vint visibleCount = expectedSize / itemSize;
				if (count < visibleCount)
				{
					visibleCount = count;
				}
				else if (count > visibleCount)
				{
					vint deltaA = expectedSize - count * itemSize;
					vint deltaB = itemSize - deltaA;
					if (deltaB < deltaA)
					{
						visibleCount++;
					}
				}
				return visibleCount * itemSize;
			}

			GuiVirtualRepeatCompositionBase::ItemStyleRecord GuiVirtualRepeatCompositionBase::CreateStyle(vint index)
			{
				auto source = itemSource->Get(index);
				auto itemStyle = itemTemplate(source);
				AddChild(itemStyle);
				return itemStyle;
			}

			void GuiVirtualRepeatCompositionBase::DeleteStyle(ItemStyleRecord style)
			{
				SafeDeleteComposition(style);
			}

			void GuiVirtualRepeatCompositionBase::OnViewChangedInternal(Rect oldBounds, Rect newBounds)
			{
				if (itemTemplate && itemSource)
				{
					vint endIndex = startIndex + visibleStyles.Count() - 1;
					vint newStartIndex = 0;
					vint itemCount = itemSource->GetCount();
					Layout_BeginPlaceItem(true, newBounds, newStartIndex);
					if (newStartIndex < 0) newStartIndex = 0;

					StyleList newVisibleStyles;
					for (vint i = newStartIndex; i < itemCount; i++)
					{
						bool reuseOldStyle = startIndex <= i && i <= endIndex;
						auto style = reuseOldStyle ? visibleStyles[i - startIndex] : CreateStyle(i);
						newVisibleStyles.Add(style);

						Rect bounds;
						Margin alignmentToParent;
						Layout_PlaceItem(true, !reuseOldStyle, i, style, newBounds, bounds, alignmentToParent);
						if (Layout_IsItemOutOfViewBounds(i, style, bounds, newBounds))
						{
							break;
						}

						bounds.x1 -= newBounds.x1;
						bounds.x2 -= newBounds.x1;
						bounds.y1 -= newBounds.y1;
						bounds.y2 -= newBounds.y1;
					}

					vint newEndIndex = newStartIndex + newVisibleStyles.Count() - 1;
					for (auto [style, i] : indexed(visibleStyles))
					{
						vint index = startIndex + i;
						if (index < newStartIndex || index > newEndIndex)
						{
							DeleteStyle(visibleStyles[i]);
						}
					}
					CopyFrom(visibleStyles, newVisibleStyles);

					if (Layout_EndPlaceItem(true, newBounds, newStartIndex))
					{
						realFullSize = axis->VirtualSizeToRealSize(Layout_CalculateTotalSize());
						TotalSizeChanged.Execute(GuiEventArgs(this));
						AdoptedSizeInvalidated.Execute(GuiEventArgs(this));
					}
					startIndex = newStartIndex;
				}
			}

			void GuiVirtualRepeatCompositionBase::RearrangeItemBounds()
			{
				vint newStartIndex = startIndex;
				Layout_BeginPlaceItem(false, viewBounds, newStartIndex);
				for (auto [style, i] : indexed(visibleStyles))
				{
					Rect bounds;
					Margin alignmentToParent(-1, -1, -1, -1);
					Layout_PlaceItem(false, false, startIndex + i, style, viewBounds, bounds, alignmentToParent);

					bounds.x1 -= viewBounds.x1;
					bounds.x2 -= viewBounds.x1;
					bounds.y1 -= viewBounds.y1;
					bounds.y2 -= viewBounds.y1;

					style->SetAlignmentToParent(axis->VirtualMarginToRealMargin(alignmentToParent));
					style->SetExpectedBounds(axis->VirtualRectToRealRect(axis->VirtualSizeToRealSize(viewBounds.GetSize()), bounds));
				}
				Layout_EndPlaceItem(false, viewBounds, startIndex);
			}

			GuiVirtualRepeatCompositionBase::GuiVirtualRepeatCompositionBase()
			{
				AxisChanged.SetAssociatedComposition(this);
				TotalSizeChanged.SetAssociatedComposition(this);
				ViewLocationChanged.SetAssociatedComposition(this);
				AdoptedSizeInvalidated.SetAssociatedComposition(this);
			}

			GuiVirtualRepeatCompositionBase::~GuiVirtualRepeatCompositionBase()
			{
			}

			Ptr<IGuiAxis> GuiVirtualRepeatCompositionBase::GetAxis()
			{
				return axis;
			}

			void GuiVirtualRepeatCompositionBase::SetAxis(Ptr<IGuiAxis> value)
			{
				if (axis != value)
				{
					ClearItems();
					axis = value;
					if (itemTemplate && itemSource)
					{
						InstallItems();
					}
					AxisChanged.Execute(GuiEventArgs(this));
				}
			}

			Size GuiVirtualRepeatCompositionBase::GetTotalSize()
			{
				return realFullSize;
			}

			Point GuiVirtualRepeatCompositionBase::GetViewLocation()
			{
				return axis->VirtualRectToRealRect(realFullSize, viewBounds).LeftTop();
			}

			void GuiVirtualRepeatCompositionBase::SetViewLocation(Point value)
			{
				Layout_UpdateViewBounds(axis->RealRectToVirtualRect(realFullSize, Rect(value, viewBounds.GetSize())));
			}

			GuiVirtualRepeatCompositionBase::ItemStyleRecord GuiVirtualRepeatCompositionBase::GetVisibleStyle(vint itemIndex)
			{
				if (startIndex <= itemIndex && itemIndex < startIndex + visibleStyles.Count())
				{
					return visibleStyles[itemIndex - startIndex];
				}
				else
				{
					return nullptr;
				}
			}

			vint GuiVirtualRepeatCompositionBase::GetVisibleIndex(ItemStyleRecord style)
			{
				for (auto [s, i] : indexed(visibleStyles))
				{
					if (s == style)
					{
						return i + startIndex;
					}
				}
				return -1;
			}

			void GuiVirtualRepeatCompositionBase::ReloadVisibleStyles()
			{
				ClearItems();
			}
		}
	}
}