#include "../../../Source/UnitTestUtilities/GuiUnitTestProtocol.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h"

using namespace vl;
using namespace vl::unittest;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::remoteprotocol;

namespace remote_document_paragrpah_tests
{
	DocumentTextRunProperty CreateTextProp(unsigned char colorValue)
	{
		DocumentTextRunProperty prop;
		prop.textColor = Color(colorValue, colorValue, colorValue);
		prop.backgroundColor = Color(255 - colorValue, 255 - colorValue, 255 - colorValue);
		prop.fontProperties.fontFamily = L"TestFont";
		prop.fontProperties.size = 12;
		prop.fontProperties.bold = false;
		prop.fontProperties.italic = false;
		prop.fontProperties.underline = false;
		prop.fontProperties.strikeline = false;
		prop.fontProperties.antialias = true;
		prop.fontProperties.verticalAntialias = true;
		return prop;
	}

	DocumentInlineObjectRunProperty CreateInlineProp(vint callbackId, vint width = 10)
	{
		DocumentInlineObjectRunProperty prop;
		prop.size = Size(width, 10);
		prop.baseline = 8;
		prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Alone;
		prop.backgroundElementId = -1;
		prop.callbackId = callbackId;
		return prop;
	}

	WString FormatRunProperty(const DocumentTextRunProperty& prop)
	{
		return L"Text(color:" + 
			   itow(prop.textColor.r) + L"," + 
			   itow(prop.textColor.g) + L"," + 
			   itow(prop.textColor.b) + 
			   L", bg:" + 
			   itow(prop.backgroundColor.r) + L"," + 
			   itow(prop.backgroundColor.g) + L"," + 
			   itow(prop.backgroundColor.b) + L")";
	}

	WString FormatRunProperty(const DocumentInlineObjectRunProperty& prop)
	{
		return L"Inline(callback:" + 
			   itow(prop.callbackId) + 
			   L", size:" + itow(prop.size.x) + L"x" + itow(prop.size.y) + L")";
	}

	bool CompareRunProperty(
		const DocumentTextRunProperty& a,
		const DocumentTextRunProperty& b)
	{
		return a.textColor == b.textColor &&
			   a.backgroundColor == b.backgroundColor &&
			   a.fontProperties.fontFamily == b.fontProperties.fontFamily &&
			   a.fontProperties.size == b.fontProperties.size &&
			   a.fontProperties.bold == b.fontProperties.bold &&
			   a.fontProperties.italic == b.fontProperties.italic &&
			   a.fontProperties.underline == b.fontProperties.underline &&
			   a.fontProperties.strikeline == b.fontProperties.strikeline &&
			   a.fontProperties.antialias == b.fontProperties.antialias &&
			   a.fontProperties.verticalAntialias == b.fontProperties.verticalAntialias;
	}

	bool CompareRunProperty(
		const DocumentInlineObjectRunProperty& a,
		const DocumentInlineObjectRunProperty& b)
	{
		return a.size == b.size &&
			   a.baseline == b.baseline &&
			   a.breakCondition == b.breakCondition &&
			   a.backgroundElementId == b.backgroundElementId &&
			   a.callbackId == b.callbackId;
	}

	template<typename T>
	void PrintMap(
		const Dictionary<CaretRange, T>& actualMap,
		const Dictionary<CaretRange, T>& expectedMap)
	{
		TEST_PRINT(L"Actual: (" + itow(actualMap.Count()) + L")");
		for (vint i = 0; i < actualMap.Count(); i++)
		{
			auto key = actualMap.Keys()[i];
			auto value = actualMap.Values()[i];
			TEST_PRINT(L"  [" + itow(key.caretBegin) + L"," + itow(key.caretEnd) + L"]: " + FormatRunProperty(value));
		}
		
		TEST_PRINT(L"Expected: (" + itow(expectedMap.Count()) + L")");
		for (vint i = 0; i < expectedMap.Count(); i++)
		{
			auto key = expectedMap.Keys()[i];
			auto value = expectedMap.Values()[i];
			TEST_PRINT(L"  [" + itow(key.caretBegin) + L"," + itow(key.caretEnd) + L"]: " + FormatRunProperty(value));
		}
	}

	template<typename T>
	void AssertMap(
		const Dictionary<CaretRange, T>& actualMap,
		const Dictionary<CaretRange, T>& expectedMap)
	{
		bool matches = (actualMap.Count() == expectedMap.Count());
		
		if (matches)
		{
			for (vint i = 0; i < expectedMap.Count(); i++)
			{
				auto expectedKey = expectedMap.Keys()[i];
				auto expectedValue = expectedMap.Values()[i];
				
				// Check if key exists in actual map
				vint actualIndex = actualMap.Keys().IndexOf(expectedKey);
				if (actualIndex == -1)
				{
					matches = false;
					break;
				}
				
				// Check if value matches
				auto actualValue = actualMap.Values()[actualIndex];
				if (!CompareRunProperty(actualValue, expectedValue))
				{
					matches = false;
					break;
				}
			}
		}
		
		if (!matches)
		{
			PrintMap(actualMap, expectedMap);
		}
		TEST_ASSERT(matches);
	}
}
using namespace remote_document_paragrpah_tests;

TEST_FILE
{
	TEST_CATEGORY(L"CaretRange")
	{
		TEST_CASE(L"Equal ranges")
		{
			CaretRange r1{.caretBegin = 10, .caretEnd = 20};
			CaretRange r2{.caretBegin = 10, .caretEnd = 20};
			TEST_ASSERT(r1 == r2);
			TEST_ASSERT(!(r1 < r2));
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Different caretBegin")
		{
			CaretRange r1{.caretBegin = 10, .caretEnd = 20};
			CaretRange r2{.caretBegin = 15, .caretEnd = 20};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Same caretBegin, different caretEnd")
		{
			CaretRange r1{.caretBegin = 10, .caretEnd = 20};
			CaretRange r2{.caretBegin = 10, .caretEnd = 25};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Adjacent ranges - non-overlapping")
		{
			CaretRange r1{.caretBegin = 10, .caretEnd = 20};
			CaretRange r2{.caretBegin = 20, .caretEnd = 30};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
		});

		TEST_CASE(L"Overlapping ranges")
		{
			CaretRange r1{.caretBegin = 10, .caretEnd = 25};
			CaretRange r2{.caretBegin = 20, .caretEnd = 30};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
		});

		TEST_CASE(L"Contained range")
		{
			CaretRange r1{.caretBegin = 10, .caretEnd = 30};
			CaretRange r2{.caretBegin = 15, .caretEnd = 25};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);  // r1.caretBegin < r2.caretBegin
		});
	});

	TEST_CATEGORY(L"AddTextRun")
	{
		TEST_CASE(L"Add to empty map")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Add non-overlapping runs")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Adjacent runs with different properties - no merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Adjacent runs with identical properties - merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 20}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Complete overlap - replacement")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from left")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 30}, prop1);
			AddTextRun(textMap, {.caretBegin = 5, .caretEnd = 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 5, .caretEnd = 20}, prop2);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from right")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 30}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 35}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 35}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"New run completely contains old run")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 15, .caretEnd = 25}, prop1);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"New run contained within old run - split into 3")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 40}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop2);
			expectedMap.Add({.caretBegin = 30, .caretEnd = 40}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Overlap multiple consecutive runs")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(120);
			auto prop3 = CreateTextProp(140);
			auto propNew = CreateTextProp(200);
			
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
			AddTextRun(textMap, {.caretBegin = 30, .caretEnd = 40}, prop3);
			AddTextRun(textMap, {.caretBegin = 15, .caretEnd = 35}, propNew);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 15}, prop1);
			expectedMap.Add({.caretBegin = 15, .caretEnd = 35}, propNew);
			expectedMap.Add({.caretBegin = 35, .caretEnd = 40}, prop3);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Insert between two identical runs - all three merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop1);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge left neighbor only")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 20}, prop1);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge right neighbor only")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop2);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop1);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop2);
			expectedMap.Add({.caretBegin = 10, .caretEnd = 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"No merge when neighbors differ")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			auto prop3 = CreateTextProp(200);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop3);
			AddTextRun(textMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop3);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge after splitting creates merged regions")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 50}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop1);  // Replace prop2 with prop1
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 50}, prop1);  // All merged back
			
			AssertMap(textMap, expectedMap);
		});
	});

	TEST_CATEGORY(L"AddInlineObjectRun")
	{
		// Success cases
		TEST_CASE(L"Add to empty map")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			
			TEST_ASSERT(result == true);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"Add non-overlapping objects with gap")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
			
			TEST_ASSERT(result == true);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			expectedMap.Add({.caretBegin = 30, .caretEnd = 40}, prop2);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"Adjacent objects - boundary test")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			
			TEST_ASSERT(result == true);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"Multiple non-overlapping objects")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			auto prop3 = CreateInlineProp(300);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 50, .caretEnd = 60}, prop3);
			
			TEST_ASSERT(result == true);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 30, .caretEnd = 40}, prop2);
			expectedMap.Add({.caretBegin = 50, .caretEnd = 60}, prop3);
			
			AssertMap(inlineMap, expectedMap);
		});

		// Failure cases
		TEST_CASE(L"Complete overlap - exact match")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			
			TEST_ASSERT(result == false);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from left")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop1);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop2);
			
			TEST_ASSERT(result == false);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from right")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop2);
			
			TEST_ASSERT(result == false);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"New contains existing")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop1);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30}, prop2);
			
			TEST_ASSERT(result == false);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 15, .caretEnd = 25}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"New contained within existing")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30}, prop1);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop2);
			
			TEST_ASSERT(result == false);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 30}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});

		// Edge cases
		TEST_CASE(L"Gap fitting with multiple objects")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			auto prop3 = CreateInlineProp(300);
			auto prop4 = CreateInlineProp(400);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
			AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 50}, prop3);
			bool result = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop4);
			
			TEST_ASSERT(result == true);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop4);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop2);
			expectedMap.Add({.caretBegin = 40, .caretEnd = 50}, prop3);
			
			AssertMap(inlineMap, expectedMap);
		});

		TEST_CASE(L"Adjacent chain")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			auto prop3 = CreateInlineProp(300);
			
			bool result1 = AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			bool result2 = AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			bool result3 = AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop3);
			
			TEST_ASSERT(result1 == true);
			TEST_ASSERT(result2 == true);
			TEST_ASSERT(result3 == true);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop3);
			
			AssertMap(inlineMap, expectedMap);
		});
	});
}
