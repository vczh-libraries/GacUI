#include "GuiGraphicsRepeatComposition.h"
#include "../Controls/Templates/GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{

/***********************************************************************
GuiNonVirtialRepeatCompositionBase
***********************************************************************/

			void GuiNonVirtialRepeatCompositionBase::OnItemChanged(vint index, vint oldCount, vint newCount)
			{
				if (itemTemplate && itemSource)
				{
					for (vint i = oldCount - 1; i >= 0; i--)
					{
						RemoveItem(index + i);
					}

					for (vint i = 0; i < newCount; i++)
					{
						InstallItem(index + i);
					}
				}
			}

			void GuiNonVirtialRepeatCompositionBase::OnClearItems()
			{
				vint count = GetRepeatCompositionCount();
				for (vint i = count - 1; i >= 0; i--)
				{
					RemoveItem(i);
				}
			}

			void GuiNonVirtialRepeatCompositionBase::OnInstallItems()
			{
				if (itemTemplate && itemSource)
				{
					vint count = itemSource->GetCount();
					for (vint i = 0; i < count; i++)
					{
						InstallItem(i);
					}
				}
			}

			void GuiNonVirtialRepeatCompositionBase::OnUpdateContext()
			{
				vint count = GetRepeatCompositionCount();
				for (vint i = 0; i < count; i++)
				{
					auto rc = GetRepeatComposition(i);
					auto it = dynamic_cast<templates::GuiTemplate*>(rc->Children()[0]);
					it->SetContext(itemContext);
				}
			}

			void GuiNonVirtialRepeatCompositionBase::RemoveItem(vint index)
			{
				GuiItemEventArgs arguments(dynamic_cast<GuiGraphicsComposition*>(this));
				arguments.itemIndex = index;
				ItemRemoved.Execute(arguments);

				auto item = RemoveRepeatComposition(index);
				SafeDeleteComposition(item);
			}

			void GuiNonVirtialRepeatCompositionBase::InstallItem(vint index)
			{
				auto source = itemSource->Get(index);
				auto templateItem = itemTemplate(source);
				auto item = InsertRepeatComposition(index);

				templateItem->SetAlignmentToParent(Margin(0, 0, 0, 0));
				templateItem->SetContext(itemContext);
				item->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				item->AddChild(templateItem);

				GuiItemEventArgs arguments(dynamic_cast<GuiGraphicsComposition*>(this));
				arguments.itemIndex = index;
				ItemInserted.Execute(arguments);
			}

			GuiNonVirtialRepeatCompositionBase::GuiNonVirtialRepeatCompositionBase()
			{
			}

			GuiNonVirtialRepeatCompositionBase::~GuiNonVirtialRepeatCompositionBase()
			{
			}

/***********************************************************************
GuiRepeatStackComposition
***********************************************************************/

			vint GuiRepeatStackComposition::GetRepeatCompositionCount()
			{
				return GetStackItems().Count();
			}

			GuiGraphicsComposition* GuiRepeatStackComposition::GetRepeatComposition(vint index)
			{
				return GetStackItems()[index];
			}

			GuiGraphicsComposition* GuiRepeatStackComposition::InsertRepeatComposition(vint index)
			{
				CHECK_ERROR(0 <= index && index <= GetStackItems().Count(), L"GuiRepeatStackComposition::InsertRepeatComposition(vint)#Index out of range.");
				auto item = new GuiStackItemComposition;
				InsertStackItem(index, item);
				return item;
			}

			GuiGraphicsComposition* GuiRepeatStackComposition::RemoveRepeatComposition(vint index)
			{
				auto item = GetStackItems()[index];
				RemoveChild(item);
				return item;
			}

			GuiRepeatStackComposition::GuiRepeatStackComposition()
			{
				ItemInserted.SetAssociatedComposition(this);
				ItemRemoved.SetAssociatedComposition(this);
				ContextChanged.SetAssociatedComposition(this);
			}

			GuiRepeatStackComposition::~GuiRepeatStackComposition()
			{
			}

/***********************************************************************
GuiRepeatFlowComposition
***********************************************************************/

			vint GuiRepeatFlowComposition::GetRepeatCompositionCount()
			{
				return GetFlowItems().Count();
			}

			GuiGraphicsComposition* GuiRepeatFlowComposition::GetRepeatComposition(vint index)
			{
				return GetFlowItems()[index];
			}

			GuiGraphicsComposition* GuiRepeatFlowComposition::InsertRepeatComposition(vint index)
			{
				CHECK_ERROR(0 <= index && index <= GetFlowItems().Count(), L"GuiRepeatStackComposition::InsertRepeatComposition(vint)#Index out of range.");
				auto item = new GuiFlowItemComposition;
				InsertFlowItem(index, item);
				return item;
			}

			GuiGraphicsComposition* GuiRepeatFlowComposition::RemoveRepeatComposition(vint index)
			{
				auto item = GetFlowItems()[index];
				RemoveChild(item);
				return item;
			}

			GuiRepeatFlowComposition::GuiRepeatFlowComposition()
			{
				ItemInserted.SetAssociatedComposition(this);
				ItemRemoved.SetAssociatedComposition(this);
				ContextChanged.SetAssociatedComposition(this);
			}

			GuiRepeatFlowComposition::~GuiRepeatFlowComposition()
			{
			}
		}
	}
}