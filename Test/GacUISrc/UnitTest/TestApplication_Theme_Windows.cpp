#include "../../../Source/GacUI.h"

#if defined VCZH_MSVC
#include "TestApplication_Theme_Windows.h"
#include "../../../Source/Skins/DarkSkin/Config/DarkSkinConfig.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::compositions;
using namespace vl::presentation::windows;

namespace
{
	class ThemeObservation : public WindowsUIAutomationContext
	{
	public:
		List<GuiControl*> invalidations;
		List<GuiControl*> nameChanges;
		List<GuiControl*> textChanges;
		GuiControl* movedControl = nullptr;
		double oldLeft = 0, newLeft = 0;
		WString oldName, newName;

		ThemeObservation() : WindowsUIAutomationContext(false) {}

		void Raise(Ptr<WindowsUIAutomationNode> node, EVENTID eventId)override
		{
			if (eventId == UIA_Text_TextChangedEventId && Subscribed(eventId)) textChanges.Add(node->control);
			WindowsUIAutomationContext::Raise(node, eventId);
		}

		void RaiseChildrenInvalidated(Ptr<WindowsUIAutomationNode> node)override
		{
			invalidations.Add(node->control);
			WindowsUIAutomationContext::RaiseChildrenInvalidated(node);
		}

		void RaisePropertyChanged(Ptr<WindowsUIAutomationNode> node, PROPERTYID property, VARIANT before, VARIANT after)override
		{
			if (property == UIA_NamePropertyId && before.vt == VT_BSTR && after.vt == VT_BSTR)
			{
				nameChanges.Add(node->control);
				oldName = WString::CopyFrom(before.bstrVal, SysStringLen(before.bstrVal));
				newName = WString::CopyFrom(after.bstrVal, SysStringLen(after.bstrVal));
			}
			if (property == UIA_BoundingRectanglePropertyId && node->control == movedControl)
			{
				LONG left = 0;
				TEST_ASSERT(before.vt == (VT_ARRAY | VT_R8) && after.vt == (VT_ARRAY | VT_R8));
				TEST_ASSERT(SafeArrayGetElement(before.parray, &left, &oldLeft) == S_OK);
				TEST_ASSERT(SafeArrayGetElement(after.parray, &left, &newLeft) == S_OK);
			}
			WindowsUIAutomationContext::RaisePropertyChanged(node, property, before, after);
		}
	};

	WindowsUIAutomationProvider* Provider(const ComPtr<IRawElementProviderSimple>& value)
	{
		return static_cast<WindowsUIAutomationProvider*>(value.Obj());
	}

	void DrainPostedNotifications()
	{
		MSG message;
		while (PeekMessage(&message, nullptr, WM_APP, 0xBFFF, PM_REMOVE)) DispatchMessage(&message);
	}

	void TestNativeThemeObservation(Ptr<ThemeObservation> context)
	{
		auto skin = Ptr(new darkskin::Theme);
		theme::RegisterTheme(skin);
		auto testTheme = Ptr(new theme::ThemeTemplates);
		testTheme->Name = L"UIA theme observation";
		GuiDocumentLabel* templateDocument = nullptr;
		GuiLabel* templateLabel = nullptr;
		GuiLabel* nestedLabel = nullptr;
		bool labelDocument = false;
		auto original = theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::GroupBox);
		testTheme->GroupBox = [&](const reflection::description::Value& value)
		{
			auto result = original(value);
			templateDocument = new GuiDocumentLabel(theme::ThemeName::DocumentLabel);
			templateDocument->SetText(L"Document in this template");
			result->AddChild(templateDocument->GetBoundsComposition());
			templateLabel = new GuiLabel(theme::ThemeName::Label);
			templateLabel->SetText(L"Original label");
			templateLabel->GetBoundsComposition()->SetExpectedBounds({ 15,30,115,60 });
			result->AddChild(templateLabel->GetBoundsComposition());
			if (labelDocument) SetWindowsUIAutomationLabel(templateDocument, templateLabel);
			auto group = new GuiControl(theme::ThemeName::CustomControl);
			group->SetControlTemplate(theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::CustomControl));
			nestedLabel = new GuiLabel(theme::ThemeName::Label);
			nestedLabel->SetControlTemplate(theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::Label));
			nestedLabel->SetText(L"Nested label");
			group->AddChild(nestedLabel);
			result->AddChild(group->GetBoundsComposition());
			return result;
		};
		theme::RegisterTheme(testTheme);
		{
			GuiWindow window(theme::ThemeName::Window);
			auto container = new GuiControl(theme::ThemeName::GroupBox);
			container->GetBoundsComposition()->SetAlignmentToParent({ 0,0,0,0 });
			window.GetContainerComposition()->AddChild(container->GetBoundsComposition());
			auto button = new GuiButton(theme::ThemeName::Button);
			button->SetText(L"Action");
			button->GetBoundsComposition()->SetExpectedBounds({ 200,80,330,120 });
			container->GetContainerComposition()->AddChild(button->GetBoundsComposition());
			vint clicks = 0;
			button->Clicked.AttachLambda([&](auto, auto&) { clicks++; });
			window.SetClientSize({ 420,300 });
			window.ForceCalculateSizeImmediately();

			GetApplication()->InvokeInMainThread(&window, [&]()
			{
				auto native = window.GetNativeWindow();
				ComPtr<IRawElementProviderSimple> root;
				bool changeLabelInAfter = false;
				auto earlyAfter = window.AfterRebuildingTheme.AttachLambda([&](auto, auto&)
				{
					if (!changeLabelInAfter) return;
					templateLabel->SetText(L"Changed in AfterRebuildingTheme");
					DrainPostedNotifications();
					TEST_ASSERT(context->nameChanges.Count() == 1 && context->nameChanges[0] == templateLabel);
					TEST_ASSERT(context->oldName == L"Original label" && context->newName == L"Changed in AfterRebuildingTheme");
					IRawElementProviderFragment* child = nullptr;
					TEST_ASSERT(Provider(root)->Navigate(NavigateDirection_FirstChild, &child) == S_OK && child);
					child->Release();
				});
				auto early = window.BeforeRebuildingTheme.AttachLambda([&](auto, auto&)
				{
					TEST_ASSERT(!context->roots.Keys().Contains(native));
					context->BindWindows();
					TEST_ASSERT(context->roots.Keys().Contains(native));
					root = context->roots[native]->Provider();
					TEST_ASSERT(Provider(root)->AdviseEventAdded(UIA_StructureChangedEventId, nullptr) == S_OK);
					IRawElementProviderFragment* child = nullptr;
					TEST_ASSERT(Provider(root)->Navigate(NavigateDirection_FirstChild, &child) == S_OK && !child);
				});
				window.RefreshThemes();
				window.BeforeRebuildingTheme.Detach(early);
				window.ForceCalculateSizeImmediately();
				DrainPostedNotifications();
				context->invalidations.Clear();

				auto retained = context->Control(button)->Provider();
				auto replaced = context->Control(templateDocument)->Provider();
				ITextRangeProvider* range = nullptr;
				TEST_ASSERT(Provider(replaced)->get_DocumentRange(&range) == S_OK && range);
				button->SetFocused();
				UiaRect bounds;
				TEST_ASSERT(Provider(retained)->get_BoundingRectangle(&bounds) == S_OK);
				TEST_ASSERT(bounds.width > 0 && bounds.height > 0);
				auto x = bounds.left + bounds.width / 2;
				auto y = bounds.top + bounds.height / 2;
				IRawElementProviderFragment* originalHit = nullptr;
				TEST_ASSERT(Provider(root)->ElementProviderFromPoint(x, y, &originalHit) == S_OK);
				TEST_ASSERT(originalHit == static_cast<IRawElementProviderFragment*>(Provider(retained)));
				originalHit->Release();
				auto probe = window.BeforeRebuildingTheme.AttachLambda([&](auto, auto&)
				{
					IRawElementProviderFragment* child = nullptr;
					TEST_ASSERT(Provider(root)->Navigate(NavigateDirection_LastChild, &child) == S_OK && !child);
					auto checkRootOrEmpty = [&]()
					{
						if (child)
						{
							TEST_ASSERT(child == static_cast<IRawElementProviderFragment*>(Provider(root)));
							child->Release();
							child = nullptr;
						}
					};
					TEST_ASSERT(Provider(root)->GetFocus(&child) == S_OK);
					checkRootOrEmpty();
					TEST_ASSERT(Provider(root)->ElementProviderFromPoint(x, y, &child) == S_OK);
					checkRootOrEmpty();
					TEST_ASSERT(Provider(retained)->Invoke() == UIA_E_ELEMENTNOTAVAILABLE);
					BSTR text = nullptr;
					TEST_ASSERT(range->GetText(-1, &text) == UIA_E_ELEMENTNOTAVAILABLE && !text);
				});
				TEST_ASSERT(Provider(retained)->Invoke() == S_OK);
				context->Notify(context->Control(&window), true);
				window.RefreshThemes();
				window.BeforeRebuildingTheme.Detach(probe);
				TEST_ASSERT(context->invalidations.Count() == 2);
				for (auto source : context->invalidations) TEST_ASSERT(source == &window);
				TEST_ASSERT(context->Control(&window)->Provider().Obj() == root.Obj());
				TEST_ASSERT(context->Control(button)->Provider().Obj() == retained.Obj());
				TEST_ASSERT(Provider(replaced)->get_BoundingRectangle(&bounds) == UIA_E_ELEMENTNOTAVAILABLE);
				SAFEARRAY* runtimeId = nullptr;
				TEST_ASSERT(Provider(replaced)->GetRuntimeId(&runtimeId) == S_OK && runtimeId);
				SafeArrayDestroy(runtimeId);
				BSTR text = nullptr;
				TEST_ASSERT(range->GetText(-1, &text) == UIA_E_ELEMENTNOTAVAILABLE && !text);
				range->Release();
				DrainPostedNotifications();
				TEST_ASSERT(clicks == 0 && context->invalidations.Count() == 2);
				IRawElementProviderFragment* finished = nullptr;
				TEST_ASSERT(Provider(root)->Navigate(NavigateDirection_FirstChild, &finished) == S_OK && finished);
				finished->Release();

				context->invalidations.Clear();
				window.RefreshThemes();
				TEST_ASSERT(context->invalidations.Count() == 1 && context->invalidations[0] == &window);
				TEST_ASSERT(Provider(root)->AdviseEventRemoved(UIA_StructureChangedEventId, nullptr) == S_OK);
				auto properties = SafeArrayCreateVector(VT_I4, 0, 1);
				LONG index = 0, nameId = UIA_NamePropertyId;
				TEST_ASSERT(SafeArrayPutElement(properties, &index, &nameId) == S_OK);
				TEST_ASSERT(Provider(root)->AdviseEventAdded(UIA_AutomationPropertyChangedEventId, properties) == S_OK);
				context->Notify(context->Control(&window), true);
				window.RefreshThemes();
				nestedLabel->SetText(L"Changed nested label");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 1 && context->nameChanges[0] == nestedLabel);
				TEST_ASSERT(context->oldName == L"Nested label" && context->newName == L"Changed nested label");
				context->nameChanges.Clear();
				templateLabel->SetText(L"Original label");
				templateLabel->SetText(L"New label");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 1 && context->nameChanges[0] == templateLabel);
				TEST_ASSERT(context->oldName == L"Original label" && context->newName == L"New label");
				templateLabel->SetText(L"New label");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 1);
				window.SetText(L"New window title");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 2 && context->nameChanges[1] == &window);
				TEST_ASSERT(context->newName == L"New window title");

				labelDocument = true;
				window.RefreshThemes();
				context->nameChanges.Clear();
				templateLabel->SetText(L"Name supplied by label");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 2 && context->nameChanges.Contains(templateLabel) && context->nameChanges.Contains(templateDocument));
				TEST_ASSERT(context->oldName == L"Original label" && context->newName == L"Name supplied by label");
				SetWindowsUIAutomationLabel(templateDocument, nullptr);
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 3 && context->nameChanges[2] == templateDocument);
				context->nameChanges.Clear();
				templateLabel->SetText(L"Independent label");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 1 && context->nameChanges[0] == templateLabel);
				SetWindowsUIAutomationName(templateLabel, L"Explicit label name");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 2 && context->nameChanges[1] == templateLabel);
				TEST_ASSERT(context->oldName == L"Independent label" && context->newName == L"Explicit label name");
				templateLabel->SetText(L"Changed display text");
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 2);
				labelDocument = false;
				context->nameChanges.Clear();
				changeLabelInAfter = true;
				window.RefreshThemes();
				window.AfterRebuildingTheme.Detach(earlyAfter);
				DrainPostedNotifications();
				TEST_ASSERT(context->nameChanges.Count() == 1 && context->nameChanges[0] == templateLabel);
				TEST_ASSERT(Provider(root)->AdviseEventRemoved(UIA_AutomationPropertyChangedEventId, properties) == S_OK);
				SafeArrayDestroy(properties);

				TEST_ASSERT(Provider(root)->AdviseEventAdded(UIA_Text_TextChangedEventId, nullptr) == S_OK);
				window.RefreshThemes();
				templateDocument->SetText(L"Changed without a client tree walk");
				DrainPostedNotifications();
				TEST_ASSERT(context->textChanges.Count() == 1 && context->textChanges[0] == templateDocument);
				TEST_ASSERT(Provider(root)->AdviseEventRemoved(UIA_Text_TextChangedEventId, nullptr) == S_OK);
				auto document = context->Control(templateDocument)->Provider();
				ITextRangeProvider* oldDocument = nullptr;
				TEST_ASSERT(Provider(document)->get_DocumentRange(&oldDocument) == S_OK && oldDocument);
				templateDocument->SetDocument(Ptr(new DocumentModel));
				TEST_ASSERT(oldDocument->GetText(-1, &text) == UIA_E_ELEMENTNOTAVAILABLE && !text);
				oldDocument->Release();

				properties = SafeArrayCreateVector(VT_I4, 0, 1);
				LONG boundsId = UIA_BoundingRectanglePropertyId;
				TEST_ASSERT(SafeArrayPutElement(properties, &index, &boundsId) == S_OK);
				TEST_ASSERT(Provider(root)->AdviseEventAdded(UIA_AutomationPropertyChangedEventId, properties) == S_OK);
				window.RefreshThemes();
				window.ForceCalculateSizeImmediately();
				DrainPostedNotifications();
				context->movedControl = templateLabel;
				templateLabel->GetBoundsComposition()->SetExpectedBounds({ 25,40,135,75 });
				window.ForceCalculateSizeImmediately();
				DrainPostedNotifications();
				TEST_ASSERT(context->newLeft > context->oldLeft);
				TEST_ASSERT(Provider(root)->AdviseEventRemoved(UIA_AutomationPropertyChangedEventId, properties) == S_OK);
				SafeArrayDestroy(properties);

				GuiWindow destination(theme::ThemeName::Window);
				window.BeforeRebuildingTheme.AttachLambda([&](auto, auto&)
				{
					auto composition = button->GetBoundsComposition();
					composition->GetParent()->RemoveChild(composition);
					destination.GetContainerComposition()->AddChild(composition);
				});
				window.RefreshThemes();
				TEST_ASSERT(Provider(retained)->get_BoundingRectangle(&bounds) == UIA_E_ELEMENTNOTAVAILABLE);
				auto moved = context->Control(button)->Provider();
				TEST_ASSERT(moved.Obj() != retained.Obj() && Provider(moved)->get_BoundingRectangle(&bounds) == S_OK);
				window.Hide();
			});
			GetApplication()->Run(&window);
		}
		theme::UnregisterTheme(testTheme->Name);
		theme::UnregisterTheme(skin->Name);
	}
}

TEST_FILE
{
	TEST_CASE(L"Windows UIA theme refresh protects retained elements and observes an unvisited source")
	{
		theme_observation_tests::Run<ThemeObservation>(false, &TestNativeThemeObservation);
	});
}
#endif
