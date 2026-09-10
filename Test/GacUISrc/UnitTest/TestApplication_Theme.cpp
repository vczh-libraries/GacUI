#include "TestControls.h"

using namespace gacui_unittest_template;

namespace
{
	void ConnectThemeTestProtocol(UnitTestRemoteProtocol* protocol)
	{
		remoteprotocol::ControllerGlobalConfig config;
#if defined VCZH_WCHAR_UTF16
		config.documentCaretFromEncoding = remoteprotocol::CharacterEncoding::UTF16;
#else
		config.documentCaretFromEncoding = remoteprotocol::CharacterEncoding::UTF32;
#endif
		protocol->GetEvents()->OnControllerConnect(config);
	}
}

TEST_FILE
{
	const auto resourceControlThemeName = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="ControlThemeName" ClientSize="x:480 y:320">
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" MinSizeLimitation="LimitToElementAndChildren" CellPadding="5" BorderVisible="true">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>
          
          <Cell Site="row:0 column:0">
            <CheckBox ref.Name="checkbox1" Text="Selectable Button" Selected="true">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </CheckBox>
          </Cell>
          
          <Cell Site="row:1 column:0">
            <CheckBox ref.Name="checkbox2" Text="Selectable Button" Selected="false">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
            </CheckBox>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Theme")
	{
		TEST_CASE(L"RefreshThemes preserves overrides and safely snapshots windows")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				ConnectThemeTestProtocol(protocol);
				auto skin = Ptr(new darkskin::Theme);
				theme::RegisterTheme(skin);
				auto changingTheme = Ptr(new theme::ThemeTemplates);
				changingTheme->Name = L"RefreshThemesRegression";
				Color accent(10, 20, 30);
				auto originalLabel = theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::Label);
				changingTheme->Label = [=, &accent](const Value& value)
				{
					auto result = dynamic_cast<templates::GuiLabelTemplate*>(originalLabel(value));
					result->SetDefaultTextColor(accent);
					return result;
				};
				theme::RegisterTheme(changingTheme);
				{
					GuiWindow main(theme::ThemeName::Window);
					GuiWindow second(theme::ThemeName::Window);
					GuiPopup hidden(theme::ThemeName::Window);
					main.SetText(L"Refresh themes");
					main.SetClientSize(Size(480, 320));
					auto parent = new GuiControl(theme::ThemeName::GroupBox);
					auto overriddenParent = new GuiControl(theme::ThemeName::GroupBox);
					overriddenParent->SetControlTemplate(theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::GroupBox));
					main.GetContainerComposition()->AddChild(parent->GetBoundsComposition());
					main.GetContainerComposition()->AddChild(overriddenParent->GetBoundsComposition());
					auto addLabel = [](GuiControl* owner)
					{
						auto label = new GuiLabel(theme::ThemeName::Label);
						label->SetText(L"Retained label");
						owner->GetContainerComposition()->AddChild(label->GetBoundsComposition());
						return label;
					};
					auto child = addLabel(parent);
					auto overrideChild = addLabel(overriddenParent);
					auto overrideLeaf = addLabel(parent);
					overrideLeaf->SetControlTemplate(originalLabel);
					auto otherChild = addLabel(&second);
					auto popupChild = addLabel(&hidden);
					auto parentTemplate = overriddenParent->GetControlTemplateObject();
					auto leafTemplate = overrideLeaf->GetControlTemplateObject();
					auto container = parent->GetContainerComposition();
					auto childFlag = child->GetDisposedFlag();
					auto datePicker = new GuiDatePicker(theme::ThemeName::DatePicker);
					main.GetContainerComposition()->AddChild(datePicker->GetBoundsComposition());
					datePicker->GetControlTemplateObject();
					collections::List<Ptr<GuiDisposedFlag>> oldWindows;
					for (auto window : GetApplication()->GetWindows()) oldWindows.Add(window->GetDisposedFlag());
					auto checkColor = [&](GuiLabel* label)
					{
						TEST_ASSERT(dynamic_cast<templates::GuiLabelTemplate*>(label->GetControlTemplateObject())->GetDefaultTextColor() == accent);
					};
					protocol->OnNextIdleFrame(L"Ready", [&]()
					{
						TEST_ASSERT(!hidden.GetOpening());
						for (vint i = 0; i < 3; i++)
						{
							accent = Color((vuint8_t)(100 + i), 110, 120);
							GetApplication()->RefreshThemes();
							checkColor(child);
							checkColor(overrideChild);
							checkColor(otherChild);
							checkColor(popupChild);
							TEST_ASSERT(!childFlag->IsDisposed() && parent->GetContainerComposition() == container);
							TEST_ASSERT(overriddenParent->GetControlTemplateObject() == parentTemplate);
							TEST_ASSERT(overrideLeaf->GetControlTemplateObject() == leafTemplate);
							TEST_ASSERT(overriddenParent->GetControlTemplate() && overrideLeaf->GetControlTemplate());
							TEST_ASSERT(!parent->GetControlTemplate() && !child->GetControlTemplate());
						}
						TEST_ASSERT(collections::From(oldWindows).Any([](auto flag) {return flag->IsDisposed(); }));
						hidden.ShowPopup(&main, true);
					});
					protocol->OnNextIdleFrame(L"Hidden popup opened with refreshed label", [&]()
					{
						TEST_ASSERT(hidden.GetOpening());
						checkColor(popupChild);
						hidden.Hide();
						main.Hide();
					});
					GetApplication()->Run(&main);
				}
				theme::UnregisterTheme(changingTheme->Name);
				theme::UnregisterTheme(skin->Name);
			});
			GacUIUnitTest_Start(L"Application/RefreshThemes/OverridesAndWindows");
		});

		TEST_CASE(L"RefreshThemes preserves editing, focus and logical scroll state after layout")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				ConnectThemeTestProtocol(protocol);
				auto skin = Ptr(new darkskin::Theme);
				theme::RegisterTheme(skin);
				{
					GuiWindow main(theme::ThemeName::Window);
					main.SetText(L"Refresh state");
					main.SetClientSize(Size(800, 600));
					auto add = [&](GuiControl* control, Rect bounds)
					{
						control->GetBoundsComposition()->SetExpectedBounds(bounds);
						main.GetContainerComposition()->AddChild(control->GetBoundsComposition());
					};
					auto editor = new GuiMultilineTextBox(theme::ThemeName::MultilineTextBox);
					editor->SetAcceptTabInput(false);
					add(editor, Rect(0, 0, 300, 180));
					TEST_ASSERT(editor->GetDocument()->GetStyle(DocumentModel::DefaultStyleName, {}).backgroundColor.a == 0);
					WString original;
					for (vint i = 0; i < 40; i++) original += L"Line " + itow(i) + L" retained content\r\n";
					editor->LoadTextAndClearUndoRedo(original);
					auto scroll = new GuiScrollContainer(theme::ThemeName::ScrollView);
					add(scroll, Rect(310, 0, 600, 180));
					scroll->GetContainerComposition()->SetPreferredMinSize(Size(1000, 1000));
					auto check = new GuiSelectableButton(theme::ThemeName::CheckBox);
					add(check, Rect(0, 190, 150, 220));
					check->SetText(L"Retained selection");
					check->SetSelected(true);
					auto tab = new GuiTab(theme::ThemeName::Tab);
					add(tab, Rect(0, 230, 300, 420));
					auto firstPage = new GuiTabPage(theme::ThemeName::CustomControl);
					firstPage->SetText(L"First");
					tab->GetPages().Add(firstPage);
					auto selectedPage = new GuiTabPage(theme::ThemeName::CustomControl);
					tab->GetPages().Add(selectedPage);
					selectedPage->SetText(L"Second");
					tab->SetSelectedPage(selectedPage);
					Point editorPosition;
					Ptr<DocumentModel> document;
					WString edited;
					protocol->OnNextIdleFrame(L"Ready", [&]()
					{
						editor->SetFocused();
						editor->SetCaret(TextPos(30, 0), TextPos(30, 0));
						editor->SetSelectionText(L"first ");
						editor->SetSelectionText(L"second ");
						TEST_ASSERT(editor->Undo());
						editor->SetCaret(TextPos(30, 1), TextPos(30, 4));
						scroll->SetViewPosition(Point(120, 150));
					});
					protocol->OnNextIdleFrame(L"Edited and scrolled", [&]()
					{
						document = editor->GetDocument();
						edited = editor->GetText();
						editorPosition = editor->GetViewPosition();
						TEST_ASSERT(editorPosition.y > 0 && scroll->GetViewPosition() == Point(120, 150));
						TEST_ASSERT(editor->GetFocused() && editor->CanUndo() && editor->CanRedo() && editor->GetModified());
						GetApplication()->RefreshThemes();
					});
					protocol->OnNextIdleFrame(L"Refreshed and layout settled", [&]()
					{
						TEST_ASSERT(editor->GetDocument() == document && editor->GetText() == edited);
						TEST_ASSERT(editor->GetCaretBegin() == TextPos(30, 1) && editor->GetCaretEnd() == TextPos(30, 4));
						TEST_ASSERT(editor->GetFocused() && editor->CanUndo() && editor->CanRedo() && editor->GetModified());
						TEST_ASSERT(editor->GetViewPosition() == editorPosition);
						TEST_ASSERT(scroll->GetViewPosition() == Point(120, 150));
						TEST_ASSERT(check->GetSelected() && tab->GetSelectedPage() == selectedPage);
						protocol->TypeString(L"M");
						TEST_ASSERT(editor->GetText() != edited);
						TEST_ASSERT(editor->Undo() && editor->GetText() == edited);
						TEST_ASSERT(editor->Redo());
						GetApplication()->RefreshThemes();
					});
					protocol->OnNextIdleFrame(L"Keyboard input and undo redo survive another refresh", [&]()
					{
						TEST_ASSERT(editor->GetFocused() && editor->CanUndo() && !editor->CanRedo());
						TEST_ASSERT(editor->Undo() && editor->GetText() == edited);
						protocol->KeyPress(VKEY::KEY_TAB);
						TEST_ASSERT(!editor->GetFocused() && editor->GetText() == edited);
						protocol->KeyPress(VKEY::KEY_TAB, false, true, false);
						TEST_ASSERT(editor->GetFocused());
						main.Hide();
					});
					GetApplication()->Run(&main);
				}
				theme::UnregisterTheme(skin->Name);
			});
			GacUIUnitTest_Start(L"Application/RefreshThemes/EditingAndScroll");
		});

		TEST_CASE(L"ControlThemeName")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());

					checkbox1->SetControlThemeName(theme::ThemeName::RadioButton);
					checkbox2->SetControlThemeName(theme::ThemeName::RadioButton);

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::RadioButton);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::RadioButton);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Become RadioButton", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					checkbox1->SetControlThemeName(theme::ThemeName::CheckBox);
					checkbox2->SetControlThemeName(theme::ThemeName::CheckBox);

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Back to CheckBox", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/ControlThemeName"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceControlThemeName
			);
		});

		TEST_CASE(L"ControlTemplate")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				auto checkboxTheme = theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::CheckBox);
				auto radiobuttonTheme = theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::RadioButton);

				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());

					checkbox1->SetControlTemplate(radiobuttonTheme);
					checkbox2->SetControlTemplate(radiobuttonTheme);

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox1->GetControlTemplate());
					TEST_ASSERT(checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Become RadioButton", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto checkbox1 = FindObjectByName<GuiSelectableButton>(window, L"checkbox1");
					auto checkbox2 = FindObjectByName<GuiSelectableButton>(window, L"checkbox2");

					checkbox1->SetControlTemplate({});
					checkbox2->SetControlTemplate({});

					TEST_ASSERT(checkbox1->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(checkbox2->GetControlThemeName() == theme::ThemeName::CheckBox);
					TEST_ASSERT(!checkbox1->GetControlTemplate());
					TEST_ASSERT(!checkbox2->GetControlTemplate());
				});
				protocol->OnNextIdleFrame(L"Back to CheckBox", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/ControlTemplate"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceControlThemeName
			);
		});
	});
}
