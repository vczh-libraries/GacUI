#include "../../../Source/GacUI.h"
#if defined VCZH_MSVC
#include "../Generated_FullControlTest/FullControlTestRawElement.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../../../Source/PlatformProviders/Hosted/GuiHostedController.h"
#include "../../../Source/PlatformProviders/Windows/ServicesImpl/WindowsImageService.h"
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

class RawInterpretorState : public Object
{
public:
	vint							created = 0;
	vint							initialized = 0;
	vint							finalized = 0;
	vint							destroyed = 0;
	vint							attachedAtDestruction = 0;
	vint							painted = 0;
	List<GuiRawElementBase*>		live;
	List<Pair<WString, WString>>	initialValues;
};

class CountingRawInterpretor : public Object, public IGuiGraphicsRawInterpretor
{
protected:
	Ptr<RawInterpretorState>		state;
	GuiRawElementBase*				element = nullptr;
	Ptr<IGuiGraphicsEventHandler>	handler;
public:
	CountingRawInterpretor(Ptr<RawInterpretorState> _state) : state(_state)
	{
		state->created++;
	}

	~CountingRawInterpretor()
	{
		if (handler) state->attachedAtDestruction++;
		state->destroyed++;
	}

	void Initialize(IGuiGraphicsElement* value) override
	{
		element = dynamic_cast<GuiRawElementBase*>(value);
		state->initialized++;
		TEST_ASSERT(!state->live.Contains(element));
		state->live.Add(element);
		state->initialValues.Add({ element->GetInterpretor(), element->GetData() });
		if (auto d2d = dynamic_cast<GuiDirect2DElement*>(element))
		{
			handler = d2d->Rendering.AttachLambda([this](GuiGraphicsComposition*, GuiDirect2DElementEventArgs&) { state->painted++; });
		}
		else
		{
			handler = dynamic_cast<GuiGDIElement*>(element)->Rendering.AttachLambda([this](GuiGraphicsComposition*, GuiGDIElementEventArgs&) { state->painted++; });
		}
	}

	void Finalize() override
	{
		TEST_ASSERT(state->live.Remove(element));
		if (auto d2d = dynamic_cast<GuiDirect2DElement*>(element))
		{
			TEST_ASSERT(d2d->Rendering.Detach(handler));
		}
		else
		{
			TEST_ASSERT(dynamic_cast<GuiGDIElement*>(element)->Rendering.Detach(handler));
		}
		handler = nullptr;
		state->finalized++;
	}
};

class CountingRawInterpretorFactory : public Object, public IGuiGraphicsRawInterpretorFactory
{
protected:
	Ptr<RawInterpretorState> state;
public:
	CountingRawInterpretorFactory(Ptr<RawInterpretorState> _state) : state(_state) {}
	IGuiGraphicsRawInterpretor* Create() override { return new CountingRawInterpretor(state); }
};

void TestRawInterpretorLifetime()
{
	auto manager = GetGuiGraphicsResourceManager();
	auto state = Ptr(new RawInterpretorState);
	auto factory = Ptr(new CountingRawInterpretorFactory(state));
	{
		GuiHostedController controller(GetCurrentController());
		GuiHostedGraphicsResourceManager hosted(&controller, manager);
		hosted.RegisterRawInterpretorFactory(L"A", factory);
		manager->RegisterRawInterpretorFactory(L"B", factory);
		TEST_ASSERT(manager->GetRawInterpretorFactory(L"A") == factory.Obj());
		TEST_ASSERT(hosted.GetRawInterpretorFactory(L"B") == factory.Obj());
		TEST_ASSERT(!hosted.GetRawInterpretorFactory(L"missing"));
		TEST_ERROR(hosted.RegisterRawInterpretorFactory(L"A", factory));
		TEST_ASSERT(hosted.CreateRawElement().Cast<GuiRawElementBase>());
	}
	{
		auto raw = manager->CreateRawElement().Cast<GuiRawElementBase>();
		TEST_ASSERT(raw->GetInterpretor() == L"" && !raw->GetOwnerComposition());
		raw->SetId(L"A");
		raw->SetData(L"before");
		TEST_ASSERT(state->created == 0);
		raw->SetInterpretorAndData(L"A", L"first");
		TEST_ASSERT(state->created == 1 && state->initialized == 1);
		TEST_ASSERT(state->initialValues[0].key == L"A" && state->initialValues[0].value == L"first");
		raw->SetData(L"data only");
		raw->SetId(L"");
		raw->SetInterpretor(L"A");
		raw->SetInterpretorAndData(L"A", L"data only");
		TEST_ASSERT(state->created == 1 && state->finalized == 0);
		raw->SetInterpretorAndData(L"B", L"second");
		TEST_ASSERT(state->created == 2 && state->initialized == 2 && state->finalized == 1 && state->destroyed == 1);
		TEST_ASSERT(state->initialValues[1].key == L"B" && state->initialValues[1].value == L"second");
		raw->SetInterpretor(L"");
		TEST_ASSERT(state->finalized == 2 && state->destroyed == 2);
		raw->SetInterpretor(L"A");
		raw->SetInterpretor(L"missing");
		TEST_ASSERT(raw->GetInterpretor() == L"missing");
		TEST_ASSERT(state->created == 3 && state->finalized == 3 && state->destroyed == 3);
		manager->RegisterRawInterpretorFactory(L"missing", factory);
		raw->SetData(L"no late retry");
		raw->SetInterpretor(L"missing");
		TEST_ASSERT(state->created == 3);
		raw->SetInterpretor(L"A");
		{
			auto other = manager->CreateRawElement().Cast<GuiRawElementBase>();
			other->SetInterpretor(L"A");
			TEST_ASSERT(state->created == 5 && state->live.Count() == 2);
			TEST_ASSERT(state->live[0] != state->live[1]);
		}
		TEST_ASSERT(state->finalized == 4 && state->destroyed == 4 && state->live.Count() == 1);
	}
	// There was no composition or target to detach the handler before final release.
	TEST_ASSERT(state->created == 5 && state->initialized == 5 && state->finalized == 5 && state->destroyed == 5);
	TEST_ASSERT(state->live.Count() == 0);

	auto native = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
	native->SetClientSize({ 240,240 });
	{
		auto raw = manager->CreateRawElement().Cast<GuiRawElementBase>();
		auto renderer = raw->GetRenderer();
		auto target = manager->GetRenderTarget(native);
		renderer->SetRenderTarget(target);
		raw->SetInterpretor(L"A");
		target->StartRendering();
		renderer->Render({ 0,0,200,200 });
		target->StopRendering();
		TEST_ASSERT(state->painted == 1 && state->created == 6);
		renderer->SetRenderTarget(nullptr);
		manager->RecreateRenderTarget(native);
		target = manager->GetRenderTarget(native);
		renderer->SetRenderTarget(target);
		target->StartRendering();
		renderer->Render({ 0,0,200,200 });
		target->StopRendering();
		TEST_ASSERT(state->painted == 2 && state->created == 6 && state->finalized == 5);
		raw->SetInterpretor(L"");
		TEST_ASSERT(state->finalized == 6 && state->destroyed == 6);
		raw->SetInterpretor(L"B");
		// Final release with an attached target still cleans the handler before its event dies.
	}
	TEST_ASSERT(state->created == 7 && state->finalized == 7 && state->destroyed == 7);
	TEST_ASSERT(state->attachedAtDestruction == 0);
	GetCurrentController()->WindowService()->DestroyNativeWindow(native);
}

// Render the real demo handlers into readable pixels, independently of desktop visibility.
Array<vuint8_t> RenderRawPixels(Ptr<GuiRawElementBase> raw, Rect bounds, bool detachTarget = true)
{
	Array<vuint8_t> pixels(240 * 240 * 4);
	if (auto d2d = raw.Cast<GuiDirect2DElement>())
	{
		IWICBitmap* bitmap = nullptr;
		TEST_ASSERT(SUCCEEDED(windows::GetWICImagingFactory()->CreateBitmap(240, 240, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bitmap)));
		ComPtr<IWICBitmap> bitmapHolder = bitmap;
		auto factory = elements_windows_d2d::GetWindowsDirect2DObjectProvider()->GetDirect2DFactory();
		ID2D1RenderTarget* target = nullptr;
		TEST_ASSERT(SUCCEEDED(factory->CreateWicBitmapRenderTarget(bitmap, D2D1::RenderTargetProperties(), &target)));
		ComPtr<ID2D1RenderTarget> targetHolder = target;
		GuiDirect2DElementEventArgs args(d2d.Obj(), target, nullptr, factory, bounds);
		target->BeginDraw();
		target->Clear(D2D1::ColorF(0, 0, 0));
		// Deliberately omit AfterRenderTargetChanged to exercise late brush creation.
		d2d->Rendering.Execute(args);
		TEST_ASSERT(SUCCEEDED(target->EndDraw()));
		TEST_ASSERT(SUCCEEDED(bitmap->CopyPixels(nullptr, 240 * 4, (UINT)pixels.Count(), &pixels[0])));
		if (detachTarget) d2d->BeforeRenderTargetChanged.Execute(args);
	}
	else
	{
		auto gdi = raw.Cast<GuiGDIElement>();
		windows::WinBitmap bitmap(240, 240, windows::WinBitmap::vbb32Bits, true);
		PatBlt(bitmap.GetWinDC()->GetHandle(), 0, 0, 240, 240, BLACKNESS);
		GuiGDIElementEventArgs args(gdi.Obj(), bitmap.GetWinDC(), bounds);
		gdi->Rendering.Execute(args);
		GdiFlush();
		for (vint y = 0; y < 240; y++)
		{
			memcpy(&pixels[y * 240 * 4], bitmap.GetScanLines()[y], 240 * 4);
		}
	}
	return pixels;
}

vint CountRawPixels(const Array<vuint8_t>& pixels, Rect area, Color color)
{
	vint count = 0;
	for (vint y = area.y1; y < area.y2; y++)
	{
		for (vint x = area.x1; x < area.x2; x++)
		{
			auto offset = (y * 240 + x) * 4;
			if (abs(pixels[offset] - color.b) < 8 && abs(pixels[offset + 1] - color.g) < 8 && abs(pixels[offset + 2] - color.r) < 8) count++;
		}
	}
	return count;
}

void TestRawPainterPixels()
{
	auto raw = GetGuiGraphicsResourceManager()->CreateRawElement().Cast<GuiRawElementBase>();
	Rect bounds(0, 0, 240, 240);
	auto yellow = Color(255, 255, 0);
	raw->SetInterpretorAndData(L"FullControlTest::Clock", L"12:00:00");
	auto pixels = RenderRawPixels(raw, bounds);
	TEST_ASSERT(CountRawPixels(pixels, bounds, Color(255, 255, 255)) > 300);
	TEST_ASSERT(CountRawPixels(pixels, { 118,35,123,100 }, yellow) > 30);
	raw->SetData(L"12:00:15");
	pixels = RenderRawPixels(raw, bounds);
	TEST_ASSERT(CountRawPixels(pixels, { 140,118,210,123 }, yellow) > 30);
	TEST_ASSERT(CountRawPixels(pixels, { 118,35,123,100 }, yellow) == 0);
	for (auto name : { L"", L"unregistered painter" })
	{
		raw->SetInterpretor(name);
		TEST_ASSERT(CountRawPixels(RenderRawPixels(raw, bounds), bounds, Color(0, 0, 0)) == 240 * 240);
	}
	raw->SetInterpretor(L"");
	auto attachment = demo::AttachFullControlTestTouchingElement(raw);
	raw->SetData(L"120,120,1,0,1,7,40,40");
	pixels = RenderRawPixels(raw, bounds);
	TEST_ASSERT(CountRawPixels(pixels, bounds, Color(135, 206, 235)) > 15000);
	TEST_ASSERT(CountRawPixels(pixels, { 20,20,60,60 }, yellow) > 70);
	// Interpretor replacement must leave the manual attachment installed.
	raw->SetInterpretor(L"FullControlTest::Clock");
	raw->SetInterpretor(L"");
	raw->SetData(L"100,100,0.5,0,1,7,180,180");
	pixels = RenderRawPixels(raw, { 10,10,230,230 });
	TEST_ASSERT(CountRawPixels(pixels, bounds, Color(135, 206, 235)) > 3500);
	TEST_ASSERT(CountRawPixels(pixels, { 170,170,210,210 }, yellow) > 70);
	TEST_ASSERT(CountRawPixels(pixels, { 20,20,60,60 }, yellow) == 0);
	attachment = nullptr;
	TEST_ASSERT(CountRawPixels(RenderRawPixels(raw, bounds), bounds, Color(0, 0, 0)) == 240 * 240);
	// Finalization releases Clock brushes even without another target-change event.
	raw->SetInterpretorAndData(L"FullControlTest::Clock", L"12:00:15");
	TEST_ASSERT(CountRawPixels(RenderRawPixels(raw, bounds, false), bounds, yellow) > 30);
}

void TestNativeRawElements(bool direct2D)
{
	auto manager = GetGuiGraphicsResourceManager();
	demo::RegisterFullControlTestClockInterpretor();
	TestRawInterpretorLifetime();
	TestRawPainterPixels();
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

	renderer.RawElementInstalled.Add([&](const GuiRemoteRawElementEventArgs& args)
	{
		installs++;
		TEST_ASSERT(renderer.GetRawElements().Contains(args.element.Obj()));
		TEST_ASSERT(args.element->GetId() == L"" && args.element->GetInterpretor() == L"" && args.element->GetData() == L"");
		TEST_ASSERT(!args.element->GetOwnerComposition());
		retained = args.element;
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
		removalOrder.Add(L"first uninstall");
	});
	renderer.RawElementUninstalled.Add([&](const GuiRemoteRawElementEventArgs& args)
	{
		TEST_ASSERT(renderer.GetRawElements().Contains(args.element.Obj()));
		removalOrder.Add(L"second uninstall");
	});
	WString expectedId, expectedInterpretor, expectedData;
	renderer.RawElementChanged.Add([&](const GuiRemoteRawElementEventArgs& args)
	{
		changed++;
		TEST_ASSERT(args.element->GetId() == expectedId);
		TEST_ASSERT(args.element->GetData() == expectedData);
		TEST_ASSERT(args.element->GetInterpretor() == expectedInterpretor);
	});
	auto creations = Ptr(new List<RendererCreation>);
	creations->Add({ 42,RendererType::Raw });
	renderer.RequestRendererCreated(creations);
	TEST_ASSERT(installs == 1);
	if (direct2D) TEST_ASSERT(targetAssignments == 1);
	auto first = retained;
	auto update = [&](const WString& id, const WString& interpretor, const WString& data)
	{
		expectedId = id;
		expectedInterpretor = interpretor;
		expectedData = data;
		ElementBeginRendering begin;
		begin.updatedElements = Ptr(new List<OrdinaryElementDescVariant>);
		begin.updatedElements->Add(ElementDesc_Raw{ .id = 42, .rawId = id, .interpretor = interpretor, .data = data });
		// Both the native dispatcher and the JSON schema participate in this check.
		ElementBeginRendering decoded;
		ConvertJsonToCustomType(ConvertCustomTypeToJson(begin), decoded);
		renderer.RequestRendererBeginRendering(decoded);
	};
	update(L"", L"", L"");
	TEST_ASSERT(changed == 0);
	update(L"clock", L"FullControlTest::Clock", L"12:34:56");
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
	retained->SetInterpretor(L"");
	retained->SetInterpretor(L"FullControlTest::Clock");
	// No DOM update, input, timer or resize accompanies this data change.
	auto previousPaints = paints;
	update(L"clock", L"FullControlTest::Clock", L"12:34:57");
	renderer.RequestRendererEndRendering(2);
	TEST_ASSERT(paints > previousPaints && paintedData == L"12:34:57");
	TEST_ASSERT(changed == 2);
	for (auto name : { L"", L"unknown remote painter", L"FullControlTest::Clock" })
	{
		previousPaints = paints;
		update(L"clock", name, L"12:34:57");
		TEST_ASSERT(renderer.needRefresh);
		renderer.RequestRendererEndRendering(2);
		TEST_ASSERT(paints > previousPaints && paintedData == L"12:34:57");
	}
	TEST_ASSERT(changed == 5);
	previousPaints = paints;
	update(L"new id", L"FullControlTest::Clock", L"12:34:57");
	TEST_ASSERT(!renderer.needRefresh);
	renderer.RequestRendererEndRendering(3);
	TEST_ASSERT(paints == previousPaints && changed == 6);
	update(L"new id", L"FullControlTest::Clock", L"12:34:57");
	TEST_ASSERT(changed == 6);
	update(L"", L"FullControlTest::Clock", L"12:34:57");
	TEST_ASSERT(changed == 7);
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
