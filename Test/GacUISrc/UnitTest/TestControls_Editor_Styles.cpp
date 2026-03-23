#include "TestControls.h"

using namespace gacui_unittest_template;

namespace
{
	Ptr<DocumentStyleProperties> MakeStyleWithBold(bool bold)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->bold = bold;
		return style;
	}

	Ptr<DocumentStyleProperties> MakeStyleWithItalic(bool italic)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->italic = italic;
		return style;
	}

	Ptr<DocumentStyleProperties> MakeStyleWithColor(Color color)
	{
		auto style = Ptr(new DocumentStyleProperties);
		style->color = color;
		return style;
	}

	void RegisterStyle(GuiDocumentCommonInterface* textBox, const WString& styleName, const WString& parentStyleName, Ptr<DocumentStyleProperties> properties)
	{
		auto document = textBox->GetDocument();
		auto style = Ptr(new DocumentStyle);
		style->parentStyleName = parentStyleName;
		style->styles = properties;
		document->styles.Add(styleName, style);
	}

	Nullable<WString> SummarizeName(GuiDocumentCommonInterface* textBox, vint begin, vint end)
	{
		return textBox->SummarizeStyleName(TextPos(0, begin), TextPos(0, end));
	}

	Nullable<WString> SummarizeName(GuiDocumentCommonInterface* textBox, vint row, vint begin, vint end)
	{
		return textBox->SummarizeStyleName(TextPos(row, begin), TextPos(row, end));
	}
}

TEST_FILE
{
	const auto resource_DocumentViewer = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiDocumentViewer Style Test" ClientSize="x:480 y:320">
        <DocumentViewer ref.Name="textBox" EditMode="Editable">
          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
        </DocumentViewer>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Styles")
	{
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

			TEST_CATEGORY(L"MultiParagraph")
			{
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

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 5), TextPos(2, 5), WString::Unmanaged(L"MyBold"));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							// Paragraph 0: [0,5) unstyled, [5,10) styled
							TEST_ASSERT(!SummarizeName(textBox, 0, 0, 5));
							auto p0 = SummarizeName(textBox, 0, 5, 10);
							TEST_ASSERT(p0);
							TEST_ASSERT(p0.Value() == WString::Unmanaged(L"MyBold"));

							// Paragraph 1: fully styled
							auto p1 = SummarizeName(textBox, 1, 0, 10);
							TEST_ASSERT(p1);
							TEST_ASSERT(p1.Value() == WString::Unmanaged(L"MyBold"));

							// Paragraph 2: [0,5) styled, [5,10) unstyled
							auto p2h = SummarizeName(textBox, 2, 0, 5);
							TEST_ASSERT(p2h);
							TEST_ASSERT(p2h.Value() == WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(!SummarizeName(textBox, 2, 5, 10));

							// Cross-paragraph summarization sanity checks
							auto mid = textBox->SummarizeStyleName(TextPos(0, 5), TextPos(2, 5));
							TEST_ASSERT(mid);
							TEST_ASSERT(mid.Value() == WString::Unmanaged(L"MyBold"));
							TEST_ASSERT(!textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10)));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/MultiParagraph_AcrossParagraphs"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});

				TEST_CASE(L"FullDocument")
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

							RegisterStyle(textBox, WString::Unmanaged(L"MyBold"), WString::Empty, MakeStyleWithBold(true));
							textBox->EditStyleName(TextPos(0, 0), TextPos(2, 10), WString::Unmanaged(L"MyBold"));
						});

						protocol->OnNextIdleFrame(L"Verify", [=]()
						{
							auto window = GetApplication()->GetMainWindow();
							auto textBox = FindObjectByName<GuiDocumentViewer>(window, L"textBox");

							for (vint row = 0; row < 3; row++)
							{
								auto s = SummarizeName(textBox, row, 0, 10);
								TEST_ASSERT(s);
								TEST_ASSERT(s.Value() == WString::Unmanaged(L"MyBold"));
							}

							auto all = textBox->SummarizeStyleName(TextPos(0, 0), TextPos(2, 10));
							TEST_ASSERT(all);
							TEST_ASSERT(all.Value() == WString::Unmanaged(L"MyBold"));

							window->Hide();
						});
					});
					GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
						WString::Unmanaged(L"Controls/Editor/Features/Styles/EditStyleName/MultiParagraph_FullDocument"),
						WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
						resource_DocumentViewer
					);
				});
			});
		});
	});
}
