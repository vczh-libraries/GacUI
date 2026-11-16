#include "TestControls.h"

using namespace gacui_unittest_template;

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
		for (auto name : { L"a", L"b", L"readOnly" })
		{
			bool visible = name == WString(L"a") || (name == WString(L"b") ? expected.bVisible : expected.readOnlyVisible);
			vint count = 0;
			for (auto tag : tags)
			{
				if (tag->id != ids[name]) continue;
				count++;
				auto value = tag->content.editableAttribute.Value();
				TEST_ASSERT(value.focused == (tag->id == expected.focused));
				TEST_ASSERT(value.editable == (name == WString(L"a") ? expected.aEditable : name == WString(L"b") && expected.bEditable));
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
        <Control ref.Name="parent">
          <att.BoundsComposition-set ExpectedBounds="x:10 y:55 w:540 h:150" MinSizeLimitation="NoLimit"/>
          <MultilineTextBox ref.Name="b" Text="Beta">
            <att.BoundsComposition-set ExpectedBounds="x:0 y:0 w:520 h:130"/>
          </MultilineTextBox>
        </Control>
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
}
