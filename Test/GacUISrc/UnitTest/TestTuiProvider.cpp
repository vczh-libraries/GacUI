#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/TUI/TuiController.h"
#include "../../../Source/PlatformProviders/TUI/TuiGraphics.h"
#include "../../../Source/PlatformProviders/Hosted/GuiHostedController.h"
#include "../../../Source/PlatformProviders/Hosted/GuiHostedGraphics.h"
#include "../../../Source/Controls/ListControlPackage/TuiItemTemplates.h"
#include "../Generated_TuiSkin/TuiSkinConfig.h"
#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
#include <algorithm>

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::console;

namespace tui_provider_tests
{
	using namespace vl::console::unittest;

	class TuiTestBackend : public Object, public ITuiBackend
	{
	public:
		vint width = 16;
		vint height = 8;
		vint starts = 0;
		vint stops = 0;
		vint presents = 0;
		vint nextEvent = 0;
		vuint64_t time = 0;
		List<TuiBackendEvent> events;
		Array<TuiPixel> frame;

		TuiColorMode Start(const TuiStartOptions&) override { starts++; return TuiColorMode::TrueColor; }
		void Stop() override { stops++; }
		bool TryGetConsoleSize(vint& x, vint& y) override { x = width; y = height; return true; }
		vuint64_t GetMonotonicTime() override { return time; }
		bool ReadEvent(vint milliseconds, TuiBackendEvent& event) override
		{
			time += std::max((vint)1, milliseconds);
			if (nextEvent == events.Count()) return false;
			event = events[nextEvent++];
			if (event.type == TuiBackendEventType::Resize) { width = event.width; height = event.height; }
			return true;
		}
		void Render(const TuiPixel* buffer, vint x, vint y, TuiColorMode) override
		{
			presents++;
			frame.Resize(x * y);
			for (vint i = 0; i < frame.Count(); i++) frame[i] = buffer[i];
		}
	};

	class TuiTestResourceService : public Object, public INativeResourceService
	{
	public:
		FontProperties font;
		INativeCursor* GetSystemCursor(INativeCursor::SystemCursorType) override { return nullptr; }
		INativeCursor* GetDefaultSystemCursor() override { return nullptr; }
		FontProperties GetDefaultFont() override { auto result = font; result.fontFamily = L"TuiFont"; result.size = 1; return result; }
		void SetDefaultFont(const FontProperties& value) override { font = value; }
		void EnumerateFonts(List<WString>& fonts) override { fonts.Add(L"TuiFont"); }
		WString GetOSSuperKeyName() override { return L"Super"; }
	};

	class TuiTestInputService : public Object, public INativeInputService
	{
	public:
		bool enabled = false;
		void StartTimer() override { enabled = true; TUI::StartTimer(16); }
		void StopTimer() override { enabled = false; TUI::StopTimer(); }
		bool IsTimerEnabled() override { return enabled; }
		bool IsKeyPressing(VKEY) override { return false; }
		bool IsKeyToggled(VKEY) override { return false; }
		WString GetKeyName(VKEY) override { return L"TestKey"; }
		VKEY GetKey(const WString&) override { return VKEY::KEY_UNKNOWN; }
		vint RegisterGlobalShortcutKey(bool, bool, bool, bool, VKEY) override { return 1; }
		bool UnregisterGlobalShortcutKey(vint) override { return true; }
	};

	class TuiTestController : public TuiControllerBase
	{
	public:
		Func<void()> test;
		WString title;
		vint pumps = 0;
		TuiTestResourceService resources;
		TuiTestInputService input;
		INativeResourceService* ResourceService() override { return &resources; }
		INativeInputService* InputService() override { return &input; }
		INativeClipboardService* ClipboardService() override { return nullptr; }
		INativeImageService* ImageService() override { return nullptr; }
		WString GetExecutablePath() override { return L"TuiTest"; }
		void Starting() override { test(); Stop(); }
		void PumpPlatformEvents() override { pumps++; }
		void ApplyTitle(const WString& value) override { title = value; }
	};

	class TuiCountingParagraph : public TuiGraphicsParagraph
	{
	public:
		vint layouts = 0;
		TuiCountingParagraph(const WString& text, TuiGraphicsLayoutProvider* provider, TuiGraphicsRenderTarget* target, IGuiGraphicsParagraphCallback* callback)
			: TuiGraphicsParagraph(text, provider, target, callback) {}
		Size GetSize() override { if (dirty) layouts++; return TuiGraphicsParagraph::GetSize(); }
		void Render(Rect bounds) override { if (dirty) layouts++; TuiGraphicsParagraph::Render(bounds); }
	};

	class TuiCountingLayoutProvider : public TuiGraphicsLayoutProvider
	{
	public:
		vint creations = 0;
		Ptr<TuiCountingParagraph> last;
		Ptr<IGuiGraphicsParagraph> CreateParagraph(const WString& text, IGuiGraphicsRenderTarget* target, IGuiGraphicsParagraphCallback* callback) override
		{
			creations++;
			last = Ptr(new TuiCountingParagraph(text, this, dynamic_cast<TuiGraphicsRenderTarget*>(target), callback));
			return last;
		}
	};

	class TuiCountingResources : public TuiGraphicsResourceManager
	{
	public:
		TuiCountingLayoutProvider provider;
		IGuiGraphicsLayoutProvider* GetLayoutProvider() override { return &provider; }
	};

	class TuiTestWindowListener : public INativeWindowListener
	{
	public:
		vint moved = 0;
		vint destroying = 0;
		vint destroyed = 0;
		vint keys = 0;
		vint releases = 0;
		vint chars = 0;
		vint mouse = 0;
		vint wheel = 0;
		NativeWindowKeyInfo key;
		NativeWindowCharInfo character;
		NativeWindowMouseInfo position;
		void Moved() override { moved++; }
		void Destroying() override { destroying++; }
		void Destroyed() override { destroyed++; }
		void KeyDown(const NativeWindowKeyInfo& info) override { keys++; key = info; }
		void KeyUp(const NativeWindowKeyInfo&) override { releases++; }
		void Char(const NativeWindowCharInfo& info) override { chars++; character = info; }
		void MouseMoving(const NativeWindowMouseInfo& info) override { mouse++; position = info; }
		void VerticalWheel(const NativeWindowMouseInfo& info) override { wheel = info.wheel; }
	};

	class TuiTestTimerListener : public INativeControllerListener
	{
	public:
		vint ticks = 0;
		void GlobalTimer() override { ticks++; }
	};

	class TuiTestClosingListener : public INativeWindowListener
	{
	public:
		Func<void(bool&)> before;
		Func<void()> after;
		Func<void()> closed;
		void BeforeClosing(bool& cancel) override { before(cancel); }
		void AfterClosing() override { after(); }
		void Closed() override { closed(); }
	};

	void TuiRunTest(const Func<void(TuiTestBackend*, TuiTestController*)>& test, Size size = Size(16, 8))
	{
		auto backend = Ptr(new TuiTestBackend);
		backend->width = size.x;
		backend->height = size.y;
		ScopedTuiBackend scoped(backend);
		TuiTestController controller;
		controller.test = [&]() { test(backend.Obj(), &controller); };
		TEST_ASSERT(TUI::InstallListener(&controller));
		TUI::Start({});
		TEST_ASSERT(TUI::UninstallListener(&controller));
		TEST_ASSERT(backend->starts == 1 && backend->stops == 1);
		TEST_ASSERT(!TUI::IsInUse());
		TEST_ASSERT(GetTuiApplication() == nullptr);
	}
}

using namespace tui_provider_tests;

TEST_FILE
{
	TEST_CASE(L"TUI presets share every neutral role and preserve the default palette")
	{
		auto skyblue = tuiskin::CreateSkyblueColorPackage();
		TEST_ASSERT(tuiskin::CreateDefaultColorPackage() == skyblue);
		TEST_ASSERT(skyblue.ControlBorderFocused == Color(0x87, 0xCE, 0xFA));
		TEST_ASSERT(skyblue.MenuBackgroundHighlighted == Color(0, 0, 0x80));
		List<Color> accents;
		for (auto colors : { tuiskin::CreatePinkColorPackage(), tuiskin::CreateOrangeColorPackage(), tuiskin::CreateGrassPackage(), tuiskin::CreateEmeraldPackage(), skyblue, tuiskin::CreatePurplePackage() })
		{
			TEST_ASSERT(!accents.Contains(colors.ControlBorderFocused));
			accents.Add(colors.ControlBorderFocused);
			TEST_ASSERT(colors.ControlBorderFocused.a == 255 && colors.MenuBackgroundHighlighted.a == 255);
			TEST_ASSERT(colors.ItemBackgroundSelected == colors.ControlBorderFocused);
			TEST_ASSERT(colors.ButtonBackgroundHighlighted == colors.ControlBorderFocused);
			TEST_ASSERT(colors.ItemBackgroundHighlighted == colors.MenuBackgroundHighlighted);
			colors.ControlBorderFocused = skyblue.ControlBorderFocused;
			colors.ItemBackgroundSelected = skyblue.ItemBackgroundSelected;
			colors.ButtonBackgroundHighlighted = skyblue.ButtonBackgroundHighlighted;
			colors.ItemBackgroundHighlighted = skyblue.ItemBackgroundHighlighted;
			colors.MenuBackgroundHighlighted = skyblue.MenuBackgroundHighlighted;
			TEST_ASSERT(colors == skyblue);
		}
	});

	TEST_CASE(L"TUI live themes refresh retained headers, menus, date combos and trees")
	{
		using namespace vl::presentation::unittest;
		GacUIUnitTest_SetGuiMainProxy([](auto, auto)
		{
			TuiRunTest([](TuiTestBackend*, TuiTestController* controller)
			{
				using namespace controls;
				using namespace compositions;
				auto previousResources = GetGuiGraphicsResourceManager();
				auto previousController = GetNativeController();
				auto previousHosted = GetHostedApplication();
				GuiHostedController hosted(controller);
				TuiGraphicsResourceManager resources;
				GuiHostedGraphicsResourceManager hostedResources(&hosted, &resources);
				SetNativeController(&hosted);
				SetHostedApplication(nullptr);
				SetHostedApplication(hosted.GetHostedApplication());
				SetTuiApplication(controller);
				SetGuiGraphicsResourceManager(&hostedResources);
				controller->CallbackService()->InstallListener(&resources);
				RegisterTuiRenderers();
				hosted.Initialize();
				tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage());
				auto skin = Ptr(new tuiskin::TuiTheme);
				theme::RegisterTheme(skin);
				{
					GuiWindow main(theme::ThemeName::SystemFrameWindow);
					main.SetClientSize(Size(120, 40));
					auto add = [&](GuiControl* control, Rect bounds)
					{
						main.GetContainerComposition()->AddChild(control->GetBoundsComposition());
						control->GetBoundsComposition()->SetExpectedBounds(bounds);
					};
					auto listView = new GuiListView(theme::ThemeName::ListView);
					add(listView, Rect(0, 0, 45, 12));
					auto column = Ptr(new list::ListViewColumn(L"Retained", 60));
					column->SetSortingState(ColumnSortingState::Ascending);
					auto filter = new GuiMenu(theme::ThemeName::Menu, listView);
					column->SetDropdownPopup(filter);
					listView->GetColumns().Add(column);
					for (vint i = 0; i < 40; i++)
					{
						auto item = Ptr(new list::ListViewItem);
						item->SetText(L"Row " + itow(i));
						listView->GetItems().Add(item);
					}
					listView->SetView(ListViewView::Detail);
					listView->SetSelected(20, true);
					auto menu = new GuiMenuButton(theme::ThemeName::MenuItemButton);
					add(menu, Rect(0, 14, 20, 15));
					vint menuActions = 0;
					menu->BeforeSubMenuOpening.AttachLambda([&](auto, auto&) {menuActions++; });
					auto combo = new GuiComboBoxBase(theme::ThemeName::ComboBox, false);
					add(combo, Rect(25, 14, 45, 15));
					auto contentPopup = combo->CreateSubMenu();
					auto content = new GuiLabel(theme::ThemeName::Label);
					content->SetText(L"Retained content");
					contentPopup->GetContainerComposition()->AddChild(content->GetBoundsComposition());
					auto contentTemplate = contentPopup->GetControlTemplateObject();
					auto date = new GuiDateComboBox(theme::ThemeName::DateComboBox);
					add(date, Rect(0, 16, 30, 17));
					auto selectedDate = DateTime::FromDateTime(2024, 2, 29);
					date->SetSelectedDate(selectedDate);
					auto treeView = new GuiTreeView(theme::ThemeName::TreeView);
					add(treeView, Rect(50, 0, 75, 12));
					GuiWindow modal(theme::ThemeName::Window);
					modal.SetClientSize(Size(30, 8));
					auto node = Ptr(new tree::MemoryNodeProvider);
					treeView->Nodes()->SetTreeViewData(node.Obj(), Ptr(new tree::TreeViewItem(nullptr, L"Expanded")));
					treeView->Nodes()->Children().Add(node);
					auto childNode = Ptr(new tree::MemoryNodeProvider);
					treeView->Nodes()->SetTreeViewData(childNode.Obj(), Ptr(new tree::TreeViewItem(nullptr, L"Child")));
					node->Children().Add(childNode);
					node->SetExpanding(true);
					List<Func<void()>> steps;
					Point viewPosition;
					NativeRect mainBounds;
					NativeRect modalBounds;
					list::ListViewColumnItemArranger* arranger = nullptr;
					GuiListViewColumnHeader* header = nullptr;
					Ptr<GuiDisposedFlag> headerFlag;
					Ptr<GuiDisposedFlag> arrowFlag;
					steps.Add([&]()
					{
						main.ForceCalculateSizeImmediately();
						listView->CalculateView();
					});
					steps.Add([&]()
					{
						listView->SetViewPosition(Point(5, 10));
						viewPosition = listView->GetViewPosition();
						TEST_ASSERT(viewPosition.x > 0 && viewPosition.y > 0);
						mainBounds = main.GetNativeWindow()->GetBounds();
						arranger = dynamic_cast<list::ListViewColumnItemArranger*>(listView->GetArranger());
						TEST_ASSERT(arranger && arranger->GetColumnButtons().Count() == 1);
						header = arranger->GetColumnButtons()[0];
						headerFlag = header->GetDisposedFlag();
					});
					for (auto colors : { tuiskin::CreatePinkColorPackage(), tuiskin::CreateOrangeColorPackage(), tuiskin::CreateGrassPackage(), tuiskin::CreateEmeraldPackage(), tuiskin::CreatePurplePackage(), tuiskin::CreateSkyblueColorPackage() })
					{
						steps.Add([&, colors]()
						{
							arrowFlag = header->GetSubMenuHost()->GetDisposedFlag();
							tuiskin::SetColorPackage(colors);
							GetApplication()->RefreshThemes();
							main.ForceCalculateSizeImmediately();
						});
						steps.Add([&, colors]()
						{
							TEST_ASSERT(main.GetOpening() && main.GetNativeWindow()->GetBounds() == mainBounds);
							TEST_ASSERT(!headerFlag->IsDisposed() && arranger->GetColumnButtons()[0] == header);
							TEST_ASSERT(arrowFlag->IsDisposed());
							TEST_ASSERT(column->GetSize() == 60 && header->GetColumnSortingState() == ColumnSortingState::Ascending);
							TEST_ASSERT(header->GetSubMenu() == filter && column->GetDropdownPopup() == filter);
							auto headerTemplate = dynamic_cast<templates::GuiListViewColumnHeaderTemplate*>(header->GetControlTemplateObject());
							headerTemplate->SetState(ButtonState::Active);
							TEST_ASSERT(headerTemplate->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->GetColor() == colors.ButtonBackgroundHighlighted);
							headerTemplate->SetState(ButtonState::Pressed);
							TEST_ASSERT(headerTemplate->GetOwnedElement().Cast<GuiSolidBackgroundElement>()->GetColor() == colors.ButtonBackgroundPressed);
							TEST_ASSERT(listView->GetSelected(20) && node->GetExpanding());
							TEST_ASSERT(listView->GetViewPosition() == viewPosition);
							TEST_ASSERT(date->GetSelectedDate().osMilliseconds == selectedDate.osMilliseconds);
							TEST_ASSERT(contentPopup->GetControlTemplateObject() == contentTemplate);
							TEST_ASSERT(content->GetText() == L"Retained content");
							auto actions = menuActions;
							menu->GetSubMenuHost()->BeforeClicked.Execute(menu->GetNotifyEventArguments());
							TEST_ASSERT(menuActions == actions + 1);
						});
					}
					steps.Add([&]()
					{
						combo->SetSubMenuOpening(true);
						TEST_ASSERT(contentPopup->GetOpening());
						GetApplication()->RefreshThemes();
					});
					steps.Add([&]()
					{
						TEST_ASSERT(contentPopup->GetOpening());
						combo->SetSubMenuOpening(false);
						date->SetSubMenuOpening(true);
						TEST_ASSERT(date->GetSubMenuOpening());
						GetApplication()->RefreshThemes();
					});
					steps.Add([&]()
					{
						TEST_ASSERT(date->GetSubMenuOpening());
						date->SetSubMenuOpening(false);
						modal.ShowModal(&main, []() {});
					});
					steps.Add([&]()
					{
						TEST_ASSERT(modal.GetOpening());
						modalBounds = modal.GetNativeWindow()->GetBounds();
						tuiskin::SetColorPackage(tuiskin::CreatePurplePackage());
						GetApplication()->RefreshThemes();
						modal.ForceCalculateSizeImmediately();
					});
					steps.Add([&]()
					{
						TEST_ASSERT(modal.GetOpening() && modal.GetNativeWindow()->GetBounds() == modalBounds);
						modal.Hide();
					});
					steps.Add([&]() {main.Hide(); });
					vint nextStep = 0;
					Func<void()> runStep;
					runStep = [&]()
					{
						steps[nextStep++]();
						if (nextStep < steps.Count())
						{
							// Allow rendering and its queued view calculations to finish before observing state.
							GetApplication()->InvokeInMainThread(&main, [&]()
							{
								GetApplication()->InvokeInMainThread(&main, runStep);
							});
						}
					};
					GetApplication()->InvokeInMainThread(&main, runStep);
					controller->InputService()->StartTimer();
					GetApplication()->Run(&main);
				}
				tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage());
				theme::UnregisterTheme(skin->Name);
				hosted.Finalize();
				controller->CallbackService()->UninstallListener(&resources);
				SetGuiGraphicsResourceManager(previousResources);
				SetTuiApplication(nullptr);
				SetHostedApplication(nullptr);
				SetHostedApplication(previousHosted);
				SetNativeController(previousController);
			}, Size(120, 40));
		});
		GacUIUnitTest_Start(L"Tui/RefreshThemes");
	});

	TEST_CASE(L"TUI document minimum bounds include the final block caret")
	{
		TuiRunTest([](TuiTestBackend*, TuiTestController* controller)
		{
			auto previousResources = GetGuiGraphicsResourceManager();
			auto previousController = GetNativeController();
			TuiGraphicsResourceManager resources;
			SetGuiGraphicsResourceManager(&resources);
			SetNativeController(controller);
			SetTuiApplication(controller);
			RegisterTuiRenderers();
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			TuiGraphicsRenderTarget target(window);
			target.StartHostedRendering();
			target.StartRendering();
			for (WString text : {L"", L"Archer", L"\u4F60\u597D\U0001F600", L"a\tX"})
			{
				auto document = Ptr(new DocumentModel);
				auto paragraph = Ptr(new DocumentParagraphRun);
				auto run = Ptr(new DocumentTextRun);
				run->text = text;
				paragraph->runs.Add(run);
				document->paragraphs.Add(paragraph);
				auto element = Ptr(GuiDocumentElement::Create());
				element->SetDocument(document);
				element->SetWrapLine(false);
				element->SetParagraphPadding(false);
				auto renderer = element->GetRenderer();
				renderer->SetRenderTarget(&target);
				renderer->Render(Rect(0, 0, 16, 1));
				element->SetCaret(TextPos(0, text.Length()), TextPos(0, text.Length()), true);
				element->SetCaretColor(Color(255, 255, 255));
				element->SetCaretVisible(true);
				auto caret = element->GetCaretBounds(TextPos(0, text.Length()), true);
				auto bounds = Rect(Point(), renderer->GetMinSize());
				TEST_ASSERT(bounds.Contains(caret.LeftTop()));
				target.Fill(Rect(0, 0, 16, 8), Color(0, 0, 0));
				renderer->Render(bounds);
				TEST_ASSERT(TUI::GetBuffer()[caret.x1].backgroundColor == TuiColor(255, 255, 255));
				element->BlinkCaret();
				target.Fill(Rect(0, 0, 16, 8), Color(0, 0, 0));
				renderer->Render(bounds);
				TEST_ASSERT(TUI::GetBuffer()[caret.x1].backgroundColor == TuiColor(0, 0, 0));
			}
			target.StopRendering();
			target.StopHostedRendering();
			controller->DestroyNativeWindow(window);
			SetTuiApplication(nullptr);
			SetNativeController(previousController);
			SetGuiGraphicsResourceManager(previousResources);
		});
	});

	TEST_CASE(L"TUI grid selection preserves separator backgrounds in the composed row")
	{
		using namespace vl::presentation::unittest;
		GacUIUnitTest_SetGuiMainProxy([](auto, auto)
		{
			TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
			{
				using namespace compositions;
				using namespace controls::list;
				auto previousResources = GetGuiGraphicsResourceManager();
				auto previousController = GetNativeController();
				TuiGraphicsResourceManager resources;
				SetGuiGraphicsResourceManager(&resources);
				SetNativeController(controller);
				SetTuiApplication(controller);
				RegisterTuiRenderers();
				auto colors = tuiskin::CreateDefaultColorPackage();
				tuiskin::SetColorPackage(colors);
				auto window = controller->CreateNativeWindow(INativeWindow::Normal);
				resources.NativeWindowCreated(window);
				window->Show();
				auto target = resources.GetRenderTarget(window);
				auto root = new GuiBoundsComposition;
				root->SetExpectedBounds(Rect(0, 0, 16, 8));
				{
					GuiGraphicsHost host(nullptr, root);
					host.SetNativeWindow(window);
					tuiskin::TuiItemBackgroundTemplate* rows[2];
					CellBorderVisualizerTemplate* cells[2][2];
					for (vint r = 0; r < 2; r++)
					{
						auto row = new tuiskin::TuiItemBackgroundTemplate;
						rows[r] = row;
						row->SetGridRow(true);
						row->SetVisuallyEnabled(true);
						row->SetExpectedBounds(Rect(0, r * 2, 16, r * 2 + 2));
						root->AddChild(row);
						for (vint c = 0; c < 2; c++)
						{
							auto cell = new CellBorderVisualizerTemplate;
							cells[r][c] = cell;
							cell->SetExpectedBounds(Rect(c * 6, 0, c * 6 + 6, 2));
							cell->SetItemSeparatorColor(colors.ControlBorder);
							row->GetContainerComposition()->AddChild(cell);
							auto content = new SubColumnVisualizerTemplate;
							content->SetText(L"X");
							content->SetFont(controller->ResourceService()->GetDefaultFont());
							content->SetAlignmentToParent(Margin(0, 0, 0, 0));
							cell->GetContainerComposition()->AddChild(content);
						}
					}
					auto ordinaryRow = new tuiskin::TuiItemBackgroundTemplate;
					ordinaryRow->SetExpectedBounds(Rect(0, 4, 16, 5));
					ordinaryRow->SetSelected(true);
					root->AddChild(ordinaryRow);
					for (vint selected : {-1, -2, 0, 1, 2, 3, -1})
					{
						for (vint r = 0; r < 2; r++)
						{
							rows[r]->SetSelected(selected >= 0 && selected / 2 == r);
							rows[r]->SetState(selected == -2 ? controls::ButtonState::Active : controls::ButtonState::Normal);
							for (vint c = 0; c < 2; c++)
							{
								cells[r][c]->SetSelected(selected == r * 2 + c);
								TuiUpdateGridCellColors(cells[r][c]);
							}
						}
						root->ForceCalculateSizeImmediately();
						target->StartHostedRendering();
						target->StartRendering();
						root->Render({});
						target->StopRendering();
						target->StopHostedRendering();
						for (vint x = 0; x < 16; x++)
						{
							TEST_ASSERT(TUI::GetBuffer()[4 * 16 + x].backgroundColor == TuiColor(135, 206, 250));
						}
						for (vint r = 0; r < 2; r++)
						{
							for (vint x = 0; x < 16; x++)
							{
								auto pixel = TUI::GetBuffer()[(r * 2 + 1) * 16 + x];
								TEST_ASSERT(pixel.backgroundColor == TuiColor(0, 0, 0));
								if (x < 12)
								{
									TEST_ASSERT(pixel.glyph == TuiPixelGlyph::Mergeable);
									TEST_ASSERT(pixel.foregroundColor == TuiColor(128, 128, 128));
								}
							}
							for (vint c = 0; c < 2; c++)
							{
								auto color = selected == r * 2 + c ? TuiColor(135, 206, 250)
									: rows[r]->GetSelected() || selected == -2 ? TuiColor(0, 0, 128) : TuiColor(0, 0, 0);
								TEST_ASSERT(TUI::GetBuffer()[r * 2 * 16 + c * 6 + 2].backgroundColor == color);
							}
						}
					}
					host.SetNativeWindow(nullptr);
				}
				SafeDeleteComposition(root);
				resources.NativeWindowDestroying(window);
				controller->DestroyNativeWindow(window);
				SetTuiApplication(nullptr);
				SetNativeController(previousController);
				SetGuiGraphicsResourceManager(previousResources);
			});
		});
		GacUIUnitTest_Start(L"Tui/GridSeparators");
	});

	TEST_CASE(L"TUI configured tabs share paragraph and label geometry")
	{
		TEST_ASSERT(TuiConfiguration().tabInterval == 4);
		for (vint interval : {4, 8})
		{
			TuiRunTest([=](TuiTestBackend*, TuiTestController* controller)
			{
				TuiConfiguration configuration;
				configuration.tabInterval = interval;
				TuiGraphicsResourceManager resources(configuration);
				auto provider = static_cast<TuiGraphicsLayoutProvider*>(resources.GetLayoutProvider());
				configuration.tabInterval = 1;
				TEST_ASSERT(provider->GetConfiguration().tabInterval == interval);
				auto previousResources = GetGuiGraphicsResourceManager();
				SetGuiGraphicsResourceManager(&resources);
				RegisterTuiRenderers();
				auto window = controller->CreateNativeWindow(INativeWindow::Normal);
				window->Show();
				TuiGraphicsRenderTarget target(window);
				target.StartHostedRendering();
				target.StartRendering();
				for (WString text : {L"\tX", L"a\tX", L"abc\tX", L"abcd\tX", L"\t\tX", L"\u4E2D\tX", L"\U0001F600\tX"})
				{
					auto paragraph = provider->CreateParagraph(text, &target, nullptr);
					auto x = text == L"\t\tX" ? interval * 2 : text == L"abcd\tX" && interval == 4 ? 8 : interval;
					auto position = text.Length() - 1;
					TEST_ASSERT(paragraph->GetSize() == Size(x + 1, 1));
					TEST_ASSERT(paragraph->GetCaretBounds(position, true).LeftTop() == Point(x, 0));
					TEST_ASSERT(paragraph->GetCaretFromPoint(Point(x, 0)) == position);
					bool front = true;
					TEST_ASSERT(paragraph->GetCaret(position - 1, IGuiGraphicsParagraph::CaretMoveRight, front) == position);
					paragraph->SetBackgroundColor(position - 1, 1, Color(20, 30, 40));
					paragraph->SetStyle(position - 1, 1, IGuiGraphicsParagraph::Underline);
					paragraph->Render(Rect(0, 0, 16, 1));
					if (x < 16) TEST_ASSERT(TUI::GetBuffer()[x].GetChar32() == U'X');
					TEST_ASSERT(TUI::GetBuffer()[x - 1].backgroundColor == TuiColor(20, 30, 40));
				}
				{
					auto paragraph = provider->CreateParagraph(L"a\tX", &target, nullptr);
					paragraph->SetMaxWidth(12);
					for (auto alignment : {Alignment::Left, Alignment::Center, Alignment::Right})
					{
						paragraph->SetParagraphAlignment(alignment);
						auto offset = alignment == Alignment::Left ? 0 : alignment == Alignment::Center ? (11 - interval) / 2 : 11 - interval;
						TEST_ASSERT(paragraph->GetCaretBounds(2, true).x1 == interval + offset);
						paragraph->Render(Rect(2, 2, 14, 3));
						TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2 + interval + offset].GetChar32() == U'X');
					}
					paragraph->SetParagraphAlignment(Alignment::Left);
					target.PushClipper(Rect(2, 3, 10, 4), nullptr);
					paragraph->Render(Rect(0, 3, 12, 4));
					target.PopClipper(nullptr);
					TEST_ASSERT(TUI::GetBuffer()[3 * 16 + interval].GetChar32() == U'X');
					auto multiline = provider->CreateParagraph(L"a\tX\r\na\tX", nullptr, nullptr);
					TEST_ASSERT(multiline->GetCaretBounds(7, true).LeftTop() == Point(interval, 1));
					multiline->SetWrapLine(true);
					multiline->SetMaxWidth(interval);
					TEST_ASSERT(multiline->GetSize() == Size(interval, 4));
					TEST_ASSERT(multiline->GetCaretBounds(2, false).LeftTop() == Point(0, 1));
					auto objectParagraph = provider->CreateParagraph(L"#\tX", nullptr, nullptr);
					IGuiGraphicsParagraph::InlineObjectProperties object;
					object.size = Size(interval + 1, 1);
					TEST_ASSERT(objectParagraph->SetInlineObject(0, 1, object));
					TEST_ASSERT(objectParagraph->GetCaretBounds(2, true).x1 == interval * 2);
					auto label = Ptr(GuiSolidLabelElement::Create());
					label->SetText(L"a\tX");
					auto renderer = label->GetRenderer();
					renderer->SetRenderTarget(&target);
					TEST_ASSERT(renderer->GetMinSize() == Size(interval + 1, 1));
					renderer->Render(Rect(2, 4, 14, 5));
					TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 2 + interval].GetChar32() == U'X');
					label->SetEllipse(true);
					renderer->Render(Rect(2, 5, 2 + interval, 6));
					TEST_ASSERT(TUI::GetBuffer()[5 * 16 + 3].GetChar32() == U'\u2026');
					TEST_ASSERT(TuiEllipsizeText(L"a\tX", interval + 1, interval) == L"a\tX");
				}
				target.StopRendering();
				target.StopHostedRendering();
				controller->DestroyNativeWindow(window);
				SetGuiGraphicsResourceManager(previousResources);
			});
		}
		TuiConfiguration invalid;
		invalid.tabInterval = 0;
		TEST_EXCEPTION(TuiGraphicsLayoutProvider provider(invalid), Error, [](const Error&) {});
		invalid.tabInterval = -1;
		TEST_EXCEPTION(TuiGraphicsLayoutProvider provider(invalid), Error, [](const Error&) {});
	});

	TEST_CASE(L"TUI scalar offsets, tab stops, CRLF, wrapping and caret sides")
	{
		TuiGraphicsLayoutProvider provider;
		auto paragraph = provider.CreateParagraph(L"A\t\x4E2D\U0001F600\r\nZ", nullptr, nullptr);
#ifdef VCZH_WCHAR_UTF16
		const vint supplementaryEnd = 5;
		TEST_ASSERT(!paragraph->IsValidCaret(4));
#else
		const vint supplementaryEnd = 4;
#endif
		TEST_ASSERT(paragraph->GetSize() == Size(8, 2));
		TEST_ASSERT(paragraph->GetCaretBounds(2, true).LeftTop() == Point(4, 0));
		TEST_ASSERT(paragraph->GetCaretBounds(3, true).LeftTop() == Point(6, 0));
		TEST_ASSERT(!paragraph->IsValidCaret(supplementaryEnd + 1));
		TEST_ASSERT(paragraph->GetCaretBounds(supplementaryEnd + 2, true).LeftTop() == Point(0, 1));
		TEST_ASSERT(paragraph->GetNearestCaretFromTextPos(3, false) == 3);
		bool front = true;
		TEST_ASSERT(paragraph->GetCaret(3, IGuiGraphicsParagraph::CaretMoveRight, front) == supplementaryEnd);
		TEST_ASSERT(paragraph->GetCaret(supplementaryEnd, IGuiGraphicsParagraph::CaretMoveLeft, front) == 3);
		TEST_ASSERT(paragraph->GetCaretFromPoint(Point(5, 0)) == 3);
		auto natural = paragraph->GetSize();
		paragraph->SetFont(0, 1, L"OtherFont");
		paragraph->SetSize(0, 1, 1000);
		TEST_ASSERT(paragraph->GetSize() == natural);

		auto wrapped = provider.CreateParagraph(L"ABCD", nullptr, nullptr);
		wrapped->SetWrapLine(true);
		wrapped->SetMaxWidth(2);
		TEST_ASSERT(wrapped->GetSize() == Size(2, 2));
		TEST_ASSERT(wrapped->GetCaretBounds(2, true).LeftTop() == Point(2, 0));
		TEST_ASSERT(wrapped->GetCaretBounds(2, false).LeftTop() == Point(0, 1));
		wrapped->SetMaxWidth(5);
		wrapped->SetParagraphAlignment(Alignment::Right);
		TEST_ASSERT(wrapped->GetCaretBounds(0, true).LeftTop() == Point(1, 0));
		auto empty = provider.CreateParagraph(L"", nullptr, nullptr);
		TEST_ASSERT(empty->GetSize() == Size(0, 1));
		TEST_ASSERT(empty->IsValidCaret(0));
		TEST_ASSERT(TuiEllipsizeText(L"AB\r\nABCDE", 3) == L"AB\r\nAB\u2026");
		TEST_ASSERT(TuiEllipsizeText(L"\u4E2D\U0001F600", 2) == L"\u2026");
		TEST_ASSERT(TuiEllipsizeText(L"ABC", 0) == L"");
	});

	TEST_CASE(L"TUI inline objects are atomic and reset to native text")
	{
		TuiGraphicsLayoutProvider provider;
		auto paragraph = provider.CreateParagraph(L"AxyzB", nullptr, nullptr);
		IGuiGraphicsParagraph::InlineObjectProperties object;
		object.size = Size(6, 3);
		object.breakCondition = IGuiGraphicsParagraph::Alone;
		object.callbackId = 12;
		TEST_ASSERT(paragraph->SetInlineObject(1, 3, object));
		TEST_ASSERT(!paragraph->SetInlineObject(2, 2, object));
		TEST_ASSERT(!paragraph->IsValidCaret(2));
		TEST_ASSERT(paragraph->GetSize() == Size(8, 3));
		object.backgroundColor = Color(20, 30, 40);
		TEST_ASSERT(paragraph->SetInlineObject(1, 3, object));
		object.callbackId = 13;
		TEST_ASSERT(!paragraph->SetInlineObject(1, 3, object));
		object.callbackId = 12;
		TEST_ASSERT(paragraph->GetNearestCaretFromTextPos(2, true) == 1);
		TEST_ASSERT(paragraph->GetNearestCaretFromTextPos(2, false) == 4);
		vint start = -1;
		vint length = -1;
		auto hit = paragraph->GetInlineObjectFromPoint(Point(2, 1), start, length);
		TEST_ASSERT(hit && hit.Value().callbackId == 12 && start == 1 && length == 3);
		TEST_ASSERT(paragraph->ResetInlineObject(1, 3));
		TEST_ASSERT(paragraph->IsValidCaret(2));
		TEST_ASSERT(paragraph->GetSize() == Size(5, 1));

		auto baseline = provider.CreateParagraph(L"A#B", nullptr, nullptr);
		object.size = Size(2, 3);
		object.baseline = 1;
		TEST_ASSERT(baseline->SetInlineObject(1, 1, object));
		TEST_ASSERT(baseline->GetCaretBounds(0, true).y1 == 0);
		TEST_ASSERT(baseline->GetSize() == Size(4, 3));
		for (auto condition : {IGuiGraphicsParagraph::StickToPreviousRun, IGuiGraphicsParagraph::StickToNextRun})
		{
			auto joined = provider.CreateParagraph(condition == IGuiGraphicsParagraph::StickToPreviousRun ? L"AB#" : L"A#B", nullptr, nullptr);
			object.size = Size(2, 1);
			object.baseline = -1;
			object.breakCondition = condition;
			TEST_ASSERT(joined->SetInlineObject(condition == IGuiGraphicsParagraph::StickToPreviousRun ? 2 : 1, 1, object));
			joined->SetWrapLine(true);
			joined->SetMaxWidth(3);
			TEST_ASSERT(joined->GetSize() == Size(3, 2));
			TEST_ASSERT(joined->GetCaretBounds(1, false).LeftTop() == Point(0, 1));
		}
	});

	TEST_CASE(L"TUI inline callbacks use paragraph coordinates and resize the next layout")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			class Callback : public Object, public IGuiGraphicsParagraphCallback
			{
			public:
				Rect location;
				Size OnRenderInlineObject(vint callbackId, Rect bounds) override
				{
					TEST_ASSERT(callbackId == 12);
					location = bounds;
					return Size(4, 3);
				}
			} callback;
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			TuiGraphicsRenderTarget target(window);
			TuiGraphicsLayoutProvider provider;
			auto paragraph = provider.CreateParagraph(L"A#B", &target, &callback);
			IGuiGraphicsParagraph::InlineObjectProperties object;
			object.callbackId = 12;
			object.size = Size(2, 1);
			TEST_ASSERT(paragraph->SetInlineObject(1, 1, object));
			target.StartHostedRendering();
			target.StartRendering();
			paragraph->Render(Rect(3, 4, 12, 8));
			TEST_ASSERT(callback.location == Rect(1, 0, 3, 1));
			TEST_ASSERT(paragraph->GetSize() == Size(6, 3));
			target.StopRendering();
			target.StopHostedRendering();
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI window geometry, events, timer work and explicit stopping")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			TuiTestWindowListener listener;
			window->InstallListener(&listener);
			TEST_ASSERT(window->GetClientSize() == NativeSize(16, 8));
			TEST_ASSERT(!window->IsVisible());
			window->SetBounds(NativeRect(NativePoint(100, 200), NativeSize(40, 20)));
			TEST_ASSERT(window->GetBounds().LeftTop() == NativePoint());
			TEST_ASSERT(window->GetClientSize() == NativeSize(40, 20));
			TEST_ASSERT(backend->width == 16 && backend->height == 8);
			window->SetTitle(L"Terminal title");
			TEST_ASSERT(controller->title == L"Terminal title");
			window->ShowMaximized();
			TEST_ASSERT(window->IsVisible() && window->IsActivated());
			TEST_ASSERT(!window->IsCustomFrameModeEnabled());
			TEST_ASSERT(window->Convert(NativePoint(3, 4)) == Point(3, 4));
			TEST_ASSERT(controller->ResourceService()->GetDefaultFont().size == 1);
			TEST_ASSERT(controller->ResourceService()->GetDefaultFont().fontFamily == L"TuiFont");
			List<WString> fonts;
			controller->ResourceService()->EnumerateFonts(fonts);
			TEST_ASSERT(fonts.Count() == 1 && fonts[0] == L"TuiFont");
			TEST_ASSERT(!controller->AutomationService());

			TuiBackendEvent key;
			key.type = TuiBackendEventType::KeyDown;
			key.keyInfo.code = VKEY::KEY_A;
			key.keyInfo.alt = true;
			backend->events.Add(key);
			auto release = key;
			release.type = TuiBackendEventType::KeyUp;
			backend->events.Add(release);
			TuiBackendEvent character;
			character.type = TuiBackendEventType::Char;
			character.charInfo.code = L'a';
			backend->events.Add(character);
			TuiBackendEvent mouse;
			mouse.type = TuiBackendEventType::MouseMove;
			mouse.mouseInfo.x = 3;
			mouse.mouseInfo.y = 4;
			mouse.mouseInfo.alt = true;
			backend->events.Add(mouse);
			mouse.type = TuiBackendEventType::MouseVerticalWheel;
			mouse.mouseInfo.wheel = -120;
			backend->events.Add(mouse);
			TuiBackendEvent resize;
			resize.type = TuiBackendEventType::Resize;
			resize.width = 10;
			resize.height = 5;
			backend->events.Add(resize);
			for (vint i = 0; i < 6; i++) controller->RunOneCycle();
			TEST_ASSERT(listener.keys == 1 && listener.releases == 1 && listener.chars == 1);
			TEST_ASSERT(listener.key.alt && !listener.key.osSuper && listener.character.code == L'a');
			TEST_ASSERT(listener.mouse == 1 && listener.position.x == 3 && listener.position.y == 4);
			TEST_ASSERT(listener.wheel == -120);
			TEST_ASSERT(window->GetClientSize() == NativeSize(10, 5) && listener.moved >= 2);

			for (bool osSuper : { true, false })
			for (bool alt : { true, false })
			{
				key.keyInfo.code = VKEY::KEY_Q;
				key.keyInfo.ctrl = true;
				key.keyInfo.alt = alt;
				key.keyInfo.osSuper = osSuper;
				backend->events.Add(key);
				character.charInfo.ctrl = true;
				character.charInfo.alt = alt;
				character.charInfo.osSuper = osSuper;
				backend->events.Add(character);
				mouse.type = TuiBackendEventType::MouseMove;
				mouse.mouseInfo.alt = alt;
				mouse.mouseInfo.osSuper = osSuper;
				backend->events.Add(mouse);
				for (vint i = 0; i < 3; i++) controller->RunOneCycle();
				TEST_ASSERT(listener.key.code == VKEY::KEY_Q && listener.key.ctrl);
				TEST_ASSERT(listener.key.alt == alt && listener.key.osSuper == osSuper);
				TEST_ASSERT(listener.character.ctrl && listener.character.alt == alt && listener.character.osSuper == osSuper);
				TEST_ASSERT(listener.position.alt == alt && listener.position.osSuper == osSuper);
			}

			TuiTestTimerListener timer;
			controller->CallbackService()->InstallListener(&timer);
			vint invoked = 0;
			const auto owner = Thread::GetCurrentThreadId();
			controller->AsyncService()->InvokeInMainThread(window, [&]()
			{
				TEST_ASSERT(Thread::GetCurrentThreadId() == owner);
				invoked++;
			});
			controller->AsyncService()->DelayExecuteInMainThread([&]() { invoked++; }, 0);
			controller->InputService()->StartTimer();
			for (vint i = 0; i < 20 && invoked != 2; i++) controller->RunOneCycle();
			TEST_ASSERT(invoked == 2 && timer.ticks > 0);
			controller->Stop();
			const auto ticks = timer.ticks;
			controller->Timer();
			TEST_ASSERT(timer.ticks == ticks && !controller->RunOneCycle());
			controller->CallbackService()->UninstallListener(&timer);
			controller->DestroyNativeWindow(window);
			TEST_ASSERT(listener.destroying == 1 && listener.destroyed == 1);
		});
	});

	TEST_CASE(L"TUI Run fits the viewport before opening and dispatching application work")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			TEST_ASSERT(!window->IsVisible());
			window->SetClientSize(NativeSize(120, 40));
			bool invoked = false;
			controller->AsyncService()->InvokeInMainThread(window, [&]()
			{
				TEST_ASSERT(window->IsVisible());
				TEST_ASSERT(window->GetClientSize() == NativeSize(16, 8));
				invoked = true;
				controller->Stop();
			});
			controller->InputService()->StartTimer();
			controller->Run(window);
			TEST_ASSERT(invoked);
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI Hide and Close query once, support cancellation and stop after notifications")
	{
		for (bool closeWindow : { false, true })
		TuiRunTest([=](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			List<WString> events;
			bool veto = true;
			TuiTestClosingListener listener;
			TuiTestClosingListener removed;
			removed.before = [](bool&) { TEST_ASSERT(false); };
			removed.after = []() { TEST_ASSERT(false); };
			removed.closed = []() { TEST_ASSERT(false); };
			listener.before = [&](bool& cancel)
			{
				events.Add(L"BeforeClosing");
				window->UninstallListener(&removed);
				TEST_ASSERT(window->IsVisible() && !TUI::IsStopRequested());
				window->Hide(closeWindow);
				cancel = veto;
			};
			listener.after = [&]()
			{
				events.Add(L"AfterClosing");
				TEST_ASSERT(window->IsVisible() && !TUI::IsStopRequested());
				window->Hide(closeWindow);
			};
			listener.closed = [&]()
			{
				events.Add(L"Closed");
				TEST_ASSERT(!window->IsVisible() && !TUI::IsStopRequested());
				window->Hide(closeWindow);
				window->UninstallListener(&listener);
			};
			window->InstallListener(&listener);
			window->InstallListener(&removed);
			window->Hide(closeWindow);
			TEST_ASSERT(events.Count() == 1 && events[0] == L"BeforeClosing");
			TEST_ASSERT(window->IsVisible() && !TUI::IsStopRequested());
			veto = false;
			window->Hide(closeWindow);
			TEST_ASSERT(events.Count() == 4);
			TEST_ASSERT(events[1] == L"BeforeClosing" && events[2] == L"AfterClosing" && events[3] == L"Closed");
			TEST_ASSERT(!window->IsVisible() && TUI::IsStopRequested());
			window->Hide(closeWindow);
			TEST_ASSERT(events.Count() == 4);
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI hosted startup, child dismissal and direct or queued main closing")
	{
		for (auto size : { Size(100, 30), Size(80, 25) })
		for (bool closeWindow : { false, true })
		for (bool queued : { false, true })
		TuiRunTest([=](TuiTestBackend* backend, TuiTestController* controller)
		{
			GuiHostedController hosted(controller);
			hosted.Initialize();
			auto physical = controller->GetMainWindow();
			auto main = hosted.WindowService()->CreateNativeWindow(INativeWindow::Normal);
			main->SetClientSize(NativeSize(120, 40));
			vint queries = 0;
			vint ready = 0;
			vint closed = 0;
			bool veto = true;
			TuiTestClosingListener listener;
			listener.before = [&](bool& cancel) { queries++; cancel = veto; };
			listener.after = [&]() { ready++; TEST_ASSERT(!TUI::IsStopRequested()); };
			listener.closed = [&]() { closed++; };
			main->InstallListener(&listener);
			controller->AsyncService()->InvokeInMainThread(physical, [&]()
			{
				TEST_ASSERT(main->GetClientSize() == NativeSize(size.x, size.y));
				TEST_ASSERT(physical->GetClientSize() == main->GetClientSize());
				TEST_ASSERT(physical->IsVisible());
				auto child = hosted.WindowService()->CreateNativeWindow(INativeWindow::Normal);
				child->Show();
				child->Hide(closeWindow);
				TEST_ASSERT(!child->IsVisible() && !TUI::IsStopRequested());
				hosted.WindowService()->DestroyNativeWindow(child);
				main->Hide(closeWindow);
				TEST_ASSERT(queries == 1 && ready == 0 && closed == 0);
				TEST_ASSERT(main->IsVisible() && !TUI::IsStopRequested());
				veto = false;
				if (queued)
				{
					hosted.AsyncService()->InvokeInMainThread(main, [&]() { main->Hide(closeWindow); });
				}
				else
				{
					main->Hide(closeWindow);
				}
			});
			controller->InputService()->StartTimer();
			hosted.WindowService()->Run(main);
			TEST_ASSERT(queries == 2 && ready == 1 && closed == 1);
			TEST_ASSERT(TUI::IsStopRequested() && !physical->IsVisible());
			hosted.Finalize();
		}, size);
	});

	TEST_CASE(L"TUI application main closing respects hosted modal interception")
	{
		using namespace vl::presentation::unittest;
		GacUIUnitTest_SetGuiMainProxy([](auto, auto)
		{
			TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
			{
				using namespace controls;
				auto previousResources = GetGuiGraphicsResourceManager();
				auto previousController = GetNativeController();
				auto previousHosted = GetHostedApplication();
				GuiHostedController hosted(controller);
				TuiGraphicsResourceManager resources;
				GuiHostedGraphicsResourceManager hostedResources(&hosted, &resources);
				SetNativeController(&hosted);
				SetHostedApplication(nullptr);
				SetHostedApplication(hosted.GetHostedApplication());
				SetTuiApplication(controller);
				SetGuiGraphicsResourceManager(&hostedResources);
				controller->CallbackService()->InstallListener(&resources);
				RegisterTuiRenderers();
				hosted.Initialize();
				tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage());
				auto skin = Ptr(new tuiskin::TuiTheme);
				theme::RegisterTheme(skin);
				{
					GuiWindow main(theme::ThemeName::SystemFrameWindow);
					GuiWindow modal(theme::ThemeName::Window);
					main.GetBoundsComposition()->SetMinSizeLimitation(compositions::GuiGraphicsComposition::NoLimit);
					main.SetClientSize(Size(120, 40));
					vint queries = 0;
					vint ready = 0;
					vint closed = 0;
					bool veto = true;
					main.WindowClosing.AttachLambda([&](auto, compositions::GuiRequestEventArgs& args) { queries++; args.cancel = veto; });
					main.WindowReadyToClose.AttachLambda([&](auto, auto&) { ready++; });
					main.WindowClosed.AttachLambda([&](auto, auto&) { closed++; });
					GetApplication()->InvokeInMainThread(&main, [&]()
					{
						main.ForceCalculateSizeImmediately();
						TEST_ASSERT(main.GetClientSize() == Size(100, 30));
						TEST_ASSERT(main.GetBoundsComposition()->GetCachedBounds().GetSize() == Size(100, 30));
						modal.ShowModal(&main, []() {});
						TEST_ASSERT(modal.GetOpening());
						main.Hide();
						TEST_ASSERT(!modal.GetOpening() && main.GetOpening());
						TEST_ASSERT(queries == 0 && ready == 0 && !TUI::IsStopRequested());
						GetApplication()->InvokeInMainThread(&main, [&]()
						{
							main.Hide();
							TEST_ASSERT(queries == 1 && ready == 0 && !TUI::IsStopRequested());
							veto = false;
							main.Close();
						});
					});
					controller->InputService()->StartTimer();
					GetApplication()->Run(&main);
					TEST_ASSERT(queries == 2 && ready == 1 && closed == 1);
				}
				theme::UnregisterTheme(skin->Name);
				hosted.Finalize();
				controller->CallbackService()->UninstallListener(&resources);
				SetGuiGraphicsResourceManager(previousResources);
				SetTuiApplication(nullptr);
				SetHostedApplication(nullptr);
				SetHostedApplication(previousHosted);
				SetNativeController(previousController);
			}, Size(100, 30));
		});
		GacUIUnitTest_Start(L"Tui/Closing");
	});

	TEST_CASE(L"TUI registered renderers reuse label paragraphs and layouts")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto previousResources = GetGuiGraphicsResourceManager();
			TuiCountingResources resources;
			SetGuiGraphicsResourceManager(&resources);
			RegisterTuiRenderers();
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			TuiGraphicsRenderTarget target(window);
			TuiGraphicsRenderTarget replacement(window);
			target.StartHostedRendering();
			target.StartRendering();
			{
				auto background = Ptr(GuiSolidBackgroundElement::Create());
				auto border = Ptr(GuiSolidBorderElement::Create());
				auto tuiBorder = Ptr(TuiBorderElement::Create());
				background->SetColor(Color(10, 20, 30));
				border->SetColor(Color(100, 110, 120));
				tuiBorder->SetColor(Color(130, 140, 150));
				tuiBorder->SetLineStyle(TuiLineStyle::Double);
				background->GetRenderer()->SetRenderTarget(&target);
				border->GetRenderer()->SetRenderTarget(&target);
				tuiBorder->GetRenderer()->SetRenderTarget(&target);
				background->GetRenderer()->Render(Rect(0, 0, 16, 8));
				border->GetRenderer()->Render(Rect(0, 0, 4, 4));
				tuiBorder->GetRenderer()->Render(Rect(5, 0, 9, 4));
				TEST_ASSERT(TUI::GetBuffer()[0].GetChar32() == U'\u250C');
				TEST_ASSERT(TUI::GetBuffer()[5].GetChar32() == U'\u2554');
				TEST_ASSERT(TUI::GetBuffer()[7 * 16 + 15].backgroundColor.r == 10);

				auto label = Ptr(GuiSolidLabelElement::Create());
				auto renderer = label->GetRenderer();
				TEST_ASSERT(renderer->GetMinSize() == Size(0, 1));
				renderer->SetRenderTarget(&target);
				label->SetText(L"ABCD");
				label->SetColor(Color(200, 210, 220));
				auto paragraph = resources.provider.last;
				auto creations = resources.provider.creations;
				renderer->Render(Rect(0, 0, 6, 3));
				TEST_ASSERT(renderer->GetMinSize() == Size(4, 1));
				auto layouts = paragraph->layouts;
				renderer->Render(Rect(0, 0, 6, 3));
				TEST_ASSERT(resources.provider.creations == creations && paragraph->layouts == layouts);
				label->SetColor(Color(230, 240, 250));
				auto font = label->GetFont();
				font.bold = true;
				font.underline = true;
				label->SetFont(font);
				renderer->Render(Rect(0, 0, 6, 3));
				TEST_ASSERT(resources.provider.creations == creations && paragraph->layouts == layouts);
				TEST_ASSERT(TUI::GetBuffer()[0].foregroundColor.r == 230);
				TEST_ASSERT(TUI::GetBuffer()[0].character.style.bold && TUI::GetBuffer()[0].character.style.underline);
				label->SetHorizontalAlignment(Alignment::Right);
				label->SetVerticalAlignment(Alignment::Bottom);
				renderer->Render(Rect(0, 0, 6, 3));
				TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].GetChar32() == U'A');
				TEST_ASSERT(resources.provider.last == paragraph && paragraph->layouts == layouts + 1);
				label->SetHorizontalAlignment(Alignment::Center);
				label->SetVerticalAlignment(Alignment::Center);
				renderer->Render(Rect(0, 0, 6, 3));
				TEST_ASSERT(TUI::GetBuffer()[16 + 1].GetChar32() == U'A');
				label->SetHorizontalAlignment(Alignment::Left);
				label->SetVerticalAlignment(Alignment::Top);
				label->SetWrapLine(true);
				label->SetWrapLineHeightCalculation(true);
				renderer->Render(Rect(0, 0, 2, 4));
				TEST_ASSERT(resources.provider.last == paragraph && renderer->GetMinSize() == Size(0, 2));
				label->SetWrapLineHeightCalculation(false);
				TEST_ASSERT(renderer->GetMinSize() == Size(0, 0));
				label->SetWrapLine(false);
				renderer->Render(Rect(0, 0, 8, 4));
				TEST_ASSERT(resources.provider.last == paragraph && renderer->GetMinSize() == Size(4, 1));

				label->SetText(L"A\r\nZ");
				TEST_ASSERT(renderer->GetMinSize() == Size(4, 1));
				label->SetMultiline(true);
				TEST_ASSERT(renderer->GetMinSize() == Size(1, 2));
				label->SetText(L"A\t\x4E2D\U0001F600\r\nZ");
				TEST_ASSERT(renderer->GetMinSize() == Size(8, 2));
				renderer->Render(Rect(0, 0, 8, 3));
				TEST_ASSERT(TUI::GetBuffer()[4].GetChar32() == U'\u4E2D');
				TEST_ASSERT(TUI::GetBuffer()[5].glyph == TuiPixelGlyph::WideCharContinuation);
				TEST_ASSERT(TUI::GetBuffer()[6].GetChar32() == U'\U0001F600');
				TEST_ASSERT(TUI::GetBuffer()[7].glyph == TuiPixelGlyph::WideCharContinuation);
				label->SetText(L"\u4E2D");
				label->SetEllipse(true);
				renderer->Render(Rect(0, 0, 0, 2));
				paragraph = resources.provider.last;
				label->SetText(L"\u4E2D\r\n\u4E2D");
				TEST_ASSERT(renderer->GetMinSize() == Size(0, 2));
				label->SetText(L"\u4E2D");
				renderer->Render(Rect(0, 0, 1, 2));
				paragraph = resources.provider.last;
				label->SetText(L"\u4E2D\u6587");
				TEST_ASSERT(resources.provider.last == paragraph);
				renderer->Render(Rect(0, 0, 2, 2));
				TEST_ASSERT(resources.provider.last == paragraph);
				label->SetEllipse(false);
				TEST_ASSERT(renderer->GetMinSize() == Size(4, 1));
				label->SetEllipse(true);
				paragraph = resources.provider.last;
				renderer->Render(Rect(0, 0, 3, 2));
				TEST_ASSERT(resources.provider.last != paragraph);
				paragraph = resources.provider.last;
				renderer->SetRenderTarget(&replacement);
				TEST_ASSERT(resources.provider.last != paragraph && resources.provider.last->GetRenderTarget() == &replacement);
				renderer->SetRenderTarget(&target);
				label->SetEllipse(false);
				label->SetText(L"ABCDE");
				target.Fill(Rect(0, 0, 16, 8), Color(0, 0, 0));
				target.PushClipper(Rect(2, 1, 4, 2), nullptr);
				renderer->Render(Rect(0, 1, 8, 3));
				TEST_ASSERT(TUI::GetBuffer()[16 + 1].GetChar32() == 0);
				TEST_ASSERT(TUI::GetBuffer()[16 + 2].GetChar32() == U'C');
				TEST_ASSERT(TUI::GetBuffer()[16 + 4].GetChar32() == 0);
				target.PopClipper(nullptr);
			}
			target.StopRendering();
			target.StopHostedRendering();
			controller->DestroyNativeWindow(window);
			SetGuiGraphicsResourceManager(previousResources);
		});
	});

	TEST_CASE(L"TUI nested and empty clippers restore drawing across resize")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			TuiGraphicsRenderTarget target(window);
			target.StartHostedRendering();
			target.StartRendering();
			target.Fill(Rect(0, 0, 16, 8), Color(10, 20, 30));
			target.PushClipper(Rect(2, 1, 12, 7), nullptr);
			target.PushClipper(Rect(4, 2, 10, 6), nullptr);
			backend->events.Add({ .type = TuiBackendEventType::Resize, .width = 6, .height = 4 });
			controller->RunOneCycle();
			target.Fill(Rect(-100, -100, 100, 100), Color(110, 120, 130, 128));
			TEST_ASSERT(TUI::GetBuffer()[2 * 6 + 4].backgroundColor == TuiColor({ 60, 70, 80 }));
			TEST_ASSERT(TUI::GetBuffer()[2 * 6 + 3].backgroundColor == TuiColor({ 10, 20, 30 }));
			target.PushClipper(Rect(12, 7, 14, 8), nullptr);
			target.Border(Rect(4, 2, 1000000, 1000000), Color(255, 0, 0), TuiLineStyle::Thin, {});
			target.PopClipper(nullptr);
			target.Border(Rect(-1000000, -1000000, 1000000, 1000000), Color(255, 0, 0, 128), TuiLineStyle::Thin, {});
			TEST_ASSERT(TUI::GetBuffer()[2 * 6 + 4].GetChar32() == 0);
			target.PopClipper(nullptr);
			target.Fill(Rect(-100, -100, 100, 100), Color(20, 30, 40));
			TEST_ASSERT(TUI::GetBuffer()[1 * 6 + 2].backgroundColor == TuiColor({ 20, 30, 40 }));
			TEST_ASSERT(TUI::GetBuffer()[1 * 6 + 1].backgroundColor == TuiColor({ 10, 20, 30 }));
			target.PopClipper(nullptr);
			target.StopRendering();
			target.StopHostedRendering();
			TEST_ASSERT(backend->presents == 1);
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI alpha borders and text preserve clipped colors, styles and wide carets")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			TuiGraphicsRenderTarget target(window);
			target.StartHostedRendering();
			target.StartRendering();
			target.Fill(Rect(0, 0, 16, 8), Color(10, 20, 30));
			target.Print(Point(2, 2), U'X', Color(30, 40, 50), Color(0, 0, 0, 0), { true, true, true, true });
			target.PushClipper(Rect(2, 2, 6, 5), nullptr);
			target.Border(Rect(0, 2, 10, 7), Color(130, 140, 150, 128), TuiLineStyle::Double, {});
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].GetChar32() == U'\u2550');
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].foregroundColor == TuiColor({ 80, 90, 100 }));
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].backgroundColor == TuiColor({ 10, 20, 30 }));
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 2].GetChar32() == 0);
			target.Fill(Rect(2, 2, 6, 5), Color(1, 2, 3, 0));
			target.Border(Rect(2, 2, 6, 5), Color(1, 2, 3, 0), TuiLineStyle::Thin, {});
			target.Print(Point(2, 2), U'Y', Color(1, 2, 3, 0), Color(1, 2, 3, 0), {});
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].GetChar32() == U'\u2550');
			target.Print(Point(2, 2), U'Z', Color(180, 190, 200, 128), Color(110, 120, 130, 128), { true, true, true, true });
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].foregroundColor == TuiColor({ 130, 140, 150 }));
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].backgroundColor == TuiColor({ 60, 70, 80 }));
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].character.style == TuiTextStyle({ true, true, true, true }));
			target.PopClipper(nullptr);
			target.Print(Point(1, 3), U'\u4E2D', Color(40, 50, 60), Color(70, 80, 90), { true, true, true, true });
			target.PushClipper(Rect(2, 3, 6, 5), nullptr);
			target.Caret(Point(2, 3), Color(150, 160, 170));
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 1].backgroundColor == TuiColor({ 70, 80, 90 }));
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 2].glyph == TuiPixelGlyph::WideCharContinuation);
			target.PopClipper(nullptr);
			target.PushClipper(Rect(1, 3, 3, 4), nullptr);
			target.Caret(Point(2, 3), Color(150, 160, 170));
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 1].backgroundColor == TuiColor({ 150, 160, 170 }));
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 2].backgroundColor == TuiColor({ 150, 160, 170 }));
			target.PopClipper(nullptr);
			target.StopRendering();
			target.StopHostedRendering();
			TEST_ASSERT(backend->presents == 1);
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI nested clippers restore drawing before and after resize")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			window->Show();
			TuiGraphicsRenderTarget target(window);
			target.StartHostedRendering();
			target.StartRendering();
			target.PushClipper(Rect(2, 1, 10, 6), nullptr);
			target.PushClipper(Rect(4, 2, 8, 5), nullptr);
			target.Fill(Rect(0, 0, 16, 8), Color(200, 10, 20));
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 4].backgroundColor.r == 200);
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 3].backgroundColor.r == 0);
			target.PushClipper(Rect(12, 6, 15, 8), nullptr);
			target.Fill(Rect(0, 0, 16, 8), Color(100, 100, 100));
			target.Caret(Point(4, 2), Color(100, 100, 100));
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 4].backgroundColor.r == 200);
			target.PopClipper(nullptr);
			target.Fill(Rect(0, 0, 16, 8), Color(20, 30, 40));
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 4].backgroundColor.r == 20);
			target.PopClipper(nullptr);
			target.Border(Rect(0, 1, 12, 7), Color(255, 255, 255), TuiLineStyle::Double, {});
			TEST_ASSERT(TUI::GetBuffer()[16 + 2].GetChar32() == U'\u2550');
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].GetChar32() == 0);
			target.Print(Point(1, 3), U'\u4E2D', Color(255, 255, 255), Color(), {});
			target.Print(Point(9, 3), U'\u4E2D', Color(255, 255, 255), Color(), {});
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 2].GetChar32() == 0);
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 9].GetChar32() == 0);
			target.Print(Point(2, 3), U'\u4E2D', Color(255, 255, 255), Color(), {});
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 3].glyph == TuiPixelGlyph::WideCharContinuation);
			target.Caret(Point(10, 4), Color(150, 160, 170));
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 10].backgroundColor.r == 0);
			target.Caret(Point(2, 4), Color(150, 160, 170));
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 2].backgroundColor.r == 150);
			target.PopClipper(nullptr);

			TuiBackendEvent resize;
			resize.type = TuiBackendEventType::Resize;
			resize.width = 6;
			resize.height = 4;
			backend->events.Add(resize);
			controller->RunOneCycle();
			target.Fill(Rect(0, 0, 16, 8), Color(30, 40, 50));
			target.Border(Rect(0, 0, 8, 6), Color(255, 255, 255), TuiLineStyle::Thin, {});
			TEST_ASSERT(TUI::GetBufferWidth() == 6 && TUI::GetBufferHeight() == 4);
			TEST_ASSERT(TUI::GetBuffer()[5].GetChar32() == U'\u2500');
			TEST_ASSERT(TUI::GetBuffer()[3 * 6].GetChar32() == U'\u2502');
			TEST_ASSERT(TUI::GetBuffer()[3 * 6 + 5].GetChar32() == 0);
			TEST_ASSERT(TUI::GetBuffer()[3 * 6 + 5].backgroundColor.r == 30);
			target.StopRendering();
			target.StopHostedRendering();
			controller->DestroyNativeWindow(window);
		});
	});

	TEST_CASE(L"TUI composed frame, clipping, border merging, alpha and wide-cell repair")
	{
		TuiRunTest([](TuiTestBackend* backend, TuiTestController* controller)
		{
			auto window = controller->CreateNativeWindow(INativeWindow::Normal);
			TuiGraphicsRenderTarget target(window);
			target.StartRendering();
			target.Fill(Rect(0, 0, 16, 8), Color(255, 0, 0));
			target.StopRendering();
			TEST_ASSERT(backend->presents == 0);
			window->Show();
			target.StartHostedRendering();
			target.StartRendering();
			target.Fill(Rect(0, 0, 16, 8), Color(10, 20, 30));
			target.Border(Rect(0, 1, 8, 2), Color(255, 255, 255), TuiLineStyle::Thin, {});
			target.Border(Rect(3, 0, 4, 4), Color(255, 255, 255), TuiLineStyle::Thin, {});
			TEST_ASSERT(TUI::GetBuffer()[19].GetChar32() == U'\u253C');
			target.StopRendering();
			target.StartRendering();
			target.PushClipper(Rect(2, 2, 8, 6), nullptr);
			target.Border(Rect(0, 2, 10, 7), Color(255, 255, 255), TuiLineStyle::Double, {});
			TEST_ASSERT(TUI::GetBuffer()[2 * 16 + 2].GetChar32() == U'\u2550');
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 2].GetChar32() == 0);
			target.Print(Point(7, 3), U'\u4E2D', Color(255, 255, 255), Color(0, 0, 0, 0), {});
			TEST_ASSERT(TUI::GetBuffer()[3 * 16 + 7].GetChar32() == 0);
			target.PopClipper(nullptr);
			target.Print(Point(4, 4), U'\u4E2D', Color(255, 255, 255), Color(0, 0, 0, 0), {true, true, true, true});
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 5].glyph == TuiPixelGlyph::WideCharContinuation);
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 4].character.style.bold);
			target.Fill(Rect(5, 4, 6, 5), Color(200, 100, 50));
			TEST_ASSERT(TUI::GetBuffer()[4 * 16 + 4].GetChar32() == 0);
			target.Fill(Rect(10, 0, 11, 1), Color(110, 120, 130, 128));
			auto color = TUI::GetBuffer()[10].backgroundColor;
			TEST_ASSERT(color.r == 60 && color.g == 70 && color.b == 80);
			target.StopRendering();
			TEST_ASSERT(backend->presents == 0);
			target.StopHostedRendering();
			TEST_ASSERT(backend->presents == 1);
			target.StartHostedRendering();
			target.StopHostedRendering();
			TEST_ASSERT(backend->presents == 2 && backend->frame[19].GetChar32() == 0);
			controller->DestroyNativeWindow(window);
		});
	});
}
