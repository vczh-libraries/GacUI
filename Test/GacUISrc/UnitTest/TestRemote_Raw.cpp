#include "TestRemote_GraphicsHost_Shared.h"
#include "TestControls.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_BasicElements.h"
#include <typeinfo>

class RawStateProtocol : public GraphicsHostRenderingProtocol
{
public:
	RawStateProtocol(List<WString>& eventLogs)
		: UnitTestRemoteProtocolBase(MakeUnitTestScreenConfig())
		, GraphicsHostRenderingProtocol(eventLogs)
	{
	}

	vint frames = 0;
	bool idle = false;
	List<ElementDesc_Raw> updates;

	void ProcessRemoteEvents() override
	{
		// Hosted rendering includes a settling frame after updating a window.
		if (idle) GraphicsHostRenderingProtocol::ProcessRemoteEvents();
	}

	void RequestRendererIdle() override
	{
		idle = true;
		GraphicsHostRenderingProtocol::RequestRendererIdle();
	}

	void RequestRendererBeginRendering(const ElementBeginRendering& arguments) override
	{
		idle = false;
		frames++;
		if (arguments.updatedElements)
		{
			for (auto& update : *arguments.updatedElements.Obj())
			{
				if (auto raw = update.TryGet<ElementDesc_Raw>()) updates.Add(*raw);
			}
		}
		GraphicsHostRenderingProtocol::RequestRendererBeginRendering(arguments);
	}
};

TEST_FILE
{
	TEST_CATEGORY(L"Raw core state, dirty flags, repaint scheduling and JSON transport")
	{
		List<WString> eventLogs;
		RawStateProtocol protocol(eventLogs);
		GuiWindow* window = nullptr;
		Ptr<GuiRawElementBase> raw;
		vint settledFrames = 0;
		vint settledUpdates = 0;
		auto expect = [&](const WString& id, const WString& data)
		{
			TEST_ASSERT(protocol.updates.Count() > 0);
			auto& desc = protocol.updates[protocol.updates.Count() - 1];
			TEST_ASSERT(desc.rawId == id);
			TEST_ASSERT(desc.data == data);
		};
		protocol.OnNextFrame([&]()
		{
			expect(L"", L"");
			auto renderer = dynamic_cast<elements_remoteprotocol::GuiRawElementRenderer*>(raw->GetRenderer());
			TEST_ASSERT(renderer);
			TEST_ASSERT(!renderer->IsUpdated());
			raw->SetId(L"");
			raw->SetData(L"");
			TEST_ASSERT(!renderer->IsUpdated());
			settledFrames = protocol.frames;
			settledUpdates = protocol.updates.Count();
			raw->SetId(L"intermediate");
			raw->SetId(L"identifier only");
			TEST_ASSERT(renderer->IsUpdated());
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.frames == settledFrames);
			TEST_ASSERT(protocol.updates.Count() == settledUpdates);
			window->GetGraphicsHost()->RequestRender();
		});
		protocol.OnNextFrame([&]()
		{
			expect(L"identifier only", L"");
			raw->SetData(L"Unicode: 时钟 \"quoted\"\nline");
			TEST_ASSERT(raw->GetData()[9] == 0x65F6);
			TEST_ASSERT(raw->GetData()[10] == 0x949F);
		});
		protocol.OnNextFrame([&]()
		{
			expect(L"identifier only", L"Unicode: 时钟 \"quoted\"\nline");
			raw->SetId(L"both");
			raw->SetData(L"12:34:56");
		});
		protocol.OnNextFrame([&]()
		{
			expect(L"both", L"12:34:56");
			settledFrames = protocol.frames;
			raw->SetId(L"both");
			raw->SetData(L"12:34:56");
		});
		protocol.OnNextFrame([&]()
		{
			TEST_ASSERT(protocol.frames == settledFrames);
			raw->SetId(L"");
			window->GetGraphicsHost()->RequestRender();
		});
		protocol.OnNextFrame([&]()
		{
			expect(L"", L"12:34:56");
			raw->SetData(L"");
		});
		protocol.OnNextFrame([&]()
		{
			expect(L"", L"");
			auto duplicate = GetGuiGraphicsResourceManager()->CreateRawElement().Cast<GuiRawElementBase>();
			TEST_ASSERT(duplicate && duplicate.Obj() != raw.Obj());
			TEST_ASSERT(duplicate->GetId() == raw->GetId());
			TEST_ASSERT(duplicate->GetData() == L"");
			duplicate->SetId(L"same");
			raw->SetId(L"same");
			TEST_ASSERT(duplicate->GetId() == raw->GetId());
			auto renderer = dynamic_cast<elements_remoteprotocol::GuiRawElementRenderer*>(raw->GetRenderer());
			List<OrdinaryElementDescVariant> full;
			renderer->ResetUpdated();
			renderer->SendUpdateElementMessages(true, full);
			TEST_ASSERT(full.Count() == 1);
			TEST_ASSERT(full[0].Get<ElementDesc_Raw>().rawId == L"same");
			TEST_ASSERT(full[0].Get<ElementDesc_Raw>().data == L"");
			protocol.GetEvents()->OnControllerDisconnect();
			protocol.updates.Clear();
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
		});
		protocol.OnNextFrame([&]()
		{
			expect(L"same", L"");
			window->Hide();
		});
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());
			auto theme = Ptr(new EmptyControlTheme);
			theme::RegisterTheme(theme);
			{
				GuiWindow host(theme::ThemeName::Window);
				window = &host;
				host.SetClientSize({ 200,100 });
				raw = GetGuiGraphicsResourceManager()->CreateRawElement().Cast<GuiRawElementBase>();
				TEST_CASE_ASSERT(raw);
				TEST_CASE_ASSERT(typeid(*raw.Obj()) == typeid(GuiRawElementBase));
				auto bounds = new GuiBoundsComposition;
				bounds->SetAlignmentToParent({ 0,0,0,0 });
				bounds->SetOwnedElement(raw);
				host.GetContainerComposition()->AddChild(bounds);
				TEST_CASE_ASSERT(raw->GetOwnerComposition() == bounds);
				GetApplication()->Run(&host);
				raw = nullptr;
			}
			theme::UnregisterTheme(theme->Name);
			TEST_CASE_ASSERT(protocol.nextEventIndex == protocol.processRemoteEvents.Count());
		});
		StartRemoteControllerTest(protocol);
	});

	TEST_CASE(L"Raw XML snapshot with separate identifier and data buttons")
	{
		const auto resource = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Raw element state" ClientSize="x:360 y:200">
        <ez:Layout>
          <ez:Top>
            <Button ref.Name="changeId" Text="Change Id" ev.Clicked-eval='raw.Id = "changed identifier";'/>
          </ez:Top>
          <ez:Top>
            <Button ref.Name="changeData" Text="Change Data" ev.Clicked-eval='raw.Data = "A long Unicode value: 时钟 — this text should be ellipsized on one line in the snapshot viewer.";'/>
          </ez:Top>
          <ez:Fill>
            <Bounds PreferredMinSize="x:200 y:50">
              <Raw ref.Name="raw" Id="initial identifier" Data="initial data"/>
            </Bounds>
          </ez:Fill>
        </ez:Layout>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";
		GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			auto assertState = [=](const WString& id, const WString& data)
			{
				auto& frames = *protocol->GetLoggedTrace().frames.Obj();
				auto& frame = frames[frames.Count() - 1];
				vint count = 0;
				for (auto& desc : frame.elements->Values())
				{
					if (auto raw = desc.TryGet<ElementDesc_Raw>())
					{
						TEST_ASSERT(raw->rawId == id);
						TEST_ASSERT(raw->data == data);
						count++;
					}
				}
				TEST_ASSERT(count == 1);
			};
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				assertState(L"initial identifier", L"initial data");
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiButton>(GetApplication()->GetMainWindow(), L"changeId")));
			});
			protocol->OnNextIdleFrame(L"Id changed independently", [=]()
			{
				assertState(L"changed identifier", L"initial data");
				protocol->LClick(protocol->LocationOf(FindObjectByName<GuiButton>(GetApplication()->GetMainWindow(), L"changeData")));
			});
			protocol->OnNextIdleFrame(L"Data changed to long Unicode text", [=]()
			{
				assertState(L"changed identifier", L"A long Unicode value: 时钟 — this text should be ellipsized on one line in the snapshot viewer.");
				GetApplication()->GetMainWindow()->Close();
			});
		});
		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(L"Elements/Raw/Properties", L"gacuisrc_unittest::MainWindow", resource);
	});
}
