# !!!EXECUTION!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## 1. Add tests for RemoveStyleName / SummarizeStyleName / RenameStyle

### File to edit

- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

### Where to insert

Find the end of the existing block:

```cpp
		TEST_CATEGORY(L"EditStyleName")
		{
			...
		});
```

Insert the following code immediately after it, still inside `TEST_CATEGORY(L"Styles")` and before the existing closing `	});` / `}` at the end of the file.

### Code to insert

```cpp
		TEST_CATEGORY(L"RemoveStyleName")
		{
			TEST_CATEGORY(L"SingleParagraph")
			{
				TEST_CASE(L"FullRange")
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

							RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleA"));
							textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/SingleParagraph_FullRange"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"PartialRange")
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

							RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleA"));
							textBox->RemoveStyleName(TextPos(0, 3), TextPos(0, 7));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto s0 = SummarizeName(textBox, 0, 3);
							TEST_ASSERT(s0);
							TEST_ASSERT(s0.Value() == WString::Unmanaged(L"StyleA"));
							TEST_ASSERT(!SummarizeName(textBox, 3, 7));
							auto s2 = SummarizeName(textBox, 7, 10);
							TEST_ASSERT(s2);
							TEST_ASSERT(s2.Value() == WString::Unmanaged(L"StyleA"));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/SingleParagraph_PartialRange"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"NoStyle")
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

							textBox->RemoveStyleName(TextPos(0, 0), TextPos(0, 10));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							TEST_ASSERT(!SummarizeName(textBox, 0, 10));
							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/SingleParagraph_NoStyle"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"OverlappingStyles")
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

							RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
							RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));
							textBox->EditStyleName(TextPos(0, 0), TextPos(0, 5), WString::Unmanaged(L"StyleA"));
							textBox->EditStyleName(TextPos(0, 5), TextPos(0, 10), WString::Unmanaged(L"StyleB"));
							textBox->RemoveStyleName(TextPos(0, 3), TextPos(0, 7));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto s0 = SummarizeName(textBox, 0, 3);
							TEST_ASSERT(s0);
							TEST_ASSERT(s0.Value() == WString::Unmanaged(L"StyleA"));
							TEST_ASSERT(!SummarizeName(textBox, 3, 7));
							auto s2 = SummarizeName(textBox, 7, 10);
							TEST_ASSERT(s2);
							TEST_ASSERT(s2.Value() == WString::Unmanaged(L"StyleB"));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/SingleParagraph_OverlappingStyles"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});
			});

			TEST_CATEGORY(L"MultiParagraph")
			{
				TEST_CASE(L"SameRange")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();
							textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

							RegisterStyle(textBox, WString::Unmanaged(L"MyStyle"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 5), TextPos(2, 5), WString::Unmanaged(L"MyStyle"));
							textBox->RemoveStyleName(TextPos(0, 5), TextPos(2, 5));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							TEST_ASSERT(!SummarizeName(textBox, 0, 5, 10));
							TEST_ASSERT(!SummarizeName(textBox, 1, 0, 10));
							TEST_ASSERT(!SummarizeName(textBox, 2, 0, 5));
							TEST_ASSERT(!textBox->SummarizeStyleName(TextPos(0, 5), TextPos(2, 5)));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/MultiParagraph_SameRange"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"EdgeRetention")
				{
					TooltipTimer timer;
					GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
					{
						protocol->OnNextIdleFrame(L"Init", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
							textBox->SetFocused();
							textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

							RegisterStyle(textBox, WString::Unmanaged(L"MyStyle"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(2, 10), WString::Unmanaged(L"MyStyle"));
							textBox->RemoveStyleName(TextPos(0, 5), TextPos(2, 5));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							auto p0h = SummarizeName(textBox, 0, 0, 5);
							TEST_ASSERT(p0h);
							TEST_ASSERT(p0h.Value() == WString::Unmanaged(L"MyStyle"));
							TEST_ASSERT(!SummarizeName(textBox, 0, 5, 10));

							TEST_ASSERT(!SummarizeName(textBox, 1, 0, 10));

							TEST_ASSERT(!SummarizeName(textBox, 2, 0, 5));
							auto p2t = SummarizeName(textBox, 2, 5, 10);
							TEST_ASSERT(p2t);
							TEST_ASSERT(p2t.Value() == WString::Unmanaged(L"MyStyle"));

							TEST_ASSERT(!textBox->SummarizeStyleName(TextPos(0, 5), TextPos(2, 5)));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/MultiParagraph_EdgeRetention"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});
			});
		});

		TEST_CATEGORY(L"SummarizeStyleName")
		{
			TEST_CASE(L"MixedStyles")
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

						RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
						RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 5), WString::Unmanaged(L"StyleA"));
						textBox->EditStyleName(TextPos(0, 5), TextPos(0, 10), WString::Unmanaged(L"StyleB"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						TEST_ASSERT(!SummarizeName(textBox, 0, 10));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/MixedStyles"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"UniformStyle")
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

						RegisterStyle(textBox, WString::Unmanaged(L"MyStyle"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyStyle"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						auto summary = SummarizeName(textBox, 0, 10);
						TEST_ASSERT(summary);
						TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyStyle"));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/UniformStyle"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"MultiParagraph_Uniform")
			{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Init", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						textBox->SetFocused();
						textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

						RegisterStyle(textBox, WString::Unmanaged(L"MyStyle"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(2, 10), WString::Unmanaged(L"MyStyle"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						auto summary = textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10));
						TEST_ASSERT(summary);
						TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyStyle"));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/MultiParagraph_Uniform"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"MultiParagraph_Mixed")
			{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Init", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						textBox->SetFocused();
						textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

						RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
						RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));
						textBox->EditStyleName(TextPos(0, 0), TextPos(1, 10), WString::Unmanaged(L"StyleA"));
						textBox->EditStyleName(TextPos(2, 0), TextPos(2, 10), WString::Unmanaged(L"StyleB"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						auto p0 = SummarizeName(textBox, 0, 0, 10);
						TEST_ASSERT(p0);
						TEST_ASSERT(p0.Value() == WString::Unmanaged(L"StyleA"));
						auto p1 = SummarizeName(textBox, 1, 0, 10);
						TEST_ASSERT(p1);
						TEST_ASSERT(p1.Value() == WString::Unmanaged(L"StyleA"));
						auto p2 = SummarizeName(textBox, 2, 0, 10);
						TEST_ASSERT(p2);
						TEST_ASSERT(p2.Value() == WString::Unmanaged(L"StyleB"));
						TEST_ASSERT(!textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10)));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/MultiParagraph_Mixed"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"PartiallyStyled")
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

						RegisterStyle(textBox, WString::Unmanaged(L"MyStyle"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 5), WString::Unmanaged(L"MyStyle"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						TEST_ASSERT(!SummarizeName(textBox, 0, 10));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/PartiallyStyled"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"NoStyle")
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
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						TEST_ASSERT(!SummarizeName(textBox, 0, 10));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/NoStyle"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"ReversedRange")
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

						RegisterStyle(textBox, WString::Unmanaged(L"MyStyle"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 2), TextPos(0, 5), WString::Unmanaged(L"MyStyle"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						auto forward = textBox->SummarizeStyleName(TextPos(0, 2), TextPos(0, 5));
						auto reversed = textBox->SummarizeStyleName(TextPos(0, 5), TextPos(0, 2));
						TEST_ASSERT(forward);
						TEST_ASSERT(reversed);
						TEST_ASSERT(forward.Value() == WString::Unmanaged(L"MyStyle"));
						TEST_ASSERT(reversed.Value() == WString::Unmanaged(L"MyStyle"));

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/ReversedRange"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});
		});

		TEST_CATEGORY(L"RenameStyle")
		{
			TEST_CASE(L"Success")
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

						RegisterStyle(textBox, WString::Unmanaged(L"OldName"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"OldName"));
						textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
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
					WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/Success"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"ParentReference")
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

						RegisterStyle(textBox, WString::Unmanaged(L"Parent"), WString::Empty, MakeStyleWithBold(true));
						RegisterStyle(textBox, WString::Unmanaged(L"Child"), WString::Unmanaged(L"Parent"), MakeStyleWithBold(false));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"Child"));
						textBox->RenameStyle(WString::Unmanaged(L"Parent"), WString::Unmanaged(L"NewParent"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
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
					WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/ParentReference"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"MultipleRanges")
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

						RegisterStyle(textBox, WString::Unmanaged(L"OldName"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 2), WString::Unmanaged(L"OldName"));
						textBox->EditStyleName(TextPos(0, 5), TextPos(0, 7), WString::Unmanaged(L"OldName"));
						textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						auto a = SummarizeName(textBox, 0, 2);
						TEST_ASSERT(a);
						TEST_ASSERT(a.Value() == WString::Unmanaged(L"NewName"));
						TEST_ASSERT(!SummarizeName(textBox, 2, 5));
						auto b = SummarizeName(textBox, 5, 7);
						TEST_ASSERT(b);
						TEST_ASSERT(b.Value() == WString::Unmanaged(L"NewName"));

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/MultipleRanges"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"ExistingNameFails")
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

						RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
						RegisterStyle(textBox, WString::Unmanaged(L"StyleB"), WString::Empty, MakeStyleWithBold(false));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleA"));
						textBox->RenameStyle(WString::Unmanaged(L"StyleA"), WString::Unmanaged(L"StyleB"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						auto summary = SummarizeName(textBox, 0, 10);
						TEST_ASSERT(summary);
						TEST_ASSERT(summary.Value() == WString::Unmanaged(L"StyleA"));

						auto document = textBox->GetDocument();
						TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"StyleA")));
						TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"StyleB")));
						auto styleA = document->styles[WString::Unmanaged(L"StyleA")];
						TEST_ASSERT(styleA->styles->bold);
						TEST_ASSERT(styleA->styles->bold.Value() == true);
						auto styleB = document->styles[WString::Unmanaged(L"StyleB")];
						TEST_ASSERT(styleB->styles->bold);
						TEST_ASSERT(styleB->styles->bold.Value() == false);

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/ExistingNameFails"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"NonExistentFails")
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

						RegisterStyle(textBox, WString::Unmanaged(L"StyleA"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"StyleA"));
						textBox->RenameStyle(WString::Unmanaged(L"Missing"), WString::Unmanaged(L"NewName"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						auto summary = SummarizeName(textBox, 0, 10);
						TEST_ASSERT(summary);
						TEST_ASSERT(summary.Value() == WString::Unmanaged(L"StyleA"));

						auto document = textBox->GetDocument();
						TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));
						TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"StyleA")));

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/NonExistentFails"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"MultiParagraph")
			{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Init", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						textBox->SetFocused();
						textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

						RegisterStyle(textBox, WString::Unmanaged(L"OldName"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(2, 10), WString::Unmanaged(L"OldName"));
						textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

						for (vint row = 0; row < 3; row++)
						{
							auto s = SummarizeName(textBox, row, 0, 10);
							TEST_ASSERT(s);
							TEST_ASSERT(s.Value() == WString::Unmanaged(L"NewName"));
						}

						auto all = textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10));
						TEST_ASSERT(all);
						TEST_ASSERT(all.Value() == WString::Unmanaged(L"NewName"));

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/MultiParagraph"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});
		});
```

# FIXING ATTEMPTS

- None.

# !!!FINISHED!!!
