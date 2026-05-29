#include "GuiSharedAutomationService.h"

namespace vl
{
	namespace presentation
	{
		using namespace remoteprotocol;

/***********************************************************************
DumpRemoteProtocolRenderingDom
***********************************************************************/

		WString DumpRemoteProtocolRenderingDom(
			const WString& title,
			const remoteprotocol::WindowSizingConfig& windowSizingConfig,
			Ptr<remoteprotocol::RenderingDom> renderingDom,
			collections::Dictionary<vint, collections::Pair<remoteprotocol::RendererType, Nullable<remoteprotocol::UnitTest_ElementDescVariant>>>& elementData
		)
		{
			auto domRoot = Ptr(new glr::json::JsonObject);
			ConvertCustomTypeToJsonField(domRoot, L"Title", title);
			ConvertCustomTypeToJsonField(domRoot, L"Window", windowSizingConfig);

			{
				auto field = Ptr(new glr::json::JsonObjectField);
				field->name.value = WString::Unmanaged(L"Elements");

				auto jsonArray = Ptr(new glr::json::JsonArray);
				field->value = jsonArray;
				domRoot->fields.Add(field);

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
			if (command == L"!Exit")
			{
				nativeController->WindowService()->GetMainWindow()->Hide(true);
				return WString::Empty;
			}
			CHECK_FAIL(L"Not Implemented!");
		}
	}
}