#include "GuiToolstripMenu.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../../GraphicsComposition/GuiGraphicsStackComposition.h"
#include "../../GraphicsComposition/GuiGraphicsSharedSizeComposition.h"
#include "../../GraphicsHost/GuiGraphicsHost_ShortcutKey.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace compositions;

/***********************************************************************
GuiToolstripCollectionBase
***********************************************************************/

			const wchar_t* const IToolstripUpdateLayoutInvoker::Identifier = L"vl::presentation::controls::IToolstripUpdateLayoutInvoker";

			void GuiToolstripCollectionBase::InvokeUpdateLayout()
			{
				if(contentCallback)
				{
					contentCallback->UpdateLayout();
				}
			}

			bool GuiToolstripCollectionBase::QueryInsert(vint index, GuiControl* const& child)
			{
				return !items.Contains(child) && !child->GetBoundsComposition()->GetParent();
			}

			void GuiToolstripCollectionBase::BeforeRemove(vint index, GuiControl* const& child)
			{
				if (auto invoker = child->QueryTypedService<IToolstripUpdateLayoutInvoker>())
				{
					invoker->SetCallback(nullptr);
				}
				InvokeUpdateLayout();
			}

			void GuiToolstripCollectionBase::AfterInsert(vint index, GuiControl* const& child)
			{
				if (auto invoker = child->QueryTypedService<IToolstripUpdateLayoutInvoker>())
				{
					invoker->SetCallback(contentCallback);
				}
				InvokeUpdateLayout();
			}

			void GuiToolstripCollectionBase::AfterRemove(vint index, vint count)
			{
				InvokeUpdateLayout();
			}

			GuiToolstripCollectionBase::GuiToolstripCollectionBase(IToolstripUpdateLayout* _contentCallback)
				:contentCallback(_contentCallback)
			{
			}

			GuiToolstripCollectionBase::~GuiToolstripCollectionBase()
			{
			}

/***********************************************************************
GuiToolstripCollection
***********************************************************************/

			void GuiToolstripCollection::UpdateItemVisibility(vint index, GuiControl* child)
			{
				auto stackItem = stackComposition->GetStackItems()[index];
				if (child->GetVisible())
				{
					stackItem->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					child->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				}
				else
				{
					stackItem->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
					child->GetBoundsComposition()->SetAlignmentToParent(Margin(-1, -1, -1, -1));
				}
			}

			void GuiToolstripCollection::OnItemVisibleChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				auto child = sender->GetRelatedControl();
				vint index = IndexOf(child);
				UpdateItemVisibility(index, child);
				InvokeUpdateLayout();
			}

			void GuiToolstripCollection::BeforeRemove(vint index, GuiControl* const& child)
			{
				GuiStackItemComposition* stackItem = stackComposition->GetStackItems().Get(index);

				auto eventHandler = eventHandlers[index];
				child->VisibleChanged.Detach(eventHandler);
				eventHandlers.RemoveAt(index);

				GuiToolstripCollectionBase::BeforeRemove(index, child);
			}

			void GuiToolstripCollection::AfterInsert(vint index, GuiControl* const& child)
			{
				{
					GuiStackItemComposition* stackItem = new GuiStackItemComposition;
					stackItem->AddChild(child->GetBoundsComposition());
					stackComposition->InsertStackItem(index, stackItem);
				}
				{
					auto eventHandler = child->VisibleChanged.AttachMethod(this, &GuiToolstripCollection::OnItemVisibleChanged);
					eventHandlers.Insert(index, eventHandler);
				}
				UpdateItemVisibility(index, child);
				GuiToolstripCollectionBase::AfterInsert(index, child);
			}

			void GuiToolstripCollection::AfterRemove(vint index, vint count)
			{
				for (vint i = 0; i < count; i++)
				{
					auto stackItem = stackComposition->GetStackItems().Get(index);
					stackComposition->RemoveChild(stackItem);
					SafeDeleteComposition(stackItem);
				}
				GuiToolstripCollectionBase::AfterRemove(index, count);
			}

			GuiToolstripCollection::GuiToolstripCollection(IToolstripUpdateLayout* _contentCallback, compositions::GuiStackComposition* _stackComposition)
				:GuiToolstripCollectionBase(_contentCallback)
				,stackComposition(_stackComposition)
			{
			}

			GuiToolstripCollection::~GuiToolstripCollection()
			{
			}

/***********************************************************************
GuiToolstripMenu
***********************************************************************/

			void GuiToolstripMenu::UpdateLayout()
			{
				sharedSizeRootComposition->ForceCalculateSizeImmediately();
			}

			GuiToolstripMenu::GuiToolstripMenu(theme::ThemeName themeName, GuiControl* _owner)
				:GuiMenu(themeName, _owner)
			{
				sharedSizeRootComposition = new GuiSharedSizeRootComposition();
				sharedSizeRootComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				sharedSizeRootComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				containerComposition->AddChild(sharedSizeRootComposition);

				stackComposition=new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Vertical);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				sharedSizeRootComposition->AddChild(stackComposition);
				
				toolstripItems = new GuiToolstripCollection(this, stackComposition);
			}

			GuiToolstripMenu::~GuiToolstripMenu()
			{
			}

			collections::ObservableListBase<GuiControl*>& GuiToolstripMenu::GetToolstripItems()
			{
				return *toolstripItems.Obj();
			}

/***********************************************************************
GuiToolstripMenuBar
***********************************************************************/
			
			GuiToolstripMenuBar::GuiToolstripMenuBar(theme::ThemeName themeName)
				:GuiMenuBar(themeName)
			{
				stackComposition=new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Horizontal);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				containerComposition->AddChild(stackComposition);

				toolstripItems=new GuiToolstripCollection(nullptr, stackComposition);
			}

			GuiToolstripMenuBar::~GuiToolstripMenuBar()
			{
			}

			collections::ObservableListBase<GuiControl*>& GuiToolstripMenuBar::GetToolstripItems()
			{
				return *toolstripItems.Obj();
			}

/***********************************************************************
GuiToolstripToolBar
***********************************************************************/
				
			GuiToolstripToolBar::GuiToolstripToolBar(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				stackComposition=new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Horizontal);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				containerComposition->AddChild(stackComposition);

				toolstripItems=new GuiToolstripCollection(nullptr, stackComposition);
			}

			GuiToolstripToolBar::~GuiToolstripToolBar()
			{
			}

			collections::ObservableListBase<GuiControl*>& GuiToolstripToolBar::GetToolstripItems()
			{
				return *toolstripItems.Obj();
			}

/***********************************************************************
GuiToolstripButton
***********************************************************************/

			void GuiToolstripButton::SetCallback(IToolstripUpdateLayout* _callback)
			{
				callback = _callback;
			}

			void GuiToolstripButton::OnActiveAlt()
			{
				auto host = GetSubMenuHost();
				if (host == this)
				{
					GuiMenuButton::OnActiveAlt();
				}
				else
				{
					host->QueryTypedService<IGuiAltAction>()->OnActiveAlt();
				}
			}

			void GuiToolstripButton::UpdateCommandContent()
			{
				if(command)
				{
					SetLargeImage(command->GetLargeImage());
					SetImage(command->GetImage());
					SetText(command->GetText());
					SetEnabled(command->GetEnabled());
					SetSelected(command->GetSelected());
					if(command->GetShortcut())
					{
						SetShortcutText(command->GetShortcut()->GetName());
					}
					else
					{
						SetShortcutText(L"");
					}
				}
				else
				{
					SetLargeImage(nullptr);
					SetImage(nullptr);
					SetText(L"");
					SetEnabled(true);
					SetSelected(false);
					SetShortcutText(L"");
				}
			}

			void GuiToolstripButton::OnLayoutAwaredPropertyChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (callback)
				{
					callback->UpdateLayout();
				}
			}

			void GuiToolstripButton::OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(command)
				{
					command->Executed.ExecuteWithNewSender(arguments, sender);
				}
			}

			void GuiToolstripButton::OnCommandDescriptionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				UpdateCommandContent();
			}

			GuiToolstripButton::GuiToolstripButton(theme::ThemeName themeName)
				:GuiMenuButton(themeName)
				,command(0)
			{
				SetAutoFocus(false);
				Clicked.AttachMethod(this, &GuiToolstripButton::OnClicked);
				TextChanged.AttachMethod(this, &GuiToolstripButton::OnLayoutAwaredPropertyChanged);
				ShortcutTextChanged.AttachMethod(this, &GuiToolstripButton::OnLayoutAwaredPropertyChanged);
			}

			GuiToolstripButton::~GuiToolstripButton()
			{
			}

			GuiToolstripCommand* GuiToolstripButton::GetCommand()
			{
				return command;
			}

			void GuiToolstripButton::SetCommand(GuiToolstripCommand* value)
			{
				if(command!=value)
				{
					if(command)
					{
						command->DescriptionChanged.Detach(descriptionChangedHandler);
					}
					command=0;
					descriptionChangedHandler=0;
					if(value)
					{
						command=value;
						descriptionChangedHandler=command->DescriptionChanged.AttachMethod(this, &GuiToolstripButton::OnCommandDescriptionChanged);
					}
					UpdateCommandContent();
				}
			}

			GuiToolstripMenu* GuiToolstripButton::GetToolstripSubMenu()
			{
				return dynamic_cast<GuiToolstripMenu*>(GetSubMenu());
			}

			GuiToolstripMenu* GuiToolstripButton::EnsureToolstripSubMenu()
			{
				if (!GetSubMenu())
				{
					CreateToolstripSubMenu({});
				}
				return dynamic_cast<GuiToolstripMenu*>(GetSubMenu());
			}

			void GuiToolstripButton::CreateToolstripSubMenu(TemplateProperty<templates::GuiMenuTemplate> subMenuTemplate)
			{
				if (!subMenu)
				{
					auto newSubMenu = new GuiToolstripMenu(theme::ThemeName::Menu, this);
					if (subMenuTemplate)
					{
						newSubMenu->SetControlTemplate(subMenuTemplate);
					}
					else
					{
						newSubMenu->SetControlTemplate(GetControlTemplateObject(true)->GetSubMenuTemplate());
					}
					SetSubMenu(newSubMenu, true);
				}
			}

			IDescriptable* GuiToolstripButton::QueryService(const WString& identifier)
			{
				if (identifier == IToolstripUpdateLayoutInvoker::Identifier)
				{
					return (IToolstripUpdateLayoutInvoker*)this;
				}
				else
				{
					return GuiMenuButton::QueryService(identifier);
				}
			}

/***********************************************************************
GuiToolstripNestedContainer
***********************************************************************/

			void GuiToolstripNestedContainer::UpdateLayout()
			{
				if (callback)
				{
					callback->UpdateLayout();
				}
			}

			void GuiToolstripNestedContainer::SetCallback(IToolstripUpdateLayout* _callback)
			{
				callback = _callback;
			}

			GuiToolstripNestedContainer::GuiToolstripNestedContainer(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
			}

			GuiToolstripNestedContainer::~GuiToolstripNestedContainer()
			{
			}

			IDescriptable* GuiToolstripNestedContainer::QueryService(const WString& identifier)
			{
				if (identifier == IToolstripUpdateLayoutInvoker::Identifier)
				{
					return (IToolstripUpdateLayoutInvoker*)this;
				}
				else
				{
					return GuiControl::QueryService(identifier);
				}
			}

/***********************************************************************
GuiToolstripGroupContainer::GroupCollection
***********************************************************************/

			void GuiToolstripGroupContainer::GroupCollection::BeforeRemove(vint index, GuiControl* const& child)
			{
				auto controlStackItem = container->stackComposition->GetStackItems()[index * 2];
				CHECK_ERROR(controlStackItem->RemoveChild(child->GetBoundsComposition()), L"GuiToolstripGroupContainer::GroupCollection::BeforeRemove(vint, GuiControl* const&)#Internal error");
			}

			void GuiToolstripGroupContainer::GroupCollection::AfterInsert(vint index, GuiControl* const& child)
			{
				auto controlStackItem = new GuiStackItemComposition;
				child->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				CHECK_ERROR(controlStackItem->AddChild(child->GetBoundsComposition()), L"GuiToolstripGroupContainer::GroupCollection::AfterInsert(vint, GuiControl* const&)#Internal error");

				if (container->stackComposition->GetStackItems().Count() > 0)
				{
					auto splitterStackItem = new GuiStackItemComposition;
					auto splitter = new GuiControl(container->splitterThemeName);
					if (splitterTemplate)
					{
						splitter->SetControlTemplate(splitterTemplate);
					}
					splitter->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					splitterStackItem->AddChild(splitter->GetBoundsComposition());
					container->stackComposition->InsertStackItem(index == 0 ? 0 : index * 2 - 1, splitterStackItem);
				}

				container->stackComposition->InsertStackItem(index * 2, controlStackItem);

				GuiToolstripCollectionBase::AfterInsert(index, child);
			}

			void GuiToolstripGroupContainer::GroupCollection::AfterRemove(vint index, vint count)
			{
				vint min = index * 2;
				vint max = (index + count - 1) * 2;
				for (vint i = min; i <= max; i++)
				{
					auto stackItem = container->stackComposition->GetStackItems()[min];
					container->stackComposition->RemoveChild(stackItem);
					SafeDeleteComposition(stackItem);
				}
				GuiToolstripCollectionBase::AfterRemove(index, count);
			}

			GuiToolstripGroupContainer::GroupCollection::GroupCollection(GuiToolstripGroupContainer* _container)
				:GuiToolstripCollectionBase(_container)
				, container(_container)
			{
			}

			GuiToolstripGroupContainer::GroupCollection::~GroupCollection()
			{
			}

			GuiToolstripGroupContainer::ControlTemplatePropertyType GuiToolstripGroupContainer::GroupCollection::GetSplitterTemplate()
			{
				return splitterTemplate;
			}

			void GuiToolstripGroupContainer::GroupCollection::SetSplitterTemplate(const ControlTemplatePropertyType& value)
			{
				splitterTemplate = value;
				RebuildSplitters();
			}

			void GuiToolstripGroupContainer::GroupCollection::RebuildSplitters()
			{
				auto stack = container->stackComposition;
				vint count = stack->GetStackItems().Count();
				for (vint i = 1; i < count; i += 2)
				{
					auto stackItem = stack->GetStackItems()[i];
					{
						auto control = stackItem->Children()[0]->GetAssociatedControl();
						CHECK_ERROR(control != nullptr, L"GuiToolstripGroupContainer::GroupCollection::RebuildSplitters()#Internal error");
						stackItem->RemoveChild(control->GetBoundsComposition());
						delete control;
					}
					{
						auto control = new GuiControl(container->splitterThemeName);
						if (splitterTemplate)
						{
							control->SetControlTemplate(splitterTemplate);
						}
						control->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						stackItem->AddChild(control->GetBoundsComposition());
					}
				}
			}

/***********************************************************************
GuiToolstripGroupContainer
***********************************************************************/

			void GuiToolstripGroupContainer::OnParentLineChanged()
			{
				auto direction = GuiStackComposition::Horizontal;
				if (auto service = QueryTypedService<IGuiMenuService>())
				{
					if (service->GetPreferredDirection() == IGuiMenuService::Vertical)
					{
						direction = GuiStackComposition::Vertical;
					}
				}

				if (direction != stackComposition->GetDirection())
				{
					if (direction == GuiStackComposition::Vertical)
					{
						splitterThemeName = theme::ThemeName::MenuSplitter;
					}
					else
					{
						splitterThemeName = theme::ThemeName::ToolstripSplitter;
					}

					stackComposition->SetDirection(direction);
					splitterThemeName = splitterThemeName;
					groupCollection->RebuildSplitters();
					UpdateLayout();
				}

				GuiControl::OnParentLineChanged();
			}

			GuiToolstripGroupContainer::GuiToolstripGroupContainer(theme::ThemeName themeName)
				:GuiToolstripNestedContainer(themeName)
				, splitterThemeName(theme::ThemeName::ToolstripSplitter)
			{
				stackComposition = new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Horizontal);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				containerComposition->AddChild(stackComposition);

				groupCollection = new GroupCollection(this);
			}

			GuiToolstripGroupContainer::~GuiToolstripGroupContainer()
			{
			}

			GuiToolstripGroupContainer::ControlTemplatePropertyType GuiToolstripGroupContainer::GetSplitterTemplate()
			{
				return groupCollection->GetSplitterTemplate();
			}

			void GuiToolstripGroupContainer::SetSplitterTemplate(const ControlTemplatePropertyType& value)
			{
				groupCollection->SetSplitterTemplate(value);
			}

			collections::ObservableListBase<GuiControl*>& GuiToolstripGroupContainer::GetToolstripItems()
			{
				return *groupCollection.Obj();
			}

/***********************************************************************
GuiToolstripGroup
***********************************************************************/

			void GuiToolstripGroup::OnParentLineChanged()
			{
				auto direction = GuiStackComposition::Horizontal;
				if (auto service = QueryTypedService<IGuiMenuService>())
				{
					if (service->GetPreferredDirection() == IGuiMenuService::Vertical)
					{
						direction = GuiStackComposition::Vertical;
					}
				}

				if (direction != stackComposition->GetDirection())
				{
					stackComposition->SetDirection(direction);
					UpdateLayout();
				}

				GuiControl::OnParentLineChanged();
			}

			GuiToolstripGroup::GuiToolstripGroup(theme::ThemeName themeName)
				:GuiToolstripNestedContainer(themeName)
			{
				stackComposition = new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Horizontal);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				containerComposition->AddChild(stackComposition);

				toolstripItems = new GuiToolstripCollection(nullptr, stackComposition);
			}

			GuiToolstripGroup::~GuiToolstripGroup()
			{
			}

			collections::ObservableListBase<GuiControl*>& GuiToolstripGroup::GetToolstripItems()
			{
				return *toolstripItems.Obj();
			}
		}
	}
}