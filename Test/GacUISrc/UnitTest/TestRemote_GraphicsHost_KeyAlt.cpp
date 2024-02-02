#include "TestRemote_GraphicsHost_Shared.h"

namespace remote_graphics_host_tests
{
	template<typename ...TArgs>
	void AssertAltLabels(List<GuiControl*>& altControls, TArgs&& ...args)
	{
		const wchar_t* expected[] = { args... };
		TEST_ASSERT(altControls.Count() == sizeof...(args));
		for (vint i = 0; i < altControls.Count(); i++)
		{
			GuiLabel* altLabel = nullptr;
			auto altAction = altControls[i]->QueryTypedService<IGuiAltAction>();
			auto altComposition = altAction->GetAltComposition();
			for (auto candidate : altComposition->Children())
			{
				if (auto label = dynamic_cast<GuiLabel*>(candidate->GetRelatedControl()))
				{
					if (label->GetControlThemeName() == theme::ThemeName::ShortcutKey)
					{
						altLabel = label;
						break;
					}
				}
			}

			if (altLabel)
			{
				TEST_ASSERT(expected[i] != nullptr);
				TEST_ASSERT(altLabel->GetText() == expected[i]);
			}
			else
			{
				TEST_ASSERT(expected[i] == nullptr);
			}
		}
	}

	GuiButton* CreateButton(List<GuiControl*>& altControls, GuiWindow* controlHost, const wchar_t* alt)
	{
		auto control = new GuiButton(theme::ThemeName::Button);
		control->SetAlt(alt);
		controlHost->AddChild(control);
		altControls.Add(control);
		return control;
	}
}
using namespace remote_graphics_host_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Alt single level and escape / activate")
	{
		GraphicsHostProtocol protocol;
		List<WString> eventLogs;
		List<GuiControl*> altControls;
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			CreateButton(altControls, controlHost, L"A");
			CreateButton(altControls, controlHost, L"B");
			CreateButton(altControls, controlHost, L"C");
			AssertAltLabels(altControls, nullptr, nullptr, nullptr);
		});

		protocol.OnNextFrame([&]()
		{
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
}