#ifndef GACUISRC_REMOTE_RENDERINGPROTOCOL
#define GACUISRC_REMOTE_RENDERINGPROTOCOL

#include "TestRemote_Shared_NoRenderingProtocol.h"

namespace remote_protocol_tests
{
	class SingleScreenRenderingProtocol : public SingleScreenProtocol
	{
	public:
		collections::List<WString>& eventLogs;

		SingleScreenRenderingProtocol(SingleScreenConfig _globalConfig, collections::List<WString>& _eventLogs)
			: SingleScreenProtocol(_globalConfig)
			, eventLogs(_eventLogs)
		{
		}

		void RequestRendererCreated(const Ptr<List<RendererCreation>>& arguments) override
		{
			eventLogs.Add(stream::GenerateToStream([&](stream::TextWriter& writer)
			{
				writer.WriteString(L"Created(");
				for (auto [creation, index] : indexed(*arguments.Obj()))
				{
					if (index > 0) writer.WriteString(L", ");
					writer.WriteString(L"<");
					writer.WriteString(itow(creation.id));
					writer.WriteString(L":");
					switch (creation.type)
					{
					case RendererType::FocusRectangle:
						writer.WriteString(L"FocusRectangle");
						break;
					case RendererType::SolidBorder:
						writer.WriteString(L"SolidBorder");
						break;
					case RendererType::SinkBorder:
						writer.WriteString(L"SinkBorder");
						break;
					case RendererType::SinkSplitter:
						writer.WriteString(L"SinkSplitter");
						break;
					case RendererType::SolidBackground:
						writer.WriteString(L"SolidBackground");
						break;
					case RendererType::GradientBackground:
						writer.WriteString(L"GradientBackground");
						break;
					case RendererType::InnerShadow:
						writer.WriteString(L"InnerShadow");
						break;
					case RendererType::SolidLabel:
						writer.WriteString(L"SolidLabel");
						break;
					case RendererType::Polygon:
						writer.WriteString(L"Polygon");
						break;
					default:
						CHECK_FAIL(L"Unrecognized RenderType");
					}
					writer.WriteString(L">");
				}
				writer.WriteString(L")");
			}));
		}

		void RequestRendererDestroyed(const Ptr<List<vint>>& arguments) override
		{
			eventLogs.Add(stream::GenerateToStream([&](stream::TextWriter& writer)
			{
				writer.WriteString(L"Destroyed(");
				for (auto [id, index] : indexed(*arguments.Obj()))
				{
					if (index > 0) writer.WriteString(L", ");
					writer.WriteString(itow(id));
				}
				writer.WriteString(L")");
			}));
		}

		void RequestRendererBeginRendering(vint id) override
		{
			eventLogs.Add(WString::Unmanaged(L"Begin()"));
			ElementMeasurings arguments;
			events->RespondRendererBeginRendering(id, arguments);
		}

		void RequestRendererEndRendering(vint id) override
		{
			eventLogs.Add(WString::Unmanaged(L"End()"));
			ElementMeasurings arguments;
			events->RespondRendererEndRendering(id, arguments);
		}

		WString RectToString(Rect rect)
		{
			return L"{" + itow(rect.x1) + L"," + itow(rect.y1) + L":" + itow(rect.Width()) + L"," + itow(rect.Height()) + L"}";
		}

		void RequestRendererRenderElement(const ElementRendering& arguments) override
		{
			eventLogs.Add(
				L"Render("
				+ itow(arguments.id)
				+ L", " + RectToString(arguments.bounds)
				+ L", " + RectToString(arguments.clipper)
				+ L")"
				);
		}
	};
}

#endif