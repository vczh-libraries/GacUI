#include "GuiSharedAutomationService_Controls.h"

namespace vl
{
	namespace presentation
	{

/***********************************************************************
AutomationService
***********************************************************************/

		WString AutomationService::DumpControlTreeInternal(bool withCompositionsAndElements)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		bool AutomationService::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
AutomationServiceHosted
***********************************************************************/

		WString AutomationServiceHosted::DumpControlTreeInternal(bool withCompositionsAndElements)
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		bool AutomationServiceHosted::CanDumpControlTree()
		{
			return true;
		}

/***********************************************************************
DumpWindowClientArea
***********************************************************************/

		Ptr<glr::json::JsonNode> DumpWindowClientArea(controls::GuiWindow* window, bool withCompositionsAndElements, Nullable<WString> windowId, Point offsetLogical, double scaleX, double scaleY)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}