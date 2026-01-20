#include "TestControls.h"
#include "TestItemProviders.h"

using namespace gacui_unittest_template;

namespace
{
	Ptr<DocumentStyleProperties> MakeFontFaceStyle(const WString& face)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->face = face;
		return style;
	}

	Ptr<DocumentStyleProperties> MakeFontFaceAndSizeStyle(const WString& face, double size)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->face = face;
		style->size = DocumentFontSize(size, false);
		return style;
	}

	Ptr<DocumentStyleProperties> MakeTextStyle(bool bold, bool italic, bool underline, bool strikeline)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->bold = bold;
		style->italic = italic;
		style->underline = underline;
		style->strikeline = strikeline;
		return style;
	}

	Ptr<DocumentStyleProperties> MakeColorStyle(Color color)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->color = color;
		return style;
	}

	Ptr<DocumentStyleProperties> Summarize(GuiDocumentLabel* textBox, vint begin, vint end)
	{
		return textBox->SummarizeStyle(TextPos(0, begin), TextPos(0, end));
	}

	Ptr<DocumentStyleProperties> Summarize(GuiDocumentLabel* textBox, vint row, vint begin, vint end)
	{
		return textBox->SummarizeStyle(TextPos(row, begin), TextPos(row, end));
	}

	static NativePoint CaretToGlobalPoint(UnitTestRemoteProtocol* protocol, GuiDocumentLabel* textBox, TextPos caret)
	{
		auto caretBounds = textBox->GetCaretBounds(caret, false);
		auto container = textBox->GetContainerComposition();
		auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);

		auto x = topLeft.x + caretBounds.x1 + caretBounds.Width() / 2 + 4;
		auto y = topLeft.y + caretBounds.y1 + caretBounds.Height() / 2;
		return { x, y };
	}

	template<vint Count>
	static void AssertEventLogAndClear(List<WString>& eventLog, const wchar_t* (&expected)[Count])
	{
		AssertCallbacks(eventLog, expected);
		eventLog.Clear();
	}
}

TEST_FILE
{
	const auto resource_DocumentLabel = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentLabel RichText Test" ClientSize="x:480 y:320">
        <DocumentLabel ref.Name="textBox" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </DocumentLabel>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"SingleParagraph")
	{
		TEST_CASE(L"Font_OneCall")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789");
					textBox->EditStyle(TextPos(0, 2), TextPos(0, 5), MakeFontFaceAndSizeStyle(WString::Unmanaged(L"Arial"), 16));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					auto summary = Summarize(textBox, 2, 5);
					TEST_ASSERT(summary->face);
					TEST_ASSERT(summary->face.Value() == WString::Unmanaged(L"Arial"));
					TEST_ASSERT(summary->size);
					TEST_ASSERT(summary->size.Value() == DocumentFontSize(16, false));
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/SingleParagraph_Font_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"FontStyle_OneCall")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789");
					textBox->EditStyle(TextPos(0, 2), TextPos(0, 5), MakeTextStyle(true, true, false, false));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					auto summary = Summarize(textBox, 2, 5);
					TEST_ASSERT(summary->bold);
					TEST_ASSERT(summary->bold.Value() == true);
					TEST_ASSERT(summary->italic);
					TEST_ASSERT(summary->italic.Value() == true);
					TEST_ASSERT(summary->underline);
					TEST_ASSERT(summary->underline.Value() == false);
					TEST_ASSERT(summary->strikeline);
					TEST_ASSERT(summary->strikeline.Value() == false);
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/SingleParagraph_FontStyle_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Color_OneCall")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789");
					textBox->EditStyle(TextPos(0, 2), TextPos(0, 5), MakeColorStyle(Color(255, 0, 0)));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					auto summary = Summarize(textBox, 2, 5);
					TEST_ASSERT(summary->color);
					TEST_ASSERT(summary->color.Value() == Color(255, 0, 0));
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/SingleParagraph_Color_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"AllProperties_SameRange")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789");

					textBox->EditStyle(TextPos(0, 2), TextPos(0, 5), MakeFontFaceAndSizeStyle(WString::Unmanaged(L"Arial"), 16));
					textBox->EditStyle(TextPos(0, 2), TextPos(0, 5), MakeTextStyle(true, false, true, false));
					textBox->EditStyle(TextPos(0, 2), TextPos(0, 5), MakeColorStyle(Color(255, 0, 0)));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					auto summary = Summarize(textBox, 2, 5);

					TEST_ASSERT(summary->face);
					TEST_ASSERT(summary->face.Value() == WString::Unmanaged(L"Arial"));
					TEST_ASSERT(summary->size);
					TEST_ASSERT(summary->size.Value() == DocumentFontSize(16, false));
					TEST_ASSERT(summary->bold);
					TEST_ASSERT(summary->bold.Value() == true);
					TEST_ASSERT(summary->italic);
					TEST_ASSERT(summary->italic.Value() == false);
					TEST_ASSERT(summary->underline);
					TEST_ASSERT(summary->underline.Value() == true);
					TEST_ASSERT(summary->strikeline);
					TEST_ASSERT(summary->strikeline.Value() == false);
					TEST_ASSERT(summary->color);
					TEST_ASSERT(summary->color.Value() == Color(255, 0, 0));

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/SingleParagraph_AllProperties_SameRange"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_SameProperty_FontFace")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789");

					textBox->EditStyle(TextPos(0, 1), TextPos(0, 6), MakeFontFaceStyle(WString::Unmanaged(L"FontA")));
					textBox->EditStyle(TextPos(0, 4), TextPos(0, 9), MakeFontFaceStyle(WString::Unmanaged(L"FontB")));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto summary = Summarize(textBox, 1, 4);
						TEST_ASSERT(summary->face);
						TEST_ASSERT(summary->face.Value() == WString::Unmanaged(L"FontA"));
					}
					{
						auto summary = Summarize(textBox, 4, 9);
						TEST_ASSERT(summary->face);
						TEST_ASSERT(summary->face.Value() == WString::Unmanaged(L"FontB"));
					}
					{
						auto summary = Summarize(textBox, 1, 9);
						TEST_ASSERT(!summary->face);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/SingleParagraph_Overlap_SameProperty_FontFace"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_DifferentProperties_BoldThenColor")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789");

					textBox->EditStyle(TextPos(0, 1), TextPos(0, 6), MakeTextStyle(true, false, false, false));
					textBox->EditStyle(TextPos(0, 4), TextPos(0, 9), MakeColorStyle(Color(255, 0, 0)));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto summary = Summarize(textBox, 1, 4);
						TEST_ASSERT(summary->bold);
						TEST_ASSERT(summary->bold.Value() == true);
					}
					{
						auto summary = Summarize(textBox, 4, 6);
						TEST_ASSERT(summary->bold);
						TEST_ASSERT(summary->bold.Value() == true);
						TEST_ASSERT(summary->color);
						TEST_ASSERT(summary->color.Value() == Color(255, 0, 0));
					}
					{
						auto summary = Summarize(textBox, 6, 9);
						TEST_ASSERT(summary->color);
						TEST_ASSERT(summary->color.Value() == Color(255, 0, 0));
					}
					{
						auto summary = Summarize(textBox, 1, 9);
						TEST_ASSERT(!summary->bold);
						TEST_ASSERT(!summary->color);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/SingleParagraph_Overlap_DifferentProperties_BoldThenColor"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_SameProperty_BoldTrueThenFalse")
		{
			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789");

					textBox->EditStyle(TextPos(0, 1), TextPos(0, 9), MakeTextStyle(true, false, false, false));
					textBox->EditStyle(TextPos(0, 4), TextPos(0, 6), MakeTextStyle(false, false, false, false));
				});

				protocol->OnNextIdleFrame(L"Verify", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto summary = Summarize(textBox, 1, 4);
						TEST_ASSERT(summary->bold);
						TEST_ASSERT(summary->bold.Value() == true);
					}
					{
						auto summary = Summarize(textBox, 4, 6);
						TEST_ASSERT(summary->bold);
						TEST_ASSERT(summary->bold.Value() == false);
					}
					{
						auto summary = Summarize(textBox, 6, 9);
						TEST_ASSERT(summary->bold);
						TEST_ASSERT(summary->bold.Value() == true);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/SingleParagraph_Overlap_SameProperty_BoldTrueThenFalse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CATEGORY(L"Hyperlink")
		{
			TEST_CASE(L"SingleParagraph_EditRemove_Model")
			{
				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Ready", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						textBox->SetFocused();
						textBox->LoadTextAndClearUndoRedo(L"0123456789");
					});

					protocol->OnNextIdleFrame(L"Loaded", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						textBox->EditHyperlink(
							0, 2, 8,
							WString::Unmanaged(L"refA"),
							WString::Unmanaged(L"#NormalLink"),
							WString::Unmanaged(L"#ActiveLink")
						);
					});

					protocol->OnNextIdleFrame(L"Created Hyperlink", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						auto doc = textBox->GetDocument();

						auto package = doc->GetHyperlink(0, 3, 3);
						TEST_ASSERT(package->hyperlinks.Count() > 0);
						TEST_ASSERT(package->hyperlinks[0]->reference == WString::Unmanaged(L"refA"));

						textBox->EditHyperlink(
							0, 2, 8,
							WString::Unmanaged(L"refB"),
							WString::Unmanaged(L"#NormalLink"),
							WString::Unmanaged(L"#ActiveLink")
						);
					});

					protocol->OnNextIdleFrame(L"Edited Hyperlink", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						auto doc = textBox->GetDocument();

						auto package = doc->GetHyperlink(0, 4, 4);
						TEST_ASSERT(package->hyperlinks.Count() > 0);
						TEST_ASSERT(package->hyperlinks[0]->reference == WString::Unmanaged(L"refB"));

						textBox->RemoveHyperlink(0, 4, 6);
					});

					protocol->OnNextIdleFrame(L"Removed Hyperlink", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						auto doc = textBox->GetDocument();

						TEST_ASSERT(doc->GetHyperlink(0, 3, 3)->hyperlinks.Count() == 0);
						TEST_ASSERT(doc->GetHyperlink(0, 7, 7)->hyperlinks.Count() == 0);

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_SingleParagraph_EditRemove_Model"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentLabel
				);
			});

			TEST_CASE(L"SingleParagraph_ActivateExecute")
			{
				List<WString> events;
				NativePoint inside = { 0,0 };
				NativePoint outside = { 0,0 };

				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						textBox->SetFocused();
						textBox->LoadTextAndClearUndoRedo(L"0123456789");
					});

					protocol->OnNextIdleFrame(L"Loaded", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

							textBox->EditHyperlink(
							0, 2, 5,
							WString::Unmanaged(L"refA"),
							WString::Unmanaged(L"#NormalLink"),
							WString::Unmanaged(L"#ActiveLink")
							);

						textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);

						textBox->ActiveHyperlinkChanged.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
						{
							events.Add(L"Activated " + textBox->GetActiveHyperlinkReference());
						});
						textBox->ActiveHyperlinkExecuted.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
						{
							events.Add(L"Executed " + textBox->GetActiveHyperlinkReference());
						});

						inside = CaretToGlobalPoint(protocol, textBox, TextPos(0, 3));
						{
							auto container = textBox->GetContainerComposition();
							auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);
							outside = { topLeft.x - 10, topLeft.y - 10 };
						}
					});

					protocol->OnNextIdleFrame(L"Created Hyperlink", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(inside);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refA"));
						const wchar_t* expected[] = { L"Activated refA" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Hovered Hyperlink", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->LClick(inside);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refA"));
						const wchar_t* expected[] = { L"Executed refA" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Clicked Hyperlink", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(outside);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L""));
						const wchar_t* expected[] = { L"Activated " };
						AssertEventLogAndClear(events, expected);

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_SingleParagraph_ActivateExecute"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentLabel
				);
			});

			TEST_CASE(L"MultiParagraph_ActivateExecute")
			{
				List<WString> events;
				NativePoint insideP0 = { 0,0 };
				NativePoint insideP1 = { 0,0 };
				NativePoint outside = { 0,0 };

				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						textBox->SetFocused();
						textBox->LoadTextAndClearUndoRedo(L"012345\r\n\r\n6789AB\r\n\r\nCDEF");
					});

					protocol->OnNextIdleFrame(L"Loaded", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

							textBox->EditHyperlink(
							0, 1, 3,
							WString::Unmanaged(L"refP0"),
							WString::Unmanaged(L"#NormalLink"),
							WString::Unmanaged(L"#ActiveLink")
							);
					});

					protocol->OnNextIdleFrame(L"Created P0", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

							textBox->EditHyperlink(
							1, 2, 5,
							WString::Unmanaged(L"refP1"),
							WString::Unmanaged(L"#NormalLink"),
							WString::Unmanaged(L"#ActiveLink")
							);

						textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);

						textBox->ActiveHyperlinkChanged.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
						{
							events.Add(L"Activated " + textBox->GetActiveHyperlinkReference());
						});
						textBox->ActiveHyperlinkExecuted.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
						{
							events.Add(L"Executed " + textBox->GetActiveHyperlinkReference());
						});

						insideP0 = CaretToGlobalPoint(protocol, textBox, TextPos(0, 2));
						insideP1 = CaretToGlobalPoint(protocol, textBox, TextPos(1, 3));
						{
							auto container = textBox->GetContainerComposition();
							auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);
							outside = { topLeft.x - 10, topLeft.y - 10 };
						}
					});

					protocol->OnNextIdleFrame(L"Created P1", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(insideP0);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP0"));
						const wchar_t* expected[] = { L"Activated refP0" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Hovered P0", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->LClick(insideP0);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP0"));
						const wchar_t* expected[] = { L"Executed refP0" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Clicked P0", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(insideP1);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP1"));
						const wchar_t* expected[] = { L"Activated refP1" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Hovered P1", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->LClick(insideP1);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refP1"));
						const wchar_t* expected[] = { L"Executed refP1" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Clicked P1", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(outside);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L""));
						const wchar_t* expected[] = { L"Activated " };
						AssertEventLogAndClear(events, expected);

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_MultiParagraph_ActivateExecute"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentLabel
				);
			});

			TEST_CASE(L"Overlap_Remove_DoesNotLeaveStaleActive")
			{
				List<WString> events;
				NativePoint insideA = { 0,0 };
				NativePoint insideB = { 0,0 };
				NativePoint outside = { 0,0 };
				Ptr<DocumentStyleProperties> baselineOutside;

				TooltipTimer timer;
				GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						textBox->SetFocused();
						textBox->LoadTextAndClearUndoRedo(L"0123456789");
					});

					protocol->OnNextIdleFrame(L"Loaded", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

							textBox->EditHyperlink(
							0, 2, 7,
							WString::Unmanaged(L"refA"),
							WString::Unmanaged(L"#NormalLink"),
							WString::Unmanaged(L"#ActiveLink")
							);
					});

					protocol->OnNextIdleFrame(L"Created A", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

							textBox->EditHyperlink(
							0, 5, 9,
							WString::Unmanaged(L"refB"),
							WString::Unmanaged(L"#NormalLink"),
							WString::Unmanaged(L"#ActiveLink")
							);

						baselineOutside = Summarize(textBox, 0, 0, 2);

						textBox->SetEditMode(GuiDocumentEditMode::ViewOnly);
						textBox->ActiveHyperlinkChanged.AttachLambda([&, textBox](GuiGraphicsComposition*, GuiEventArgs&)
						{
							events.Add(L"Activated " + textBox->GetActiveHyperlinkReference());
						});

						insideA = CaretToGlobalPoint(protocol, textBox, TextPos(0, 3));
						insideB = CaretToGlobalPoint(protocol, textBox, TextPos(0, 8));
						{
							auto container = textBox->GetContainerComposition();
							auto topLeft = protocol->LocationOf(container, 0.0, 0.0, 0, 0);
							outside = { topLeft.x - 10, topLeft.y - 10 };
						}
					});

					protocol->OnNextIdleFrame(L"Created B", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(insideA);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refA"));
						const wchar_t* expected[] = { L"Activated refA" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Hovered A", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(insideB);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L"refB"));
						const wchar_t* expected[] = { L"Activated refB" };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Hovered B", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

							textBox->RemoveHyperlink(0, 6, 7);
						events.Clear();
					});

					protocol->OnNextIdleFrame(L"Removed Overlap", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						auto doc = textBox->GetDocument();

						protocol->MouseMove(insideA);
						auto package = doc->GetHyperlink(0, 3, 3);
						auto expectedRef = package->hyperlinks.Count() == 0 ? WString::Unmanaged(L"") : package->hyperlinks[0]->reference;
						WString expectedEvent = L"Activated " + expectedRef;
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == expectedRef);
						const wchar_t* expected[] = { expectedEvent.Buffer() };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Hovered A After Remove", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						auto doc = textBox->GetDocument();

						protocol->MouseMove(insideB);
						auto package = doc->GetHyperlink(0, 8, 8);
						auto expectedRef = package->hyperlinks.Count() == 0 ? WString::Unmanaged(L"") : package->hyperlinks[0]->reference;
						WString expectedEvent = L"Activated " + expectedRef;
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == expectedRef);
						const wchar_t* expected[] = { expectedEvent.Buffer() };
						AssertEventLogAndClear(events, expected);
					});

					protocol->OnNextIdleFrame(L"Hovered B After Remove", [&, protocol]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

						protocol->MouseMove(outside);
						TEST_ASSERT(textBox->GetActiveHyperlinkReference() == WString::Unmanaged(L""));
						const wchar_t* expected[] = { L"Activated " };
						AssertEventLogAndClear(events, expected);

						{
							auto afterOutside = Summarize(textBox, 0, 0, 2);
							TEST_ASSERT(baselineOutside->face == afterOutside->face);
							TEST_ASSERT(baselineOutside->size == afterOutside->size);
							TEST_ASSERT(baselineOutside->color == afterOutside->color);
							TEST_ASSERT(baselineOutside->backgroundColor == afterOutside->backgroundColor);
							TEST_ASSERT(baselineOutside->bold == afterOutside->bold);
							TEST_ASSERT(baselineOutside->italic == afterOutside->italic);
							TEST_ASSERT(baselineOutside->underline == afterOutside->underline);
							TEST_ASSERT(baselineOutside->strikeline == afterOutside->strikeline);
							TEST_ASSERT(baselineOutside->antialias == afterOutside->antialias);
							TEST_ASSERT(baselineOutside->verticalAntialias == afterOutside->verticalAntialias);
						}

						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/Features/RichText/Hyperlink_Overlap_Remove_DoesNotLeaveStaleActive"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource_DocumentLabel
				);
			});
		});

	TEST_CATEGORY(L"MultiParagraph")
	{
		TEST_CASE(L"Font_OneCall")
		{
			Nullable<WString> baseS0hFace;
			Nullable<DocumentFontSize> baseS0hSize;
			Nullable<WString> baseS2tFace;
			Nullable<DocumentFontSize> baseS2tSize;

			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						baseS0hFace = s0h->face;
						baseS0hSize = s0h->size;
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						baseS2tFace = s2t->face;
						baseS2tSize = s2t->size;
					}

					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), MakeFontFaceAndSizeStyle(WString::Unmanaged(L"Arial"), 16));
				});

				protocol->OnNextIdleFrame(L"Verify", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(s0h->face == baseS0hFace);
						TEST_ASSERT(s0h->size == baseS0hSize);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->face);
						TEST_ASSERT(s0t->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s0t->size);
						TEST_ASSERT(s0t->size.Value() == DocumentFontSize(16, false));
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->face);
						TEST_ASSERT(s1->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s1->size);
						TEST_ASSERT(s1->size.Value() == DocumentFontSize(16, false));
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->face);
						TEST_ASSERT(s2h->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s2h->size);
						TEST_ASSERT(s2h->size.Value() == DocumentFontSize(16, false));
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(s2t->face == baseS2tFace);
						TEST_ASSERT(s2t->size == baseS2tSize);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Font_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});
		
		TEST_CASE(L"FontStyle_OneCall")
		{
			Nullable<bool> baseS0hBold;
			Nullable<bool> baseS0hItalic;
			Nullable<bool> baseS0hUnderline;
			Nullable<bool> baseS0hStrikeline;
			Nullable<bool> baseS2tBold;
			Nullable<bool> baseS2tItalic;
			Nullable<bool> baseS2tUnderline;
			Nullable<bool> baseS2tStrikeline;

			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						baseS0hBold = s0h->bold;
						baseS0hItalic = s0h->italic;
						baseS0hUnderline = s0h->underline;
						baseS0hStrikeline = s0h->strikeline;
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						baseS2tBold = s2t->bold;
						baseS2tItalic = s2t->italic;
						baseS2tUnderline = s2t->underline;
						baseS2tStrikeline = s2t->strikeline;
					}

					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), MakeTextStyle(true, true, false, false));
				});

				protocol->OnNextIdleFrame(L"Verify", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(s0h->bold == baseS0hBold);
						TEST_ASSERT(s0h->italic == baseS0hItalic);
						TEST_ASSERT(s0h->underline == baseS0hUnderline);
						TEST_ASSERT(s0h->strikeline == baseS0hStrikeline);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->bold);
						TEST_ASSERT(s0t->bold.Value() == true);
						TEST_ASSERT(s0t->italic);
						TEST_ASSERT(s0t->italic.Value() == true);
						TEST_ASSERT(s0t->underline);
						TEST_ASSERT(s0t->underline.Value() == false);
						TEST_ASSERT(s0t->strikeline);
						TEST_ASSERT(s0t->strikeline.Value() == false);
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->bold);
						TEST_ASSERT(s1->bold.Value() == true);
						TEST_ASSERT(s1->italic);
						TEST_ASSERT(s1->italic.Value() == true);
						TEST_ASSERT(s1->underline);
						TEST_ASSERT(s1->underline.Value() == false);
						TEST_ASSERT(s1->strikeline);
						TEST_ASSERT(s1->strikeline.Value() == false);
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->bold);
						TEST_ASSERT(s2h->bold.Value() == true);
						TEST_ASSERT(s2h->italic);
						TEST_ASSERT(s2h->italic.Value() == true);
						TEST_ASSERT(s2h->underline);
						TEST_ASSERT(s2h->underline.Value() == false);
						TEST_ASSERT(s2h->strikeline);
						TEST_ASSERT(s2h->strikeline.Value() == false);
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(s2t->bold == baseS2tBold);
						TEST_ASSERT(s2t->italic == baseS2tItalic);
						TEST_ASSERT(s2t->underline == baseS2tUnderline);
						TEST_ASSERT(s2t->strikeline == baseS2tStrikeline);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_FontStyle_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Color_OneCall")
		{
			Nullable<Color> baseS0hColor;
			Nullable<Color> baseS2tColor;

			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						baseS0hColor = s0h->color;
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						baseS2tColor = s2t->color;
					}

					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), MakeColorStyle(Color(255, 0, 0)));
				});

				protocol->OnNextIdleFrame(L"Verify", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(s0h->color == baseS0hColor);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->color);
						TEST_ASSERT(s0t->color.Value() == Color(255, 0, 0));
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->color);
						TEST_ASSERT(s1->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->color);
						TEST_ASSERT(s2h->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(s2t->color == baseS2tColor);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Color_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"AllProperties_OneCall")
		{
			Nullable<WString> baseS0hFace;
			Nullable<DocumentFontSize> baseS0hSize;
			Nullable<bool> baseS0hBold;
			Nullable<bool> baseS0hItalic;
			Nullable<bool> baseS0hUnderline;
			Nullable<bool> baseS0hStrikeline;
			Nullable<Color> baseS0hColor;
			Nullable<WString> baseS2tFace;
			Nullable<DocumentFontSize> baseS2tSize;
			Nullable<bool> baseS2tBold;
			Nullable<bool> baseS2tItalic;
			Nullable<bool> baseS2tUnderline;
			Nullable<bool> baseS2tStrikeline;
			Nullable<Color> baseS2tColor;

			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						baseS0hFace = s0h->face;
						baseS0hSize = s0h->size;
						baseS0hBold = s0h->bold;
						baseS0hItalic = s0h->italic;
						baseS0hUnderline = s0h->underline;
						baseS0hStrikeline = s0h->strikeline;
						baseS0hColor = s0h->color;
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						baseS2tFace = s2t->face;
						baseS2tSize = s2t->size;
						baseS2tBold = s2t->bold;
						baseS2tItalic = s2t->italic;
						baseS2tUnderline = s2t->underline;
						baseS2tStrikeline = s2t->strikeline;
						baseS2tColor = s2t->color;
					}

					auto style = Ptr(new DocumentStyleProperties);
					style->face = WString::Unmanaged(L"Arial");
					style->size = DocumentFontSize(16, false);
					style->bold = true;
					style->italic = false;
					style->underline = true;
					style->strikeline = false;
					style->color = Color(255, 0, 0);
					textBox->EditStyle(TextPos(0, 2), TextPos(2, 5), style);
				});

				protocol->OnNextIdleFrame(L"Verify", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0h = Summarize(textBox, 0, 0, 2);
						TEST_ASSERT(s0h->face == baseS0hFace);
						TEST_ASSERT(s0h->size == baseS0hSize);
						TEST_ASSERT(s0h->bold == baseS0hBold);
						TEST_ASSERT(s0h->italic == baseS0hItalic);
						TEST_ASSERT(s0h->underline == baseS0hUnderline);
						TEST_ASSERT(s0h->strikeline == baseS0hStrikeline);
						TEST_ASSERT(s0h->color == baseS0hColor);
					}
					{
						auto s0t = Summarize(textBox, 0, 2, 10);
						TEST_ASSERT(s0t->face);
						TEST_ASSERT(s0t->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s0t->size);
						TEST_ASSERT(s0t->size.Value() == DocumentFontSize(16, false));
						TEST_ASSERT(s0t->bold);
						TEST_ASSERT(s0t->bold.Value() == true);
						TEST_ASSERT(s0t->italic);
						TEST_ASSERT(s0t->italic.Value() == false);
						TEST_ASSERT(s0t->underline);
						TEST_ASSERT(s0t->underline.Value() == true);
						TEST_ASSERT(s0t->strikeline);
						TEST_ASSERT(s0t->strikeline.Value() == false);
						TEST_ASSERT(s0t->color);
						TEST_ASSERT(s0t->color.Value() == Color(255, 0, 0));
					}
					{
						auto s1 = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(s1->face);
						TEST_ASSERT(s1->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s1->size);
						TEST_ASSERT(s1->size.Value() == DocumentFontSize(16, false));
						TEST_ASSERT(s1->bold);
						TEST_ASSERT(s1->bold.Value() == true);
						TEST_ASSERT(s1->italic);
						TEST_ASSERT(s1->italic.Value() == false);
						TEST_ASSERT(s1->underline);
						TEST_ASSERT(s1->underline.Value() == true);
						TEST_ASSERT(s1->strikeline);
						TEST_ASSERT(s1->strikeline.Value() == false);
						TEST_ASSERT(s1->color);
						TEST_ASSERT(s1->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2h = Summarize(textBox, 2, 0, 5);
						TEST_ASSERT(s2h->face);
						TEST_ASSERT(s2h->face.Value() == WString::Unmanaged(L"Arial"));
						TEST_ASSERT(s2h->size);
						TEST_ASSERT(s2h->size.Value() == DocumentFontSize(16, false));
						TEST_ASSERT(s2h->bold);
						TEST_ASSERT(s2h->bold.Value() == true);
						TEST_ASSERT(s2h->italic);
						TEST_ASSERT(s2h->italic.Value() == false);
						TEST_ASSERT(s2h->underline);
						TEST_ASSERT(s2h->underline.Value() == true);
						TEST_ASSERT(s2h->strikeline);
						TEST_ASSERT(s2h->strikeline.Value() == false);
						TEST_ASSERT(s2h->color);
						TEST_ASSERT(s2h->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2t = Summarize(textBox, 2, 5, 10);
						TEST_ASSERT(s2t->face == baseS2tFace);
						TEST_ASSERT(s2t->size == baseS2tSize);
						TEST_ASSERT(s2t->bold == baseS2tBold);
						TEST_ASSERT(s2t->italic == baseS2tItalic);
						TEST_ASSERT(s2t->underline == baseS2tUnderline);
						TEST_ASSERT(s2t->strikeline == baseS2tStrikeline);
						TEST_ASSERT(s2t->color == baseS2tColor);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_AllProperties_OneCall"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_SameProperty_FontFace")
		{
			Nullable<WString> baseS2tFace;

			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					{
						auto s2t = Summarize(textBox, 2, 9, 10);
						baseS2tFace = s2t->face;
					}

					textBox->EditStyle(TextPos(0, 1), TextPos(2, 6), MakeFontFaceStyle(WString::Unmanaged(L"FontA")));
					textBox->EditStyle(TextPos(1, 4), TextPos(2, 9), MakeFontFaceStyle(WString::Unmanaged(L"FontB")));
				});

				protocol->OnNextIdleFrame(L"Verify", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0 = Summarize(textBox, 0, 1, 10);
						TEST_ASSERT(s0->face);
						TEST_ASSERT(s0->face.Value() == WString::Unmanaged(L"FontA"));
					}
					{
						auto s1a = Summarize(textBox, 1, 0, 4);
						TEST_ASSERT(s1a->face);
						TEST_ASSERT(s1a->face.Value() == WString::Unmanaged(L"FontA"));
					}
					{
						auto s1b = Summarize(textBox, 1, 4, 10);
						TEST_ASSERT(s1b->face);
						TEST_ASSERT(s1b->face.Value() == WString::Unmanaged(L"FontB"));
					}
					{
						auto s1m = Summarize(textBox, 1, 0, 10);
						TEST_ASSERT(!s1m->face);
					}
					{
						auto s2 = Summarize(textBox, 2, 0, 9);
						TEST_ASSERT(s2->face);
						TEST_ASSERT(s2->face.Value() == WString::Unmanaged(L"FontB"));
					}
					{
						auto s2t = Summarize(textBox, 2, 9, 10);
						TEST_ASSERT(s2t->face == baseS2tFace);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Overlap_SameProperty_FontFace"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_DifferentProperties_BoldThenColor")
		{
			Nullable<Color> baseS1aColor;
			Nullable<bool> baseS2bBold;

			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					{
						auto s1a = Summarize(textBox, 1, 0, 4);
						baseS1aColor = s1a->color;
					}
					{
						auto s2b = Summarize(textBox, 2, 6, 9);
						baseS2bBold = s2b->bold;
					}

					textBox->EditStyle(TextPos(0, 1), TextPos(2, 6), MakeTextStyle(true, false, false, false));
					textBox->EditStyle(TextPos(1, 4), TextPos(2, 9), MakeColorStyle(Color(255, 0, 0)));
				});

				protocol->OnNextIdleFrame(L"Verify", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s1a = Summarize(textBox, 1, 0, 4);
						TEST_ASSERT(s1a->bold);
						TEST_ASSERT(s1a->bold.Value() == true);
						TEST_ASSERT(s1a->color == baseS1aColor);
					}
					{
						auto s1b = Summarize(textBox, 1, 4, 10);
						TEST_ASSERT(s1b->bold);
						TEST_ASSERT(s1b->bold.Value() == true);
						TEST_ASSERT(s1b->color);
						TEST_ASSERT(s1b->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2a = Summarize(textBox, 2, 0, 6);
						TEST_ASSERT(s2a->bold);
						TEST_ASSERT(s2a->bold.Value() == true);
						TEST_ASSERT(s2a->color);
						TEST_ASSERT(s2a->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2b = Summarize(textBox, 2, 6, 9);
						TEST_ASSERT(s2b->bold == baseS2bBold);
						TEST_ASSERT(s2b->color);
						TEST_ASSERT(s2b->color.Value() == Color(255, 0, 0));
					}
					{
						auto s2m = Summarize(textBox, 2, 0, 10);
						TEST_ASSERT(!s2m->bold);
						TEST_ASSERT(!s2m->color);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Overlap_DifferentProperties_BoldThenColor"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});

		TEST_CASE(L"Overlap_SameProperty_BoldTrueThenFalse")
		{
			Nullable<bool> baseS2tBold;

			TooltipTimer timer;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Init", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
					textBox->SetFocused();
					textBox->LoadTextAndClearUndoRedo(L"0123456789\r\n\r\nabcdefghij\r\n\r\nKLMNOPQRST");

					{
						auto s2t = Summarize(textBox, 2, 9, 10);
						baseS2tBold = s2t->bold;
					}

					textBox->EditStyle(TextPos(0, 1), TextPos(2, 9), MakeTextStyle(true, false, false, false));
					textBox->EditStyle(TextPos(1, 4), TextPos(2, 6), MakeTextStyle(false, false, false, false));
				});

				protocol->OnNextIdleFrame(L"Verify", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");

					{
						auto s0 = Summarize(textBox, 0, 1, 10);
						TEST_ASSERT(s0->bold);
						TEST_ASSERT(s0->bold.Value() == true);
					}
					{
						auto s1a = Summarize(textBox, 1, 0, 4);
						TEST_ASSERT(s1a->bold);
						TEST_ASSERT(s1a->bold.Value() == true);
					}
					{
						auto s1b = Summarize(textBox, 1, 4, 10);
						TEST_ASSERT(s1b->bold);
						TEST_ASSERT(s1b->bold.Value() == false);
					}
					{
						auto s2a = Summarize(textBox, 2, 0, 6);
						TEST_ASSERT(s2a->bold);
						TEST_ASSERT(s2a->bold.Value() == false);
					}
					{
						auto s2b = Summarize(textBox, 2, 6, 9);
						TEST_ASSERT(s2b->bold);
						TEST_ASSERT(s2b->bold.Value() == true);
					}
					{
						auto s2t = Summarize(textBox, 2, 9, 10);
						TEST_ASSERT(s2t->bold == baseS2tBold);
					}

					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Controls/Editor/Features/RichText/MultiParagraph_Overlap_SameProperty_BoldTrueThenFalse"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resource_DocumentLabel
			);
		});
	});
	});
}
