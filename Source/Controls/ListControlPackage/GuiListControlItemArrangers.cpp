#include "GuiListControlItemArrangers.h"
#include "../../GraphicsComposition/GuiGraphicsRepeatComposition.h"
#include "../Templates/GuiControlTemplates.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../GuiButtonControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;

			namespace list
			{

/***********************************************************************
RangedItemArrangerBase
***********************************************************************/

				void RangedItemArrangerBase::InvalidateAdoptedSize()
				{
					if (listControl)
					{
						listControl->AdoptedSizeInvalidated.Execute(listControl->GetNotifyEventArguments());
					}
				}

				vint RangedItemArrangerBase::CalculateAdoptedSize(vint expectedSize, vint count, vint itemSize)
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

				RangedItemArrangerBase::ItemStyleRecord RangedItemArrangerBase::CreateStyle(vint index)
				{
					GuiSelectableButton* backgroundButton = nullptr;
					if (listControl->GetDisplayItemBackground())
					{
						backgroundButton = new GuiSelectableButton(theme::ThemeName::ListItemBackground);
						if (auto style = listControl->TypedControlTemplateObject(true)->GetBackgroundTemplate())
						{
							backgroundButton->SetControlTemplate(style);
						}
						backgroundButton->SetAutoFocus(false);
						backgroundButton->SetAutoSelection(false);
					}

					auto itemStyle = callback->RequestItem(index, backgroundButton->GetBoundsComposition());
					if (backgroundButton)
					{
						itemStyle->SetAlignmentToParent(Margin(0, 0, 0, 0));
						itemStyle->SelectedChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
						{
							backgroundButton->SetSelected(itemStyle->GetSelected());
						});

						backgroundButton->SetSelected(itemStyle->GetSelected());
						backgroundButton->GetContainerComposition()->AddChild(itemStyle);
					}
					return { itemStyle, backgroundButton };
				}

				void RangedItemArrangerBase::DeleteStyle(ItemStyleRecord style)
				{
					callback->ReleaseItem(style.key);
					if (style.value)
					{
						SafeDeleteControl(style.value);
					}
				}

				compositions::GuiBoundsComposition* RangedItemArrangerBase::GetStyleBounds(ItemStyleRecord style)
				{
					return style.value ? style.value->GetBoundsComposition() : style.key;
				}

				void RangedItemArrangerBase::ClearStyles()
				{
					startIndex = 0;
					if (callback)
					{
						// TODO: (enumerable) foreach
						for (vint i = 0; i < visibleStyles.Count(); i++)
						{
							DeleteStyle(visibleStyles[i]);
						}
					}
					visibleStyles.Clear();
					viewBounds = Rect(0, 0, 0, 0);
					InvalidateItemSizeCache();
					InvalidateAdoptedSize();
				}

				void RangedItemArrangerBase::OnViewChangedInternal(Rect oldBounds, Rect newBounds)
				{
					vint endIndex = startIndex + visibleStyles.Count() - 1;
					vint newStartIndex = 0;
					vint itemCount = itemProvider->Count();
					BeginPlaceItem(true, newBounds, newStartIndex);
					if (newStartIndex < 0) newStartIndex = 0;

					StyleList newVisibleStyles;
					for (vint i = newStartIndex; i < itemCount; i++)
					{
						bool reuseOldStyle = startIndex <= i && i <= endIndex;
						auto style = reuseOldStyle ? visibleStyles[i - startIndex] : CreateStyle(i);
						newVisibleStyles.Add(style);

						Rect bounds;
						Margin alignmentToParent;
						PlaceItem(true, !reuseOldStyle, i, style, newBounds, bounds, alignmentToParent);
						if (IsItemOutOfViewBounds(i, style, bounds, newBounds))
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

					if (EndPlaceItem(true, newBounds, newStartIndex))
					{
						callback->OnTotalSizeChanged();
						InvalidateAdoptedSize();
					}
					startIndex = newStartIndex;
				}

				void RangedItemArrangerBase::RearrangeItemBounds()
				{
					vint newStartIndex = startIndex;
					BeginPlaceItem(false, viewBounds, newStartIndex);
					// TODO: (enumerable) foreach
					for (vint i = 0; i < visibleStyles.Count(); i++)
					{
						auto style = visibleStyles[i];
						Rect bounds;
						Margin alignmentToParent(-1, -1, -1, -1);
						PlaceItem(false, false, startIndex + i, style, viewBounds, bounds, alignmentToParent);

						bounds.x1 -= viewBounds.x1;
						bounds.x2 -= viewBounds.x1;
						bounds.y1 -= viewBounds.y1;
						bounds.y2 -= viewBounds.y1;

						callback->SetStyleAlignmentToParent(GetStyleBounds(style), alignmentToParent);
						callback->SetStyleBounds(GetStyleBounds(style), bounds);
					}
					EndPlaceItem(false, viewBounds, startIndex);
				}

				RangedItemArrangerBase::RangedItemArrangerBase()
				{
				}

				RangedItemArrangerBase::~RangedItemArrangerBase()
				{
				}

				void RangedItemArrangerBase::OnAttached(GuiListControl::IItemProvider* provider)
				{
					itemProvider = provider;
					if (provider)
					{
						OnItemModified(0, 0, provider->Count());
					}
				}

				void RangedItemArrangerBase::OnItemModified(vint start, vint count, vint newCount)
				{
					if (callback && !itemProvider->IsEditing())
					{
						suppressOnViewChanged = true;
						{
							vint visibleCount = visibleStyles.Count();
							vint itemCount = itemProvider->Count();
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
									oldIndex = index - newCount + count;
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
								visibleStyles[i].key->SetIndex(startIndex + i);
							}
						}
						suppressOnViewChanged = false;

						callback->OnTotalSizeChanged();
						callback->SetViewLocation(viewBounds.LeftTop());
						InvalidateAdoptedSize();
					}
				}

				void RangedItemArrangerBase::AttachListControl(GuiListControl* value)
				{
					listControl = value;
					InvalidateAdoptedSize();
				}

				void RangedItemArrangerBase::DetachListControl()
				{
					listControl = 0;
				}

				GuiListControl::IItemArrangerCallback* RangedItemArrangerBase::GetCallback()
				{
					return callback;
				}

				void RangedItemArrangerBase::SetCallback(GuiListControl::IItemArrangerCallback* value)
				{
					if (callback != value)
					{
						ClearStyles();
						callback = value;
					}
				}

				Size RangedItemArrangerBase::GetTotalSize()
				{
					if (callback)
					{
						return OnCalculateTotalSize();
					}
					else
					{
						return Size(0, 0);
					}
				}

				GuiListControl::ItemStyle* RangedItemArrangerBase::GetVisibleStyle(vint itemIndex)
				{
					if (startIndex <= itemIndex && itemIndex < startIndex + visibleStyles.Count())
					{
						return visibleStyles[itemIndex - startIndex].key;
					}
					else
					{
						return nullptr;
					}
				}

				vint RangedItemArrangerBase::GetVisibleIndex(GuiListControl::ItemStyle* style)
				{
					// TODO: (enumerable) foreach:indexed
					for (vint i = 0; i < visibleStyles.Count(); i++)
					{
						if (visibleStyles[i].key == style)
						{
							return i + startIndex;
						}
					}
					return -1;
				}

				void RangedItemArrangerBase::ReloadVisibleStyles()
				{
					ClearStyles();
				}

				void RangedItemArrangerBase::OnViewChanged(Rect bounds)
				{
					if (!suppressOnViewChanged)
					{
						suppressOnViewChanged = true;
						Rect oldBounds = viewBounds;
						viewBounds = bounds;
						if (callback)
						{
							OnViewChangedInternal(oldBounds, viewBounds);
							RearrangeItemBounds();
						}
						suppressOnViewChanged = false;
					}
				}

/***********************************************************************
VirtualRepeatRangedItemArrangerBase
***********************************************************************/

				FreeHeightItemArranger::FreeHeightItemArranger()
				{
				}

				FreeHeightItemArranger::~FreeHeightItemArranger()
				{
				}

				FixedHeightItemArranger::FixedHeightItemArranger()
				{
				}

				FixedHeightItemArranger::~FixedHeightItemArranger()
				{
				}

				FixedSizeMultiColumnItemArranger::FixedSizeMultiColumnItemArranger()
				{
				}

				FixedSizeMultiColumnItemArranger::~FixedSizeMultiColumnItemArranger()
				{
				}

				FixedHeightMultiColumnItemArranger::FixedHeightMultiColumnItemArranger()
				{
				}

				FixedHeightMultiColumnItemArranger::~FixedHeightMultiColumnItemArranger()
				{
				}
			}
		}
	}
}