# !!!EXECUTION!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## STEP 1: Add Undo/Redo tests for style operations

### File to edit

- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

### Insert location

Inside `TEST_CATEGORY(L"Styles")`, insert the following block **immediately before** the existing:

- `TEST_CATEGORY(L"SummarizeStyleName")`

### Code to insert

```cpp
		TEST_CATEGORY(L"UndoRedo")
		{
			TEST_CATEGORY(L"EditStyleName")
			{
				TEST_CASE(L"Undo_EditStyleName")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"EditStyleName", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/Undo_EditStyleName"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"Redo_EditStyleName")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							TEST_ASSERT(textBox->Redo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Redo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/Redo_EditStyleName"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"MultipleEdits_UndoAll_ReturnsToOriginal")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
							textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));

							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Edit then Remove", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));

							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo Remove", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo Apply", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/MultipleEdits_UndoAll_ReturnsToOriginal"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"UndoRedo_ReachingHistoryEnds_ReturnsFalse_AndStateCorrect")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Applied", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							while (textBox->CanUndo())
							{
								TEST_ASSERT(textBox->Undo());
							}
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							TEST_ASSERT(!textBox->Undo());
						});

						protocol->OnNextIdleFrame(L"Undo until the end", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							while (textBox->CanRedo())
							{
								TEST_ASSERT(textBox->Redo());
							}
							TEST_ASSERT(!textBox->CanRedo());
							TEST_ASSERT(textBox->CanUndo());

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(!textBox->Redo());

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/UndoRedo_ReachingHistoryEnds_ReturnsFalse_AndStateCorrect"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"RedoHistoryDropped_OnNewEdit")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
							RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));

							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleA"));
							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());

							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleB"));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo then Edit Again", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"StyleB"));

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/EditStyleName/RedoHistoryDropped_OnNewEdit"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});
			});

			TEST_CATEGORY(L"RemoveStyleName")
			{
				TEST_CASE(L"Undo_RemoveStyleName")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
							textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Removed", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));
							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/Undo_RemoveStyleName"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"Redo_RemoveStyleName")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));

							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
							textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

							TEST_ASSERT(textBox->Redo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Redo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/Redo_RemoveStyleName"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"CanUndoCanRedo_Transitions")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Applied then Removed", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 10));

							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo Remove", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyBold"));

							TEST_ASSERT(textBox->Redo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
							TEST_ASSERT(!SummarizeName(textBox, 0, 10));

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RemoveStyleName/CanUndoCanRedo_Transitions"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});
			});

			TEST_CATEGORY(L"RenameStyle")
			{
				TEST_CASE(L"Undo_RenameStyle")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"OldName"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"OldName"));

							textBox->ClearUndoRedo();
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"RenameStyle", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"NewName"));

							auto document = textBox->GetDocument();
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"OldName")));
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));

							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"OldName"));

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"OldName")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/Undo_RenameStyle"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"Redo_RenameStyle")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
							RegisterStyle(textBox, WString::Unmanaged(L"OldName"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"OldName"));
							textBox->ClearUndoRedo();
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"OldName"));

							TEST_ASSERT(textBox->Redo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Redo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"NewName"));

							auto document = textBox->GetDocument();
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"OldName")));
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/Redo_RenameStyle"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"Undo_RenameStyle_ParentReferences")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"Parent"), WString::Empty, MakeStyleWithBold(true));
							RegisterStyle(textBox, WString::Unmanaged(L"Child"), WString::Unmanaged(L"Parent"), MakeStyleWithBold(false));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"Child"));

							textBox->ClearUndoRedo();
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
							textBox->RenameStyle(WString::Unmanaged(L"Parent"), WString::Unmanaged(L"NewParent"));
							TEST_ASSERT(textBox->CanUndo());
						});

						protocol->OnNextIdleFrame(L"Rename Parent", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"Parent")));
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewParent")));
							TEST_ASSERT(document->styles[WString::Unmanaged(L"Child")]->parentStyleName == WString::Unmanaged(L"NewParent"));

							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(textBox->CanRedo());
							TEST_ASSERT(!textBox->CanUndo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Parent")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"NewParent")));
							TEST_ASSERT(document->styles[WString::Unmanaged(L"Child")]->parentStyleName == WString::Unmanaged(L"Parent"));

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/Undo_RenameStyle_ParentReferences"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"Redo_RenameStyle_ParentReferences")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							RegisterStyle(textBox, WString::Unmanaged(L"Parent"), WString::Empty, MakeStyleWithBold(true));
							RegisterStyle(textBox, WString::Unmanaged(L"Child"), WString::Unmanaged(L"Parent"), MakeStyleWithBold(false));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"Child"));

							textBox->ClearUndoRedo();
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							textBox->RenameStyle(WString::Unmanaged(L"Parent"), WString::Unmanaged(L"NewParent"));
							TEST_ASSERT(textBox->CanUndo());

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"Parent")));
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewParent")));
							TEST_ASSERT(document->styles[WString::Unmanaged(L"Child")]->parentStyleName == WString::Unmanaged(L"NewParent"));

							TEST_ASSERT(textBox->Undo());
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Undo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Parent")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"NewParent")));
							TEST_ASSERT(document->styles[WString::Unmanaged(L"Child")]->parentStyleName == WString::Unmanaged(L"Parent"));

							TEST_ASSERT(textBox->Redo());
							TEST_ASSERT(textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
						});

						protocol->OnNextIdleFrame(L"Redo", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"Parent")));
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"NewParent")));
							TEST_ASSERT(document->styles[WString::Unmanaged(L"Child")]->parentStyleName == WString::Unmanaged(L"NewParent"));

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/Redo_RenameStyle_ParentReferences"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"MultipleRenames_UndoAll_ReturnsToA")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();

							textBox->LoadTextAndClearUndoRedo(L"0123456789");
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());
							RegisterStyle(textBox, WString::Unmanaged(L"A"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"A"));

							textBox->ClearUndoRedo();
							TEST_ASSERT(!textBox->CanUndo());
							TEST_ASSERT(!textBox->CanRedo());

							textBox->RenameStyle(WString::Unmanaged(L"A"), WString::Unmanaged(L"B"));
							textBox->RenameStyle(WString::Unmanaged(L"B"), WString::Unmanaged(L"C"));
							TEST_ASSERT(textBox->CanUndo());
						});

						protocol->OnNextIdleFrame(L"A->B->C", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"C"));

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"C")));

							TEST_ASSERT(textBox->Undo());
						});

						protocol->OnNextIdleFrame(L"Undo C->B", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"B"));

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"B")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"C")));

							TEST_ASSERT(textBox->Undo());
						});

						protocol->OnNextIdleFrame(L"Undo B->A", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto summary = SummarizeName(textBox, 0, 10);
							TEST_ASSERT(summary);
							TEST_ASSERT(summary.Value() == WString::Unmanaged(L"A"));

							auto document = textBox->GetDocument();
							TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"A")));
							TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"B")));

							window->Hide();
						});
					});

					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/UndoRedo/RenameStyle/MultipleRenames_UndoAll_ReturnsToA"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});
			});
		});
```

## STEP 2: Build and run UnitTest

```powershell
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotDebug_Stop.ps1

cd C:\Code\VczhLibraries\GacUI\Test\GacUISrc
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64

cd C:\Code\VczhLibraries\GacUI\Test\GacUISrc
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest -Configuration Debug -Platform x64
```

Verification expectations:
- Build succeeds.
- UnitTest passes.
- No unexpected `*.UI.errors.txt` artifacts are produced.

# FIXING ATTEMPTS

- N/A (execution document only).

# !!!FINISHED!!!
