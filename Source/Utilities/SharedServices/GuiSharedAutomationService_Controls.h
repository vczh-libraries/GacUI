/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Default Service Implementation

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE_CONTROLS
#define VCZH_PRESENTATION_UTILITIES_SHAREDSERVICES_SHAREDAUTOMATIONSERVICE_CONTROLS

#include "../../Application/Controls/GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiWindow;
		}

		/*
		* Schema:
		*/
		extern Ptr<glr::json::JsonNode>			DumpWindowClientArea(controls::GuiWindow* window, bool withCompositionsAndElements, Nullable<WString> windowId, Point offsetLogical, double scaleX, double scaleY);
	}
}

#endif