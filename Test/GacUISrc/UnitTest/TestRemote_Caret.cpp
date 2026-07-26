#include "../../../Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

using namespace vl;
using namespace vl::unittest;
using namespace vl::collections;
using namespace vl::presentation::elements;
using namespace vl::presentation::remoteprotocol;

class TestGuiRemoteGraphicsParagraph : public GuiRemoteGraphicsParagraph
{
public:
	TestGuiRemoteGraphicsParagraph(const WString& text, vl::presentation::GuiRemoteController* remote)
		: GuiRemoteGraphicsParagraph(text, remote, nullptr, nullptr, nullptr)
	{
	}

	GuiRemoteCaretCache* GetCaretCacheForTest()
	{
		return GetCaretCache();
	}

	vint NativeTextPosToRemoteTextPosForTest(vint textPos)
	{
		return NativeTextPosToRemoteTextPos(textPos);
	}

	vint RemoteTextPosToNativeTextPosForTest(vint textPos)
	{
		return RemoteTextPosToNativeTextPos(textPos);
	}
};

template<vint Size>
void AssertCaretMapping(const Array<vint>& actual, const vint(&expected)[Size])
{
	TEST_ASSERT(actual.Count() == Size);
	for (vint i = 0; i < Size; i++)
	{
		TEST_ASSERT(actual[i] == expected[i]);
	}
}

template<vint NativeSize, vint RemoteSize>
void AssertCaretCache(
	const WString& text,
	CharacterEncoding encoding,
	const vint(&nativeToRemote)[NativeSize],
	const vint(&remoteToNative)[RemoteSize])
{
	GuiRemoteCaretCache cache(text, encoding);
	AssertCaretMapping(cache.nativeToRemote, nativeToRemote);
	AssertCaretMapping(cache.remoteToNative, remoteToNative);
}

TEST_FILE
{
	TEST_CATEGORY(L"GuiRemoteCaretCache")
	{
		TEST_CASE(L"Matching native encoding bypasses cache")
		{
			vl::presentation::GuiRemoteController remote(nullptr);
			auto text = WString::Unmanaged(L"A¢中𦁚B");
			TestGuiRemoteGraphicsParagraph paragraph(text, &remote);

			TEST_ASSERT(paragraph.GetCaretCacheForTest() == nullptr);
			for (vint textPos = 0; textPos <= text.Length(); textPos++)
			{
				TEST_ASSERT(paragraph.NativeTextPosToRemoteTextPosForTest(textPos) == textPos);
				TEST_ASSERT(paragraph.RemoteTextPosToNativeTextPosForTest(textPos) == textPos);
			}
		});

		TEST_CASE(L"Empty and ASCII text")
		{
			const vint empty[] = { 0 };
			const vint ascii[] = { 0,1,2,3 };

			AssertCaretCache(L"", CharacterEncoding::UTF8, empty, empty);
			AssertCaretCache(L"", CharacterEncoding::UTF16, empty, empty);
			AssertCaretCache(L"", CharacterEncoding::UTF32, empty, empty);
			AssertCaretCache(L"ABC", CharacterEncoding::UTF8, ascii, ascii);
			AssertCaretCache(L"ABC", CharacterEncoding::UTF16, ascii, ascii);
			AssertCaretCache(L"ABC", CharacterEncoding::UTF32, ascii, ascii);
		});

		TEST_CASE(L"Mixed text")
		{
			auto text = WString::Unmanaged(L"A¢中𦁚B");

#if defined VCZH_WCHAR_UTF16
			const vint utf8NativeToRemote[] = { 0,1,3,6,6,10,11 };
			const vint utf8RemoteToNative[] = { 0,1,1,2,2,2,3,3,3,3,5,6 };
			const vint utf16NativeToRemote[] = { 0,1,2,3,3,5,6 };
			const vint utf16RemoteToNative[] = { 0,1,2,3,3,5,6 };
			const vint utf32NativeToRemote[] = { 0,1,2,3,3,4,5 };
			const vint utf32RemoteToNative[] = { 0,1,2,3,5,6 };
#elif defined VCZH_WCHAR_UTF32
			const vint utf8NativeToRemote[] = { 0,1,3,6,10,11 };
			const vint utf8RemoteToNative[] = { 0,1,1,2,2,2,3,3,3,3,4,5 };
			const vint utf16NativeToRemote[] = { 0,1,2,3,5,6 };
			const vint utf16RemoteToNative[] = { 0,1,2,3,3,4,5 };
			const vint utf32NativeToRemote[] = { 0,1,2,3,4,5 };
			const vint utf32RemoteToNative[] = { 0,1,2,3,4,5 };
#endif

			AssertCaretCache(text, CharacterEncoding::UTF8, utf8NativeToRemote, utf8RemoteToNative);
			AssertCaretCache(text, CharacterEncoding::UTF16, utf16NativeToRemote, utf16RemoteToNative);
			AssertCaretCache(text, CharacterEncoding::UTF32, utf32NativeToRemote, utf32RemoteToNative);
		});

#if defined VCZH_WCHAR_UTF16
		TEST_CASE(L"Invalid UTF-16 sequences")
		{
			const wchar_t buffer[] =
			{
				L'A',
				(wchar_t)0xD800,
				L'B',
				(wchar_t)0xDC00,
				(wchar_t)0xD858,
				(wchar_t)0xDC5A,
				L'C',
				(wchar_t)0xD800,
			};
			auto text = WString::CopyFrom(buffer, sizeof(buffer) / sizeof(*buffer));

			const vint utf8NativeToRemote[] = { 0,1,2,3,4,4,8,9,10 };
			const vint utf8RemoteToNative[] = { 0,1,2,3,4,4,4,4,6,7,8 };
			const vint utf16NativeToRemote[] = { 0,1,2,3,4,4,6,7,8 };
			const vint utf16RemoteToNative[] = { 0,1,2,3,4,4,6,7,8 };
			const vint utf32NativeToRemote[] = { 0,1,2,3,4,4,5,6,7 };
			const vint utf32RemoteToNative[] = { 0,1,2,3,4,6,7,8 };

			AssertCaretCache(text, CharacterEncoding::UTF8, utf8NativeToRemote, utf8RemoteToNative);
			AssertCaretCache(text, CharacterEncoding::UTF16, utf16NativeToRemote, utf16RemoteToNative);
			AssertCaretCache(text, CharacterEncoding::UTF32, utf32NativeToRemote, utf32RemoteToNative);
		});
#endif
	});
}
