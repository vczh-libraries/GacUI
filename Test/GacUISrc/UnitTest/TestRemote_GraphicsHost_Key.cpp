#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"Focus and press keys on buttons")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		GuiWindow* controlHost = nullptr;
		GuiButton* x = nullptr, * y = nullptr, * z = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto b = controlHost->GetBoundsComposition();

			x = new GuiButton(theme::ThemeName::Button);
			x->GetBoundsComposition()->SetExpectedBounds(Rect({ 10,10 }, { 100,100 }));

			y = new GuiButton(theme::ThemeName::Button);
			y->GetBoundsComposition()->SetExpectedBounds(Rect({ 10,10 }, { 80,80}));

			z = new GuiButton(theme::ThemeName::Button);
			z->GetBoundsComposition()->SetExpectedBounds(Rect({ 60,60 }, { 100,100 }));

			x->AddChild(y);
			controlHost->AddChild(x);
			controlHost->AddChild(z);

			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(b->GetCachedBounds() == Rect({ 0,0 }, { 640,480 }));
			TEST_ASSERT(x->GetBoundsComposition()->GetCachedBounds() == Rect({ 10,10 }, { 100,100 }));
			TEST_ASSERT(x->GetContainerComposition()->GetCachedBounds() == Rect({ 0,0 }, { 100,100 }));
			TEST_ASSERT(y->GetBoundsComposition()->GetCachedBounds() == Rect({ 10,10 }, { 80,80 }));
			TEST_ASSERT(y->GetContainerComposition()->GetCachedBounds() == Rect({ 0,0 }, { 80,80 }));
			TEST_ASSERT(z->GetBoundsComposition()->GetCachedBounds() == Rect({ 60,60 }, { 100,100 }));
			TEST_ASSERT(z->GetContainerComposition()->GetCachedBounds() == Rect({ 0,0 }, { 100,100 }));

			AttachAndLogEvents(b, L"host.bounds", eventLogs);
			AttachAndLogEvents(x->GetFocusableComposition(), L"x", eventLogs);
			AttachAndLogEvents(y->GetFocusableComposition(), L"y", eventLogs);
			AttachAndLogEvents(z->GetFocusableComposition(), L"z", eventLogs);
		});

		protocol.OnNextFrame([&]()
		{
			y->SetFocused();
			AssertEventLogs(
				eventLogs
				);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_A));
			protocol.events->OnIOChar(MakeCharInfo(false, true, false, L'B'));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, true, VKEY::KEY_C));
			AssertEventLogs(
				eventLogs,

				L"y->host.bounds.KeyPreview(C:A)",
				L"y->x.KeyPreview(C:A)",
				L"y.KeyPreview(C:A)",
				L"y.KeyDown(C:A)",
				L"y->x.KeyDown(C:A)",
				L"y->host.bounds.KeyDown(C:A)",

				L"y->host.bounds.CharPreview(S:B)",
				L"y->x.CharPreview(S:B)",
				L"y.CharPreview(S:B)",
				L"y.Char(S:B)",
				L"y->x.Char(S:B)",
				L"y->host.bounds.Char(S:B)",

				L"y->host.bounds.KeyPreview(A:C)",
				L"y->x.KeyPreview(A:C)",
				L"y.KeyPreview(A:C)",
				L"y.KeyUp(A:C)",
				L"y->x.KeyUp(A:C)",
				L"y->host.bounds.KeyUp(A:C)"
				);

			z->SetFocused();
			AssertEventLogs(
				eventLogs
				);

			protocol.events->OnIOKeyDown(MakeKeyInfo(true, false, false, VKEY::KEY_A));
			protocol.events->OnIOChar(MakeCharInfo(false, true, false, L'B'));
			protocol.events->OnIOKeyUp(MakeKeyInfo(false, false, true, VKEY::KEY_C));
			AssertEventLogs(
				eventLogs,

				L"z->host.bounds.KeyPreview(C:A)",
				L"z.KeyPreview(C:A)",
				L"z.KeyDown(C:A)",
				L"z->host.bounds.KeyDown(C:A)",

				L"z->host.bounds.CharPreview(S:B)",
				L"z.CharPreview(S:B)",
				L"z.Char(S:B)",
				L"z->host.bounds.Char(S:B)",

				L"z->host.bounds.KeyPreview(A:C)",
				L"z.KeyPreview(A:C)",
				L"z.KeyUp(A:C)",
				L"z->host.bounds.KeyUp(A:C)"
				);
		});

		protocol.OnNextFrame([&]()
		{
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		BatchedProtocol batchedProtocol(&protocol);
		SetupRemoteNativeController(&batchedProtocol);
		SetGuiMainProxy({});
	});

	// TODO:
	//   Key/Char event propogation and directly focusing a control
}