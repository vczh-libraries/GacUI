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

	WString FormatRunProperty(const DocumentRunProperty& prop)
	{
		if (auto textProp = prop.TryGet<DocumentTextRunProperty>())
		{
			return FormatRunProperty(*textProp);
		}
		else
		{
			auto inlineProp = prop.Get<DocumentInlineObjectRunProperty>();
			return FormatRunProperty(inlineProp);
		}
	}

	bool CompareRunProperty(
		const DocumentRunProperty& a,
		const DocumentRunProperty& b)
	{
		if (a.Index() != b.Index())
			return false;
		
		if (auto textA = a.TryGet<DocumentTextRunProperty>())
		{
			auto textB = b.Get<DocumentTextRunProperty>();
			return CompareRunProperty(*textA, textB);
		}
		else
		{
			auto inlineA = a.Get<DocumentInlineObjectRunProperty>();
			auto inlineB = b.Get<DocumentInlineObjectRunProperty>();
			return CompareRunProperty(inlineA, inlineB);
		}
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
	// Format a remoteprotocol::DocumentRun for debugging
	WString FormatDocumentRun(const remoteprotocol::DocumentRun& run)
	{
		return L"[" + itow(run.caretBegin) + L"," + itow(run.caretEnd) + L") -> " + FormatRunProperty(run.props);
	}

	// Print List of DocumentRun for debugging
	WString PrintDiffList(const Ptr<List<remoteprotocol::DocumentRun>>& diff)
	{
		if (!diff || diff->Count() == 0)
		{
			return L"(empty)";
		}
		
		WString result;
		for (vint i = 0; i < diff->Count(); i++)
		{
			result += L"  " + FormatDocumentRun(diff->Get(i)) + L"\r\n";
		}
		return result;
	}

	// Compare two remoteprotocol::DocumentRun for equality
	bool CompareDocumentRun(const remoteprotocol::DocumentRun& a, const remoteprotocol::DocumentRun& b)
	{
		return a.caretBegin == b.caretBegin &&
		       a.caretEnd == b.caretEnd &&
		       CompareRunProperty(a.props, b.props);
	}

	// Assert that two diff lists are equal
	void AssertDiffList(
		const Ptr<List<remoteprotocol::DocumentRun>>& actual,
		const Ptr<List<remoteprotocol::DocumentRun>>& expected,
		const wchar_t* message
	)
	{
		vint actualCount = actual ? actual->Count() : 0;
		vint expectedCount = expected ? expected->Count() : 0;
		
		if (actualCount != expectedCount)
		{
			auto msg = WString::Unmanaged(message) + L"\r\nExpected count: " + itow(expectedCount)
			         + L", Actual count: " + itow(actualCount)
			         + L"\r\nExpected:\r\n" + PrintDiffList(expected)
			         + L"Actual:\r\n" + PrintDiffList(actual);
			UnitTest::PrintMessage(msg, UnitTest::MessageKind::Error);
			TEST_ASSERT(false);
		}
		
		for (vint i = 0; i < expectedCount; i++)
		{
			auto&& e = expected->Get(i);
			auto&& a = actual->Get(i);
			
			if (!CompareDocumentRun(a, e))
			{
				auto msg = WString::Unmanaged(message) + L"\r\nDiff at index " + itow(i)
				         + L"\r\nExpected: " + FormatDocumentRun(e)
				         + L"\r\nActual: " + FormatDocumentRun(a)
				         + L"\r\nFull Expected:\r\n" + PrintDiffList(expected)
				         + L"Full Actual:\r\n" + PrintDiffList(actual);
				UnitTest::PrintMessage(msg, UnitTest::MessageKind::Error);
				TEST_ASSERT(false);
			}
		}
	}

	// Print List of callback IDs for debugging
	WString PrintCallbackIdList(const Ptr<List<vint>>& ids)
	{
		if (!ids || ids->Count() == 0)
		{
			return L"(empty)";
		}
		
		WString result = L"  ";
		for (vint i = 0; i < ids->Count(); i++)
		{
			if (i > 0) result += L", ";
			result += itow(ids->Get(i));
		}
		return result + L"\r\n";
	}

	// Assert that two callback ID lists are equal
	void AssertCallbackIdList(
		const Ptr<List<vint>>& actual,
		const Ptr<List<vint>>& expected,
		const wchar_t* message
	)
	{
		vint actualCount = actual ? actual->Count() : 0;
		vint expectedCount = expected ? expected->Count() : 0;
		
		if (actualCount != expectedCount)
		{
			auto msg = WString::Unmanaged(message) + L"\r\nExpected count: " + itow(expectedCount)
			         + L", Actual count: " + itow(actualCount)
			         + L"\r\nExpected: " + PrintCallbackIdList(expected)
			         + L"Actual: " + PrintCallbackIdList(actual);
			UnitTest::PrintMessage(msg, UnitTest::MessageKind::Error);
			TEST_ASSERT(false);
		}
		
		for (vint i = 0; i < expectedCount; i++)
		{
			if (expected->Get(i) != actual->Get(i))
			{
				auto msg = WString::Unmanaged(message) + L"\r\nDiff at index " + itow(i)
				         + L": Expected " + itow(expected->Get(i)) + L", Actual " + itow(actual->Get(i))
				         + L"\r\nExpected: " + PrintCallbackIdList(expected)
				         + L"Actual: " + PrintCallbackIdList(actual);
				UnitTest::PrintMessage(msg, UnitTest::MessageKind::Error);
				TEST_ASSERT(false);
			}
		}
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
	
	TEST_CATEGORY(L"ResetInlineObjectRun")
	{
		// Success cases - exact match removal
		TEST_CASE(L"Remove from single object map")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20});
			
			TEST_ASSERT(result == true);
			TEST_ASSERT(inlineMap.Count() == 0);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove from map with multiple objects - remove middle")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			auto prop3 = CreateInlineProp(300);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
			AddInlineObjectRun(inlineMap, {.caretBegin = 50, .caretEnd = 60}, prop3);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40});
			
			TEST_ASSERT(result == true);
			TEST_ASSERT(inlineMap.Count() == 2);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			expectedMap.Add({.caretBegin = 50, .caretEnd = 60}, prop3);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove from map with multiple objects - remove first")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			auto prop3 = CreateInlineProp(300);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop3);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10});
			
			TEST_ASSERT(result == true);
			TEST_ASSERT(inlineMap.Count() == 2);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop2);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop3);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove from map with multiple objects - remove last")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			auto prop3 = CreateInlineProp(300);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
			AddInlineObjectRun(inlineMap, {.caretBegin = 50, .caretEnd = 60}, prop3);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 50, .caretEnd = 60});
			
			TEST_ASSERT(result == true);
			TEST_ASSERT(inlineMap.Count() == 2);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			expectedMap.Add({.caretBegin = 30, .caretEnd = 40}, prop2);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		// Failure cases - no exact match
		TEST_CASE(L"Remove from empty map")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 0);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with non-existent range")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with partial overlap from left")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with partial overlap from right")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with range that contains existing object")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 15, .caretEnd = 25}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with range contained within existing object")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 25});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 30}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with adjacent range on left side")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with adjacent range on right side")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with same begin, different end")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 25});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Remove with different begin, same end")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 5, .caretEnd = 20});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		// Edge cases
		TEST_CASE(L"Multiple removals in sequence")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			auto prop3 = CreateInlineProp(300);
			auto prop4 = CreateInlineProp(400);
			auto prop5 = CreateInlineProp(500);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop2);
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, prop3);
			AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop4);
			AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 50}, prop5);
			
			TEST_ASSERT(inlineMap.Count() == 5);
			
			// Remove second object
			bool result1 = ResetInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20});
			TEST_ASSERT(result1 == true);
			TEST_ASSERT(inlineMap.Count() == 4);
			
			// Remove fourth object (was originally at position 3, now at position 2)
			bool result2 = ResetInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40});
			TEST_ASSERT(result2 == true);
			TEST_ASSERT(inlineMap.Count() == 3);
			
			// Verify remaining objects are correct
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 0, .caretEnd = 10}, prop1);
			expectedMap.Add({.caretBegin = 20, .caretEnd = 30}, prop3);
			expectedMap.Add({.caretBegin = 40, .caretEnd = 50}, prop5);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Attempt to remove already-removed object")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
			
			// First removal succeeds
			bool result1 = ResetInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20});
			TEST_ASSERT(result1 == true);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			// Second removal of same object fails
			bool result2 = ResetInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20});
			TEST_ASSERT(result2 == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			// Map contains only the second object
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 30, .caretEnd = 40}, prop2);
			
			AssertMap(inlineMap, expectedMap);
		});
	
		TEST_CASE(L"Zero-length range removal")
		{
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			
			// Add normal object
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			
			// Try to remove with zero-length range (should fail - no exact match)
			bool result = ResetInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 15});
			
			TEST_ASSERT(result == false);
			TEST_ASSERT(inlineMap.Count() == 1);
			
			DocumentInlineObjectRunPropertyMap expectedMap;
			expectedMap.Add({.caretBegin = 10, .caretEnd = 20}, prop1);
			
			AssertMap(inlineMap, expectedMap);
		});
	});
	
	TEST_CATEGORY(L"MergeRuns")
	{
		TEST_CASE(L"Empty maps")
		{
			DocumentTextRunPropertyMap textMap;
			DocumentInlineObjectRunPropertyMap inlineMap;
			DocumentRunPropertyMap result;
			
			MergeRuns(textMap, inlineMap, result);
			
			TEST_ASSERT(result.Count() == 0);
		});
	
		TEST_CASE(L"Only text runs")
		{
			DocumentTextRunPropertyMap textMap;
			auto prop1 = CreateTextProp(100);
			auto prop2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, prop1);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, prop2);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			DocumentRunPropertyMap result;
			
			MergeRuns(textMap, inlineMap, result);
			
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(prop1));
			expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(prop2));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Only inline objects")
		{
			DocumentTextRunPropertyMap textMap;
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto prop1 = CreateInlineProp(100);
			auto prop2 = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, prop1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 30, .caretEnd = 40}, prop2);
			
			DocumentRunPropertyMap result;
			
			MergeRuns(textMap, inlineMap, result);
			
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(prop1));
			expectedResult.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(prop2));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Both types, no overlap")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp1 = CreateTextProp(100);
			auto textProp2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, textProp1);
			AddTextRun(textMap, {.caretBegin = 40, .caretEnd = 50}, textProp2);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp = CreateInlineProp(200);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, inlineProp);
			
			DocumentRunPropertyMap result;
			
			MergeRuns(textMap, inlineMap, result);
			
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp1));
			expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(inlineProp));
			expectedResult.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(textProp2));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Text completely overlaps inline - text removed")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp = CreateTextProp(100);
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 30}, textProp);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp = CreateInlineProp(200);
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, inlineProp);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			// Text [0,30] containing Inline [10,20] should result in:
			// Text [0,10], Inline [10,20], Text [20,30]
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp));
			expectedResult.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(inlineProp));
			expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(textProp));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Text partial overlap from left - text split")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp = CreateTextProp(100);
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 25}, textProp);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp = CreateInlineProp(200);
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, inlineProp);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			// Text [0,25] overlapping Inline [20,30] from left should result in:
			// Text [0,20], Inline [20,30]
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 20}, DocumentRunProperty(textProp));
			expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(inlineProp));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Text partial overlap from right - text split")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp = CreateTextProp(100);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 40}, textProp);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp = CreateInlineProp(200);
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 25}, inlineProp);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			// Text [20,40] overlapping Inline [10,25] from right should result in:
			// Inline [10,25], Text [25,40]
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 10, .caretEnd = 25}, DocumentRunProperty(inlineProp));
			expectedResult.Add({.caretBegin = 25, .caretEnd = 40}, DocumentRunProperty(textProp));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"One text run overlaps multiple inline objects - fragmentation")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp = CreateTextProp(100);
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 50}, textProp);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp1 = CreateInlineProp(201, 5);
			auto inlineProp2 = CreateInlineProp(202, 5);
			auto inlineProp3 = CreateInlineProp(203, 5);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 15}, inlineProp1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 25}, inlineProp2);
			AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 45}, inlineProp3);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			// Text [0,50] with three inline objects should fragment into:
			// Text [0,10], Inline [10,15], Text [15,20], Inline [20,25], 
			// Text [25,40], Inline [40,45], Text [45,50]
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp));
			expectedResult.Add({.caretBegin = 10, .caretEnd = 15}, DocumentRunProperty(inlineProp1));
			expectedResult.Add({.caretBegin = 15, .caretEnd = 20}, DocumentRunProperty(textProp));
			expectedResult.Add({.caretBegin = 20, .caretEnd = 25}, DocumentRunProperty(inlineProp2));
			expectedResult.Add({.caretBegin = 25, .caretEnd = 40}, DocumentRunProperty(textProp));
			expectedResult.Add({.caretBegin = 40, .caretEnd = 45}, DocumentRunProperty(inlineProp3));
			expectedResult.Add({.caretBegin = 45, .caretEnd = 50}, DocumentRunProperty(textProp));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Multiple text runs overlap one inline object")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp1 = CreateTextProp(100);
			auto textProp2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 15}, textProp1);
			AddTextRun(textMap, {.caretBegin = 25, .caretEnd = 40}, textProp2);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp = CreateInlineProp(200);
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 30}, inlineProp);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			// Text [0,15] and Text [25,40] with Inline [10,30] should result in:
			// Text [0,10], Inline [10,30], Text [30,40]
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp1));
			expectedResult.Add({.caretBegin = 10, .caretEnd = 30}, DocumentRunProperty(inlineProp));
			expectedResult.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(textProp2));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Interleaved runs with gaps")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp1 = CreateTextProp(100);
			auto textProp2 = CreateTextProp(150);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, textProp1);
			AddTextRun(textMap, {.caretBegin = 25, .caretEnd = 35}, textProp2);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp1 = CreateInlineProp(200);
			auto inlineProp2 = CreateInlineProp(250);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 15, .caretEnd = 20}, inlineProp1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 45}, inlineProp2);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			// All four runs should be preserved with gaps
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp1));
			expectedResult.Add({.caretBegin = 15, .caretEnd = 20}, DocumentRunProperty(inlineProp1));
			expectedResult.Add({.caretBegin = 25, .caretEnd = 35}, DocumentRunProperty(textProp2));
			expectedResult.Add({.caretBegin = 40, .caretEnd = 45}, DocumentRunProperty(inlineProp2));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Adjacent text runs separated by inline - no merge")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp = CreateTextProp(100);  // Same property
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 10}, textProp);
			AddTextRun(textMap, {.caretBegin = 20, .caretEnd = 30}, textProp);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp = CreateInlineProp(200);
			AddInlineObjectRun(inlineMap, {.caretBegin = 10, .caretEnd = 20}, inlineProp);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			// Despite identical properties, text runs should remain separate
			// because MergeRuns doesn't perform merging
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(textProp));
			expectedResult.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(inlineProp));
			expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(textProp));
			
			AssertMap(result, expectedResult);
		});
	
		TEST_CASE(L"Multiple simultaneous overlaps")
		{
			DocumentTextRunPropertyMap textMap;
			auto textProp1 = CreateTextProp(100);
			auto textProp2 = CreateTextProp(120);
			auto textProp3 = CreateTextProp(140);
			
			AddTextRun(textMap, {.caretBegin = 0, .caretEnd = 25}, textProp1);
			AddTextRun(textMap, {.caretBegin = 35, .caretEnd = 55}, textProp2);
			AddTextRun(textMap, {.caretBegin = 65, .caretEnd = 100}, textProp3);
			
			DocumentInlineObjectRunPropertyMap inlineMap;
			auto inlineProp1 = CreateInlineProp(201);
			auto inlineProp2 = CreateInlineProp(202);
			auto inlineProp3 = CreateInlineProp(203);
			auto inlineProp4 = CreateInlineProp(204);
			
			AddInlineObjectRun(inlineMap, {.caretBegin = 20, .caretEnd = 30}, inlineProp1);
			AddInlineObjectRun(inlineMap, {.caretBegin = 40, .caretEnd = 50}, inlineProp2);
			AddInlineObjectRun(inlineMap, {.caretBegin = 70, .caretEnd = 75}, inlineProp3);
			AddInlineObjectRun(inlineMap, {.caretBegin = 80, .caretEnd = 85}, inlineProp4);
			
			DocumentRunPropertyMap result;
			MergeRuns(textMap, inlineMap, result);
			
			DocumentRunPropertyMap expectedResult;
			expectedResult.Add({.caretBegin = 0, .caretEnd = 20}, DocumentRunProperty(textProp1));
			expectedResult.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(inlineProp1));
			expectedResult.Add({.caretBegin = 35, .caretEnd = 40}, DocumentRunProperty(textProp2));
			expectedResult.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(inlineProp2));
			expectedResult.Add({.caretBegin = 50, .caretEnd = 55}, DocumentRunProperty(textProp2));
			expectedResult.Add({.caretBegin = 65, .caretEnd = 70}, DocumentRunProperty(textProp3));
			expectedResult.Add({.caretBegin = 70, .caretEnd = 75}, DocumentRunProperty(inlineProp3));
			expectedResult.Add({.caretBegin = 75, .caretEnd = 80}, DocumentRunProperty(textProp3));
			expectedResult.Add({.caretBegin = 80, .caretEnd = 85}, DocumentRunProperty(inlineProp4));
			expectedResult.Add({.caretBegin = 85, .caretEnd = 100}, DocumentRunProperty(textProp3));
			
			AssertMap(result, expectedResult);
		});
	});
	
	TEST_CATEGORY(L"DiffRuns")
	{
		// Helper to create expected DocumentRun list
		auto MakeExpectedList = []() {
			return Ptr(new List<remoteprotocol::DocumentRun>());
		};
		
		// Helper to add expected DocumentRun
		auto AddExpectedRun = [](Ptr<List<remoteprotocol::DocumentRun>> list, vint begin, vint end, const DocumentRunProperty& prop) {
			remoteprotocol::DocumentRun run;
			run.caretBegin = begin;
			run.caretEnd = end;
			run.props = prop;
			list->Add(run);
		};
		
		// Helper to create expected callback ID list
		auto MakeExpectedIds = [](std::initializer_list<vint> ids) {
			auto list = Ptr(new List<vint>());
			for (auto id : ids)
			{
				list->Add(id);
			}
			return list;
		};
		
		TEST_CASE(L"Empty maps produce empty diff")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			
			DiffRuns(oldRuns, newRuns, desc);
			
			TEST_ASSERT(!desc.runsDiff || desc.runsDiff->Count() == 0);
			TEST_ASSERT(!desc.createdInlineObjects || desc.createdInlineObjects->Count() == 0);
			TEST_ASSERT(!desc.removedInlineObjects || desc.removedInlineObjects->Count() == 0);
		});
		
		TEST_CASE(L"Old map empty, new map has text runs")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			newRuns.Add({.caretBegin = 0, .caretEnd = 5}, DocumentRunProperty(CreateTextProp(0xFF)));
			newRuns.Add({.caretBegin = 5, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x00)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 5, DocumentRunProperty(CreateTextProp(0xFF)));
			AddExpectedRun(expected, 5, 10, DocumentRunProperty(CreateTextProp(0x00)));
			
			AssertDiffList(desc.runsDiff, expected, L"All new runs should be in diff");
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline objects created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline objects removed");
		});
		
		TEST_CASE(L"Old map empty, new map has inline objects")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			newRuns.Add({.caretBegin = 0, .caretEnd = 1}, DocumentRunProperty(CreateInlineProp(5, 100)));
			newRuns.Add({.caretBegin = 5, .caretEnd = 6}, DocumentRunProperty(CreateInlineProp(7, 50)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 1, DocumentRunProperty(CreateInlineProp(5, 100)));
			AddExpectedRun(expected, 5, 6, DocumentRunProperty(CreateInlineProp(7, 50)));
			
			AssertDiffList(desc.runsDiff, expected, L"All new inline objects in diff");
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({5, 7}), L"Inline objects created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline objects removed");
		});
	
		TEST_CASE(L"New map empty, old map has inline objects - ERROR")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			oldRuns.Add({.caretBegin = 0, .caretEnd = 1}, DocumentRunProperty(CreateInlineProp(5, 100)));
			oldRuns.Add({.caretBegin = 5, .caretEnd = 6}, DocumentRunProperty(CreateInlineProp(7, 50)));
			
			// This should trigger CHECK_ERROR because old runs are not covered by new runs
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
	
		TEST_CASE(L"Same key, same value - no diff entry")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			auto prop = DocumentRunProperty(CreateTextProp(0xFF));
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, prop);
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, prop);
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			TEST_ASSERT(!desc.runsDiff || desc.runsDiff->Count() == 0);
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline objects created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline objects removed");
		});		TEST_CASE(L"Same key, different value - entry in diff")
			{
				DocumentRunPropertyMap oldRuns, newRuns;
				oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0xFF)));
				newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x00)));
				
				remoteprotocol::ElementDesc_DocumentParagraph desc;
				DiffRuns(oldRuns, newRuns, desc);
				
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateTextProp(0x00)));
			
			AssertDiffList(desc.runsDiff, expected, L"Changed run should be in diff");
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline objects created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline objects removed");
		});
	});
	
	TEST_CATEGORY(L"DiffRuns (Complex)")
	{
		// Helper functions reused from DiffRuns category
		auto MakeExpectedList = []() {
			return Ptr(new List<remoteprotocol::DocumentRun>());
		};
		
		auto AddExpectedRun = [](Ptr<List<remoteprotocol::DocumentRun>> list, vint begin, vint end, const DocumentRunProperty& prop) {
			remoteprotocol::DocumentRun run;
			run.caretBegin = begin;
			run.caretEnd = end;
			run.props = prop;
			list->Add(run);
		};
		
		auto MakeExpectedIds = [](std::initializer_list<vint> ids) {
			auto list = Ptr(new List<vint>());
			for (auto id : ids)
			{
				list->Add(id);
			}
			return list;
		};
	
		// Category 1: Valid Text → Inline Object Transformations
		
		TEST_CASE(L"Text→Inline: Same range, type changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text run at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: inline object at same range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(1, 100)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Verify diff contains the new inline object
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateInlineProp(1, 100)));
			AssertDiffList(desc.runsDiff, expected, L"Text→Inline at same range");
			
			// Verify callback tracking
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({1}), L"Inline object created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Inline: Different cuts - 3 old runs → 2 new inlines (decreased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three text runs at [0, 10], [10, 20], [20, 30] (cut at 10, 20)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// New: two inline objects at [0, 15], [15, 30] (cut at 15) - different cuts, fewer runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 15}, DocumentRunProperty(CreateInlineProp(3, 150)));
			newRuns.Add({.caretBegin = 15, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(4, 150)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Both new inline objects in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 15, DocumentRunProperty(CreateInlineProp(3, 150)));
			AddExpectedRun(expected, 15, 30, DocumentRunProperty(CreateInlineProp(4, 150)));
			AssertDiffList(desc.runsDiff, expected, L"3 text runs → 2 inlines with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({3, 4}), L"Two inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Inline: Different cuts - 2 old runs → 3 new inlines (increased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two text runs at [0, 15], [15, 30] (cut at 15)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 15, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// New: three inline objects at [0, 10], [10, 20], [20, 30] (cut at 10, 20) - different cuts, more runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateInlineProp(5, 50)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(6, 60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(7, 70)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three inline objects in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateInlineProp(5, 50)));
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateInlineProp(6, 60)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateInlineProp(7, 70)));
			AssertDiffList(desc.runsDiff, expected, L"2 text runs → 3 inlines with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({5, 6, 7}), L"Three inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
	
		// Category 2: Valid Inline Object → Text Transformations
		
		TEST_CASE(L"Inline→Text: Same range, type changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: inline object at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(10, 100)));
			
			// New: text run at same range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Verify diff contains the new text run
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0xA0)));
			AssertDiffList(desc.runsDiff, expected, L"Inline→Text at same range");
			
			// Verify callback tracking: old inline removed
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({10}), L"Inline object removed");
		});
		
		TEST_CASE(L"Inline→Text: Different cuts - 3 old inlines → 2 new text runs (decreased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three inline objects at [0, 8], [8, 16], [16, 24] (cut at 8, 16)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 8}, DocumentRunProperty(CreateInlineProp(12, 20)));
			oldRuns.Add({.caretBegin = 8, .caretEnd = 16}, DocumentRunProperty(CreateInlineProp(13, 30)));
			oldRuns.Add({.caretBegin = 16, .caretEnd = 24}, DocumentRunProperty(CreateInlineProp(14, 40)));
			
			// New: two text runs at [0, 12], [12, 24] (cut at 12) - different cuts, fewer runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 12}, DocumentRunProperty(CreateTextProp(0xFF)));
			newRuns.Add({.caretBegin = 12, .caretEnd = 24}, DocumentRunProperty(CreateTextProp(0xCC)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Both new text runs in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 12, DocumentRunProperty(CreateTextProp(0xFF)));
			AddExpectedRun(expected, 12, 24, DocumentRunProperty(CreateTextProp(0xCC)));
			AssertDiffList(desc.runsDiff, expected, L"3 inlines → 2 text runs with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({12, 13, 14}), L"Three inlines removed");
		});
		
		TEST_CASE(L"Inline→Text: Different cuts - 2 old inlines → 3 new text runs (increased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two inline objects at [10, 25], [25, 40] (cut at 25)
			oldRuns.Add({.caretBegin = 10, .caretEnd = 25}, DocumentRunProperty(CreateInlineProp(15, 150)));
			oldRuns.Add({.caretBegin = 25, .caretEnd = 40}, DocumentRunProperty(CreateInlineProp(16, 150)));
			
			// New: three text runs at [10, 20], [20, 30], [30, 40] (cut at 20, 30) - different cuts, more runs
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x40)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			newRuns.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(CreateTextProp(0xC0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three text runs in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0x40)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0x80)));
			AddExpectedRun(expected, 30, 40, DocumentRunProperty(CreateTextProp(0xC0)));
			AssertDiffList(desc.runsDiff, expected, L"2 inlines → 3 text runs with different cuts");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({15, 16}), L"Two inlines removed");
		});
	
		// Category 3: Text → Text Property Changes
		
		TEST_CASE(L"Text→Text: Same range, property changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text with color 0x80
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: same range but different color
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0xFF)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Changed property should appear in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateTextProp(0xFF)));
			AssertDiffList(desc.runsDiff, expected, L"Text property changed");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Text: CRITICAL - Multiple old runs merge to single new run")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			auto prop1 = CreateTextProp(0x40);
			auto prop2 = CreateTextProp(0x80);
			
			// Old: three runs with alternating properties
			oldRuns.Add({.caretBegin = 1, .caretEnd = 2}, DocumentRunProperty(prop1));
			oldRuns.Add({.caretBegin = 2, .caretEnd = 5}, DocumentRunProperty(prop2));
			oldRuns.Add({.caretBegin = 5, .caretEnd = 10}, DocumentRunProperty(prop1));
			
			// New: single merged run covering entire range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 1, .caretEnd = 10}, DocumentRunProperty(prop1));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// CRITICAL: Must report complete new range (1,10), NOT partial like (2,5)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 1, 10, DocumentRunProperty(prop1));
			AssertDiffList(desc.runsDiff, expected, L"CRITICAL: Merged range uses new key (1,10)");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Text: Different cuts - 2 old runs → 3 new runs (increased, properties changed)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two runs at [0, 15], [15, 30] (cut at 15)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 15, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: three runs at [0, 10], [10, 20], [20, 30] (cut at 10, 20) with different properties
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x20)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three new runs appear in diff (different cuts + properties changed)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateTextProp(0x20)));
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0x60)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0xA0)));
			AssertDiffList(desc.runsDiff, expected, L"2→3 runs, different cuts + property changes");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Text→Text: Different cuts - 4 old runs → 2 new runs (decreased, merged with new property)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			auto propFinal = CreateTextProp(0xFF);
			
			// Old: four runs at [0, 5], [5, 10], [10, 18], [18, 24] (cut at 5, 10, 18)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 5}, DocumentRunProperty(CreateTextProp(0x20)));
			oldRuns.Add({.caretBegin = 5, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 18}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 18, .caretEnd = 24}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: two runs at [0, 12], [12, 24] (cut at 12) with uniform property
			newRuns.Add({.caretBegin = 0, .caretEnd = 12}, DocumentRunProperty(propFinal));
			newRuns.Add({.caretBegin = 12, .caretEnd = 24}, DocumentRunProperty(propFinal));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Both new runs in diff (different cuts + property changed)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 12, DocumentRunProperty(propFinal));
			AddExpectedRun(expected, 12, 24, DocumentRunProperty(propFinal));
			AssertDiffList(desc.runsDiff, expected, L"4→2 runs, different cuts + property changes");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
	
		// Category 4: Inline Object → Inline Object Changes
		
		TEST_CASE(L"Inline→Inline: Same range, callback ID changed")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: inline with callback ID 20
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(20, 100)));
			
			// New: different inline at same range (fully covers old - VALID)
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(21, 100)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// New inline appears in diff (property changed)
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateInlineProp(21, 100)));
			AssertDiffList(desc.runsDiff, expected, L"Inline replaced at same range");
			
			// Old inline removed, new inline created
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({21}), L"New inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({20}), L"Old inline removed");
		});
		
		TEST_CASE(L"Inline→Inline: Different cuts - 3 old inlines → 2 new inlines (decreased count)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three inline objects at [0, 6], [6, 12], [12, 18] (cut at 6, 12)
			oldRuns.Add({.caretBegin = 0, .caretEnd = 6}, DocumentRunProperty(CreateInlineProp(23, 30)));
			oldRuns.Add({.caretBegin = 6, .caretEnd = 12}, DocumentRunProperty(CreateInlineProp(24, 40)));
			oldRuns.Add({.caretBegin = 12, .caretEnd = 18}, DocumentRunProperty(CreateInlineProp(25, 50)));
			
			// New: two inlines at [0, 9], [9, 18] (cut at 9) - different cuts, fewer runs
			newRuns.Add({.caretBegin = 0, .caretEnd = 9}, DocumentRunProperty(CreateInlineProp(26, 90)));
			newRuns.Add({.caretBegin = 9, .caretEnd = 18}, DocumentRunProperty(CreateInlineProp(27, 90)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// New inlines in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 9, DocumentRunProperty(CreateInlineProp(26, 90)));
			AddExpectedRun(expected, 9, 18, DocumentRunProperty(CreateInlineProp(27, 90)));
			AssertDiffList(desc.runsDiff, expected, L"3→2 inlines with different cuts");
			
			// Old inlines removed, new inlines created
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({26, 27}), L"Two new inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({23, 24, 25}), L"Three old inlines removed");
		});
	
		// Category 5: Error Detection - Invalid Range Coverage
		
		TEST_CASE(L"ERROR: Text→Inline with partial coverage (shifted right)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: inline at [15, 25] - only covers [15, 20], leaving [10, 15] uncovered - INVALID
			newRuns.Add({.caretBegin = 15, .caretEnd = 25}, DocumentRunProperty(CreateInlineProp(100, 50)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Text→Text with partial coverage (shifted left)")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: text at [5, 15] - only covers [10, 15], leaving [15, 20] uncovered - INVALID
			newRuns.Add({.caretBegin = 5, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Text→Text with truncated coverage")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: text at [10, 20]
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: text at [10, 15] - only covers [10, 15], leaving [15, 20] uncovered - INVALID
			newRuns.Add({.caretBegin = 10, .caretEnd = 15}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Multiple old runs with gap in new coverage")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: two consecutive text runs
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			
			// New: two runs with a gap [5, 7] uncovered - INVALID
			newRuns.Add({.caretBegin = 0, .caretEnd = 5}, DocumentRunProperty(CreateTextProp(0x80)));
			newRuns.Add({.caretBegin = 7, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// This should trigger CHECK_ERROR for first old run [0, 10]
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
		
		TEST_CASE(L"ERROR: Inline→Text with partial coverage")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: inline at [20, 30]
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(50, 80)));
			
			// New: text at [22, 35] - only covers [22, 30], leaving [20, 22] uncovered - INVALID
			newRuns.Add({.caretBegin = 22, .caretEnd = 35}, DocumentRunProperty(CreateTextProp(0x60)));
			
			// This should trigger CHECK_ERROR and result in TEST_ERROR
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			TEST_ERROR(DiffRuns(oldRuns, newRuns, desc));
		});
	
		// Category 6: Complex Mixed Scenarios
		
		TEST_CASE(L"Mixed transformations: text→inline, inline→text, text→text in one diff")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: mixed runs
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));  // will become inline
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateInlineProp(30, 50)));  // will become text
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x60)));  // property change
			
			// New: transformed runs (all fully cover old - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateInlineProp(31, 100)));  // text→inline
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x80)));  // inline→text
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xA0)));  // text→text
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three transformations in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateInlineProp(31, 100)));
			AddExpectedRun(expected, 10, 20, DocumentRunProperty(CreateTextProp(0x80)));
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0xA0)));
			AssertDiffList(desc.runsDiff, expected, L"Mixed transformations");
			
			// Callback tracking: one created, one removed
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({31}), L"Inline 31 created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({30}), L"Inline 30 removed");
		});
		
		TEST_CASE(L"Range splitting + type changes")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: single text run [0, 9]
			oldRuns.Add({.caretBegin = 0, .caretEnd = 9}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: split into inline + text + inline (fully covers old - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 3}, DocumentRunProperty(CreateInlineProp(40, 30)));
			newRuns.Add({.caretBegin = 3, .caretEnd = 6}, DocumentRunProperty(CreateTextProp(0xA0)));
			newRuns.Add({.caretBegin = 6, .caretEnd = 9}, DocumentRunProperty(CreateInlineProp(41, 40)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// All three new runs in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 3, DocumentRunProperty(CreateInlineProp(40, 30)));
			AddExpectedRun(expected, 3, 6, DocumentRunProperty(CreateTextProp(0xA0)));
			AddExpectedRun(expected, 6, 9, DocumentRunProperty(CreateInlineProp(41, 40)));
			AssertDiffList(desc.runsDiff, expected, L"Split with type changes");
			
			// Two inline objects created
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({40, 41}), L"Two inlines created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		// Category 7: Edge Cases
		
		TEST_CASE(L"Modify first run in paragraph")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three runs, first will change
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: first run property changed, others unchanged (all fully covered - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0xFF)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Only first run in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 0, 10, DocumentRunProperty(CreateTextProp(0xFF)));
			AssertDiffList(desc.runsDiff, expected, L"Only first run modified");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Modify last run in paragraph")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: three runs, last will change
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x80)));
			
			// New: last run becomes inline, others unchanged (all fully covered - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x40)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x60)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateInlineProp(50, 100)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Only last run in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateInlineProp(50, 100)));
			AssertDiffList(desc.runsDiff, expected, L"Only last run modified");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({50}), L"Inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
		
		TEST_CASE(L"Modify middle run surrounded by unchanged runs")
		{
			DocumentRunPropertyMap oldRuns, newRuns;
			
			// Old: five runs, middle one will change
			oldRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x20)));
			oldRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x40)));
			oldRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0x60)));
			oldRuns.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(CreateTextProp(0x80)));
			oldRuns.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			// New: middle run property changed (all fully covered - VALID)
			newRuns.Add({.caretBegin = 0, .caretEnd = 10}, DocumentRunProperty(CreateTextProp(0x20)));
			newRuns.Add({.caretBegin = 10, .caretEnd = 20}, DocumentRunProperty(CreateTextProp(0x40)));
			newRuns.Add({.caretBegin = 20, .caretEnd = 30}, DocumentRunProperty(CreateTextProp(0xFF)));
			newRuns.Add({.caretBegin = 30, .caretEnd = 40}, DocumentRunProperty(CreateTextProp(0x80)));
			newRuns.Add({.caretBegin = 40, .caretEnd = 50}, DocumentRunProperty(CreateTextProp(0xA0)));
			
			remoteprotocol::ElementDesc_DocumentParagraph desc;
			DiffRuns(oldRuns, newRuns, desc);
			
			// Only middle run in diff
			auto expected = MakeExpectedList();
			AddExpectedRun(expected, 20, 30, DocumentRunProperty(CreateTextProp(0xFF)));
			AssertDiffList(desc.runsDiff, expected, L"Only middle run modified");
			
			AssertCallbackIdList(desc.createdInlineObjects, MakeExpectedIds({}), L"No inline created");
			AssertCallbackIdList(desc.removedInlineObjects, MakeExpectedIds({}), L"No inline removed");
		});
	});
}
