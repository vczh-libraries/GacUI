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
				stackItem->RemoveChild(child->GetBoundsComposition());

				GuiToolstripCollectionBase::BeforeRemove(index, child);
				delete stackItem;
				delete child;
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
		}
	}
}