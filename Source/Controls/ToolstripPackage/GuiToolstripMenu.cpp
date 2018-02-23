#include "GuiToolstripMenu.h"
#include "../Templates/GuiThemeStyleFactory.h"

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

			void GuiToolstripCollectionBase::OnInterestingMenuButtonPropertyChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				InvokeUpdateLayout();
			}

			bool GuiToolstripCollectionBase::QueryInsert(vint index, GuiControl* const& child)
			{
				return !items.Contains(child);
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

			void GuiToolstripCollection::BeforeRemove(vint index, GuiControl* const& child)
			{
				GuiStackItemComposition* stackItem = stackComposition->GetStackItems().Get(index);
				stackComposition->RemoveChild(stackItem);

				GuiToolstripCollectionBase::BeforeRemove(index, child);
				SafeDeleteComposition(stackItem);
			}

			void GuiToolstripCollection::AfterInsert(vint index, GuiControl* const& child)
			{
				GuiStackItemComposition* stackItem=new GuiStackItemComposition;
				child->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackItem->AddChild(child->GetBoundsComposition());
				stackComposition->InsertChild(index, stackItem);

				GuiToolstripCollectionBase::AfterInsert(index, child);
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

			void GuiToolstripButton::UpdateCommandContent()
			{
				if(command)
				{
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
					SetImage(0);
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
						newSubMenu->SetControlTemplate(GetControlTemplateObject()->GetSubMenuTemplate());
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
				auto splitterStackItem =
					container->stackComposition->GetStackItems().Count() == 1 ? nullptr :
					index == 0 ? container->stackComposition->GetStackItems()[1] :
					container->stackComposition->GetStackItems()[index * 2 - 1]
					;

				container->stackComposition->RemoveChild(controlStackItem);
				if (splitterStackItem) container->stackComposition->RemoveChild(splitterStackItem);

				GuiToolstripCollectionBase::BeforeRemove(index, child);

				SafeDeleteComposition(controlStackItem);
				SafeDeleteComposition(splitterStackItem);
			}

			void GuiToolstripGroupContainer::GroupCollection::AfterInsert(vint index, GuiControl* const& child)
			{
				auto controlStackItem = new GuiStackItemComposition;
				child->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				controlStackItem->AddChild(child->GetBoundsComposition());
				container->stackComposition->InsertChild(index * 2, controlStackItem);

				if (container->stackComposition->GetStackItems().Count() > 1)
				{
					auto splitterStackItem = new GuiStackItemComposition;
					auto splitter = new GuiControl(container->splitterThemeName);
					if (splitterTemplate)
					{
						splitter->SetControlTemplate(splitterTemplate);
					}
					splitter->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					splitterStackItem->AddChild(splitter->GetBoundsComposition());
					container->stackComposition->InsertChild(index == 0 ? 1 : index * 2 - 1, splitterStackItem);
				}

				GuiToolstripCollectionBase::AfterInsert(index, child);
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