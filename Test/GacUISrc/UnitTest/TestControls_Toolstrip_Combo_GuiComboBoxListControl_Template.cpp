#include "TestControls_List.h"
using namespace gacui_unittest_template;

namespace gacui_unittest_template
{
	void GuiComboBoxListControl_TestCases(
		WString resourceXml,
		WString pathFragment
	)
	{
		TEST_CASE(L"Properties")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					TEST_ASSERT(comboBox->GetContainedListControl() == listControl);
					TEST_ASSERT(comboBox->GetItemProvider() == listControl->GetItemProvider());
					TEST_ASSERT(comboBox->GetSelectedIndex() == -1);
					TEST_ASSERT(comboBox->GetSelectedItem().IsNull());
					TEST_ASSERT(comboBox->GetText() == L"");
					comboBox->SetSelectedIndex(0);
					TEST_ASSERT(comboBox->GetSelectedIndex() == 0);
					auto value = UnboxValue<Ptr<TextItem>>(comboBox->GetSelectedItem());
					TEST_ASSERT(value->GetText() == L"Item 1");
					TEST_ASSERT(comboBox->GetText() == L"Item 1");
				});
				protocol->OnNextIdleFrame(L"Select First", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					comboBox->SetSelectedIndex(4);
					TEST_ASSERT(comboBox->GetSelectedIndex() == 4);
					auto value = UnboxValue<Ptr<TextItem>>(comboBox->GetSelectedItem());
					TEST_ASSERT(value->GetText() == L"Item 5");
					TEST_ASSERT(comboBox->GetText() == L"Item 5");
				});
				protocol->OnNextIdleFrame(L"Select Last", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/Combo/") + pathFragment + WString::Unmanaged(L"/Properties"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Click")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					TEST_ASSERT(comboBox->GetSelectedIndex() == -1);
					TEST_ASSERT(comboBox->GetSelectedItem().IsNull());
					TEST_ASSERT(comboBox->GetText() == L"");
					auto location = protocol->LocationOf(comboBox);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Expand", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					auto listControl = FindObjectByName<GuiListControl>(window, L"list");
					LClickListItem(protocol, listControl, 1);
					TEST_ASSERT(comboBox->GetSelectedIndex() == 1);
					auto value = UnboxValue<Ptr<TextItem>>(comboBox->GetSelectedItem());
					TEST_ASSERT(value->GetText() == L"Item 2");
					TEST_ASSERT(comboBox->GetText() == L"Item 2");
				});
				protocol->OnNextIdleFrame(L"Click 2nd", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/Combo/") + pathFragment + WString::Unmanaged(L"/Click"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Key")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					TEST_ASSERT(comboBox->GetSelectedIndex() == -1);
					TEST_ASSERT(comboBox->GetSelectedItem().IsNull());
					TEST_ASSERT(comboBox->GetText() == L"");
					comboBox->SetFocused();
				});
				protocol->OnNextIdleFrame(L"Focused", [=]()
				{
					protocol->KeyPress(VKEY::KEY_SPACE);
				});
				protocol->OnNextIdleFrame(L"[SPACE]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_HOME);
				});
				protocol->OnNextIdleFrame(L"[HOME]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					protocol->KeyPress(VKEY::KEY_RETURN);
					TEST_ASSERT(comboBox->GetSelectedIndex() == 0);
					auto value = UnboxValue<Ptr<TextItem>>(comboBox->GetSelectedItem());
					TEST_ASSERT(value->GetText() == L"Item 1");
					TEST_ASSERT(comboBox->GetText() == L"Item 1");
				});
				protocol->OnNextIdleFrame(L"[ENTER]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/Combo/") + pathFragment + WString::Unmanaged(L"/Key"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});

		TEST_CASE(L"Alt")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					TEST_ASSERT(comboBox->GetSelectedIndex() == -1);
					TEST_ASSERT(comboBox->GetSelectedItem().IsNull());
					TEST_ASSERT(comboBox->GetText() == L"");
					protocol->KeyPress(VKEY::KEY_MENU);
				});
				protocol->OnNextIdleFrame(L"[ALT]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_C);
				});
				protocol->OnNextIdleFrame(L"[C]", [=]()
				{
					protocol->KeyPress(VKEY::KEY_END);
				});
				protocol->OnNextIdleFrame(L"[END]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"combo");
					protocol->KeyPress(VKEY::KEY_RETURN);
					TEST_ASSERT(comboBox->GetSelectedIndex() == 4);
					auto value = UnboxValue<Ptr<TextItem>>(comboBox->GetSelectedItem());
					TEST_ASSERT(value->GetText() == L"Item 5");
					TEST_ASSERT(comboBox->GetText() == L"Item 5");
				});
				protocol->OnNextIdleFrame(L"[ENTER]", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Toolstrip/Combo/") + pathFragment + WString::Unmanaged(L"/Alt"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceXml
				);
		});
	}
}