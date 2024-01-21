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
	template<typename T> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const T&) = delete;
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<bool>(const bool& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<vint>(const vint& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<float>(const float& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<double>(const double& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<WString>(const WString& value);

	template<typename T> void ConvertJsonToCustomType(Ptr<glr::json::JsonNode>, T&) = delete;
	template<> void ConvertJsonToCustomType<bool>(Ptr<glr::json::JsonNode> node, bool& value);
	template<> void ConvertJsonToCustomType<vint>(Ptr<glr::json::JsonNode> node, vint& value);
	template<> void ConvertJsonToCustomType<float>(Ptr<glr::json::JsonNode> node, float& value);
	template<> void ConvertJsonToCustomType<double>(Ptr<glr::json::JsonNode> node, double& value);
	template<> void ConvertJsonToCustomType<WString>(Ptr<glr::json::JsonNode> node, WString& value);
}

#endif
