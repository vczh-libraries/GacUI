#include "../../../Source/UnitTestUtilities/GuiUnitTestProtocol.h"

using namespace vl;
using namespace vl::unittest;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::remoteprotocol;

namespace remote_document_paragrpah_tests
{
	DocumentTextRunProperty CreateTextProp(vint colorValue)
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
		prop.breakCondition = IGuiGraphicsParagraph::BreakCondition::Follow;
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
			CaretRange r1{10, 20};
			CaretRange r2{10, 20};
			TEST_ASSERT(r1 == r2);
			TEST_ASSERT(!(r1 < r2));
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Different caretBegin")
		{
			CaretRange r1{10, 20};
			CaretRange r2{15, 20};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Same caretBegin, different caretEnd")
		{
			CaretRange r1{10, 20};
			CaretRange r2{10, 25};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
			TEST_ASSERT(!(r2 < r1));
		});

		TEST_CASE(L"Adjacent ranges - non-overlapping")
		{
			CaretRange r1{10, 20};
			CaretRange r2{20, 30};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
		});

		TEST_CASE(L"Overlapping ranges")
		{
			CaretRange r1{10, 25};
			CaretRange r2{20, 30};
			TEST_ASSERT(r1 != r2);
			TEST_ASSERT(r1 < r2);
		});

		TEST_CASE(L"Contained range")
		{
			CaretRange r1{10, 30};
			CaretRange r2{15, 25};
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
			
			AddTextRun(textMap, {0, 10}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Add non-overlapping runs")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			expectedMap.Add({20, 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Adjacent runs with different properties - no merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {10, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			expectedMap.Add({10, 20}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Adjacent runs with identical properties - merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 20}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Complete overlap - replacement")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 20}, prop1);
			AddTextRun(textMap, {10, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 20}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from left")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 30}, prop1);
			AddTextRun(textMap, {5, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({5, 20}, prop2);
			expectedMap.Add({20, 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Partial overlap from right")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 30}, prop1);
			AddTextRun(textMap, {20, 35}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 20}, prop1);
			expectedMap.Add({20, 35}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"New run completely contains old run")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {15, 25}, prop1);
			AddTextRun(textMap, {10, 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"New run contained within old run - split into 3")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {10, 40}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 20}, prop1);
			expectedMap.Add({20, 30}, prop2);
			expectedMap.Add({30, 40}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Overlap multiple consecutive runs")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(120);
			auto prop3 = CreateTextProp(140);
			auto propNew = CreateTextProp(200);
			
			AddTextRun(textMap, {10, 20}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			AddTextRun(textMap, {30, 40}, prop3);
			AddTextRun(textMap, {15, 35}, propNew);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({10, 15}, prop1);
			expectedMap.Add({15, 35}, propNew);
			expectedMap.Add({35, 40}, prop3);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Insert between two identical runs - all three merge")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop1);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge left neighbor only")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 20}, prop1);
			expectedMap.Add({20, 30}, prop2);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge right neighbor only")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 10}, prop2);
			AddTextRun(textMap, {20, 30}, prop1);
			AddTextRun(textMap, {10, 20}, prop1);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop2);
			expectedMap.Add({10, 30}, prop1);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"No merge when neighbors differ")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			auto prop3 = CreateTextProp(200);
			
			AddTextRun(textMap, {0, 10}, prop1);
			AddTextRun(textMap, {20, 30}, prop3);
			AddTextRun(textMap, {10, 20}, prop2);
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 10}, prop1);
			expectedMap.Add({10, 20}, prop2);
			expectedMap.Add({20, 30}, prop3);
			
			AssertMap(textMap, expectedMap);
		});

		TEST_CASE(L"Merge after splitting creates merged regions")
		{
			Dictionary<CaretRange, DocumentTextRunProperty> textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {0, 50}, prop1);
			AddTextRun(textMap, {20, 30}, prop2);
			AddTextRun(textMap, {20, 30}, prop1);  // Replace prop2 with prop1
			
			Dictionary<CaretRange, DocumentTextRunProperty> expectedMap;
			expectedMap.Add({0, 50}, prop1);  // All merged back
			
			AssertMap(textMap, expectedMap);
		});
	});
}
