#include "TestControls.h"

using namespace gacui_unittest_template;
using namespace vl::collections;

namespace editable_metadata_tests
{
	void CollectTags(Ptr<remoteprotocol::RenderingDom> dom, List<Ptr<remoteprotocol::RenderingDom>>& tags)
	{
		if (dom->content.editableAttribute)
		{
			TEST_ASSERT(dom->id % 4 == 2);
			tags.Add(dom);
		}
		if (dom->children)
		{
			for (auto child : *dom->children.Obj()) CollectTags(child, tags);
		}
	}

	vint FocusedTag(UnitTestRemoteProtocol* protocol, vint expectedCount)
	{
		List<Ptr<remoteprotocol::RenderingDom>> tags;
		CollectTags(protocol->GetCompletedRenderingFrame()->renderingDom, tags);
		TEST_ASSERT(tags.Count() == expectedCount);
		vint focused = -1;
		for (auto tag : tags)
		{
			if (tag->content.editableAttribute.Value().focused)
			{
				TEST_ASSERT(focused == -1);
				focused = tag->id;
			}
		}
		return focused;
	}

	GuiControl* FindControl(const WString& name)
	{
		return FindObjectByName<GuiControl>(GetApplication()->GetMainWindow(), name);
	}

	void AssertStored(GuiControl* control, bool focused, bool editable)
	{
		auto value = control->GetFocusableComposition()->GetEditableAttributes();
		TEST_ASSERT(value.featured);
		TEST_ASSERT(value.focused == focused);
		TEST_ASSERT(value.editable == editable);
	}

	struct ExpectedEditors
	{
		vint focused = -1;
		bool aEditable = true;
		bool bEditable = true;
		bool bVisible = true;
		bool readOnlyVisible = true;
	};

	void AssertEditors(UnitTestRemoteProtocol* protocol, const Dictionary<WString, vint>& ids, ExpectedEditors expected)
	{
		List<Ptr<remoteprotocol::RenderingDom>> tags;
		CollectTags(protocol->GetCompletedRenderingFrame()->renderingDom, tags);
		TEST_ASSERT(tags.Count() == 1 + (expected.bVisible ? 1 : 0) + (expected.readOnlyVisible ? 1 : 0));
		TEST_ASSERT(FocusedTag(protocol, tags.Count()) == expected.focused);
		for (WString name : { L"a", L"b", L"readOnly" })
		{
			bool visible = name == L"a" || (name == L"b" ? expected.bVisible : expected.readOnlyVisible);
			vint count = 0;
			for (auto tag : tags)
			{
				if (tag->id != ids[name]) continue;
				count++;
				auto value = tag->content.editableAttribute.Value();
				TEST_ASSERT(value.focused == (tag->id == expected.focused));
				TEST_ASSERT(value.editable == (name == L"a" ? expected.aEditable : name == L"b" && expected.bEditable));
			}
			TEST_ASSERT(count == (visible ? 1 : 0));
		}
	}
}

TEST_FILE
{
	using namespace editable_metadata_tests;

	const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Editing metadata" ClientSize="x:560 y:360">
        <SinglelineTextBox ref.Name="a" Text="Alpha">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:10 right:10 bottom:-1"/>
        </SinglelineTextBox>
        <CustomControl ref.Name="parent">
          <att.BoundsComposition-set ExpectedBounds="x1:10 y1:55 x2:550 y2:205" MinSizeLimitation="NoLimit"/>
          <MultilineTextBox ref.Name="b" Text="Beta">
            <att.BoundsComposition-set ExpectedBounds="x1:0 y1:0 x2:520 y2:130"/>
          </MultilineTextBox>
        </CustomControl>
        <DocumentLabel ref.Name="readOnly" Text="Read only">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:225 right:10 bottom:-1"/>
        </DocumentLabel>
        <Button ref.Name="button" Text="Non-text focus">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:280 right:-1 bottom:-1"/>
        </Button>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	for (auto useDiff : { false,true })
	TEST_CASE(useDiff ? L"Editor metadata through synchronous JSON and DOM diffs" : L"Editor metadata through raw rendering commands")
	{
		TooltipTimer timer;
		Dictionary<WString, vint> ids;
		WString documentText;
		Point scrollPosition;
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
			{
				AssertStored(FindControl(L"a"), false, true);
				AssertStored(FindControl(L"b"), false, true);
				AssertStored(FindControl(L"readOnly"), false, false);
				TEST_ASSERT(FocusedTag(protocol, 3) == -1);
				FindControl(L"a")->SetFocused();
				AssertStored(FindControl(L"a"), true, true);
			});
			protocol->OnNextIdleFrame(L"Focused A", [&, protocol]()
			{
				ids.Add(L"a", FocusedTag(protocol, 3));
				TEST_ASSERT(ids[L"a"] != -1);
				FindControl(L"b")->SetFocused();
				AssertStored(FindControl(L"a"), false, true);
				AssertStored(FindControl(L"b"), true, true);
			});
			protocol->OnNextIdleFrame(L"Focused B", [&, protocol]()
			{
				ids.Add(L"b", FocusedTag(protocol, 3));
				TEST_ASSERT(ids[L"b"] != -1 && ids[L"b"] != ids[L"a"]);
				FindControl(L"a")->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Focused A again", [&, protocol]()
			{
				TEST_ASSERT(FocusedTag(protocol, 3) == ids[L"a"]);
				FindControl(L"readOnly")->SetFocused();
				AssertStored(FindControl(L"readOnly"), true, false);
			});
			protocol->OnNextIdleFrame(L"Focused default ViewOnly label", [&, protocol]()
			{
				ids.Add(L"readOnly", FocusedTag(protocol, 3));
				TEST_ASSERT(ids[L"readOnly"] != -1 && ids[L"readOnly"] != ids[L"a"] && ids[L"readOnly"] != ids[L"b"]);
				AssertEditors(protocol, ids, { .focused = ids[L"readOnly"] });
				FindControl(L"button")->SetFocused();
				TEST_ASSERT(!FindControl(L"button")->GetFocusableComposition()->GetEditableAttributes().featured);
			});
			protocol->OnNextIdleFrame(L"Focused non-text button", [&, protocol]()
			{
				AssertEditors(protocol, ids, {});
				FindControl(L"a")->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Focused editable A", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"] });
				dynamic_cast<GuiDocumentCommonInterface*>(FindControl(L"a"))->SetEditMode(GuiDocumentEditMode::Selectable);
				AssertStored(FindControl(L"a"), true, false);
			});
			protocol->OnNextIdleFrame(L"A became Selectable without other input", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .aEditable = false });
				TEST_ASSERT(FindControl(L"a")->GetText() == L"Alpha");
				dynamic_cast<GuiDocumentCommonInterface*>(FindControl(L"a"))->SetEditMode(GuiDocumentEditMode::Editable);
			});
			protocol->OnNextIdleFrame(L"A became Editable again", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"] });
				dynamic_cast<GuiDocumentCommonInterface*>(FindControl(L"a"))->SetEditMode(GuiDocumentEditMode::ViewOnly);
			});
			protocol->OnNextIdleFrame(L"A became ViewOnly", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .aEditable = false });
				dynamic_cast<GuiDocumentCommonInterface*>(FindControl(L"a"))->SetEditMode(GuiDocumentEditMode::Editable);
				FindControl(L"a")->SetEnabled(false);
				AssertStored(FindControl(L"a"), false, false);
				FindControl(L"b")->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Disabled A and focused B", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"b"], .aEditable = false });
				FindControl(L"a")->SetEnabled(true);
				AssertStored(FindControl(L"a"), false, true);
			});
			protocol->OnNextIdleFrame(L"Re-enabled A without stealing focus", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"b"] });
				FindControl(L"parent")->SetEnabled(false);
				TEST_ASSERT(FindControl(L"b")->GetEnabled() && !FindControl(L"b")->GetVisuallyEnabled());
				AssertStored(FindControl(L"b"), false, false);
				FindControl(L"a")->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Inherited disablement of B", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .bEditable = false });
				dynamic_cast<GuiDocumentCommonInterface*>(FindControl(L"b"))->SetEditMode(GuiDocumentEditMode::Selectable);
				AssertStored(FindControl(L"b"), false, false);
				FindControl(L"parent")->SetEnabled(true);
				AssertStored(FindControl(L"b"), false, false);
			});
			protocol->OnNextIdleFrame(L"Parent re-enabled with B Selectable", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .bEditable = false });
				dynamic_cast<GuiDocumentCommonInterface*>(FindControl(L"b"))->SetEditMode(GuiDocumentEditMode::Editable);
			});
			protocol->OnNextIdleFrame(L"Unfocused B became editable", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"] });
				FindControl(L"b")->SetVisible(false);
			});
			protocol->OnNextIdleFrame(L"B hidden", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .bVisible = false });
				FindControl(L"b")->SetVisible(true);
			});
			protocol->OnNextIdleFrame(L"B shown with retained owner ID", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"] });
				FindControl(L"parent")->SetVisible(false);
			});
			protocol->OnNextIdleFrame(L"B hidden by its ancestor", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .bVisible = false });
				FindControl(L"parent")->SetVisible(true);
			});
			protocol->OnNextIdleFrame(L"Ancestor shown with retained B ID", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"] });
				FindControl(L"b")->GetBoundsComposition()->SetExpectedBounds(Rect(-30, 0, 490, 130));
			});
			protocol->OnNextIdleFrame(L"B partially clipped", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"] });
				List<Ptr<remoteprotocol::RenderingDom>> tags;
				CollectTags(protocol->GetCompletedRenderingFrame()->renderingDom, tags);
				for (auto tag : tags)
				{
					if (tag->id == ids[L"b"])
					{
						TEST_ASSERT(tag->content.validArea.x1 > tag->content.bounds.x1);
						TEST_ASSERT(tag->content.validArea.Width() > 0);
					}
				}
				FindControl(L"b")->GetBoundsComposition()->SetExpectedBounds(Rect(600, 0, 1120, 130));
			});
			protocol->OnNextIdleFrame(L"B fully clipped", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .bVisible = false });
				FindControl(L"b")->GetBoundsComposition()->SetExpectedBounds(Rect(0, 0, 520, 130));
			});
			protocol->OnNextIdleFrame(L"B returned from clipping", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"] });
				auto editor = dynamic_cast<GuiMultilineTextBox*>(FindControl(L"b"));
				editor->SetFocused();
				editor->SelectAll();
				protocol->TypeString(L"First");
				protocol->KeyPress(VKEY::KEY_RETURN);
				protocol->TypeString(L"Second");
			});
			protocol->OnNextIdleFrame(L"Typed into multiline B", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"b"] });
				auto editor = dynamic_cast<GuiMultilineTextBox*>(FindControl(L"b"));
				TEST_ASSERT(editor->GetText() == L"First\r\nSecond");
				editor->SetCaret(TextPos(1, 1), TextPos(1, 4));
				TEST_ASSERT(editor->GetSelectionText() == L"eco");
				protocol->TypeString(L"X");
			});
			protocol->OnNextIdleFrame(L"Replaced multiline selection", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"b"] });
				auto editor = dynamic_cast<GuiMultilineTextBox*>(FindControl(L"b"));
				TEST_ASSERT(editor->GetText() == L"First\r\nSXnd");
				for (vint i = 0; i < 30; i++) documentText += L"Line " + itow(i) + L" retained content\r\n";
				editor->SetText(documentText);
				editor->SetCaret(TextPos(20, 1), TextPos(20, 4));
				editor->EnsureCaretVisible();
			});
			protocol->OnNextIdleFrame(L"Scrolled multiline document", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"b"] });
				auto editor = dynamic_cast<GuiMultilineTextBox*>(FindControl(L"b"));
				TEST_ASSERT(editor->GetViewPosition().y > 0);
				editor->GetBoundsComposition()->SetExpectedBounds(Rect(0, 0, 460, 110));
			});
			protocol->OnNextIdleFrame(L"Resized multiline editor", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"b"] });
				auto editor = dynamic_cast<GuiMultilineTextBox*>(FindControl(L"b"));
				scrollPosition = editor->GetViewPosition();
				GetApplication()->RefreshThemes();
			});
			protocol->OnNextIdleFrame(L"Refreshed templates retain editing owner", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"b"] });
				auto editor = dynamic_cast<GuiMultilineTextBox*>(FindControl(L"b"));
				TEST_ASSERT(editor->GetText() == documentText);
				TEST_ASSERT(editor->GetCaretBegin() == TextPos(20, 1) && editor->GetCaretEnd() == TextPos(20, 4));
				TEST_ASSERT(editor->GetViewPosition() == scrollPosition);
				FindControl(L"a")->SetFocused();
				SafeDeleteControl(editor);
			});
			protocol->OnNextIdleFrame(L"Deleted B owner is absent", [&, protocol]()
			{
				AssertEditors(protocol, ids, { .focused = ids[L"a"], .bVisible = false });
				GetApplication()->GetMainWindow()->Hide();
			});
		});
		UnitTestScreenConfig config;
		config.FastInitialize(1024, 768);
		config.useDomDiff = useDiff;
		config.useChannel = useDiff ? UnitTestRemoteChannel::Sync : UnitTestRemoteChannel::None;
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/EditableMetadata/") + (useDiff ? L"Diff" : L"Raw"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resource,
			{ .config = config }
			);
	});

	for (auto useDiff : { false,true })
	TEST_CASE(useDiff ? L"Featured leaves through DOM diffs" : L"Featured leaves through raw commands")
	{
		GuiBoundsComposition* owner = nullptr;
		GuiBoundsComposition* second = nullptr;
		List<vint> ids;
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [&]()
			{
				for (auto name : { L"a",L"b",L"readOnly" }) FindControl(name)->SetVisible(false);
				owner = new GuiBoundsComposition;
				owner->SetExpectedBounds(Rect(10, 10, 210, 110));
				owner->SetEditableAttributes({ true,false,false });
				for (vint i = 0; i < 2; i++)
				{
					auto leaf = new GuiBoundsComposition;
					leaf->SetExpectedBounds(Rect(10 + i * 90, 10, 90 + i * 90, 90));
					leaf->SetEditableAttributes({ true,false,false });
					owner->AddChild(leaf);
					second = leaf;
				}
				GetApplication()->GetMainWindow()->GetContainerComposition()->AddChild(owner);
			});
			protocol->OnNextIdleFrame(L"Two childless featured siblings under an equal owner", [&, protocol]()
			{
				List<Ptr<remoteprotocol::RenderingDom>> tags;
				CollectTags(protocol->GetCompletedRenderingFrame()->renderingDom, tags);
				TEST_ASSERT(tags.Count() == 3);
				for (auto tag : tags)
				{
					ids.Add(tag->id);
					TEST_ASSERT(!tag->content.editableAttribute.Value().focused && !tag->content.editableAttribute.Value().editable);
				}
				TEST_ASSERT(tags[0]->children->Count() == 2);
				TEST_ASSERT(tags[0]->children->Get(0)->id == ids[1] && tags[0]->children->Get(1)->id == ids[2]);
				TEST_ASSERT(!tags[1]->children && !tags[2]->children);
				second->SetExpectedBounds(Rect(250, 10, 330, 90));
			});
			protocol->OnNextIdleFrame(L"Fully clipped leaf is absent without unbalanced boundaries", [&, protocol]()
			{
				List<Ptr<remoteprotocol::RenderingDom>> tags;
				CollectTags(protocol->GetCompletedRenderingFrame()->renderingDom, tags);
				TEST_ASSERT(tags.Count() == 2 && tags[0]->id == ids[0] && tags[1]->id == ids[1]);
				second->SetExpectedBounds(Rect(100, 10, 180, 90));
			});
			protocol->OnNextIdleFrame(L"Leaf reappears with its retained ID", [&, protocol]()
			{
				List<Ptr<remoteprotocol::RenderingDom>> tags;
				CollectTags(protocol->GetCompletedRenderingFrame()->renderingDom, tags);
				TEST_ASSERT(tags.Count() == 3);
				for (vint i = 0; i < 3; i++) TEST_ASSERT(tags[i]->id == ids[i]);
				GetApplication()->GetMainWindow()->Hide();
			});
		});
		UnitTestScreenConfig config;
		config.FastInitialize(1024, 768);
		config.useDomDiff = useDiff;
		config.useChannel = useDiff ? UnitTestRemoteChannel::Sync : UnitTestRemoteChannel::None;
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/EditableLeaves/") + (useDiff ? L"Diff" : L"Raw"),
			L"gacuisrc_unittest::MainWindow", resource, { .config = config });
	});

	const auto windowsResource = LR"GacUISrc(
<Resource>
  <Instance name="SubWindowResource">
    <Instance ref.Class="gacuisrc_unittest::SubWindow">
      <Window ref.Name="self" Text="Read-only window" ClientSize="x:260 y:150">
        <DocumentLabel ref.Name="readOnly" Text="Read-only focus">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:10 right:10 bottom:-1"/>
        </DocumentLabel>
        <Button ref.Name="button" Text="Non-text focus">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:70 right:-1 bottom:-1"/>
        </Button>
      </Window>
    </Instance>
  </Instance>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Editable window" ClientSize="x:900 y:600">
        <SinglelineTextBox ref.Name="a" Text="Remembered focus">
          <att.BoundsComposition-set AlignmentToParent="left:10 top:10 right:10 bottom:-1"/>
        </SinglelineTextBox>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	for (auto useDiff : { false,true })
	TEST_CASE(useDiff ? L"Active and modal windows through DOM diffs" : L"Active and modal windows through raw commands")
	{
		GuiWindow* sub = nullptr;
		vint mainId = -1, subId = -1;
		vint focusNotifications = 0;
		bool modalCompleted = false;
		GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			auto assertWindows = [&, protocol](vint focused, bool mainEnabled)
			{
				TEST_ASSERT(FocusedTag(protocol, 2) == focused);
				List<Ptr<remoteprotocol::RenderingDom>> tags;
				CollectTags(protocol->GetCompletedRenderingFrame()->renderingDom, tags);
				for (auto tag : tags)
				{
					TEST_ASSERT(tag->id == mainId || tag->id == subId);
					TEST_ASSERT(tag->content.editableAttribute.Value().editable == (tag->id == mainId && mainEnabled));
				}
			};
			protocol->OnNextIdleFrame(L"Ready", [&]()
			{
				GetApplication()->GetMainWindow()->SetLocation({ {20},{20} });
				FindControl(L"a")->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Main editor focused", [&, protocol]()
			{
				mainId = FocusedTag(protocol, 1);
				TEST_ASSERT(mainId != -1);
				sub = UnboxValue<GuiWindow*>(Value::Create(L"gacuisrc_unittest::SubWindow"));
				sub->SetLocation({ {400},{20} });
				sub->Show();
				FindObjectByName<GuiControl>(sub, L"readOnly")->SetFocused();
				AssertStored(FindControl(L"a"), true, true);
			});
			protocol->OnNextIdleFrame(L"Read-only target focused in second enabled window", [&, protocol, assertWindows]()
			{
				subId = FocusedTag(protocol, 2);
				TEST_ASSERT(subId != -1 && subId != mainId);
				assertWindows(subId, true);
				FindControl(L"a")->FocusedChanged.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { focusNotifications++; });
				FindObjectByName<GuiControl>(sub, L"readOnly")->FocusedChanged.AttachLambda([&](GuiGraphicsComposition*, GuiEventArgs&) { focusNotifications++; });
				GetApplication()->GetMainWindow()->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Activation alone returns to remembered main editor", [&, assertWindows]()
			{
				assertWindows(mainId, true);
				TEST_ASSERT(focusNotifications == 0);
				AssertStored(FindObjectByName<GuiControl>(sub, L"readOnly"), true, false);
				sub->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Activation alone returns to read-only target", [&, assertWindows]()
			{
				assertWindows(subId, true);
				TEST_ASSERT(focusNotifications == 0);
				FindObjectByName<GuiControl>(sub, L"button")->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Active non-text target has no focused editing tag", [&, assertWindows]()
			{
				assertWindows(-1, true);
				sub->Hide();
				GetApplication()->GetMainWindow()->SetFocused();
			});
			protocol->OnNextIdleFrame(L"Second window hidden before modal show", [&, protocol]()
			{
				TEST_ASSERT(FocusedTag(protocol, 1) == mainId);
				sub->ShowModal(GetApplication()->GetMainWindow(), [&]() { modalCompleted = true; });
				FindObjectByName<GuiControl>(sub, L"readOnly")->SetFocused();
				AssertStored(FindControl(L"a"), false, false);
			});
			protocol->OnNextIdleFrame(L"Modal read-only target and disabled owner", [&, assertWindows]()
			{
				assertWindows(subId, false);
				TEST_ASSERT(!GetApplication()->GetMainWindow()->GetEnabled());
				sub->Hide();
			});
			protocol->OnNextIdleFrame(L"Modal completed and owner re-enabled", [&, protocol]()
			{
				TEST_ASSERT(modalCompleted && GetApplication()->GetMainWindow()->GetEnabled());
				TEST_ASSERT(FocusedTag(protocol, 1) == (FindControl(L"a")->GetFocused() ? mainId : -1));
				AssertStored(FindControl(L"a"), FindControl(L"a")->GetFocused(), true);
				SafeDeleteControl(sub);
				GetApplication()->GetMainWindow()->Hide();
			});
		});
		UnitTestScreenConfig config;
		config.FastInitialize(1024, 768);
		config.useDomDiff = useDiff;
		config.useChannel = useDiff ? UnitTestRemoteChannel::Sync : UnitTestRemoteChannel::None;
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Controls/Editor/EditableWindows/") + (useDiff ? L"Diff" : L"Raw"),
			L"gacuisrc_unittest::MainWindow", windowsResource, { .config = config });
	});
}
