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

			void GuiVirtualRepeatCompositionBase::OnItemChanged(vint start, vint oldCount, vint newCount)
			{
				suppressOnViewChanged = true;
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
					// TODO: (enumerable) foreach:indexed
					for (vint i = 0; i < visibleStyles.Count(); i++)
					{
						Callback_UpdateIndex(visibleStyles[i], startIndex + 1);
					}
				}
				suppressOnViewChanged = false;

				realFullSize = axis->VirtualSizeToRealSize(Layout_CalculateTotalSize());
				Callback_UpdateTotalSize(realFullSize);
				Callback_UpdateViewLocation(axis->VirtualRectToRealRect(realFullSize, viewBounds).LeftTop());
				Callback_InvalidateAdoptedSize();
			}

			void GuiVirtualRepeatCompositionBase::ClearItems()
			{
				startIndex = 0;
				// TODO: (enumerable) foreach
				for (vint i = 0; i < visibleStyles.Count(); i++)
				{
					DeleteStyle(visibleStyles[i]);
				}
				visibleStyles.Clear();
				viewBounds = Rect(0, 0, 0, 0);
				Callback_InvalidateAdoptedSize();
				Callback_InvalidateAdoptedSize();
			}

			void GuiVirtualRepeatCompositionBase::InstallItems()
			{
			}

			void GuiVirtualRepeatCompositionBase::UpdateContext()
			{
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
					// TODO: (enumerable) foreach:indexed
					for (vint i = 0; i < visibleStyles.Count(); i++)
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
						Callback_UpdateTotalSize(realFullSize);
						Callback_InvalidateAdoptedSize();
					}
					startIndex = newStartIndex;
				}
			}

			void GuiVirtualRepeatCompositionBase::RearrangeItemBounds()
			{
				vint newStartIndex = startIndex;
				Layout_BeginPlaceItem(false, viewBounds, newStartIndex);
				// TODO: (enumerable) foreach
				for (vint i = 0; i < visibleStyles.Count(); i++)
				{
					auto style = visibleStyles[i];
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
			}

			GuiVirtualRepeatCompositionBase::~GuiVirtualRepeatCompositionBase()
			{
			}

			Size GuiVirtualRepeatCompositionBase::GetTotalSize()
			{
				return realFullSize;
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
				// TODO: (enumerable) foreach:indexed
				for (vint i = 0; i < visibleStyles.Count(); i++)
				{
					if (visibleStyles[i] == style)
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

			Rect GuiVirtualRepeatCompositionBase::GetViewBounds()
			{
				return axis->VirtualRectToRealRect(realFullSize, viewBounds);
			}

			void GuiVirtualRepeatCompositionBase::SetViewBounds(Rect bounds)
			{
				if (!suppressOnViewChanged)
				{
					suppressOnViewChanged = true;
					Rect oldBounds = viewBounds;
					viewBounds = axis->RealRectToVirtualRect(realFullSize, bounds);
					OnViewChangedInternal(oldBounds, viewBounds);
					RearrangeItemBounds();
					suppressOnViewChanged = false;
				}
			}
		}
	}
}