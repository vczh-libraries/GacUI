# !!!PLANNING!!!

# UPDATES

# AFFECTED PROJECTS

- Build the solution in folder `REPO-ROOT\Test\GacUISrc` (Debug|x64).
- Always Run UnitTest project `UnitTest` (Debug|x64).

# EXECUTION PLAN

## STEP 1: Add new TEST_CATEGORY hierarchies

We need to add test coverage for three `vl::presentation::controls::GuiDocumentCommonInterface` APIs:
- `RemoveStyleName(TextPos begin, TextPos end)`
- `SummarizeStyleName(TextPos begin, TextPos end)`
- `RenameStyle(const WString& oldStyleName, const WString& newStyleName)`

These new tests must live under their own category hierarchies as required by the task:
- `TEST_CATEGORY(L"RemoveStyleName") / TEST_CATEGORY(L"SingleParagraph")`
- `TEST_CATEGORY(L"RemoveStyleName") / TEST_CATEGORY(L"MultiParagraph")`
- `TEST_CATEGORY(L"SummarizeStyleName")`
- `TEST_CATEGORY(L"RenameStyle")`

**File to update (test-only):**
- `REPO-ROOT\Test\GacUISrc\UnitTest\TestControls_Editor_Styles.cpp`

Insert new categories as siblings of the existing `TEST_CATEGORY(L"EditStyleName")` under `TEST_CATEGORY(L"Styles")`.

Planned insertion point (structure only):
```cpp
TEST_CATEGORY(L"Styles")
{
  TEST_CATEGORY(L"EditStyleName")
  {
    // existing tests
  }

  TEST_CATEGORY(L"RemoveStyleName")
  {
    TEST_CATEGORY(L"SingleParagraph")
    {
      // new test cases
    }

    TEST_CATEGORY(L"MultiParagraph")
    {
      // new test cases
    }
  }

  TEST_CATEGORY(L"SummarizeStyleName")
  {
    // new test cases
  }

  TEST_CATEGORY(L"RenameStyle")
  {
    // new test cases
  }
}
```

Why: the repository’s unit tests are organized by `TEST_CATEGORY` and the task explicitly requires these hierarchies so the snapshot/log paths are stable and discoverable.

## STEP 2: Implement RemoveStyleName / SingleParagraph test cases

All these tests reuse the existing harness pattern in `TestControls_Editor_Styles.cpp`:
- `resource_DocumentViewer`
- `TooltipTimer timer;`
- two frames: `Init` then `Verify`
- `RegisterStyle(...)` helper and `SummarizeName(...)` helper

Common setup:
```cpp
textBox->SetFocused();
textBox->LoadTextAndClearUndoRedo(L"0123456789");
```

### STEP 2.1: Full range removal

**Goal:** Apply a style name to `[0,10)` then remove the same range; summary should be null.

```cpp
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
```

### STEP 2.2: Partial range removal

**Goal:** Apply `[0,10)` then remove `[3,7)`; verify unaffected edges keep the style name.

```cpp
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

      auto left = SummarizeName(textBox, 0, 3);
      TEST_ASSERT(left);
      TEST_ASSERT(left.Value() == WString::Unmanaged(L"StyleA"));

      TEST_ASSERT(!SummarizeName(textBox, 3, 7));

      auto right = SummarizeName(textBox, 7, 10);
      TEST_ASSERT(right);
      TEST_ASSERT(right.Value() == WString::Unmanaged(L"StyleA"));

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/SingleParagraph_PartialRange"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

### STEP 2.3: Remove when no style is applied

**Goal:** Call `RemoveStyleName` on unstyled text; verify no crash and summary remains null.

```cpp
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
```

### STEP 2.4: Remove with overlapping styles

**Goal:** Apply `StyleA` to `[0,5)` and `StyleB` to `[5,10)`, then remove `[3,7)` and ensure the overlap becomes unstyled.

```cpp
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

      auto left = SummarizeName(textBox, 0, 3);
      TEST_ASSERT(left);
      TEST_ASSERT(left.Value() == WString::Unmanaged(L"StyleA"));

      TEST_ASSERT(!SummarizeName(textBox, 3, 7));

      auto right = SummarizeName(textBox, 7, 10);
      TEST_ASSERT(right);
      TEST_ASSERT(right.Value() == WString::Unmanaged(L"StyleB"));

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/RemoveStyleName/SingleParagraph_OverlappingStyles"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

Why: `RemoveStyleName` unwraps `DocumentStyleApplicationRun` containers. These tests validate correct splitting / unwrapping behavior through the public query surface `SummarizeStyleName`.

## STEP 3: Implement RemoveStyleName / MultiParagraph test cases

Use the same 3-paragraph text already present in this file (10 chars each):
```cpp
textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");
```

### STEP 3.1: Remove style across paragraphs (same range)

```cpp
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
```

### STEP 3.2: Partial removal across paragraphs (edge retention)

```cpp
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

      // paragraph 0: head styled, tail cleared
      auto p0h = SummarizeName(textBox, 0, 0, 5);
      TEST_ASSERT(p0h);
      TEST_ASSERT(p0h.Value() == WString::Unmanaged(L"MyStyle"));
      TEST_ASSERT(!SummarizeName(textBox, 0, 5, 10));

      // paragraph 1: fully cleared
      TEST_ASSERT(!SummarizeName(textBox, 1, 0, 10));

      // paragraph 2: head cleared, tail styled
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
```

Why: multi-paragraph removal exercises the `DocumentModel::EditContainer` machinery across multiple `DocumentParagraphRun`s; the edge-retention case verifies correct splitting on both ends.

## STEP 4: Implement SummarizeStyleName test cases

All cases should assert on `Nullable<WString>` semantics:
- null result: `TEST_ASSERT(!summary)`
- non-null: `TEST_ASSERT(summary); TEST_ASSERT(summary.Value() == ...)`

### STEP 4.1: Returns null for mixed styles

Keep this test as direct coverage of the “mixed styles => null” contract (even if other categories also observe null in specific scenarios), so it isn’t accidentally removed as a “duplicate”.

```cpp
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
```

### STEP 4.2: Returns null for partially styled text

```cpp
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
```

### STEP 4.3: Returns null for text with no style

```cpp
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
```

### STEP 4.4: Reversed begin/end normalization

`GuiDocumentCommonInterface::SummarizeStyleName` swaps reversed ranges (`begin > end`), so the reversed call should match the forward call.

```cpp
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
      textBox->EditStyleName(TextPos(0, 0), TextPos(0, 10), WString::Unmanaged(L"MyStyle"));
    });

    protocol->OnNextIdleFrame(L"Verify", [=]()
    {
      auto window = GetApplication()->GetMainWindow();
      auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

      auto forward = textBox->SummarizeStyleName(TextPos(0, 2), TextPos(0, 5));
      auto reversed = textBox->SummarizeStyleName(TextPos(0, 5), TextPos(0, 2));
      TEST_ASSERT(forward);
      TEST_ASSERT(reversed);
      TEST_ASSERT(forward.Value() == reversed.Value());
      TEST_ASSERT(forward.Value() == WString::Unmanaged(L"MyStyle"));

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/ReversedRange"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

### STEP 4.5: Returns style name for uniform style

```cpp
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
```

### STEP 4.6: Returns style name for sub range

```cpp
TEST_CASE(L"SubRange")
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

      auto summary = SummarizeName(textBox, 2, 5);
      TEST_ASSERT(summary);
      TEST_ASSERT(summary.Value() == WString::Unmanaged(L"MyStyle"));

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/SubRange"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

### STEP 4.7: Returns style name across paragraphs

```cpp
TEST_CASE(L"AcrossParagraphs")
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
    WString::Unmanaged(L"Controls/Editor/Features/Styles/SummarizeStyleName/AcrossParagraphs"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

Why: these cases cover the null-return contract (mixed/partial/none), positive uniform results (full/subrange/multi-paragraph), and the begin/end normalization behavior.

It is acceptable to omit empty-range / ambiguous edge cases unless/until the implementation contract is confirmed; prefer well-defined scenarios aligned with existing tests.

## STEP 5: Implement RenameStyle test cases

Important constraint: `GuiDocumentCommonInterface::RenameStyle` returns `void`, while the model returns `bool`. Therefore, success/failure must be verified by observing:
- `SummarizeStyleName` output on affected ranges
- `textBox->GetDocument()->styles` dictionary keys / values
- `parentStyleName` update in other styles

Use `DocumentModel::StyleMap` operations visible in production code (`styles.Keys().Contains(...)`, etc.).

### STEP 5.1: Rename a registered style (success)

```cpp
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
```

### STEP 5.2: Rename with the same name is a no-op

```cpp
TEST_CASE(L"SameNameNoOp")
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
      textBox->RenameStyle(WString::Unmanaged(L"StyleA"), WString::Unmanaged(L"StyleA"));
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

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/SameNameNoOp"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

### STEP 5.3: Rename updates parent references

```cpp
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

      // Assert key existence before using Dictionary::operator[] for clearer failures.
      TEST_ASSERT(document->styles.Keys().Contains(WString::Unmanaged(L"Child")));
      auto child = document->styles[WString::Unmanaged(L"Child")];
      TEST_ASSERT(child);
      TEST_ASSERT(child->parentStyleName == WString::Unmanaged(L"NewParent"));

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/ParentReference"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

### STEP 5.4: Rename a style applied to multiple ranges

```cpp
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
      textBox->EditStyleName(TextPos(0, 7), TextPos(0, 10), WString::Unmanaged(L"OldName"));
      textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
    });

    protocol->OnNextIdleFrame(L"Verify", [=]()
    {
      auto window = GetApplication()->GetMainWindow();
      auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

      auto a = SummarizeName(textBox, 0, 2);
      TEST_ASSERT(a);
      TEST_ASSERT(a.Value() == WString::Unmanaged(L"NewName"));

      auto b = SummarizeName(textBox, 7, 10);
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
```

### STEP 5.5: Rename to an existing style name fails (observable behavior)

```cpp
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

      // new name already exists -> model returns false; UI API is void, so verify by state.
      // Note: this API might still enqueue undo metadata; validate failure only via document state observations.
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

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/ExistingNameFails"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

### STEP 5.6: Rename a non-existent style fails

```cpp
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
      TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"Missing")));
      TEST_ASSERT(!document->styles.Keys().Contains(WString::Unmanaged(L"NewName")));

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/NonExistentFails"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

### STEP 5.7: Rename across multiple paragraphs

```cpp
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
      textBox->EditStyleName(TextPos(0, 5), TextPos(2, 5), WString::Unmanaged(L"OldName"));
      textBox->RenameStyle(WString::Unmanaged(L"OldName"), WString::Unmanaged(L"NewName"));
    });

    protocol->OnNextIdleFrame(L"Verify", [=]()
    {
      auto window = GetApplication()->GetMainWindow();
      auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

      auto p0 = SummarizeName(textBox, 0, 5, 10);
      TEST_ASSERT(p0);
      TEST_ASSERT(p0.Value() == WString::Unmanaged(L"NewName"));

      auto p1 = SummarizeName(textBox, 1, 0, 10);
      TEST_ASSERT(p1);
      TEST_ASSERT(p1.Value() == WString::Unmanaged(L"NewName"));

      auto p2 = SummarizeName(textBox, 2, 0, 5);
      TEST_ASSERT(p2);
      TEST_ASSERT(p2.Value() == WString::Unmanaged(L"NewName"));

      window->Hide();
    });
  });
  GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
    WString::Unmanaged(L"Controls/Editor/Features/Styles/RenameStyle/MultiParagraph"),
    WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
    resource_DocumentViewer
  );
});
```

Why: `DocumentModel::RenameStyle` updates (1) dictionary keys, (2) parent references, and (3) all paragraphs’ runs; these cases cover each externally-visible effect and the failure conditions.

# TEST PLAN

## Build

Follow `REPO-ROOT\.github\Guidelines\Building.md` (build via wrapper scripts only):

```powershell
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotBuild.ps1 -Configuration Debug -Platform x64
```

Success criteria: `REPO-ROOT\.github\Scripts\Build.log` ends with:
- `Build succeeded.`
- `0 Warning(s)` / `0 Error(s)` (or at least `0 Error(s)` if warnings exist)

## Run UnitTest

Follow `REPO-ROOT\.github\Guidelines\Running-UnitTest.md`:

```powershell
& REPO-ROOT\.github\Scripts\copilotDebug_Stop.ps1
cd REPO-ROOT\Test\GacUISrc
& REPO-ROOT\.github\Scripts\copilotExecute.ps1 -Mode UnitTest -Executable UnitTest -Configuration Debug -Platform x64
```

Success criteria (trust `Execute.log` only):
- `REPO-ROOT\.github\Scripts\Execute.log` ends with:
  - `Passed test files: X/X`
  - `Passed test cases: Y/Y`

## Coverage checklist

- RemoveStyleName / SingleParagraph: `FullRange`, `PartialRange`, `NoStyle`, `OverlappingStyles`
- RemoveStyleName / MultiParagraph: `SameRange`, `EdgeRetention`
- SummarizeStyleName: `UniformStyle`, `SubRange`, `AcrossParagraphs`, `MixedStyles`, `PartiallyStyled`, `NoStyle`, `ReversedRange`
- RenameStyle: `Success`, `SameNameNoOp`, `ParentReference`, `MultipleRanges`, `ExistingNameFails`, `NonExistentFails`, `MultiParagraph`

# !!!FINISHED!!!
