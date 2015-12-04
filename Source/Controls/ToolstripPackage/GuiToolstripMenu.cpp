#include "GuiToolstripMenu.h"
#include "../Styles/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace compositions;

/***********************************************************************
GuiToolstripCollection
***********************************************************************/

			void GuiToolstripCollection::InvokeUpdateLayout()
			{
				if(contentCallback)
				{
					contentCallback->UpdateLayout();
				}
			}

			void GuiToolstripCollection::OnInterestingMenuButtonPropertyChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				InvokeUpdateLayout();
			}

			bool GuiToolstripCollection::QueryInsert(vint index, GuiControl* const& child)
			{
				return true;
			}

			bool GuiToolstripCollection::QueryRemove(vint index, GuiControl* const& child)
			{
				return true;
			}

			void GuiToolstripCollection::BeforeInsert(vint index, GuiControl* const& child)
			{
			}

			void GuiToolstripCollection::BeforeRemove(vint index, GuiControl* const& child)
			{
				GuiStackItemComposition* stackItem = stackComposition->GetStackItems().Get(index);
				stackComposition->RemoveChild(stackItem);
				stackItem->RemoveChild(child->GetBoundsComposition());
				delete stackItem;
				delete child;
				InvokeUpdateLayout();
			}

			void GuiToolstripCollection::AfterInsert(vint index, GuiControl* const& child)
			{
				GuiStackItemComposition* stackItem=new GuiStackItemComposition;
				child->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackItem->AddChild(child->GetBoundsComposition());
				stackComposition->InsertChild(index, stackItem);

				GuiMenuButton* menuButton=dynamic_cast<GuiMenuButton*>(child);
				if(menuButton)
				{
					menuButton->TextChanged.AttachMethod(this, &GuiToolstripCollection::OnInterestingMenuButtonPropertyChanged);
					menuButton->ShortcutTextChanged.AttachMethod(this, &GuiToolstripCollection::OnInterestingMenuButtonPropertyChanged);
				}
				InvokeUpdateLayout();
			}

			void GuiToolstripCollection::AfterRemove(vint index, vint count)
			{
				InvokeUpdateLayout();
			}

			GuiToolstripCollection::GuiToolstripCollection(IContentCallback* _contentCallback, compositions::GuiStackComposition* _stackComposition)
				:contentCallback(_contentCallback)
				,stackComposition(_stackComposition)
			{
			}

			GuiToolstripCollection::~GuiToolstripCollection()
			{
			}

/***********************************************************************
GuiToolstripBuilder
***********************************************************************/

			GuiToolstripBuilder::GuiToolstripBuilder(Environment _environment, GuiToolstripCollection* _toolstripItems)
				:environment(_environment)
				,toolstripItems(_toolstripItems)
				,previousBuilder(0)
				,theme(0)
				,lastCreatedButton(0)
			{
			}

			GuiToolstripBuilder::~GuiToolstripBuilder()
			{
			}

			GuiToolstripBuilder* GuiToolstripBuilder::Button(Ptr<GuiImageData> image, const WString& text, GuiToolstripButton** result)
			{
				lastCreatedButton=0;
				switch(environment)
				{
				case Menu:
					lastCreatedButton=new GuiToolstripButton(theme->CreateMenuItemButtonStyle());
					break;
				case MenuBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateMenuBarButtonStyle());
					break;
				case ToolBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateToolBarButtonStyle());
					break;
				}
				if(lastCreatedButton)
				{
					lastCreatedButton->SetImage(image);
					lastCreatedButton->SetText(text);
					if(result)
					{
						*result=lastCreatedButton;
					}
					toolstripItems->Add(lastCreatedButton);
				}
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::Button(GuiToolstripCommand* command, GuiToolstripButton** result)
			{
				lastCreatedButton=0;
				switch(environment)
				{
				case Menu:
					lastCreatedButton=new GuiToolstripButton(theme->CreateMenuItemButtonStyle());
					break;
				case MenuBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateMenuBarButtonStyle());
					break;
				case ToolBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateToolBarButtonStyle());
					break;
				}
				if(lastCreatedButton)
				{
					lastCreatedButton->SetCommand(command);
					if(result)
					{
						*result=lastCreatedButton;
					}
					toolstripItems->Add(lastCreatedButton);
				}
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::DropdownButton(Ptr<GuiImageData> image, const WString& text, GuiToolstripButton** result)
			{
				lastCreatedButton=0;
				switch(environment)
				{
				case ToolBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateToolBarDropdownButtonStyle());
					break;
				default:;
				}
				if(lastCreatedButton)
				{
					lastCreatedButton->SetImage(image);
					lastCreatedButton->SetText(text);
					if(result)
					{
						*result=lastCreatedButton;
					}
					toolstripItems->Add(lastCreatedButton);
				}
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::DropdownButton(GuiToolstripCommand* command, GuiToolstripButton** result)
			{
				lastCreatedButton=0;
				switch(environment)
				{
				case ToolBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateToolBarDropdownButtonStyle());
					break;
				default:;
				}
				if(lastCreatedButton)
				{
					lastCreatedButton->SetCommand(command);
					if(result)
					{
						*result=lastCreatedButton;
					}
					toolstripItems->Add(lastCreatedButton);
				}
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::SplitButton(Ptr<GuiImageData> image, const WString& text, GuiToolstripButton** result)
			{
				lastCreatedButton=0;
				switch(environment)
				{
				case ToolBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateToolBarSplitButtonStyle());
					break;
				default:;
				}
				if(lastCreatedButton)
				{
					lastCreatedButton->SetImage(image);
					lastCreatedButton->SetText(text);
					if(result)
					{
						*result=lastCreatedButton;
					}
					toolstripItems->Add(lastCreatedButton);
				}
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::SplitButton(GuiToolstripCommand* command, GuiToolstripButton** result)
			{
				lastCreatedButton=0;
				switch(environment)
				{
				case ToolBar:
					lastCreatedButton=new GuiToolstripButton(theme->CreateToolBarSplitButtonStyle());
					break;
				default:;
				}
				if(lastCreatedButton)
				{
					lastCreatedButton->SetCommand(command);
					if(result)
					{
						*result=lastCreatedButton;
					}
					toolstripItems->Add(lastCreatedButton);
				}
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::Splitter()
			{
				lastCreatedButton=0;
				switch(environment)
				{
				case Menu:
					toolstripItems->Add(new GuiControl(theme->CreateMenuSplitterStyle()));
					break;
				case ToolBar:
					toolstripItems->Add(new GuiControl(theme->CreateToolBarSplitterStyle()));
					break;
				default:;
				}
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::Control(GuiControl* control)
			{
				toolstripItems->Add(control);
				return this;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::BeginSubMenu()
			{
				if(lastCreatedButton)
				{
					lastCreatedButton->CreateToolstripSubMenu();
					GuiToolstripMenu* menu=lastCreatedButton->GetToolstripSubMenu();
					if(menu)
					{
						menu->GetBuilder()->previousBuilder=this;
						return menu->GetBuilder();
					}
				}
				return 0;
			}

			GuiToolstripBuilder* GuiToolstripBuilder::EndSubMenu()
			{
				return previousBuilder;
			}

/***********************************************************************
GuiToolstripMenu
***********************************************************************/

			void GuiToolstripMenu::UpdateLayout()
			{
				sharedSizeRootComposition->ForceCalculateSizeImmediately();
			}

			GuiToolstripMenu::GuiToolstripMenu(IStyleController* _styleController, GuiControl* _owner)
				:GuiMenu(_styleController, _owner)
			{
				sharedSizeRootComposition = new GuiSharedSizeRootComposition();
				sharedSizeRootComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				sharedSizeRootComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				GetContainerComposition()->AddChild(sharedSizeRootComposition);

				stackComposition=new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Vertical);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				sharedSizeRootComposition->AddChild(stackComposition);
				
				toolstripItems = new GuiToolstripCollection(this, stackComposition);
				builder = new GuiToolstripBuilder(GuiToolstripBuilder::Menu, toolstripItems.Obj());
			}

			GuiToolstripMenu::~GuiToolstripMenu()
			{
			}

			GuiToolstripCollection& GuiToolstripMenu::GetToolstripItems()
			{
				return *toolstripItems.Obj();
			}

			GuiToolstripBuilder* GuiToolstripMenu::GetBuilder(theme::ITheme* themeObject)
			{
				builder->theme=themeObject?themeObject:theme::GetCurrentTheme();
				return builder.Obj();
			}

/***********************************************************************
GuiToolstripMenuBar
***********************************************************************/
			
			GuiToolstripMenuBar::GuiToolstripMenuBar(IStyleController* _styleController)
				:GuiMenuBar(_styleController)
			{
				stackComposition=new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Horizontal);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				GetContainerComposition()->AddChild(stackComposition);

				toolstripItems=new GuiToolstripCollection(0, stackComposition);
				builder=new GuiToolstripBuilder(GuiToolstripBuilder::MenuBar, toolstripItems.Obj());
			}

			GuiToolstripMenuBar::~GuiToolstripMenuBar()
			{
			}

			GuiToolstripCollection& GuiToolstripMenuBar::GetToolstripItems()
			{
				return *toolstripItems.Obj();
			}

			GuiToolstripBuilder* GuiToolstripMenuBar::GetBuilder(theme::ITheme* themeObject)
			{
				builder->theme=themeObject?themeObject:theme::GetCurrentTheme();
				return builder.Obj();
			}

/***********************************************************************
GuiToolstripToolBar
***********************************************************************/
				
			GuiToolstripToolBar::GuiToolstripToolBar(IStyleController* _styleController)
				:GuiControl(_styleController)
			{
				stackComposition=new GuiStackComposition;
				stackComposition->SetDirection(GuiStackComposition::Horizontal);
				stackComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stackComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				GetContainerComposition()->AddChild(stackComposition);

				toolstripItems=new GuiToolstripCollection(0, stackComposition);
				builder=new GuiToolstripBuilder(GuiToolstripBuilder::ToolBar, toolstripItems.Obj());
			}

			GuiToolstripToolBar::~GuiToolstripToolBar()
			{
			}

			GuiToolstripCollection& GuiToolstripToolBar::GetToolstripItems()
			{
				return *toolstripItems.Obj();
			}

			GuiToolstripBuilder* GuiToolstripToolBar::GetBuilder(theme::ITheme* themeObject)
			{
				builder->theme=themeObject?themeObject:theme::GetCurrentTheme();
				return builder.Obj();
			}

/***********************************************************************
GuiToolstripButton
***********************************************************************/

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

			GuiToolstripButton::GuiToolstripButton(IStyleController* _styleController)
				:GuiMenuButton(_styleController)
				,command(0)
			{
				Clicked.AttachMethod(this, &GuiToolstripButton::OnClicked);
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
					CreateToolstripSubMenu();
				}
				return dynamic_cast<GuiToolstripMenu*>(GetSubMenu());
			}

			void GuiToolstripButton::CreateToolstripSubMenu(GuiToolstripMenu::IStyleController* subMenuStyleController)
			{
				if(!subMenu)
				{
					GuiToolstripMenu* newSubMenu=new GuiToolstripMenu(subMenuStyleController?subMenuStyleController:styleController->CreateSubMenuStyleController(), this);
					SetSubMenu(newSubMenu, true);
				}
			}
		}
	}
}