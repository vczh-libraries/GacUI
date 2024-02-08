#ifndef GACUISRC_REMOTE_RENDERINGPROTOCOL
#define GACUISRC_REMOTE_RENDERINGPROTOCOL

#include "TestRemote_Shared_NoRenderingProtocol.h"
#include "TestRemote_Shared_LoggingWindowListener.h"

using namespace vl::presentation::elements;

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

		WString ToString(Size size)
		{
			return L"{" + itow(size.x) + L"," + itow(size.y) + L"}";
		}

		WString ToString(Ptr<List<Point>> points)
		{
			return stream::GenerateToStream([&](stream::TextWriter& writer)
			{
				writer.WriteString(L"{");
				for (auto [p, index] : indexed(*points.Obj()))
				{
					if (index > 0) writer.WriteString(L":");
					writer.WriteString(itow(p.x));
					writer.WriteString(L",");
					writer.WriteString(itow(p.y));
				}
				writer.WriteString(L"}");
			});
		}

		WString ToString(Rect rect)
		{
			return L"{" + itow(rect.x1) + L"," + itow(rect.y1) + L":" + itow(rect.Width()) + L"," + itow(rect.Height()) + L"}";
		}

		WString ToString(ElementShape shape)
		{
			switch (shape.shapeType)
			{
			case ElementShapeType::Rectangle:	return L"Rectangle";
			case ElementShapeType::Ellipse:		return L"Ellipse";
			default:							return L"{RoundRect," + itow(shape.radiusX) + L"," + itow(shape.radiusY) + L"}";
			}
		}

		WString ToString(Gui3DSplitterElement::Direction direction)
		{
			switch (direction)
			{
			case Gui3DSplitterElement::Horizontal:	return L"Horizontal";
			case Gui3DSplitterElement::Vertical:	return L"Vertical";
			default:								CHECK_FAIL(L"Unrecognized Gui3DSplitterElement::Direction");
			}
		}

		WString ToString(GuiGradientBackgroundElement::Direction direction)
		{
			switch (direction)
			{
			case GuiGradientBackgroundElement::Horizontal:	return L"Horizontal";
			case GuiGradientBackgroundElement::Vertical:	return L"Vertical";
			case GuiGradientBackgroundElement::Slash:		return L"Slash";
			case GuiGradientBackgroundElement::Backslash:	return L"Backslash";
			default:								CHECK_FAIL(L"Unrecognized GuiGradientBackgroundElement::Direction");
			}
		}

		void RequestRendererRenderElement(const ElementRendering& arguments) override
		{
			eventLogs.Add(
				L"Render("
				+ itow(arguments.id)
				+ L", " + ToString(arguments.bounds)
				+ L", " + ToString(arguments.clipper)
				+ L")"
				);
		}

		void RequestRendererUpdateElement_SolidBorder(const ElementDesc_SolidBorder& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + arguments.borderColor.ToString()
				+ L", " + ToString(arguments.shape)
				+ L")"
				);
		}

		void RequestRendererUpdateElement_SinkBorder(const ElementDesc_SinkBorder& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + arguments.leftTopColor.ToString()
				+ L", " + arguments.rightBottomColor.ToString()
				+ L")"
				);
		}

		void RequestRendererUpdateElement_SinkSplitter(const ElementDesc_SinkSplitter& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + arguments.leftTopColor.ToString()
				+ L", " + arguments.rightBottomColor.ToString()
				+ L", " + ToString(arguments.direction)
				+ L")"
				);
		}

		void RequestRendererUpdateElement_SolidBackground(const ElementDesc_SolidBackground& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + arguments.backgroundColor.ToString()
				+ L", " + ToString(arguments.shape)
				+ L")"
				);
		}

		void RequestRendererUpdateElement_GradientBackground(const ElementDesc_GradientBackground& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + arguments.leftTopColor.ToString()
				+ L", " + arguments.rightBottomColor.ToString()
				+ L", " + ToString(arguments.direction)
				+ L", " + ToString(arguments.shape)
				+ L")"
				);
		}

		void RequestRendererUpdateElement_InnerShadow(const ElementDesc_InnerShadow& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + arguments.shadowColor.ToString()
				+ L", " + itow(arguments.thickness)
				+ L")"
				);
		}

		void RequestRendererUpdateElement_Polygon(const ElementDesc_Polygon& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + ToString(arguments.size)
				+ L", " + ToString(arguments.points)
				+ L", " + arguments.borderColor.ToString()
				+ L", " + arguments.backgroundColor.ToString()
				+ L")"
			);
		}
	};

	template<typename TProtocol, typename TActionAfterRenderingStops, typename ...TRenderingEvents>
	void AssertRenderingEventLogs(TProtocol& protocol, List<WString>& eventLogs, TActionAfterRenderingStops&& actionAfterRenderingStops, TRenderingEvents&& ...renderingEvents)
	{
		protocol.OnNextFrame([=, &eventLogs]()
		{
			// GuiGraphicsHost::Render set updated = true
			// GuiHostedController::GlobalTimer set windowsUpdatedInLastFrame = true
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				renderingEvents...,
				L"End()"
				);
		});

		protocol.OnNextFrame([=, &eventLogs]()
		{
			// GuiGraphicsHost::Render set updated = false
			// GuiHostedController::GlobalTimer set windowsUpdatedInLastFrame = false
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				renderingEvents...,
				L"End()"
				);
		});

		protocol.OnNextFrame([=, &eventLogs]()
		{
			// Rendering is not triggered because GuiHostedController::windowsUpdatedInLastFrame = false
			AssertEventLogs(eventLogs);
			actionAfterRenderingStops();
		});
	}
}

#endif