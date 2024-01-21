/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_REMOTEPROTOCOLSCHEMASHARED
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_REMOTEPROTOCOLSCHEMASHARED

#include <VlppGlrParser.h>
#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl::presentation::remoteprotocol
{
	template<typename T>
	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const collections::List<T>& value)
	{
		CHECK_FAIL(L"");
	}

	template<typename T>
	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, collections::List<T>& value)
	{
	}

	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const bool& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const vint& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const float& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const double& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const WString& value);

	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, bool& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, vint& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, float& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, double& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, WString& value);
}

#endif
