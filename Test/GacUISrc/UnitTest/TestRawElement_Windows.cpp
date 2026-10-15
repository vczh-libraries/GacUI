#include "../../../Source/GacUI.h"
#if defined VCZH_MSVC
#include "../Generated_FullControlTest/FullControlTestRawElement.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../../../Source/PlatformProviders/Windows/Direct2D/Renderers/GuiGraphicsWindowsDirect2D.h"
#include "../../../Source/PlatformProviders/Windows/GDI/Renderers/GuiGraphicsWindowsGDI.h"
#include "../../../Source/PlatformProviders/Windows/Direct2D/Renderers/GuiGraphicsRenderersWindowsDirect2D.h"
#include "../../../Source/PlatformProviders/Windows/GDI/Renderers/GuiGraphicsRenderersWindowsGDI.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remote_renderer;

extern void SetGuiMainProxy(const Func<void()>& proxy);

class RawNativeEvents : public Object, public IGuiRemoteProtocolEvents
{
public:
#define EVENT_NOREQ(NAME, REQUEST) void On ## NAME() override {}
#define EVENT_REQ(NAME, REQUEST) void On ## NAME(const REQUEST&) override {}
#define EVENT_HANDLER(NAME, REQUEST, REQTAG, ...) EVENT_ ## REQTAG(NAME, REQUEST)
	GACUI_REMOTEPROTOCOL_EVENTS(EVENT_HANDLER)
#undef EVENT_HANDLER
#undef EVENT_REQ
#undef EVENT_NOREQ
#define MESSAGE_NORES(NAME, RESPONSE)
#define MESSAGE_RES(NAME, RESPONSE) void Respond ## NAME(vint, const RESPONSE&) override {}
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...) MESSAGE_ ## RESTAG(NAME, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_RES
#undef MESSAGE_NORES
};

class RawNativeRenderer : public GuiRemoteRendererSingle
{
public:
	RawNativeRenderer(INativeWindow* nativeWindow) : GuiRemoteRendererSingle(true) { window = nativeWindow; }
	using GuiRemoteRendererSingle::AfterClosing;
	using GuiRemoteRendererSingle::DisconnectFromCore;
	using GuiRemoteRendererSingle::UpdateRenderTarget;
	using GuiRemoteRendererSingle::ForceRender;
	using GuiRemoteRendererSingle::needRefresh;
};

void TestNativeRawElements(bool direct2D)
{
	auto manager = GetGuiGraphicsResourceManager();
	auto native = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
	native->SetClientSize({ 240,240 });
	RawNativeEvents responses;
	RawNativeRenderer renderer(native);
	renderer.Initialize(&responses);
	vint installs = 0, uninstalls = 0, changed = 0, paints = 0;
	vint targetAssignments = 0, targetDetachments = 0;
	List<WString> removalOrder;
	WString paintedData;
	Ptr<GuiRawElementBase> retained;
	Ptr<Object> clockAttachment;

	renderer.RawElementInstalled.Add([&](const GuiRemoteRawElementEventArgs& args)
	{
		installs++;
		TEST_ASSERT(renderer.GetRawElements().Contains(args.element.Obj()));
		TEST_ASSERT(args.element->GetId() == L"" && args.element->GetData() == L"");
		TEST_ASSERT(!args.element->GetOwnerComposition());
		retained = args.element;
		clockAttachment = demo::AttachFullControlTestRawElement(args.element);
		if (direct2D)
		{
			auto element = args.element.Cast<GuiDirect2DElement>();
			TEST_ASSERT(element);
			TEST_ASSERT(dynamic_cast<elements_windows_d2d::GuiDirect2DElementRenderer*>(element->GetRenderer()));
			element->AfterRenderTargetChanged.AttachLambda([&](GuiGraphicsComposition*, GuiDirect2DElementEventArgs&)
			{
				targetAssignments++;
			});
			element->BeforeRenderTargetChanged.AttachLambda([&](GuiGraphicsComposition*, GuiDirect2DElementEventArgs&)
			{
				targetDetachments++;
				removalOrder.Add(L"target detached");
			});
			element->Rendering.AttachLambda([&](GuiGraphicsComposition*, GuiDirect2DElementEventArgs& args)
			{
				paints++;
				paintedData = args.element->GetData();
			});
		}
		else
		{
			auto element = args.element.Cast<GuiGDIElement>();
			TEST_ASSERT(element);
			TEST_ASSERT(dynamic_cast<elements_windows_gdi::GuiGDIElementRenderer*>(element->GetRenderer()));
			element->Rendering.AttachLambda([&](GuiGraphicsComposition*, GuiGDIElementEventArgs& args)
			{
				paints++;
				paintedData = args.element->GetData();
			});
		}
	});
	renderer.RawElementUninstalled.Add([&](const GuiRemoteRawElementEventArgs& args)
	{
		TEST_ASSERT(renderer.GetRawElements().Contains(args.element.Obj()));
		uninstalls++;
		clockAttachment = nullptr;
		removalOrder.Add(L"first uninstall");
	});
	renderer.RawElementUninstalled.Add([&](const GuiRemoteRawElementEventArgs& args)
	{
		TEST_ASSERT(renderer.GetRawElements().Contains(args.element.Obj()));
		removalOrder.Add(L"second uninstall");
	});
	WString expectedId, expectedData;
	renderer.RawElementChanged.Add([&](const GuiRemoteRawElementEventArgs& args)
	{
		changed++;
		TEST_ASSERT(args.element->GetId() == expectedId);
		TEST_ASSERT(args.element->GetData() == expectedData);
	});
	auto creations = Ptr(new List<RendererCreation>);
	creations->Add({ 42,RendererType::Raw });
	renderer.RequestRendererCreated(creations);
	TEST_ASSERT(installs == 1);
	if (direct2D) TEST_ASSERT(targetAssignments == 1);
	auto first = retained;
	auto update = [&](const WString& id, const WString& data)
	{
		expectedId = id;
		expectedData = data;
		ElementBeginRendering begin;
		begin.updatedElements = Ptr(new List<OrdinaryElementDescVariant>);
		begin.updatedElements->Add(ElementDesc_Raw{ 42,id,data });
		// Both the native dispatcher and the JSON schema participate in this check.
		ElementBeginRendering decoded;
		ConvertJsonToCustomType(ConvertCustomTypeToJson(begin), decoded);
		renderer.RequestRendererBeginRendering(decoded);
	};
	update(L"", L"");
	TEST_ASSERT(changed == 0);
	update(L"clock", L"12:34:56");
	TEST_ASSERT(changed == 1 && renderer.needRefresh);
	auto dom = Ptr(new RenderingDom);
	dom->id = 1;
	dom->content.element = 42;
	dom->content.bounds = { 10,10,210,210 };
	dom->content.validArea = dom->content.bounds;
	renderer.RequestRendererRenderDom(dom);
	renderer.RequestRendererEndRendering(1);
	TEST_ASSERT(paints > 0 && paintedData == L"12:34:56");
	renderer.ForceRender();
	TEST_ASSERT(paintedData == L"12:34:56");
	// Reattach after target assignment; the first paint must recreate target-bound brushes.
	clockAttachment = nullptr;
	clockAttachment = demo::AttachFullControlTestRawElement(retained);
	// No DOM update, input, timer or resize accompanies this data change.
	auto previousPaints = paints;
	update(L"clock", L"12:34:57");
	renderer.RequestRendererEndRendering(2);
	TEST_ASSERT(paints > previousPaints && paintedData == L"12:34:57");
	TEST_ASSERT(changed == 2);
	previousPaints = paints;
	update(L"new id", L"12:34:57");
	TEST_ASSERT(!renderer.needRefresh);
	renderer.RequestRendererEndRendering(3);
	TEST_ASSERT(paints == previousPaints && changed == 3);
	update(L"new id", L"12:34:57");
	TEST_ASSERT(changed == 3);
	update(L"", L"");
	TEST_ASSERT(changed == 4);
	renderer.RequestRendererEndRendering(4);

	renderer.UpdateRenderTarget(nullptr);
	manager->RecreateRenderTarget(native);
	renderer.UpdateRenderTarget(manager->GetRenderTarget(native));
	TEST_ASSERT(renderer.GetRawElements()[0] == first && installs == 1);
	if (direct2D) TEST_ASSERT(targetAssignments == 2 && targetDetachments == 1);
	removalOrder.Clear();
	renderer.RequestRendererCreated(creations);
	TEST_ASSERT(installs == 2 && uninstalls == 1 && retained != first);
	TEST_ASSERT(
		(removalOrder[0] == L"first uninstall" && removalOrder[1] == L"second uninstall") ||
		(removalOrder[0] == L"second uninstall" && removalOrder[1] == L"first uninstall"));
	if (direct2D) TEST_ASSERT(removalOrder[2] == L"target detached");
	previousPaints = paints;
	first->GetRenderer()->Render({ 0,0,100,100 });
	TEST_ASSERT(paints == previousPaints);
	auto destroyed = Ptr(new List<vint>);
	destroyed->Add(42);
	renderer.RequestRendererDestroyed(destroyed);
	TEST_ASSERT(uninstalls == 2 && renderer.GetRawElements().Count() == 0);
	renderer.RequestRendererCreated(creations);
	renderer.DisconnectFromCore();
	TEST_ASSERT(uninstalls == 2 && renderer.GetRawElements().Count() == 1);
	renderer.AfterClosing();
	TEST_ASSERT(uninstalls == 3 && renderer.GetRawElements().Count() == 0);
	GetCurrentController()->WindowService()->DestroyNativeWindow(native);
	previousPaints = paints;
	retained->GetRenderer()->Render({ 0,0,100,100 });
	TEST_ASSERT(paints == previousPaints);
}

TEST_FILE
{
	TEST_CASE(L"Direct2D raw factories, events, data-only rendering and retained lifetime")
	{
		SetGuiMainProxy([]() { TestNativeRawElements(true); });
		SetupWindowsDirect2DRenderer();
		SetGuiMainProxy({});
	});
	TEST_CASE(L"GDI raw factories, events, data-only rendering and retained lifetime")
	{
		SetGuiMainProxy([]() { TestNativeRawElements(false); });
		SetupWindowsGDIRenderer();
		SetGuiMainProxy({});
	});
}
#endif
