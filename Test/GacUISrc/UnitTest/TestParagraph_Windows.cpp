#include "../../../Source/GacUI.h"

#if defined VCZH_MSVC
using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::elements;

extern void SetGuiMainProxy(const Func<void()>& proxy);

TEST_FILE
{
	TEST_CASE(L"GDI caret queries prepare layout after properties shape an unmeasured paragraph")
	{
		SetGuiMainProxy([]()
		{
			auto manager = GetGuiGraphicsResourceManager();
			auto native = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
			native->SetClientSize({ 240,240 });
			for (vint i = 0; i < 2; i++)
			{
				auto text = WString::Unmanaged(L"ToolbarImages/a_Align.png");
				auto paragraph = manager->GetLayoutProvider()->CreateParagraph(text, manager->GetRenderTarget(native), nullptr);
				if (i == 0)
				{
					paragraph->SetWrapLine(false);
				}
				else
				{
					paragraph->SetParagraphAlignment(Alignment::Center);
				}
				auto caret = paragraph->GetCaretBounds(text.Length(), true);
				TEST_ASSERT(caret.x1 > 0 && caret.Height() > 0);
				TEST_ASSERT(paragraph->GetCaretFromPoint(caret.LeftTop()) == text.Length());
				paragraph->SetMaxWidth(80);
				paragraph->SetWrapLine(true);
				caret = paragraph->GetCaretBounds(text.Length(), true);
				TEST_ASSERT(caret.y1 > 0 && caret.Height() > 0);
				TEST_ASSERT(paragraph->GetCaretFromPoint(caret.LeftTop()) == text.Length());
			}
			GetCurrentController()->WindowService()->DestroyNativeWindow(native);
		});
		SetupWindowsGDIRenderer();
		SetGuiMainProxy({});
	});
}
#endif
