#include "GuiListControlItemArrangers.h"

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

				void RangedItemArrangerBase::ClearStyles()
				{
					startIndex = 0;
					if (callback)
					{
						for (vint i = 0; i < visibleStyles.Count(); i++)
						{
							GuiListControl::IItemStyleController* style = visibleStyles[i];
							callback->ReleaseItem(style);
						}
					}
					visibleStyles.Clear();
					viewBounds = Rect(0, 0, 0, 0);
					OnStylesCleared();
				}

				RangedItemArrangerBase::RangedItemArrangerBase()
					:listControl(0)
					, callback(0)
					, startIndex(0)
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
					if (callback)
					{
						vint visibleCount = visibleStyles.Count();
						vint itemCount = itemProvider->Count();
						SortedList<GuiListControl::IItemStyleController*> reusedStyles;
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
									GuiListControl::IItemStyleController* style = visibleStyles[oldIndex - startIndex];
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
								GuiListControl::IItemStyleController* style = callback->RequestItem(index);
								visibleStyles.Add(style);
							}
						}

						for (vint i = 0; i < visibleCount; i++)
						{
							GuiListControl::IItemStyleController* style = visibleStyles[i];
							if (!reusedStyles.Contains(style))
							{
								callback->ReleaseItem(style);
							}
						}

						visibleStyles.RemoveRange(0, visibleCount);
						if (listControl && listControl->GetStyleProvider())
						{
							for (vint i = 0; i < visibleStyles.Count(); i++)
							{
								listControl->GetStyleProvider()->SetStyleIndex(visibleStyles[i], startIndex + i);
							}
						}

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
					if (!value)
					{
						ClearStyles();
					}
					callback = value;
				}

				Size RangedItemArrangerBase::GetTotalSize()
				{
					return OnCalculateTotalSize();
				}

				GuiListControl::IItemStyleController* RangedItemArrangerBase::GetVisibleStyle(vint itemIndex)
				{
					if (startIndex <= itemIndex && itemIndex < startIndex + visibleStyles.Count())
					{
						return visibleStyles[itemIndex - startIndex];
					}
					else
					{
						return 0;
					}
				}

				vint RangedItemArrangerBase::GetVisibleIndex(GuiListControl::IItemStyleController* style)
				{
					vint index = visibleStyles.IndexOf(style);
					return index == -1 ? -1 : index + startIndex;
				}

				void RangedItemArrangerBase::OnViewChanged(Rect bounds)
				{
					Rect oldBounds = viewBounds;
					viewBounds = bounds;
					if (callback)
					{
						OnViewChangedInternal(oldBounds, viewBounds);
					}
				}

/***********************************************************************
FixedHeightItemArranger
***********************************************************************/

				void FixedHeightItemArranger::RearrangeItemBounds()
				{
					vint x0 = -viewBounds.Left();
					vint y0 = -viewBounds.Top() + GetYOffset();
					vint width = GetWidth();
					for (vint i = 0; i < visibleStyles.Count(); i++)
					{
						GuiListControl::IItemStyleController* style = visibleStyles[i];
						vint top = y0 + (startIndex + i)*rowHeight;
						if (width == -1)
						{
							callback->SetStyleAlignmentToParent(style, Margin(0, -1, 0, -1));
							callback->SetStyleBounds(style, Rect(Point(0, top), Size(0, rowHeight)));
						}
						else
						{
							callback->SetStyleAlignmentToParent(style, Margin(-1, -1, -1, -1));
							callback->SetStyleBounds(style, Rect(Point(x0, top), Size(width, rowHeight)));
						}
					}
				}

				vint FixedHeightItemArranger::GetWidth()
				{
					return -1;
				}

				vint FixedHeightItemArranger::GetYOffset()
				{
					return 0;
				}

				void FixedHeightItemArranger::OnStylesCleared()
				{
					rowHeight = 1;
					InvalidateAdoptedSize();
				}

				Size FixedHeightItemArranger::OnCalculateTotalSize()
				{
					if (callback)
					{
						vint width = GetWidth();
						if (width < 0) width = 0;
						return Size(width, rowHeight*itemProvider->Count() + GetYOffset());
					}
					else
					{
						return Size(0, 0);
					}
				}

				void FixedHeightItemArranger::OnViewChangedInternal(Rect oldBounds, Rect newBounds)
				{
					if (callback)
					{
						if (!suppressOnViewChanged)
						{
							vint oldVisibleCount = visibleStyles.Count();
							vint newRowHeight = rowHeight;
							vint newStartIndex = (newBounds.Top() - GetYOffset()) / rowHeight;
							if (newStartIndex < 0) newStartIndex = 0;

							vint endIndex = startIndex + visibleStyles.Count() - 1;
							vint newEndIndex = (newBounds.Bottom() - 1) / newRowHeight;
							vint itemCount = itemProvider->Count();

							for (vint i = newStartIndex; i <= newEndIndex && i < itemCount; i++)
							{
								GuiListControl::IItemStyleController* style = nullptr;
								if (startIndex <= i && i <= endIndex)
								{
									style = visibleStyles[i - startIndex];
									visibleStyles.Add(style);
								}
								else
								{
									style = callback->RequestItem(i);
									visibleStyles.Add(style);
								}

								if (style)
								{
									vint styleHeight = callback->GetStylePreferredSize(style).y;
									if (newRowHeight < styleHeight)
									{
										newRowHeight = styleHeight;
										newEndIndex = newStartIndex + (newBounds.Height() - 1) / newRowHeight + 1;
										if (newEndIndex < i)
										{
											newEndIndex = i;
										}
									}
								}
							}

							for (vint i = 0; i < oldVisibleCount; i++)
							{
								vint index = startIndex + i;
								if (index < newStartIndex || newEndIndex < index)
								{
									GuiListControl::IItemStyleController* style = visibleStyles[i];
									callback->ReleaseItem(style);
								}
							}
							visibleStyles.RemoveRange(0, oldVisibleCount);

							if (rowHeight != newRowHeight)
							{
								vint offset = oldBounds.Top() - rowHeight*startIndex;
								rowHeight = newRowHeight;
								suppressOnViewChanged = true;
								callback->OnTotalSizeChanged();
								callback->SetViewLocation(Point(0, rowHeight*newStartIndex + offset));
								suppressOnViewChanged = false;
								InvalidateAdoptedSize();
							}
							startIndex = newStartIndex;
							RearrangeItemBounds();
						}
					}
				}

				FixedHeightItemArranger::FixedHeightItemArranger()
					:rowHeight(1)
					, suppressOnViewChanged(false)
				{
				}

				FixedHeightItemArranger::~FixedHeightItemArranger()
				{
				}

				vint FixedHeightItemArranger::FindItem(vint itemIndex, compositions::KeyDirection key)
				{
					vint count = itemProvider->Count();
					if (count == 0) return -1;
					vint groupCount = viewBounds.Height() / rowHeight;
					if (groupCount == 0) groupCount = 1;
					switch (key)
					{
					case KeyDirection::Up:
						itemIndex--;
						break;
					case KeyDirection::Down:
						itemIndex++;
						break;
					case KeyDirection::Home:
						itemIndex = 0;
						break;
					case KeyDirection::End:
						itemIndex = count;
						break;
					case KeyDirection::PageUp:
						itemIndex -= groupCount;
						break;
					case KeyDirection::PageDown:
						itemIndex += groupCount;
						break;
					default:
						return -1;
					}

					if (itemIndex < 0) return 0;
					else if (itemIndex >= count) return count - 1;
					else return itemIndex;
				}

				bool FixedHeightItemArranger::EnsureItemVisible(vint itemIndex)
				{
					if (callback)
					{
						if (itemIndex < 0 || itemIndex >= itemProvider->Count())
						{
							return false;
						}
						while (true)
						{
							vint yOffset = GetYOffset();
							vint top = itemIndex*rowHeight;
							vint bottom = top + rowHeight + yOffset;

							if (viewBounds.Height() < rowHeight)
							{
								if (viewBounds.Top() < bottom && top < viewBounds.Bottom())
								{
									break;
								}
							}

							Point location = viewBounds.LeftTop();
							if (top < viewBounds.Top())
							{
								location.y = top;
							}
							else if (viewBounds.Bottom() < bottom)
							{
								location.y = bottom - viewBounds.Height();
							}
							else
							{
								break;
							}
							callback->SetViewLocation(location);
						}
						return true;
					}
					return false;
				}

				Size FixedHeightItemArranger::GetAdoptedSize(Size expectedSize)
				{
					if (itemProvider)
					{
						vint offset = GetYOffset();
						vint y = expectedSize.y - offset;
						vint itemCount = itemProvider->Count();
						return Size(expectedSize.x, offset + CalculateAdoptedSize(y, itemCount, rowHeight));
					}
					return expectedSize;
				}

/***********************************************************************
FixedSizeMultiColumnItemArranger
***********************************************************************/

				void FixedSizeMultiColumnItemArranger::RearrangeItemBounds()
				{
					vint y0 = -viewBounds.Top();
					vint rowItems = viewBounds.Width() / itemSize.x;
					if (rowItems < 1) rowItems = 1;

					for (vint i = 0; i < visibleStyles.Count(); i++)
					{
						GuiListControl::IItemStyleController* style = visibleStyles[i];
						vint row = (startIndex + i) / rowItems;
						vint col = (startIndex + i) % rowItems;
						callback->SetStyleBounds(style, Rect(Point(col*itemSize.x, y0 + row*itemSize.y), itemSize));
					}
				}

				void FixedSizeMultiColumnItemArranger::CalculateRange(Size itemSize, Rect bounds, vint count, vint& start, vint& end)
				{
					vint startRow = bounds.Top() / itemSize.y;
					if (startRow < 0) startRow = 0;
					vint endRow = (bounds.Bottom() - 1) / itemSize.y;
					vint cols = bounds.Width() / itemSize.x;
					if (cols < 1) cols = 1;

					start = startRow*cols;
					end = (endRow + 1)*cols - 1;
					if (end >= count) end = count - 1;
				}

				void FixedSizeMultiColumnItemArranger::OnStylesCleared()
				{
					itemSize = Size(1, 1);
					InvalidateAdoptedSize();
				}

				Size FixedSizeMultiColumnItemArranger::OnCalculateTotalSize()
				{
					if (callback)
					{
						vint rowItems = viewBounds.Width() / itemSize.x;
						if (rowItems < 1) rowItems = 1;
						vint rows = itemProvider->Count() / rowItems;
						if (itemProvider->Count() % rowItems) rows++;

						return Size(itemSize.x*rowItems, itemSize.y*rows);
					}
					else
					{
						return Size(0, 0);
					}
				}

				void FixedSizeMultiColumnItemArranger::OnViewChangedInternal(Rect oldBounds, Rect newBounds)
				{
					if (callback)
					{
						if (!suppressOnViewChanged)
						{
							vint oldVisibleCount = visibleStyles.Count();
							Size newItemSize = itemSize;
							vint endIndex = startIndex + visibleStyles.Count() - 1;

							vint newStartIndex = 0;
							vint newEndIndex = 0;
							vint itemCount = itemProvider->Count();
							CalculateRange(newItemSize, newBounds, itemCount, newStartIndex, newEndIndex);
							if (newItemSize == Size(1, 1) && newStartIndex < newEndIndex)
							{
								newEndIndex = newStartIndex;
							}

							vint previousStartIndex = -1;
							vint previousEndIndex = -1;

							while (true)
							{
								for (vint i = newStartIndex; i <= newEndIndex; i++)
								{
									GuiListControl::IItemStyleController* style = nullptr;
									if (startIndex <= i && i <= endIndex)
									{
										style = visibleStyles[i - startIndex];
										visibleStyles.Add(style);
									}
									else if (i<previousStartIndex || i>previousEndIndex)
									{
										style = callback->RequestItem(i);

										if (i < previousStartIndex)
										{
											visibleStyles.Insert(oldVisibleCount + (i - newStartIndex), style);
										}
										else
										{
											visibleStyles.Add(style);
										}
									}

									if (style)
									{
										Size styleSize = callback->GetStylePreferredSize(style);
										if (newItemSize.x < styleSize.x) newItemSize.x = styleSize.x;
										if (newItemSize.y < styleSize.y) newItemSize.y = styleSize.y;
									}
								}

								vint updatedStartIndex = 0;
								vint updatedEndIndex = 0;
								CalculateRange(newItemSize, newBounds, itemCount, updatedStartIndex, updatedEndIndex);
								bool again = updatedStartIndex<newStartIndex || updatedEndIndex>newEndIndex;
								previousStartIndex = newStartIndex;
								previousEndIndex = newEndIndex;
								if (updatedStartIndex < newStartIndex) newStartIndex = updatedStartIndex;
								if (updatedEndIndex > newEndIndex) newEndIndex = updatedEndIndex;
								if (!again) break;
							}

							for (vint i = 0; i < oldVisibleCount; i++)
							{
								vint index = startIndex + i;
								if (index < newStartIndex || newEndIndex < index)
								{
									GuiListControl::IItemStyleController* style = visibleStyles[i];
									callback->ReleaseItem(style);
								}
							}
							visibleStyles.RemoveRange(0, oldVisibleCount);

							if (itemSize != newItemSize)
							{
								itemSize = newItemSize;
								suppressOnViewChanged = true;
								callback->OnTotalSizeChanged();
								suppressOnViewChanged = false;
								InvalidateAdoptedSize();
							}
							startIndex = newStartIndex;
							RearrangeItemBounds();
						}
					}
				}

				FixedSizeMultiColumnItemArranger::FixedSizeMultiColumnItemArranger()
					:itemSize(1, 1)
					, suppressOnViewChanged(false)
				{
				}

				FixedSizeMultiColumnItemArranger::~FixedSizeMultiColumnItemArranger()
				{
				}

				vint FixedSizeMultiColumnItemArranger::FindItem(vint itemIndex, compositions::KeyDirection key)
				{
					vint count = itemProvider->Count();
					vint columnCount = viewBounds.Width() / itemSize.x;
					if (columnCount == 0) columnCount = 1;
					vint rowCount = viewBounds.Height() / itemSize.y;
					if (rowCount == 0) rowCount = 1;

					switch (key)
					{
					case KeyDirection::Up:
						itemIndex -= columnCount;
						break;
					case KeyDirection::Down:
						itemIndex += columnCount;
						break;
					case KeyDirection::Left:
						itemIndex--;
						break;
					case KeyDirection::Right:
						itemIndex++;
						break;
					case KeyDirection::Home:
						itemIndex = 0;
						break;
					case KeyDirection::End:
						itemIndex = count;
						break;
					case KeyDirection::PageUp:
						itemIndex -= columnCount*rowCount;
						break;
					case KeyDirection::PageDown:
						itemIndex += columnCount*rowCount;
						break;
					case KeyDirection::PageLeft:
						itemIndex -= itemIndex%columnCount;
						break;
					case KeyDirection::PageRight:
						itemIndex += columnCount - itemIndex%columnCount - 1;
						break;
					default:
						return -1;
					}

					if (itemIndex < 0) return 0;
					else if (itemIndex >= count) return count - 1;
					else return itemIndex;
				}

				bool FixedSizeMultiColumnItemArranger::EnsureItemVisible(vint itemIndex)
				{
					if (callback)
					{
						if (itemIndex < 0 || itemIndex >= itemProvider->Count())
						{
							return false;
						}
						while (true)
						{
							vint rowHeight = itemSize.y;
							vint columnCount = viewBounds.Width() / itemSize.x;
							if (columnCount == 0) columnCount = 1;
							vint rowIndex = itemIndex / columnCount;

							vint top = rowIndex*rowHeight;
							vint bottom = top + rowHeight;

							if (viewBounds.Height() < rowHeight)
							{
								if (viewBounds.Top() < bottom && top < viewBounds.Bottom())
								{
									break;
								}
							}

							Point location = viewBounds.LeftTop();
							if (top < viewBounds.Top())
							{
								location.y = top;
							}
							else if (viewBounds.Bottom() < bottom)
							{
								location.y = bottom - viewBounds.Height();
							}
							else
							{
								break;
							}
							callback->SetViewLocation(location);
						}
						return true;
					}
					return false;
				}

				Size FixedSizeMultiColumnItemArranger::GetAdoptedSize(Size expectedSize)
				{
					if (itemProvider)
					{
						vint count = itemProvider->Count();
						vint columnCount = viewBounds.Width() / itemSize.x;
						vint rowCount = viewBounds.Height() / itemSize.y;
						return Size(
							CalculateAdoptedSize(expectedSize.x, columnCount, itemSize.x),
							CalculateAdoptedSize(expectedSize.y, rowCount, itemSize.y)
						);
					}
					return expectedSize;
				}

/***********************************************************************
FixedHeightMultiColumnItemArranger
***********************************************************************/

				void FixedHeightMultiColumnItemArranger::RearrangeItemBounds()
				{
					vint rows = 0;
					vint startColumn = 0;
					CalculateRange(itemHeight, viewBounds, rows, startColumn);
					vint currentWidth = 0;
					vint totalWidth = 0;
					for (vint i = 0; i < visibleStyles.Count(); i++)
					{
						vint column = i%rows;
						if (column == 0)
						{
							totalWidth += currentWidth;
							currentWidth = 0;
						}
						GuiListControl::IItemStyleController* style = visibleStyles[i];
						Size styleSize = callback->GetStylePreferredSize(style);
						if (currentWidth < styleSize.x) currentWidth = styleSize.x;
						callback->SetStyleBounds(style, Rect(Point(totalWidth, itemHeight * column), Size(0, 0)));
					}
				}

				void FixedHeightMultiColumnItemArranger::CalculateRange(vint itemHeight, Rect bounds, vint& rows, vint& startColumn)
				{
					rows = bounds.Height() / itemHeight;
					if (rows < 1) rows = 1;
					startColumn = bounds.Left() / bounds.Width();
				}

				void FixedHeightMultiColumnItemArranger::OnStylesCleared()
				{
					itemHeight = 1;
					InvalidateAdoptedSize();
				}

				Size FixedHeightMultiColumnItemArranger::OnCalculateTotalSize()
				{
					if (callback)
					{
						vint rows = viewBounds.Height() / itemHeight;
						if (rows < 1) rows = 1;
						vint columns = itemProvider->Count() / rows;
						if (itemProvider->Count() % rows) columns += 1;
						return Size(viewBounds.Width()*columns, 0);
					}
					else
					{
						return Size(0, 0);
					}
				}

				void FixedHeightMultiColumnItemArranger::OnViewChangedInternal(Rect oldBounds, Rect newBounds)
				{
					if (callback)
					{
						if (!suppressOnViewChanged)
						{
							vint oldVisibleCount = visibleStyles.Count();
							vint endIndex = startIndex + oldVisibleCount - 1;

							vint newItemHeight = itemHeight;
							vint itemCount = itemProvider->Count();

							vint previousStartIndex = -1;
							vint previousEndIndex = -1;
							vint newStartIndex = -1;
							vint newEndIndex = -1;

							while (true)
							{
								vint newRows = 0;
								vint newStartColumn = 0;
								vint currentWidth = 0;
								vint totalWidth = 0;
								CalculateRange(newItemHeight, newBounds, newRows, newStartColumn);
								newStartIndex = newRows*newStartColumn;
								vint currentItemHeight = newItemHeight;

								for (vint i = newStartIndex; i < itemCount; i++)
								{
									if (i%newRows == 0)
									{
										totalWidth += currentWidth;
										currentWidth = 0;
										if (totalWidth >= newBounds.Width())
										{
											break;
										}
									}
									newEndIndex = i;

									GuiListControl::IItemStyleController* style = nullptr;
									if (startIndex <= i && i <= endIndex)
									{
										style = visibleStyles[i - startIndex];
										visibleStyles.Add(style);
									}
									else if (i<previousStartIndex || i>previousEndIndex)
									{
										style = callback->RequestItem(i);

										if (i < previousStartIndex)
										{
											visibleStyles.Insert(oldVisibleCount + (i - newStartIndex), style);
										}
										else
										{
											visibleStyles.Add(style);
										}
									}

									if (style)
									{
										Size styleSize = callback->GetStylePreferredSize(style);
										if (currentWidth < styleSize.x) currentWidth = styleSize.x;
										if (newItemHeight < styleSize.y) newItemHeight = styleSize.y;
										if (currentItemHeight != newItemHeight) break;
									}
								}

								if (previousStartIndex == -1 || previousStartIndex < newStartIndex) previousStartIndex = newStartIndex;
								if (previousEndIndex == -1 || previousEndIndex > newEndIndex) previousEndIndex = newEndIndex;
								if (currentItemHeight == newItemHeight)
								{
									break;
								}
							}
							newStartIndex = previousStartIndex;
							newEndIndex = previousEndIndex;

							for (vint i = 0; i < oldVisibleCount; i++)
							{
								vint index = startIndex + i;
								if (index < newStartIndex || newEndIndex < index)
								{
									GuiListControl::IItemStyleController* style = visibleStyles[i];
									callback->ReleaseItem(style);
								}
							}
							visibleStyles.RemoveRange(0, oldVisibleCount);

							if (itemHeight != newItemHeight)
							{
								itemHeight = newItemHeight;
								suppressOnViewChanged = true;
								callback->OnTotalSizeChanged();
								suppressOnViewChanged = false;
								InvalidateAdoptedSize();
							}
							startIndex = newStartIndex;
							RearrangeItemBounds();
						}
					}
				}

				FixedHeightMultiColumnItemArranger::FixedHeightMultiColumnItemArranger()
					:itemHeight(1)
					, suppressOnViewChanged(false)
				{
				}

				FixedHeightMultiColumnItemArranger::~FixedHeightMultiColumnItemArranger()
				{
				}

				vint FixedHeightMultiColumnItemArranger::FindItem(vint itemIndex, compositions::KeyDirection key)
				{
					vint count = itemProvider->Count();
					vint groupCount = viewBounds.Height() / itemHeight;
					if (groupCount == 0) groupCount = 1;
					switch (key)
					{
					case KeyDirection::Up:
						itemIndex--;
						break;
					case KeyDirection::Down:
						itemIndex++;
						break;
					case KeyDirection::Left:
						itemIndex -= groupCount;
						break;
					case KeyDirection::Right:
						itemIndex += groupCount;
						break;
					case KeyDirection::Home:
						itemIndex = 0;
						break;
					case KeyDirection::End:
						itemIndex = count;
						break;
					case KeyDirection::PageUp:
						itemIndex -= itemIndex%groupCount;
						break;
					case KeyDirection::PageDown:
						itemIndex += groupCount - itemIndex%groupCount - 1;
						break;
					default:
						return -1;
					}

					if (itemIndex < 0) return 0;
					else if (itemIndex >= count) return count - 1;
					else return itemIndex;
				}

				bool FixedHeightMultiColumnItemArranger::EnsureItemVisible(vint itemIndex)
				{
					if (callback)
					{
						if (itemIndex < 0 || itemIndex >= itemProvider->Count())
						{
							return false;
						}
						while (true)
						{
							vint rowCount = viewBounds.Height() / itemHeight;
							if (rowCount == 0) rowCount = 1;
							vint columnIndex = itemIndex / rowCount;
							vint minIndex = startIndex;
							vint maxIndex = startIndex + visibleStyles.Count() - 1;

							Point location = viewBounds.LeftTop();
							if (minIndex <= itemIndex && itemIndex <= maxIndex)
							{
								Rect bounds = callback->GetStyleBounds(visibleStyles[itemIndex - startIndex]);
								if (0 < bounds.Bottom() && bounds.Top() < viewBounds.Width() && bounds.Width() > viewBounds.Width())
								{
									break;
								}
								else if (bounds.Left() < 0)
								{
									location.x -= viewBounds.Width();
								}
								else if (bounds.Right() > viewBounds.Width())
								{
									location.x += viewBounds.Width();
								}
								else
								{
									break;
								}
							}
							else if (columnIndex < minIndex / rowCount)
							{
								location.x -= viewBounds.Width();
							}
							else if (columnIndex >= maxIndex / rowCount)
							{
								location.x += viewBounds.Width();
							}
							else
							{
								break;
							}
							callback->SetViewLocation(location);
						}
						return true;
					}
					return false;
				}

				Size FixedHeightMultiColumnItemArranger::GetAdoptedSize(Size expectedSize)
				{
					if (itemProvider)
					{
						vint count = itemProvider->Count();
						return Size(expectedSize.x, CalculateAdoptedSize(expectedSize.y, count, itemHeight));
					}
					return expectedSize;
				}
			}
		}
	}
}