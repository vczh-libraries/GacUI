#include "GuiSharedAutomationService.h"

namespace vl
{
	namespace presentation
	{
		using namespace remoteprotocol;

/***********************************************************************
DumpRemoteProtocolRenderingDom
***********************************************************************/

		Ptr<glr::json::JsonNode> DumpRemoteProtocolRenderingDom(
			const WString& title,
			const remoteprotocol::WindowSizingConfig& windowSizingConfig,
			Ptr<remoteprotocol::RenderingDom> renderingDom,
			collections::Dictionary<vint, collections::Pair<remoteprotocol::RendererType, Nullable<remoteprotocol::UnitTest_ElementDescVariant>>>& elementData
		)
		{
			auto dumpRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(dumpRoot, L"Title", title);
			ConvertCustomTypeToJsonField(dumpRoot, L"Window", windowSizingConfig);

			{
				auto field = Ptr(new glr::json::JsonObjectField);
				field->name.value = WString::Unmanaged(L"Elements");

				auto jsonArray = Ptr(new glr::json::JsonArray);
				field->value = jsonArray;
				dumpRoot->fields.Add(field);

				for (auto [id, data] : elementData)
				{
					auto jsonElement = Ptr(new glr::json::JsonObject);
					jsonArray->items.Add(jsonElement);

					ConvertCustomTypeToJsonField<vint>(jsonElement, L"Id", id);
					ConvertCustomTypeToJsonField(jsonElement, L"Type", data.key);
					ConvertCustomTypeToJsonField(jsonElement, L"Data", data.value);
				}
			}

			if (renderingDom)
			{
				ConvertCustomTypeToJsonField(dumpRoot, L"Dom", renderingDom);
			}
			return dumpRoot;
		}

		WString DumpJsonToString(Ptr<glr::json::JsonNode> json)
		{
			return stream::GenerateToStream([=](stream::TextWriter& writer)
			{
				glr::json::JsonFormatting formatting;
				formatting.spaceAfterColon = true;
				formatting.spaceAfterComma = true;
				formatting.crlf = true;
				formatting.compact = true;
				formatting.indentation = L"  ";
				return glr::json::JsonPrint(json, writer, formatting);
			});
		}

/***********************************************************************
RunIOCommandOnNativeWindow
***********************************************************************/

		WString RunIOCommandOnNativeWindow(
			IoCommandState* state,
			INativeController* nativeController,
			collections::List<INativeWindowListener*>& listeners,
			WString command
		)
		{
			if (command == L"!Exit")
			{
				nativeController->WindowService()->GetMainWindow()->Hide(true);
				return WString::Empty;
			}
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}