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

				GuiListControl::ItemStyle* RangedItemArrangerBase::CreateItemTemplate(vint index)
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

					auto itemStyle = callback->RequestItem(index, (backgroundButton ? backgroundButton->GetBoundsComposition() : nullptr));
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

					return itemStyle;
				}

				RangedItemArrangerBase::RangedItemArrangerBase(compositions::GuiVirtualRepeatCompositionBase* _repeat)
					: repeat(_repeat)
				{
				}

				RangedItemArrangerBase::~RangedItemArrangerBase()
				{
					SafeDeleteComposition(repeat);
				}

				void RangedItemArrangerBase::OnAttached(GuiListControl::IItemProvider* provider)
				{
					itemProvider = provider;
					if (provider)
					{
						CHECK_FAIL(L"Not Implemented: Need to build a Ptr<IValueObservableList> from IItemProvider.");
					}
					else
					{
						repeat->SetItemSource(nullptr);
					}
				}

				void RangedItemArrangerBase::OnItemModified(vint start, vint count, vint newCount)
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				void RangedItemArrangerBase::AttachListControl(GuiListControl* value)
				{
					listControl = value;
					repeat->SetAxis(Ptr(listControl->GetAxis()));
				}

				void RangedItemArrangerBase::DetachListControl()
				{
					repeat->SetAxis(nullptr);
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
						if (callback)
						{
							repeat->GetParent()->RemoveChild(repeat);
							repeat->SetItemTemplate({});
						}
						callback = value;
						if (callback)
						{
							callback->GetContainerComposition()->AddChild(repeat);
							repeat->SetItemTemplate([](const description::Value&)->templates::GuiTemplate*
							{
								CHECK_FAIL(L"This function should not be called, it is used to enable the virtual repeat composition.");
							});
						}
					}
				}

				Size RangedItemArrangerBase::GetTotalSize()
				{
					if (callback && repeat)
					{
						return repeat->GetTotalSize();
					}
					return Size(0, 0);
				}

				GuiListControl::ItemStyle* RangedItemArrangerBase::GetVisibleStyle(vint itemIndex)
				{
					return dynamic_cast<GuiListControl::ItemStyle*>(repeat->GetVisibleStyle(itemIndex));
				}

				vint RangedItemArrangerBase::GetVisibleIndex(GuiListControl::ItemStyle* style)
				{
					return repeat->GetVisibleIndex(style);
				}

				void RangedItemArrangerBase::ReloadVisibleStyles()
				{
					if (repeat) repeat->ReloadVisibleStyles();
				}

				void RangedItemArrangerBase::OnViewChanged(Rect bounds)
				{
					repeat->SetViewLocation(bounds.LeftTop());
					repeat->SetExpectedBounds(Rect({ 0,0 }, bounds.GetSize()));
				}

				vint RangedItemArrangerBase::FindItemByVirtualKeyDirection(vint itemIndex, compositions::KeyDirection key)
				{
					return repeat->FindItemByVirtualKeyDirection(itemIndex, key);
				}

				GuiListControl::EnsureItemVisibleResult RangedItemArrangerBase::EnsureItemVisible(vint itemIndex)
				{
					switch (repeat->EnsureItemVisible(itemIndex))
					{
					case VirtualRepeatEnsureItemVisibleResult::Moved:
						return GuiListControl::EnsureItemVisibleResult::Moved;
					case VirtualRepeatEnsureItemVisibleResult::NotMoved:
						return GuiListControl::EnsureItemVisibleResult::NotMoved;
					default:
						return GuiListControl::EnsureItemVisibleResult::ItemNotExists;
					}
				}

				Size RangedItemArrangerBase::GetAdoptedSize(Size expectedSize)
				{
					return repeat->GetAdoptedSize(expectedSize);
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