#include "GuiSharedAutomationService.h"

namespace vl
{
	namespace presentation
	{
		using namespace remoteprotocol;

/***********************************************************************
DumpRemoteProtocolRenderingDom
***********************************************************************/

		WString DumpRemoteProtocolRenderingDom(const WString& title, const remoteprotocol::WindowSizingConfig& windowSizingConfig, Ptr<remoteprotocol::RenderingDom> renderingDom)
		{
			auto domRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(domRoot, L"Title", title);
			ConvertCustomTypeToJsonField(domRoot, L"Window", windowSizingConfig);
			if (renderingDom)
			{
				ConvertCustomTypeToJsonField(domRoot, L"Dom", renderingDom);
			}
			return stream::GenerateToStream([=](stream::TextWriter& writer)
			{
				glr::json::JsonFormatting formatting;
				formatting.spaceAfterColon = true;
				formatting.spaceAfterComma = true;
				formatting.crlf = true;
				formatting.compact = true;
				formatting.indentation = L"  ";
				return glr::json::JsonPrint(domRoot, writer, formatting);
			});
		}

/***********************************************************************
RunIOCommandOnNativeWindow
***********************************************************************/

		WString RunIOCommandOnNativeWindow(INativeController* nativeController, collections::List<INativeWindowListener*>& listeners, WString command)
		{
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}