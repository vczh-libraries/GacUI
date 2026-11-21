#if defined _MSC_VER
#include "TestApplication_Theme_Windows.h"
#endif
#include "TestControls.h"

using namespace gacui_unittest_template;

#if defined VCZH_MSVC
class HostedThemeAutomationProbe : public windows::WindowsUIAutomationContext
{
public:
	collections::List<vint> invalidations;

	HostedThemeAutomationProbe() : WindowsUIAutomationContext(true) {}

	void RaiseChildrenInvalidated(Ptr<windows::WindowsUIAutomationNode> node) override
	{
		invalidations.Add(node->id);
	}
};

template<typename T>
ComPtr<T> HostedThemeInterface(const ComPtr<IRawElementProviderSimple>& provider)
{
	T* result = nullptr;
	TEST_ASSERT(provider->QueryInterface(IID_PPV_ARGS(&result)) == S_OK);
	return ComPtr(result);
}
#endif

TEST_FILE
{
	const auto resourceEmptyWindows = LR"GacUISrc(
<Resource>
  <Instance name="SubWindowResource">
    <Instance ref.Class="gacuisrc_unittest::SubWindow">
      <Window ref.Name="self" Text="SubWindow" ClientSize="x:200 y:200">
        <Label Text-format="This is $(self.Text)">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </Label>
      </Window>
    </Instance>
  </Instance>

  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="WindowManagement" ClientSize="x:480 y:320">
        <Label Text="This is the main window">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:-1 bottom:-1"/>
        </Label>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Windows")
	{
#if defined VCZH_MSVC
		TEST_CASE(L"UI Automation isolates a hosted submenu theme refresh from its siblings")
		{
			theme_observation_tests::Run<HostedThemeAutomationProbe>(true, [](Ptr<HostedThemeAutomationProbe> probe)
			{
				auto skin = Ptr(new darkskin::Theme);
				theme::RegisterTheme(skin);
				{
					GuiWindow main(theme::ThemeName::Window);
					main.SetClientSize(Size(640, 400));
					auto opener = new GuiMenuButton(theme::ThemeName::MenuBarButton);
					opener->SetText(L"Menu");
					opener->SetPreferredMenuClientSize(Size(160, 100));
					opener->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 90, 45));
					main.AddChild(opener);
					auto sibling = new GuiButton(theme::ThemeName::Button);
					sibling->SetText(L"Sibling");
					sibling->GetBoundsComposition()->SetExpectedBounds(Rect(400, 150, 540, 200));
					main.AddChild(sibling);
					auto menu = opener->CreateSubMenu();
					auto item = new GuiMenuButton(theme::ThemeName::MenuItemButton);
					item->SetText(L"Menu child");
					item->SetPreferredMenuClientSize(Size(130, 80));
					item->GetBoundsComposition()->SetExpectedBounds(Rect(5, 5, 145, 55));
					menu->AddChild(item);
					auto nested = item->CreateSubMenu();
					auto command = new GuiButton(theme::ThemeName::Button);
					command->SetText(L"Nested command");
					command->GetBoundsComposition()->SetExpectedBounds(Rect(3, 3, 120, 45));
					nested->AddChild(command);
					auto hiddenOpener = new GuiMenuButton(theme::ThemeName::MenuItemButton);
					hiddenOpener->GetBoundsComposition()->SetExpectedBounds(Rect(5, 60, 145, 90));
					menu->AddChild(hiddenOpener);
					auto hiddenMenu = hiddenOpener->CreateSubMenu();
					auto hiddenCommand = new GuiButton(theme::ThemeName::Button);
					hiddenMenu->AddChild(hiddenCommand);
					GetApplication()->InvokeInMainThread(&main, [&]()
					{
						main.ForceCalculateSizeImmediately();
						opener->SetSubMenuOpening(true);
						TEST_ASSERT(menu->GetOpening());
						menu->ForceCalculateSizeImmediately();
						item->SetSubMenuOpening(true);
						TEST_ASSERT(nested->GetOpening());
						nested->ForceCalculateSizeImmediately();
						probe->BindWindows();
						auto rootProvider = probe->Control(&main)->Provider();
						auto root = HostedThemeInterface<IRawElementProviderFragmentRoot>(rootProvider);
						auto mainFragment = HostedThemeInterface<IRawElementProviderFragment>(rootProvider);
						auto advise = HostedThemeInterface<IRawElementProviderAdviseEvents>(rootProvider);
						TEST_ASSERT(advise->AdviseEventAdded(UIA_StructureChangedEventId, nullptr) == S_OK);
						auto menuNode = probe->Control(menu);
						auto menuFragment = HostedThemeInterface<IRawElementProviderFragment>(menuNode->Provider());
						auto siblingFragment = HostedThemeInterface<IRawElementProviderFragment>(probe->Control(sibling)->Provider());
						auto itemFragment = HostedThemeInterface<IRawElementProviderFragment>(probe->Control(item)->Provider());
						auto ownerFragment = HostedThemeInterface<IRawElementProviderFragment>(probe->Control(opener)->Provider());
						auto nestedFragment = HostedThemeInterface<IRawElementProviderFragment>(probe->Control(nested)->Provider());
						auto commandProvider = probe->Control(command)->Provider();
						auto commandFragment = HostedThemeInterface<IRawElementProviderFragment>(commandProvider);
						auto invoke = HostedThemeInterface<IInvokeProvider>(commandProvider);
						vint invoked = 0;
						command->Clicked.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { invoked++; });
						auto navigate = [](IRawElementProviderFragment* provider, NavigateDirection direction)
						{
							IRawElementProviderFragment* result = nullptr;
							TEST_ASSERT(provider->Navigate(direction, &result) == S_OK);
							return ComPtr(result);
						};
						auto hit = [&](const UiaRect& bounds)
						{
							TEST_ASSERT(bounds.width > 0 && bounds.height > 0);
							IRawElementProviderFragment* result = nullptr;
							TEST_ASSERT(root->ElementProviderFromPoint(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2, &result) == S_OK);
							return ComPtr(result);
						};
						auto drain = [&]()
						{
							bool drained = false;
							probe->dispatcher->Queue([&]() { drained = true; });
							MSG message;
							while (PeekMessage(&message, nullptr, WM_APP, 0xBFFF, PM_REMOVE)) DispatchMessage(&message);
							TEST_ASSERT(drained);
						};
						UiaRect itemBounds{}, siblingBounds{}, commandBounds{};
						TEST_ASSERT(itemFragment->get_BoundingRectangle(&itemBounds) == S_OK);
						TEST_ASSERT(siblingFragment->get_BoundingRectangle(&siblingBounds) == S_OK);
						TEST_ASSERT(commandFragment->get_BoundingRectangle(&commandBounds) == S_OK);
						TEST_ASSERT(hit(itemBounds) == itemFragment);
						TEST_ASSERT(hit(siblingBounds) == siblingFragment);
						TEST_ASSERT(hit(commandBounds) == commandFragment);
						TEST_ASSERT(navigate(menuFragment.Obj(), NavigateDirection_Parent) == ownerFragment);
						drain();
						probe->invalidations.Clear();
						auto hiddenNode = probe->Control(hiddenMenu);
						auto hiddenCommandNode = probe->Control(hiddenCommand);
						vint hiddenEdits = 0;
						auto hiddenUpdates = hiddenMenu->ChildCompositionUpdated.AttachLambda([&](auto, auto&)
						{
							TEST_ASSERT(!hiddenMenu->GetOpening());
							TEST_ASSERT(hiddenNode->attachment->ClosedWindow() == menu);
							hiddenEdits++;
						});
						auto before = menu->BeforeRebuildingTheme.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
						{
							TEST_ASSERT(!navigate(menuFragment.Obj(), NavigateDirection_FirstChild));
							TEST_ASSERT(navigate(siblingFragment.Obj(), NavigateDirection_Parent) == mainFragment);
							TEST_ASSERT(hit(itemBounds) == menuFragment);
							TEST_ASSERT(hit(siblingBounds) == siblingFragment);
							TEST_ASSERT(!hit(commandBounds));
							UiaRect bounds{};
							TEST_ASSERT(nestedFragment->get_BoundingRectangle(&bounds) == UIA_E_ELEMENTNOTAVAILABLE);
							TEST_ASSERT(commandFragment->get_BoundingRectangle(&bounds) == UIA_E_ELEMENTNOTAVAILABLE);
							TEST_ASSERT(invoke->Invoke() == UIA_E_ELEMENTNOTAVAILABLE);
							for (auto child = navigate(mainFragment.Obj(), NavigateDirection_FirstChild); child; child = navigate(child.Obj(), NavigateDirection_NextSibling))
								TEST_ASSERT(child != nestedFragment);
						});
						TEST_ASSERT(invoke->Invoke() == S_OK);
						menu->RefreshThemes();
						menu->BeforeRebuildingTheme.Detach(before);
						hiddenMenu->ChildCompositionUpdated.Detach(hiddenUpdates);
						drain();
						TEST_ASSERT(hiddenEdits > 0 && !hiddenNode->attachment->Closed());
						TEST_ASSERT(probe->Control(hiddenCommand) == hiddenCommandNode);
						TEST_ASSERT(invoked == 0);
						TEST_ASSERT(commandFragment->get_BoundingRectangle(&commandBounds) == S_OK);
						for (auto id : probe->invalidations) TEST_ASSERT(id == menuNode->id);
						TEST_ASSERT(probe->invalidations.Count() == 2);
						probe->invalidations.Clear();
						TEST_ASSERT(!hiddenNode->ExistingProvider());
						hiddenMenu->RefreshThemes();
						TEST_ASSERT(probe->invalidations.Count() == 0 && !hiddenNode->ExistingProvider());
						TEST_ASSERT(navigate(menuFragment.Obj(), NavigateDirection_FirstChild));
						opener->SetSubMenuOpening(false);
						opener->SetSubMenuOpening(true);
						TEST_ASSERT(menu->GetOpening());
						TEST_ASSERT(navigate(menuFragment.Obj(), NavigateDirection_Parent) == ownerFragment);
						TEST_ASSERT(advise->AdviseEventRemoved(UIA_StructureChangedEventId, nullptr) == S_OK);
						menuFragment = nullptr;
						itemFragment = nullptr;
						nestedFragment = nullptr;
						drain();
						item->SetSubMenuOpening(true);
						TEST_ASSERT(nested->GetOpening());
						probe->Collect();
						menuNode = probe->Control(menu);
						TEST_ASSERT(!menuNode->ExistingProvider());
						TEST_ASSERT(invoke->Invoke() == S_OK);
						menu->RefreshThemes();
						drain();
						TEST_ASSERT(invoked == 0);
						TEST_ASSERT(invoke->Invoke() == S_OK);
						drain();
						TEST_ASSERT(invoked == 1);
						TEST_ASSERT(advise->AdviseEventAdded(UIA_StructureChangedEventId, nullptr) == S_OK);
						menuFragment = HostedThemeInterface<IRawElementProviderFragment>(menuNode->Provider());
						itemFragment = HostedThemeInterface<IRawElementProviderFragment>(probe->Control(item)->Provider());
						opener->SetSubMenuOpening(false);
						drain();
						probe->invalidations.Clear();
						probe->Notify(menuNode, true);
						opener->DestroySubMenu();
						drain();
						TEST_ASSERT(!probe->invalidations.Contains(menuNode->id));
						TEST_ASSERT(menuFragment->get_BoundingRectangle(&itemBounds) == UIA_E_ELEMENTNOTAVAILABLE);
						TEST_ASSERT(itemFragment->get_BoundingRectangle(&itemBounds) == UIA_E_ELEMENTNOTAVAILABLE);
						TEST_ASSERT(hit(siblingBounds) == siblingFragment);
						TEST_ASSERT(advise->AdviseEventRemoved(UIA_StructureChangedEventId, nullptr) == S_OK);
						main.Hide();
					});
					GetApplication()->Run(&main);
				}
				theme::UnregisterTheme(skin->Name);
			});
		});
#endif

		for (auto closeOwner : { false,true })
		TEST_CASE(closeOwner ? L"Close popup owner" : L"Hide popup owner")
		{
			GuiWindow* owner = nullptr;
			GuiPopup* popup = nullptr;
			GuiPopup* nested = nullptr;
			GuiPopup* unrelated = nullptr;
			vint order = 0;
			bool cancel = false;
			bool modalCompleted = false;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto show = [&](vint mode)
				{
					order = 0;
					if (mode == 3) owner->ShowModal(GetApplication()->GetMainWindow(), [&]() { modalCompleted = true; });
					else owner->Show();
					auto control = FindControlByText<GuiControl>(owner, L"This is SubWindow");
					switch (mode)
					{
					case 0: popup->ShowPopup(control, Rect(0, 0, 100, 20), true); break;
					case 1: popup->ShowPopup(control, Point(30, 30)); break;
					case 2: popup->ShowPopup(control, false); break;
					case 3: popup->ShowPopup(owner, Point(30, 50)); break;
					}
					nested->ShowPopup(popup, Point(30, 30));
				};
				protocol->OnNextIdleFrame(L"Ready", [&, show]()
				{
					owner = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					owner->SetLocation({ {90},{70} });
					auto createPopup = [](const WString& caption)
					{
						auto result = new GuiPopup(theme::ThemeName::Window);
						auto label = new GuiLabel(theme::ThemeName::Label);
						label->SetText(caption);
						result->AddChild(label);
						result->SetClientSize(Size(150, 45));
						return result;
					};
					popup = createPopup(L"Owned popup");
					nested = createPopup(L"Nested popup");
					popup->WindowClosing.AttachLambda([](GuiGraphicsComposition*, GuiRequestEventArgs& arguments) { arguments.cancel = true; });
					nested->WindowClosing.AttachLambda([](GuiGraphicsComposition*, GuiRequestEventArgs& arguments) { arguments.cancel = true; });
					unrelated = createPopup(L"Main window popup");
					unrelated->ShowPopup(GetApplication()->GetMainWindow(), Point(300, 250));
					nested->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { TEST_ASSERT(order++ == 0); });
					popup->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						TEST_ASSERT(order++ == 1 && !nested->GetOpening());
					});
					owner->WindowClosing.AttachLambda([&](GuiGraphicsComposition*, GuiRequestEventArgs& arguments) { arguments.cancel = cancel; });
					owner->WindowReadyToClose.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&)
					{
						TEST_ASSERT(order == 2 && !popup->GetOpening() && !nested->GetOpening());
						TEST_ASSERT(unrelated->GetOpening());
					});
					owner->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { TEST_ASSERT(order++ == 2); });
					show(0);
				});
				for (vint mode = 0; mode < 4; mode++)
				{
					protocol->OnNextIdleFrame(L"Owned and nested popups visible " + itow(mode), [&, mode]()
					{
						if (mode == 0)
						{
							cancel = true;
							owner->Close();
							TEST_ASSERT(owner->GetOpening() && popup->GetOpening() && nested->GetOpening() && order == 0);
							cancel = false;
							popup->Close();
							TEST_ASSERT(popup->GetOpening() && nested->GetOpening() && order == 0);
						}
						if (closeOwner) owner->Close(); else owner->Hide();
					});
					protocol->OnNextIdleFrame(L"Only main owner popup survives " + itow(mode), [&, mode, show]()
					{
						TEST_ASSERT(order == 3 && !owner->GetOpening());
						if (mode < 3) show(mode + 1);
						else
						{
							TEST_ASSERT(modalCompleted && GetApplication()->GetMainWindow()->GetEnabled());
							SafeDeleteControl(nested);
							SafeDeleteControl(popup);
							SafeDeleteControl(owner);
							SafeDeleteControl(unrelated);
							GetApplication()->GetMainWindow()->Hide();
						}
					});
				}
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				closeOwner ? L"Application/Windows/ClosePopupOwner" : L"Application/Windows/HidePopupOwner",
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"), resourceEmptyWindows);
		});

		TEST_CASE(L"Popup owner reassignment and destruction")
		{
			GuiWindow* first = nullptr;
			GuiWindow* second = nullptr;
			GuiPopup* popup = nullptr;
			vint closed = 0;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&]()
				{
					first = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					second = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					first->SetLocation({ {50},{40} });
					second->SetLocation({ {230},{100} });
					first->Show();
					second->Show();
					popup = new GuiPopup(theme::ThemeName::Window);
					auto label = new GuiLabel(theme::ThemeName::Label);
					label->SetText(L"Reused popup");
					popup->AddChild(label);
					popup->SetClientSize(Size(150, 45));
					popup->WindowClosed.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { closed++; });
					popup->ShowPopup(first, Point(20, 40));
				});
				protocol->OnNextIdleFrame(L"Popup owned by first window", [&]()
				{
					popup->ShowPopup(second, Point(20, 40));
					first->Close();
					TEST_ASSERT(popup->GetOpening() && closed == 0);
				});
				protocol->OnNextIdleFrame(L"Popup follows second window", [&]()
				{
					popup->ShowPopup(NativePoint({300}, {220}));
					second->Hide();
					TEST_ASSERT(popup->GetOpening() && closed == 0);
				});
				protocol->OnNextIdleFrame(L"Unowned popup survives former owners", [&]()
				{
					first->Show();
					popup->ShowPopup(first, Point(20, 40));
				});
				protocol->OnNextIdleFrame(L"Popup reassigned before owner destruction", [&]()
				{
					delete first;
					first = nullptr;
					TEST_ASSERT(!popup->GetOpening() && closed == 1);
				});
				protocol->OnNextIdleFrame(L"Destroyed owner leaves no popup", [&]()
				{
					second->Show();
					popup->ShowPopup(second, Point(20, 40));
				});
				protocol->OnNextIdleFrame(L"Popup reused after owner destruction", [&]()
				{
					delete popup;
					popup = nullptr;
					second->Close();
				});
				protocol->OnNextIdleFrame(L"Popup deleted before its former owner", [&]()
				{
					TEST_ASSERT(!second->GetOpening() && closed == 1);
					SafeDeleteControl(second);
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/PopupOwnerReuse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"), resourceEmptyWindows);
		});

		TEST_CASE(L"Composition updates report completed direct mutations")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto other = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					auto parent = new GuiBoundsComposition;
					auto first = new GuiBoundsComposition;
					auto second = new GuiBoundsComposition;
					auto grandchild = new GuiBoundsComposition;
					collections::List<CompositionUpdateType> updates;
					vint reflected = 0;
					vint otherUpdates = 0;
					auto handler = window->ChildCompositionUpdated.AttachLambda([&](GuiGraphicsComposition* sender, GuiCompositionUpdateEventArgs& arguments)
					{
						TEST_ASSERT(sender == window->GetBoundsComposition());
						TEST_ASSERT(arguments.compositionSource == sender);
						TEST_ASSERT(arguments.parent);
						TEST_ASSERT(arguments.child);
						if (arguments.updateType == CompositionUpdateType::Removed)
						{
							TEST_ASSERT(!arguments.parent->Children().Contains(arguments.child));
							TEST_ASSERT(!arguments.child->GetParent());
							TEST_ASSERT(!arguments.child->GetRelatedGraphicsHost());
						}
						else
						{
							TEST_ASSERT(arguments.parent->Children().Contains(arguments.child));
							TEST_ASSERT(arguments.child->GetParent() == arguments.parent);
							TEST_ASSERT(arguments.child->GetRelatedControlHost() == window);
						}
						updates.Add(arguments.updateType);
					});
					Func<void(GuiGraphicsComposition*, GuiCompositionUpdateEventArgs*)> callback = [&](GuiGraphicsComposition*, GuiCompositionUpdateEventArgs* arguments)
					{
						TEST_ASSERT(arguments->parent);
						reflected++;
					};
					auto boxed = BoxValue(window);
					auto reflectedHandler = boxed.AttachEvent(L"ChildCompositionUpdated", BoxParameter(callback));
					other->ChildCompositionUpdated.AttachLambda([&](GuiGraphicsComposition*, GuiCompositionUpdateEventArgs& arguments)
					{
						TEST_ASSERT(arguments.parent == other->GetContainerComposition());
						TEST_ASSERT(arguments.child == parent);
						otherUpdates++;
					});

					TEST_ASSERT(first->AddChild(grandchild));
					TEST_ASSERT(parent->AddChild(first));
					TEST_ASSERT(parent->AddChild(second));
					TEST_ASSERT(parent->MoveChild(second, 0));
					TEST_ASSERT(updates.Count() == 0);
					TEST_ASSERT(window->GetContainerComposition()->AddChild(parent));
					TEST_ASSERT(updates.Count() == 1);
					TEST_ASSERT(updates[0] == CompositionUpdateType::Inserted);
					TEST_ASSERT(!parent->AddChild(first));
					TEST_ASSERT(!parent->AddChild(nullptr));
					TEST_ASSERT(!parent->RemoveChild(grandchild));
					TEST_ASSERT(!parent->MoveChild(grandchild, 0));
					TEST_ASSERT(parent->MoveChild(second, 0));
					TEST_ASSERT(updates.Count() == 1);
					TEST_ASSERT(parent->MoveChild(first, 0));
					TEST_ASSERT(parent->Children()[0] == first);
					TEST_ASSERT(updates.Count() == 2 && updates[1] == CompositionUpdateType::Moved);
					TEST_ASSERT(parent->RemoveChild(first));
					TEST_ASSERT(updates.Count() == 3 && updates[2] == CompositionUpdateType::Removed);
					TEST_ASSERT(parent->InsertChild(0, first));
					TEST_ASSERT(updates.Count() == 4 && updates[3] == CompositionUpdateType::Inserted);
					TEST_ASSERT(window->GetContainerComposition()->RemoveChild(parent));
					TEST_ASSERT(updates.Count() == 5 && updates[4] == CompositionUpdateType::Removed);
					TEST_ASSERT(other->GetContainerComposition()->AddChild(parent));
					TEST_ASSERT(otherUpdates == 1);
					TEST_ASSERT(other->GetContainerComposition()->RemoveChild(parent));
					TEST_ASSERT(otherUpdates == 2);
					TEST_ASSERT(window->GetContainerComposition()->AddChild(parent));
					TEST_ASSERT(updates.Count() == 6 && reflected == 6);
					TEST_ASSERT(boxed.DetachEvent(L"ChildCompositionUpdated", reflectedHandler));
					TEST_ASSERT(window->ChildCompositionUpdated.Detach(handler));
					SafeDeleteControl(other);
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/CompositionUpdates"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});

		GuiWindow* subWindowA = nullptr;
		GuiWindow* subWindowB = nullptr;

		auto createTwoWindows = [&](UnitTestRemoteProtocol* protocol)
		{
			protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
			{
				{
					subWindowA = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					subWindowA->SetText(L"SubWindow A");
					subWindowA->SetLocation({ {100},{50} });
					subWindowA->Show();
				}
				{
					subWindowB = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					subWindowB->SetText(L"SubWindow B");
					subWindowB->SetLocation({ {200},{100} });
					subWindowB->Show();
				}
			});
		};

		auto deleteTwoWindows = [&](const WString& name, UnitTestRemoteProtocol* protocol)
		{
			protocol->OnNextIdleFrame(name, [&, protocol]()
			{
				subWindowA->Hide();
				SafeDeleteControl(subWindowA);
				subWindowA = nullptr;

				subWindowB->Hide();
				SafeDeleteControl(subWindowB);
				subWindowB = nullptr;

				auto window = GetApplication()->GetMainWindow();
				window->Hide();
			});
		};

		TEST_CASE(L"Order")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				createTwoWindows(protocol);
				protocol->OnNextIdleFrame(L"Created two SubWindows", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.0, 0.0, 10, 10);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Click SubWindowA", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowB, 1.0, 1.0, -10, -10);
					protocol->LClick(location);
				});
				deleteTwoWindows(L"Click SubWindowB", protocol);
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/Order"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});

		TEST_CASE(L"Enabling")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				createTwoWindows(protocol);
				protocol->OnNextIdleFrame(L"Created two SubWindows", [&, protocol]()
				{
					subWindowA->SetEnabled(false);
					auto location = protocol->LocationOf(subWindowA, 0.0, 0.0, 10, 10);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Disable and Click SubWIndowA", [&, protocol]()
				{
					subWindowA->SetEnabled(true);
					auto location = protocol->LocationOf(subWindowA, 0.0, 0.0, 10, 10);
					protocol->LClick(location);
				});
				deleteTwoWindows(L"Enable and Click SubWindowA", protocol);
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/Enabling"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});

		TEST_CASE(L"Closing")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				createTwoWindows(protocol);
				protocol->OnNextIdleFrame(L"Created two SubWindows", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 1.0, 0.0, -15, 15);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Close SubWindowA", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowB, 1.0, 0.0, -15, 15);
					protocol->LClick(location);
				});
				deleteTwoWindows(L"Close SubWindowB", protocol);
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/Closing"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});

		TEST_CASE(L"Dragging")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				createTwoWindows(protocol);
				protocol->OnNextIdleFrame(L"Created two SubWindows", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.2, 0.0, 0, 15);
					protocol->_LDown(location);
				});
				protocol->OnNextIdleFrame(L"Begin Dragging", [&, protocol]()
				{
					auto location = protocol->GetMousePosition();
					location.x.value -= 50;
					location.y.value -= 25;
					protocol->MouseMove(location);
					protocol->_LUp();
				});
				deleteTwoWindows(L"End Dragging", protocol);
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/Dragging"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});

		TEST_CASE(L"Resizing")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				createTwoWindows(protocol);
				protocol->OnNextIdleFrame(L"Created two SubWindows", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.5, 0.0, 0, 3);
					protocol->_LDown(location);
					location.y.value -= 10;
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Top Border Up", [&, protocol]()
				{
					auto location = protocol->GetMousePosition();
					location.y.value += 9999;
					protocol->MouseMove(location);
					protocol->_LUp();
				});
				protocol->OnNextIdleFrame(L"Top Border Down", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.5, 1.0, 0, -3);
					protocol->_LDown(location);
					location.y.value += 10;
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Bottom Border Down", [&, protocol]()
				{
					auto location = protocol->GetMousePosition();
					location.y.value -= 9999;
					protocol->MouseMove(location);
					protocol->_LUp();
				});
				protocol->OnNextIdleFrame(L"Bottom Border Up", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.0, 0.5, 3, 0);
					protocol->_LDown(location);
					location.x.value -= 10;
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Left Border Left", [&, protocol]()
				{
					auto location = protocol->GetMousePosition();
					location.x.value += 9999;
					protocol->MouseMove(location);
					protocol->_LUp();
				});
				protocol->OnNextIdleFrame(L"Left Border Right", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 1.0, 0.5, -3, 0);
					protocol->_LDown(location);
					location.x.value += 10;
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Right Border Right", [&, protocol]()
				{
					auto location = protocol->GetMousePosition();
					location.x.value -= 9999;
					protocol->MouseMove(location);
					protocol->_LUp();
				});
				deleteTwoWindows(L"Right Border Left", protocol);
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/Resizing"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});

		TEST_CASE(L"ShowModal")
		{
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					subWindowA = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					subWindowA->SetText(L"SubWindow A");
					subWindowA->SetLocation({ {100},{50} });
					subWindowA->Show();
				});
				protocol->OnNextIdleFrame(L"Create SubWindowA", [&, protocol]()
				{
					subWindowB = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
					subWindowB->SetText(L"SubWindow B");
					subWindowB->SetLocation({ {200},{100} });
					subWindowB->ShowModal(subWindowA, []()
					{
						auto window = GetApplication()->GetMainWindow();
						window->SetText(L"SubWindowB Closed");
					});
				});
				protocol->OnNextIdleFrame(L"Create SubWindowB via ShowModal", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.2, 0.0, 0, 15);
					protocol->_LDown(location);
					location.x.value -= 50;
					location.y.value -= 25;
					protocol->MouseMove(location);
					protocol->_LUp();
					subWindowA->SetText(L"DraggedA");
				});
				protocol->OnNextIdleFrame(L"Drag SubWindowA", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowB, 0.2, 0.0, 0, 15);
					protocol->_LDown(location);
					location.x.value -= 50;
					location.y.value -= 25;
					protocol->MouseMove(location);
					protocol->_LUp();
					subWindowB->SetText(L"DraggedB");
				});
				protocol->OnNextIdleFrame(L"Drag SubWindowB", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowB, 1.0, 0.0, -15, 15);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Close SubWindowB", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 0.2, 0.0, 0, 15);
					protocol->_LDown(location);
					location.x.value -= 50;
					location.y.value -= 25;
					protocol->MouseMove(location);
					protocol->_LUp();
					subWindowA->SetText(L"Dragged Again");
				});
				protocol->OnNextIdleFrame(L"Drag SubWindowA", [&, protocol]()
				{
					auto location = protocol->LocationOf(subWindowA, 1.0, 0.0, -15, 15);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Close SubWindowA", [&, protocol]()
				{
					subWindowA->Hide();
					SafeDeleteControl(subWindowA);
					subWindowA = nullptr;

					subWindowB->Hide();
					SafeDeleteControl(subWindowB);
					subWindowB = nullptr;

					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Windows/ShowModal"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceEmptyWindows
			);
		});
	});
}
