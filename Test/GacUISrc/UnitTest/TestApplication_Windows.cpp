#include "TestControls.h"

using namespace gacui_unittest_template;

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
