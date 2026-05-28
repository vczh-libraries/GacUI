#include "GuiSharedAutomationService.h"

namespace vl
{
	namespace presentation
	{
		Ptr<glr::json::JsonNode> DumpWindowClientArea(controls::GuiWindow* window, bool withCompositionsAndElements, Nullable<WString> windowId, Point offsetLogical, double scaleX, double scaleY)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		WString RunIOCommand(INativeController* nativeController, collections::List<Ptr<INativeWindowListener>>& listeners, WString command)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}