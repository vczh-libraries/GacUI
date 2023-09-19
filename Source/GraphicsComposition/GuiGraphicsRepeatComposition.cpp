#include "GuiGraphicsRepeatComposition.h"
#include "../Controls/Templates/GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace reflection::description;

/***********************************************************************
GuiRepeatCompositionBase
***********************************************************************/

			GuiRepeatCompositionBase::GuiRepeatCompositionBase()
			{
			}

			GuiRepeatCompositionBase::~GuiRepeatCompositionBase()
			{
				if (itemChangedHandler)
				{
					itemSource.Cast<IValueObservableList>()->ItemChanged.Remove(itemChangedHandler);
				}
			}

			GuiRepeatCompositionBase::ItemStyleProperty GuiRepeatCompositionBase::GetItemTemplate()
			{
				return itemTemplate;
			}

			void GuiRepeatCompositionBase::SetItemTemplate(ItemStyleProperty value)
			{
				ClearItems();
				itemTemplate = value;
				if (itemTemplate && itemSource)
				{
					InstallItems();
				}
			}

			Ptr<IValueEnumerable> GuiRepeatCompositionBase::GetItemSource()
			{
				return itemSource;
			}

			void GuiRepeatCompositionBase::SetItemSource(Ptr<IValueEnumerable> value)
			{
				if (value != itemSource)
				{
					if (itemChangedHandler)
					{
						itemSource.Cast<IValueObservableList>()->ItemChanged.Remove(itemChangedHandler);
					}

					ClearItems();
					itemSource = value.Cast<IValueList>();
					if (!itemSource && value)
					{
						itemSource = IValueList::Create(GetLazyList<Value>(value));
					}

					if (itemTemplate && itemSource)
					{
						InstallItems();
					}
					if (auto observable = itemSource.Cast<IValueObservableList>())
					{
						itemChangedHandler = observable->ItemChanged.Add(this, &GuiRepeatCompositionBase::OnItemChanged);
					}
				}
			}

			description::Value GuiRepeatCompositionBase::GetContext()
			{
				return itemContext;
			}

			void GuiRepeatCompositionBase::SetContext(const description::Value& value)
			{
				if (itemContext != value)
				{
					itemContext = value;
					UpdateContext();

					GuiEventArgs arguments(dynamic_cast<GuiGraphicsComposition*>(this));
					ContextChanged.Execute(arguments);
				}
			}
		}
	}
}