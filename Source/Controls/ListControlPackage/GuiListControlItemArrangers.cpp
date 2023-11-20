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

				RangedItemArrangerBase::RangedItemArrangerBase(compositions::GuiVirtualRepeatCompositionBase* _repeat)
					: repeat(_repeat)
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
						CHECK_FAIL(L"Not Implemented: Need to build a Ptr<IValueObservableList> from IItemProvider.");
					}
				}

				void RangedItemArrangerBase::OnItemModified(vint start, vint count, vint newCount)
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				void RangedItemArrangerBase::AttachListControl(GuiListControl* value)
				{
					listControl = value;
					CHECK_FAIL(L"Observe and set repeat properties!");
				}

				void RangedItemArrangerBase::DetachListControl()
				{
					CHECK_FAIL(L"Unobserve!");
					listControl = nullptr;
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

				vint RangedItemArrangerBase::FindItem(vint itemIndex, compositions::KeyDirection key)
				{
				}

				GuiListControl::EnsureItemVisibleResult RangedItemArrangerBase::EnsureItemVisible(vint itemIndex)
				{
				}

				Size RangedItemArrangerBase::GetAdoptedSize(Size expectedSize)
				{
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