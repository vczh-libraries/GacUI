#include "TestControls.h"

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
	});
}
