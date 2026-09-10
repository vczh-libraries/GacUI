#include "TestControls.h"
#include "../../../Source/Skins/DarkSkin/Config/DarkSkinConfig.h"

using namespace gacui_unittest_template;

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
		TEST_CASE(L"DarkSkin exposes complete palettes with shared neutrals")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				ConnectThemeTestProtocol(protocol);
				struct ColorRole
				{
					Color darkskin::ColorPackage::* field;
					const wchar_t* expected;
					bool accent;
				};
				const ColorRole roles[] =
				{
					{ &darkskin::ColorPackage::GeneralBackground, L"#2D2D30", false },
					{ &darkskin::ColorPackage::GeneralBorder, L"#434346", false },
					{ &darkskin::ColorPackage::GeneralAccent, L"#007ACC", true },
					{ &darkskin::ColorPackage::ContentBackground, L"#252526", false },
					{ &darkskin::ColorPackage::ContentBorder, L"#3F3F46", false },
					{ &darkskin::ColorPackage::Transparent, L"#00000000", false },
					{ &darkskin::ColorPackage::WindowBorderActive, L"#017ACC", true },
					{ &darkskin::ColorPackage::TextSecondary, L"#999999", false },
					{ &darkskin::ColorPackage::TextDisabled, L"#6D6D6D", false },
					{ &darkskin::ColorPackage::GroupText, L"#C7C7C7", false },
					{ &darkskin::ColorPackage::TextNormal, L"#F1F1F1", false },
					{ &darkskin::ColorPackage::TextBright, L"#FFFFFF", false },
					{ &darkskin::ColorPackage::ButtonBackgroundHovered, L"#54545C", false },
					{ &darkskin::ColorPackage::ButtonBorderHovered, L"#6A6A75", false },
					{ &darkskin::ColorPackage::ControlAccentHovered, L"#1C97EA", true },
					{ &darkskin::ColorPackage::ListColumnBorder, L"#404042", false },
					{ &darkskin::ColorPackage::TabHighlightedSelected, L"#CC7ACC", true },
					{ &darkskin::ColorPackage::TabHighlightedHovered, L"#EA97EA", true },
					{ &darkskin::ColorPackage::TabHighlightedBackground, L"#604360", true },
					{ &darkskin::ColorPackage::ScrollBackground, L"#3D3D42", false },
					{ &darkskin::ColorPackage::ArrowDisabled, L"#555558", false },
					{ &darkskin::ColorPackage::ArrowAccentHovered, L"#1997EA", true },
					{ &darkskin::ColorPackage::ScrollHandleHovered, L"#9E9E9E", false },
					{ &darkskin::ColorPackage::ScrollHandlePressed, L"#EFEBEF", false },
					{ &darkskin::ColorPackage::ScrollHandle, L"#686868", false },
					{ &darkskin::ColorPackage::ProgressBackground, L"#3F3F47", false },
					{ &darkskin::ColorPackage::ProgressBorder, L"#55545A", false },
					{ &darkskin::ColorPackage::ProgressFilling, L"#07B023", true },
					{ &darkskin::ColorPackage::ItemBackgroundSelected, L"#3399FF", true },
					{ &darkskin::ColorPackage::MenuBackground, L"#1B1B1C", false },
					{ &darkskin::ColorPackage::MenuBorder, L"#333337", false },
					{ &darkskin::ColorPackage::SplitterDark, L"#222224", false },
					{ &darkskin::ColorPackage::SplitterLight, L"#464648", false },
					{ &darkskin::ColorPackage::MenuItemHovered, L"#3D3D40", false },
					{ &darkskin::ColorPackage::ComboArrowBackgroundHovered, L"#1F1F20", false },
					{ &darkskin::ColorPackage::ColumnHeaderBackgroundHovered, L"#3E3E40", false },
					{ &darkskin::ColorPackage::ColumnHeaderBackground, L"#252527", false },
					{ &darkskin::ColorPackage::ExpandingArrowHovered, L"#0A75B9", true },
					{ &darkskin::ColorPackage::RibbonExpandingArrow, L"#A0D0FF", true },
					{ &darkskin::ColorPackage::RibbonExpandingArrowPressed, L"#004879", true },
				};
				const darkskin::ColorPackage palettes[] =
				{
					darkskin::CreateDefaultColorPackage(),
					darkskin::CreateAuroraColorPackage(),
					darkskin::CreateEmberColorPackage(),
					darkskin::CreateMoonstoneColorPackage(),
					darkskin::CreateLagoonColorPackage(),
					darkskin::CreateRosewoodColorPackage(),
				};
				auto installed = darkskin::Theme::GetColorPackage();
				for (auto&& role : roles)
				{
					auto expected = Color::Parse(role.expected);
					TEST_ASSERT(palettes[0].*role.field == expected);
					TEST_ASSERT(installed.*role.field == expected);
					for (vint i = 1; i < 6; i++)
					{
						if (role.accent)
						{
							for (vint j = 0; j < i; j++)
							{
								TEST_ASSERT(palettes[i].*role.field != palettes[j].*role.field);
							}
						}
						else
						{
							TEST_ASSERT(palettes[i].*role.field == expected);
						}
					}
				}
				auto skin = Ptr(new darkskin::Theme);
				theme::RegisterTheme(skin);
				{
					GuiWindow window(theme::ThemeName::Window);
					window.SetText(L"Default palette initialized automatically");
					window.SetClientSize(Size(400, 300));
					protocol->OnNextIdleFrame(L"Default", [&]() { window.Hide(); });
					GetApplication()->Run(&window);
				}
				theme::UnregisterTheme(skin->Name);
			});
			GacUIUnitTest_Start(L"Application/DarkSkin/PaletteContract");
		});

		TEST_CASE(L"DarkSkin palette refresh updates templates and document selection")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				ConnectThemeTestProtocol(protocol);
				auto skin = Ptr(new darkskin::Theme);
				theme::RegisterTheme(skin);
				{
					GuiWindow window(theme::ThemeName::Window);
					window.SetText(L"DarkSkin palettes");
					window.SetClientSize(Size(400, 300));
					auto button = new GuiButton(theme::ThemeName::Button);
					button->SetText(L"Palette input");
					button->GetBoundsComposition()->SetExpectedBounds(Rect(10, 10, 160, 50));
					window.GetContainerComposition()->AddChild(button->GetBoundsComposition());
					auto editor = new GuiMultilineTextBox(theme::ThemeName::MultilineTextBox);
					editor->GetBoundsComposition()->SetExpectedBounds(Rect(10, 60, 380, 250));
					window.GetContainerComposition()->AddChild(editor->GetBoundsComposition());
					editor->LoadTextAndClearUndoRedo(L"Retained document");
					editor->SetCaret(TextPos(0, 0), TextPos(0, 0));
					editor->SetSelectionText(L"Edited ");
					editor->SetCaret(TextPos(0, 1), TextPos(0, 5));
					auto document = editor->GetDocument();
					auto buttonColor = [&]()
					{
						return button->GetControlTemplateObject()->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->GetColor();
					};
					for (vint preset = 0; preset < 6; preset++)
					{
						protocol->OnNextIdleFrame(L"Select palette " + itow(preset), [&, preset]()
						{
							auto colors = darkskin::Theme::CreateColorPackage(preset);
							GetApplication()->InvokeInMainThread(&window, [&, colors]()
							{
								darkskin::SetColorPackage(colors);
								GetApplication()->RefreshThemes();
							});
						});
						protocol->OnNextIdleFrame(L"Palette installed " + itow(preset), [&, preset]()
						{
							auto colors = darkskin::Theme::CreateColorPackage(preset);
							TEST_ASSERT(editor->GetDocument() == document);
							TEST_ASSERT(editor->GetCaretBegin() == TextPos(0, 1) && editor->GetCaretEnd() == TextPos(0, 5));
							TEST_ASSERT(editor->CanUndo());
							auto selected = document->GetStyle(DocumentModel::SelectionStyleName, {});
							TEST_ASSERT(selected.backgroundColor == colors.GeneralAccent);
							TEST_ASSERT(selected.color == colors.TextBright);
							TEST_ASSERT(document->GetStyle(DocumentModel::DefaultStyleName, {}).backgroundColor.a == 0);
							{
								darkskin::Theme anotherTheme;
								TEST_ASSERT(darkskin::Theme::GetColorPackage().GeneralAccent == colors.GeneralAccent);
								GuiSinglelineTextBox fresh(theme::ThemeName::SinglelineTextBox);
								TEST_ASSERT(fresh.GetControlTemplateObject() != nullptr);
								TEST_ASSERT(fresh.GetDocument()->GetStyle(DocumentModel::SelectionStyleName, {}).backgroundColor == colors.GeneralAccent);
							}
							protocol->MouseMove(protocol->LocationOf(button));
							TEST_ASSERT(buttonColor() == colors.ButtonBackgroundHovered);
							protocol->_LDown();
							TEST_ASSERT(buttonColor() == colors.GeneralAccent);
							protocol->_LUp();
							button->SetEnabled(false);
							TEST_ASSERT(buttonColor() == colors.ContentBackground);
							button->SetEnabled(true);
							protocol->MouseMove(protocol->LocationOf(editor));
						});
					}
					protocol->OnNextIdleFrame(L"Restore default", [&]()
					{
						darkskin::SetColorPackage(darkskin::CreateDefaultColorPackage());
						GetApplication()->RefreshThemes();
						window.Hide();
					});
					GetApplication()->Run(&window);
				}
				theme::UnregisterTheme(skin->Name);
			});
			GacUIUnitTest_Start(L"Application/DarkSkin/RefreshPalettes");
		});

		TEST_CASE(L"List render target detachment preserves realized rows and scrolling")
		{
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				ConnectThemeTestProtocol(protocol);
				auto skin = Ptr(new darkskin::Theme);
				theme::RegisterTheme(skin);
				{
					class DetachableTextList : public GuiTextList
					{
					public:
						DetachableTextList() : GuiTextList(theme::ThemeName::TextList) {}
						void NotifyRenderTargetDetached() { OnRenderTargetChanged(nullptr); }
					};
					GuiWindow window(theme::ThemeName::Window);
					window.SetText(L"Retained list position");
					window.SetClientSize(Size(640, 480));
					auto list = new DetachableTextList;
					list->GetBoundsComposition()->SetAlignmentToParent(Margin(5, 5, 5, 5));
					window.GetContainerComposition()->AddChild(list->GetBoundsComposition());
					vint createdRows = 0;
					list->SetItemTemplate([&](const Value&)
					{
						createdRows++;
						return new DefaultTextListItemTemplate;
					});
					for (vint i = 0; i < 60; i++) list->GetItems().Add(Ptr(new TextItem(itow(i), false)));
					protocol->OnNextIdleFrame(L"Scroll list", [&]()
					{
						list->SetSelected(3, true);
						list->SetViewPosition(Point(0, 192));
					});
					protocol->OnNextIdleFrame(L"Detach render target", [&]()
					{
						TEST_ASSERT(list->GetViewPosition() == Point(0, 192));
						auto previousCreatedRows = createdRows;
						TEST_ASSERT(previousCreatedRows > 0);
						list->NotifyRenderTargetDetached();
						TEST_ASSERT(createdRows == previousCreatedRows);
						TEST_ASSERT(list->GetViewPosition() == Point(0, 192));
						TEST_ASSERT(list->GetSelected(3) && list->GetItems().Count() == 60);
						window.Hide();
					});
					GetApplication()->Run(&window);
				}
				theme::UnregisterTheme(skin->Name);
			});
			GacUIUnitTest_Start(L"Application/DarkSkin/DetachedListScroll");
		});

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
