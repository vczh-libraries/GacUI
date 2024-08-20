#include "GuiTreeViewControls.h"
#include "GuiListControlItemArrangers.h"
#include "../GuiButtonControls.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace reflection::description;

/***********************************************************************
GuiVirtualTreeListControl
***********************************************************************/

			void GuiVirtualTreeListControl::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiVirtualTreeListControl::AfterControlTemplateInstalled_(bool initialize)
			{
			}

			void GuiVirtualTreeListControl::OnAttached(tree::INodeRootProvider* provider)
			{
			}

			void GuiVirtualTreeListControl::OnBeforeItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
			}

			void GuiVirtualTreeListControl::OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
			}

			void GuiVirtualTreeListControl::OnItemExpanded(tree::INodeProvider* node)
			{
				GuiNodeEventArgs arguments;
				(GuiEventArgs&)arguments=GetNotifyEventArguments();
				arguments.node=node;
				NodeExpanded.Execute(arguments);
			}

			void GuiVirtualTreeListControl::OnItemCollapsed(tree::INodeProvider* node)
			{
				GuiNodeEventArgs arguments;
				(GuiEventArgs&)arguments=GetNotifyEventArguments();
				arguments.node=node;
				NodeCollapsed.Execute(arguments);
			}

			vint GuiVirtualTreeListControl::FindItemByVirtualKeyDirection(vint index, compositions::KeyDirection keyDirection)
			{
				vint newIndex = GuiSelectableListControl::FindItemByVirtualKeyDirection(index, keyDirection);
				if (newIndex != -1) return newIndex;

				auto selectedNode = nodeItemView->RequestNode(index);
				if (selectedNode)
				{
					bool hasChildren = selectedNode->GetChildCount() > 0;
					bool expanding = selectedNode->GetExpanding();
					switch (keyDirection)
					{
					case KeyDirection::Right:
						if (hasChildren)
						{
							if (expanding)
							{
								selectedNode = selectedNode->GetChild(0);
							}
							else
							{
								selectedNode->SetExpanding(true);
							}
						}
						break;
					case KeyDirection::Left:
						{
							selectedNode->SetExpanding(false);
							if (!expanding || !hasChildren)
							{
								selectedNode = selectedNode->GetParent();
							}
						}
						break;
					default:;
					}
				}

				return selectedNode ? nodeItemView->CalculateNodeVisibilityIndex(selectedNode.Obj()) : -1;
			}

			void GuiVirtualTreeListControl::OnItemMouseEvent(compositions::GuiNodeMouseEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments)
			{
				auto node = GetNodeItemView()->RequestNode(arguments.itemIndex);
				if (node)
				{
					GuiNodeMouseEventArgs redirectArguments;
					(GuiMouseEventArgs&)redirectArguments = arguments;
					redirectArguments.node = node.Obj();
					nodeEvent.Execute(redirectArguments);
					(GuiMouseEventArgs&)arguments = redirectArguments;
				}
			}

			void GuiVirtualTreeListControl::OnItemNotifyEvent(compositions::GuiNodeNotifyEvent& nodeEvent, compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				if (auto node = GetNodeItemView()->RequestNode(arguments.itemIndex))
				{
					GuiNodeEventArgs redirectArguments;
					(GuiEventArgs&)redirectArguments = arguments;
					redirectArguments.node = node.Obj();
					nodeEvent.Execute(redirectArguments);
					(GuiEventArgs&)arguments = redirectArguments;
				}
			}

#define ATTACH_ITEM_MOUSE_EVENT(NODEEVENTNAME, ITEMEVENTNAME)\
					{\
						ITEMEVENTNAME.AttachFunction([this](GuiGraphicsComposition* sender, GuiItemMouseEventArgs& args){ OnItemMouseEvent(NODEEVENTNAME, sender, args); });\
					}\

#define ATTACH_ITEM_NOTIFY_EVENT(NODEEVENTNAME, ITEMEVENTNAME)\
					{\
						ITEMEVENTNAME.AttachFunction([this](GuiGraphicsComposition* sender, GuiItemEventArgs& args){ OnItemNotifyEvent(NODEEVENTNAME, sender, args); });\
					}\

			void GuiVirtualTreeListControl::OnNodeLeftButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiNodeMouseEventArgs& arguments)
			{
				if (arguments.node->GetChildCount() > 0)
				{
					arguments.node->SetExpanding(!arguments.node->GetExpanding());
				}
			}

			GuiVirtualTreeListControl::GuiVirtualTreeListControl(theme::ThemeName themeName, Ptr<tree::INodeRootProvider> _nodeRootProvider)
				:GuiSelectableListControl(themeName, new tree::NodeItemProvider(_nodeRootProvider))
			{
				nodeItemProvider = dynamic_cast<tree::NodeItemProvider*>(GetItemProvider());
				nodeItemView = dynamic_cast<tree::INodeItemView*>(GetItemProvider()->RequestView(WString::Unmanaged(tree::INodeItemView::Identifier)));

				NodeLeftButtonDown.SetAssociatedComposition(boundsComposition);
				NodeLeftButtonUp.SetAssociatedComposition(boundsComposition);
				NodeLeftButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				NodeMiddleButtonDown.SetAssociatedComposition(boundsComposition);
				NodeMiddleButtonUp.SetAssociatedComposition(boundsComposition);
				NodeMiddleButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				NodeRightButtonDown.SetAssociatedComposition(boundsComposition);
				NodeRightButtonUp.SetAssociatedComposition(boundsComposition);
				NodeRightButtonDoubleClick.SetAssociatedComposition(boundsComposition);
				NodeMouseMove.SetAssociatedComposition(boundsComposition);
				NodeMouseEnter.SetAssociatedComposition(boundsComposition);
				NodeMouseLeave.SetAssociatedComposition(boundsComposition);
				NodeExpanded.SetAssociatedComposition(boundsComposition);
				NodeCollapsed.SetAssociatedComposition(boundsComposition);

				ATTACH_ITEM_MOUSE_EVENT(NodeLeftButtonDown, ItemLeftButtonDown);
				ATTACH_ITEM_MOUSE_EVENT(NodeLeftButtonUp, ItemLeftButtonUp);
				ATTACH_ITEM_MOUSE_EVENT(NodeLeftButtonDoubleClick, ItemLeftButtonDoubleClick);
				ATTACH_ITEM_MOUSE_EVENT(NodeMiddleButtonDown, ItemMiddleButtonDown);
				ATTACH_ITEM_MOUSE_EVENT(NodeMiddleButtonUp, ItemMiddleButtonUp);
				ATTACH_ITEM_MOUSE_EVENT(NodeMiddleButtonDoubleClick, ItemMiddleButtonDoubleClick);
				ATTACH_ITEM_MOUSE_EVENT(NodeRightButtonDown, ItemRightButtonDown);
				ATTACH_ITEM_MOUSE_EVENT(NodeRightButtonUp, ItemRightButtonUp);
				ATTACH_ITEM_MOUSE_EVENT(NodeRightButtonDoubleClick, ItemRightButtonDoubleClick);
				ATTACH_ITEM_MOUSE_EVENT(NodeMouseMove, ItemMouseMove);
				ATTACH_ITEM_NOTIFY_EVENT(NodeMouseEnter, ItemMouseEnter);
				ATTACH_ITEM_NOTIFY_EVENT(NodeMouseLeave, ItemMouseLeave);

				nodeItemProvider->GetRoot()->AttachCallback(this);
				NodeLeftButtonDoubleClick.AttachMethod(this, &GuiVirtualTreeListControl::OnNodeLeftButtonDoubleClick);
			}

#undef ATTACH_ITEM_MOUSE_EVENT
#undef ATTACH_ITEM_NOTIFY_EVENT

			GuiVirtualTreeListControl::~GuiVirtualTreeListControl()
			{
			}

			tree::INodeItemView* GuiVirtualTreeListControl::GetNodeItemView()
			{
				return nodeItemView;
			}

			tree::INodeRootProvider* GuiVirtualTreeListControl::GetNodeRootProvider()
			{
				return nodeItemProvider->GetRoot().Obj();
			}

			namespace tree
			{

/***********************************************************************
TreeViewItem
***********************************************************************/

				const wchar_t* const ITreeViewItemView::Identifier = L"vl::presentation::controls::tree::ITreeViewItemView";

				TreeViewItem::TreeViewItem()
				{
				}

				TreeViewItem::TreeViewItem(const Ptr<GuiImageData>& _image, const WString& _text)
					:image(_image)
					,text(_text)
				{
				}

/***********************************************************************
TreeViewItemRootProvider
***********************************************************************/

				Ptr<GuiImageData> TreeViewItemRootProvider::GetNodeImage(INodeProvider* node)
				{
					MemoryNodeProvider* memoryNode=dynamic_cast<MemoryNodeProvider*>(node);
					if(memoryNode)
					{
						Ptr<TreeViewItem> data=memoryNode->GetData().Cast<TreeViewItem>();
						if(data)
						{
							return data->image;
						}
					}
					return 0;
				}

				WString TreeViewItemRootProvider::GetTextValue(INodeProvider* node)
				{
					MemoryNodeProvider* memoryNode = dynamic_cast<MemoryNodeProvider*>(node);
					if (memoryNode)
					{
						Ptr<TreeViewItem> data = memoryNode->GetData().Cast<TreeViewItem>();
						if (data)
						{
							return data->text;
						}
					}
					return L"";
				}

				description::Value TreeViewItemRootProvider::GetBindingValue(INodeProvider* node)
				{
					return Value::From(GetTreeViewData(node));
				}

				TreeViewItemRootProvider::TreeViewItemRootProvider()
				{
				}

				TreeViewItemRootProvider::~TreeViewItemRootProvider()
				{
				}

				IDescriptable* TreeViewItemRootProvider::RequestView(const WString& identifier)
				{
					if(identifier==ITreeViewItemView::Identifier)
					{
						return (ITreeViewItemView*)this;
					}
					else
					{
						return MemoryNodeRootProvider::RequestView(identifier);
					}
				}

				Ptr<TreeViewItem> TreeViewItemRootProvider::GetTreeViewData(INodeProvider* node)
				{
					MemoryNodeProvider* memoryNode=GetMemoryNode(node);
					if(memoryNode)
					{
						return memoryNode->GetData().Cast<TreeViewItem>();
					}
					else
					{
						return 0;
					}
				}

				void TreeViewItemRootProvider::SetTreeViewData(INodeProvider* node, Ptr<TreeViewItem> value)
				{
					MemoryNodeProvider* memoryNode=GetMemoryNode(node);
					if(memoryNode)
					{
						memoryNode->SetData(value);
					}
				}
				
				void TreeViewItemRootProvider::UpdateTreeViewData(INodeProvider* node)
				{
					MemoryNodeProvider* memoryNode=GetMemoryNode(node);
					if(memoryNode)
					{
						memoryNode->NotifyDataModified();
					}
				}
			}

/***********************************************************************
GuiVirtualTreeView
***********************************************************************/

			templates::GuiTreeItemTemplate* GuiVirtualTreeView::GetStyleFromNode(tree::INodeProvider* node)
			{
				if (itemArranger)
				{
					vint index = nodeItemView->CalculateNodeVisibilityIndex(node);
					if (index != -1)
					{
						auto style = itemArranger->GetVisibleStyle(index);
						return dynamic_cast<templates::GuiTreeItemTemplate*>(style);
					}
				}
				return nullptr;
			}

			void GuiVirtualTreeView::SetStyleExpanding(tree::INodeProvider* node, bool expanding)
			{
				if (auto treeItemStyle = GetStyleFromNode(node))
				{
					treeItemStyle->SetExpanding(expanding);
				}
			}

			void GuiVirtualTreeView::SetStyleExpandable(tree::INodeProvider* node, bool expandable)
			{
				if (auto treeItemStyle = GetStyleFromNode(node))
				{
					treeItemStyle->SetExpandable(expandable);
				}
			}

			void GuiVirtualTreeView::OnAfterItemModified(tree::INodeProvider* parentNode, vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
				GuiVirtualTreeListControl::OnAfterItemModified(parentNode, start, count, newCount, itemReferenceUpdated);
				SetStyleExpandable(parentNode, parentNode->GetChildCount() > 0);
			}

			void GuiVirtualTreeView::OnItemExpanded(tree::INodeProvider* node)
			{
				GuiVirtualTreeListControl::OnItemExpanded(node);
				SetStyleExpanding(node, true);
			}

			void GuiVirtualTreeView::OnItemCollapsed(tree::INodeProvider* node)
			{
				GuiVirtualTreeListControl::OnItemCollapsed(node);
				SetStyleExpanding(node, false);
			}
			
			void GuiVirtualTreeView::OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)
			{
				GuiVirtualTreeListControl::OnStyleInstalled(itemIndex, style, refreshPropertiesOnly);
				if (auto textItemStyle = dynamic_cast<templates::GuiTextListItemTemplate*>(style))
				{
					textItemStyle->SetTextColor(TypedControlTemplateObject(true)->GetTextColor());
				}
				if (auto treeItemStyle = dynamic_cast<templates::GuiTreeItemTemplate*>(style))
				{
					treeItemStyle->SetTextColor(TypedControlTemplateObject(true)->GetTextColor());

					if (treeViewItemView)
					{
						if (auto node = nodeItemView->RequestNode(itemIndex))
						{
							treeItemStyle->SetImage(treeViewItemView->GetNodeImage(node.Obj()));
							treeItemStyle->SetExpanding(node->GetExpanding());
							treeItemStyle->SetExpandable(node->GetChildCount() > 0);
							{
								vint level = -1;
								auto current = node;
								while (current->GetParent())
								{
									level++;
									current = current->GetParent();
								}
								treeItemStyle->SetLevel(level);
							}
						}
					}
				}
				if (refreshPropertiesOnly)
				{
					if (auto predefinedItemStyle = dynamic_cast<tree::DefaultTreeItemTemplate*>(style))
					{
						predefinedItemStyle->RefreshItem();
					}
				}
			}

			GuiVirtualTreeView::GuiVirtualTreeView(theme::ThemeName themeName, Ptr<tree::INodeRootProvider> _nodeRootProvider)
				:GuiVirtualTreeListControl(themeName, _nodeRootProvider)
			{
				treeViewItemView = dynamic_cast<tree::ITreeViewItemView*>(GetNodeRootProvider()->RequestView(WString::Unmanaged(tree::ITreeViewItemView::Identifier)));
				SetStyleAndArranger(
					[](const Value&) { return new tree::DefaultTreeItemTemplate; },
					Ptr(new list::FixedHeightItemArranger)
				);
			}

			GuiVirtualTreeView::~GuiVirtualTreeView()
			{
			}

/***********************************************************************
GuiTreeView
***********************************************************************/

			GuiTreeView::GuiTreeView(theme::ThemeName themeName)
				:GuiVirtualTreeView(themeName, Ptr(new tree::TreeViewItemRootProvider))
			{
				nodes = nodeItemProvider->GetRoot().Cast<tree::TreeViewItemRootProvider>();
			}

			GuiTreeView::~GuiTreeView()
			{
			}

			Ptr<tree::TreeViewItemRootProvider> GuiTreeView::Nodes()
			{
				return nodes;
			}

			Ptr<tree::TreeViewItem> GuiTreeView::GetSelectedItem()
			{
				Ptr<tree::TreeViewItem> result;
				vint index = GetSelectedItemIndex();
				if (index != -1)
				{
					if (auto node = nodeItemView->RequestNode(index))
					{
						if (auto memoryNode = node.Cast<tree::MemoryNodeProvider>())
						{
							result = memoryNode->GetData().Cast<tree::TreeViewItem>();
						}
					}
				}
				return result;
			}

			namespace tree
			{

/***********************************************************************
DefaultTreeItemTemplate
***********************************************************************/

				void DefaultTreeItemTemplate::OnInitialize()
				{
					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					table = new GuiTableComposition;
					AddChild(table);
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(3, 4);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());
					table->SetColumnOption(2, GuiCellOption::MinSizeOption());
					table->SetColumnOption(3, GuiCellOption::MinSizeOption());
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetCellPadding(2);
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 3, 1);
						cell->SetPreferredMinSize(Size(16, 16));

						expandingButton = new GuiSelectableButton(theme::ThemeName::TreeItemExpander);
						if (auto treeView = dynamic_cast<GuiVirtualTreeView*>(listControl))
						{
							if (auto expanderStyle = treeView->TypedControlTemplateObject(true)->GetExpandingDecoratorTemplate())
							{
								expandingButton->SetControlTemplate(expanderStyle);
							}
						}
						expandingButton->SetAutoFocus(false);
						expandingButton->SetAutoSelection(false);
						expandingButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						expandingButton->GetBoundsComposition()->GetEventReceiver()->leftButtonDoubleClick.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingButtonDoubleClick);
						expandingButton->Clicked.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingButtonClicked);
						cell->AddChild(expandingButton->GetBoundsComposition());
					}
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 2, 1, 1);
						cell->SetPreferredMinSize(Size(16, 16));

						imageElement = GuiImageFrameElement::Create();
						imageElement->SetStretch(true);
						cell->SetOwnedElement(Ptr(imageElement));
					}
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 3, 3, 1);
						cell->SetPreferredMinSize(Size(192, 0));

						textElement = GuiSolidLabelElement::Create();
						textElement->SetAlignments(Alignment::Left, Alignment::Center);
						textElement->SetEllipse(true);
						cell->SetOwnedElement(Ptr(textElement));
					}

					FontChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnFontChanged);
					TextChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnTextChanged);
					TextColorChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnTextColorChanged);
					ExpandingChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingChanged);
					ExpandableChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandableChanged);
					LevelChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnLevelChanged);
					ImageChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnImageChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
					TextChanged.Execute(compositions::GuiEventArgs(this));
					TextColorChanged.Execute(compositions::GuiEventArgs(this));
					ExpandingChanged.Execute(compositions::GuiEventArgs(this));
					ExpandableChanged.Execute(compositions::GuiEventArgs(this));
					LevelChanged.Execute(compositions::GuiEventArgs(this));
					ImageChanged.Execute(compositions::GuiEventArgs(this));
				}

				void DefaultTreeItemTemplate::OnRefresh()
				{
				}

				void DefaultTreeItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetFont(GetFont());
				}

				void DefaultTreeItemTemplate::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetText(GetText());
				}

				void DefaultTreeItemTemplate::OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetColor(GetTextColor());
				}

				void DefaultTreeItemTemplate::OnExpandingChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					expandingButton->SetSelected(GetExpanding());
				}

				void DefaultTreeItemTemplate::OnExpandableChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					expandingButton->SetVisible(GetExpandable());
				}

				void DefaultTreeItemTemplate::OnLevelChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					table->SetColumnOption(0, GuiCellOption::AbsoluteOption(GetLevel() * 12));
				}

				void DefaultTreeItemTemplate::OnImageChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if (auto imageData = GetImage())
					{
						imageElement->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						imageElement->SetImage(nullptr);
					}
				}

				void DefaultTreeItemTemplate::OnExpandingButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					arguments.handled = true;
				}

				void DefaultTreeItemTemplate::OnExpandingButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if (expandingButton->GetVisuallyEnabled())
					{
						if (auto treeControl = dynamic_cast<GuiVirtualTreeListControl*>(listControl))
						{
							if (auto view = treeControl->GetNodeItemView())
							{
								vint index = treeControl->GetArranger()->GetVisibleIndex(this);
								if (index != -1)
								{
									if (auto node = view->RequestNode(index))
									{
										bool expanding = node->GetExpanding();
										node->SetExpanding(!expanding);
									}
								}
							}
						}
					}
				}

				DefaultTreeItemTemplate::DefaultTreeItemTemplate()
				{
				}

				DefaultTreeItemTemplate::~DefaultTreeItemTemplate()
				{
				}
			}
		}
	}
}