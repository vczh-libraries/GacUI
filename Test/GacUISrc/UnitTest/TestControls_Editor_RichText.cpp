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

	Ptr<DocumentStyleProperties> Summarize(GuiDocumentLabel* textBox, vint row, vint begin, vint end)
	{
		return textBox->SummarizeStyle(TextPos(row, begin), TextPos(row, end));
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
