# !!!EXECUTION!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## STEP 1: Add EditStyleName(SingleParagraph) unit tests [DONE]

### What to change

Edit:
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

Inside the existing:

```cpp
TEST_CATEGORY(L"Styles")
{
	// INSERT HERE
});
```

add the new nested categories and test cases.


### Final code to insert (copy-pasteable)

Insert the following code inside `TEST_CATEGORY(L"Styles") { ... });`:

```cpp
	TEST_CATEGORY(L"EditStyleName")
	{
		TEST_CATEGORY(L"SingleParagraph")
		{
			TEST_CASE(L"RegisteredStyle")
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

						RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyBold"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
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
					WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_RegisteredStyle"),
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

						RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
						textBox->EditStyleName(TextPos(0, 2), TextPos(0, 5), WString::Unmanaged(L"MyBold"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						TEST_ASSERT(!SummarizeName(textBox, 0, 2));
						auto mid = SummarizeName(textBox, 2, 5);
						TEST_ASSERT(mid);
						TEST_ASSERT(mid.Value() == WString::Unmanaged(L"MyBold"));
						TEST_ASSERT(!SummarizeName(textBox, 5, 10));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_PartialRange"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"UnregisteredStyle")
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

						textBox->EditStyleName(TextPos(0, 2), TextPos(0, 5), WString::Unmanaged(L"UnregisteredStyle"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						auto summary = SummarizeName(textBox, 2, 5);
						TEST_ASSERT(summary);
						TEST_ASSERT(summary.Value() == WString::Unmanaged(L"UnregisteredStyle"));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_UnregisteredStyle"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"AdjacentRanges")
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
						auto a = SummarizeName(textBox, 0, 5);
						TEST_ASSERT(a);
						TEST_ASSERT(a.Value() == WString::Unmanaged(L"StyleA"));
						auto b = SummarizeName(textBox, 5, 10);
						TEST_ASSERT(b);
						TEST_ASSERT(b.Value() == WString::Unmanaged(L"StyleB"));
						TEST_ASSERT(!SummarizeName(textBox, 0, 10));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_AdjacentRanges"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});

			TEST_CASE(L"Overwrite")
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
						textBox->EditStyleName(TextPos(0, 3), TextPos(0, 7), WString::Unmanaged(L"StyleB"));
					});

					protocol->OnNextIdleFrame(L"Verify", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");
						auto s0 = SummarizeName(textBox, 0, 3);
						TEST_ASSERT(s0);
						TEST_ASSERT(s0.Value() == WString::Unmanaged(L"StyleA"));
						auto s1 = SummarizeName(textBox, 3, 7);
						TEST_ASSERT(s1);
						TEST_ASSERT(s1.Value() == WString::Unmanaged(L"StyleB"));
						auto s2 = SummarizeName(textBox, 7, 10);
						TEST_ASSERT(s2);
						TEST_ASSERT(s2.Value() == WString::Unmanaged(L"StyleA"));
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/SingleParagraph_Overwrite"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentViewer
				);
			});
		});
	});
```

## STEP 2: Verify by building and running UnitTest [DONE]

Build the solution (Debug|x64):

```powershell
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
& REPO-ROOT\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64
```

Run unit tests (Debug|x64):

```powershell
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest -Configuration Debug -Platform x64
```

# FIXING ATTEMPTS

## Fixing attempt No.1

- Why the original change did not work: the new UnregisteredStyle test triggers an ArrayBase::Get out-of-range in DocumentModel::CopyDocument because it collects an unregistered style name and indexes styles[styleName] without checking existence.
- What I need to do: guard CopyDocument to skip style names that are not present in the model styles before dereferencing them.
- Why this should solve the failure: it avoids invalid dictionary lookups while still letting unregistered style names fall back to the default style during rendering and summarization.

# !!!FINISHED!!!
# !!!VERIFIED!!!

