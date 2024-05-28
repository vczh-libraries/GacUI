#ifndef GACUISRC_REMOTE_RENDERINGPROTOCOL
#define GACUISRC_REMOTE_RENDERINGPROTOCOL

#include "TestRemote_Shared_NoRenderingProtocol.h"
#include "TestRemote_Shared_LoggingWindowListener.h"

using namespace vl::regex;
using namespace vl::presentation::elements;

namespace remote_protocol_tests
{
	class SingleScreenRenderingProtocol : public SingleScreenProtocol
	{
		using ImageMetadataMap = collections::Dictionary<vint, ImageMetadata>;
	public:
		collections::List<WString>&			eventLogs;
		ElementMeasurings					measuringForNextRendering;
		Regex								regexImage{ L"(<width>/d+)x(<height>/d+)" };
		ImageMetadataMap					imageMetadatas;

		SingleScreenRenderingProtocol(UnitTestScreenConfig _globalConfig, collections::List<WString>& _eventLogs)
			: SingleScreenProtocol(_globalConfig)
			, eventLogs(_eventLogs)
		{
		}

		void RequestControllerConnectionEstablished() override
		{
			SingleScreenProtocol::RequestControllerConnectionEstablished();
			imageMetadatas.Clear();
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
					case RendererType::ImageFrame:
						writer.WriteString(L"ImageFrame");
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

		WString ToString(INativeWindowListener::HitTestResult hitTestResult)
		{
			switch (hitTestResult)
			{
			case INativeWindowListener::BorderNoSizing:		return L"BorderNoSizing";
			case INativeWindowListener::BorderLeft:			return L"BorderLeft";
			case INativeWindowListener::BorderRight:		return L"BorderRight";
			case INativeWindowListener::BorderTop:			return L"BorderTop";
			case INativeWindowListener::BorderBottom:		return L"BorderBottom";
			case INativeWindowListener::BorderLeftTop:		return L"BorderLeftTop";
			case INativeWindowListener::BorderRightTop:		return L"BorderRightTop";
			case INativeWindowListener::BorderLeftBottom:	return L"BorderLeftBottom";
			case INativeWindowListener::BorderRightBottom:	return L"BorderRightBottom";
			case INativeWindowListener::Title:				return L"Title";
			case INativeWindowListener::ButtonMinimum:		return L"ButtonMinimum";
			case INativeWindowListener::ButtonMaximum:		return L"ButtonMaximum";
			case INativeWindowListener::ButtonClose:		return L"ButtonClose";
			case INativeWindowListener::Client:				return L"Client";
			case INativeWindowListener::Icon:				return L"Icon";
			default:										return L"NoDecision";
			}
		}

		WString ToString(INativeCursor::SystemCursorType cursor)
		{
			switch (cursor)
			{
			case INativeCursor::SmallWaiting:		return L"SmallWaiting";
			case INativeCursor::LargeWaiting:		return L"LargeWaiting";
			case INativeCursor::Cross:				return L"Cross";
			case INativeCursor::Hand:				return L"Hand";
			case INativeCursor::Help:				return L"Help";
			case INativeCursor::IBeam:				return L"IBeam";
			case INativeCursor::SizeAll:			return L"SizeAll";
			case INativeCursor::SizeNESW:			return L"SizeNESW";
			case INativeCursor::SizeNS:				return L"SizeNS";
			case INativeCursor::SizeNWSE:			return L"SizeNWSE";
			case INativeCursor::SizeWE:				return L"SizeWE";
			default:								return L"Arrow";
			}
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
			default:										CHECK_FAIL(L"Unrecognized GuiGradientBackgroundElement::Direction");
			}
		}

		WString ToString(ElementHorizontalAlignment alignment)
		{
			switch (alignment)
			{
			case ElementHorizontalAlignment::Center:	return L"Center";
			case ElementHorizontalAlignment::Left:		return L"Left";
			case ElementHorizontalAlignment::Right:		return L"Right";
			default:									CHECK_FAIL(L"Unrecognized ElementHorizontalAlignment");
			}
		}

		WString ToString(ElementVerticalAlignment alignment)
		{
			switch (alignment)
			{
			case ElementVerticalAlignment::Center:		return L"Center";
			case ElementVerticalAlignment::Top:			return L"Top";
			case ElementVerticalAlignment::Bottom:		return L"Bottom";
			default:									CHECK_FAIL(L"Unrecognized ElementVerticalAlignment");
			}
		}

		WString ToString(ElementSolidLabelMeasuringRequest request)
		{
			switch (request)
			{
			case ElementSolidLabelMeasuringRequest::FontHeight:		return L"FontHeight";
			case ElementSolidLabelMeasuringRequest::TotalSize:		return L"TotalSize";
			default:												CHECK_FAIL(L"Unrecognized ElementSolidLabelMeasuringRequest");
			}
		}

		WString ToString(const ImageCreation& imageCreation)
		{
			if (!imageCreation.imageDataOmitted)
			{
				CHECK_ERROR(imageCreation.imageData, L"ImageCreation::imageData should not be null when imageDataOmitted == false.");
				CHECK_ERROR(!imageMetadatas.Keys().Contains(imageCreation.id), L"ImageCreate::imageDataOmitted should be true form the second appearance.");
				U8String data;
				{
					imageCreation.imageData->SeekFromBegin(0);
					stream::StreamReader_<char8_t> reader(*imageCreation.imageData.Obj());
					data = reader.ReadToEnd();
				}
				{
					auto match = regexImage.MatchHead(data);
					CHECK_ERROR(match, L"Unrecognized ImageCreation::imageData content in unit test.");

					ImageFrameMetadata imageFrameMetadata;
					imageFrameMetadata.size.x = wtoi(u8tow(match->Groups()[regexImage.CaptureNames().IndexOf(L"width")][0].Value()));
					imageFrameMetadata.size.y = wtoi(u8tow(match->Groups()[regexImage.CaptureNames().IndexOf(L"height")][0].Value()));

					ImageMetadata imageMetadata;
					imageMetadata.id = imageCreation.id;
					imageMetadata.format = INativeImage::Png;
					imageMetadata.frames = Ptr(new List<ImageFrameMetadata>);
					imageMetadata.frames->Add(imageFrameMetadata);

					imageMetadatas.Add(imageCreation.id, imageMetadata);
				}
				return L"{id:" + itow(imageCreation.id) + L", data:" + u8tow(data) + L"}";
			}
			else
			{
				CHECK_ERROR(!imageCreation.imageData, L"ImageCreation::imageData should be null when imageDataOmitted == true.");
				CHECK_ERROR(imageMetadatas.Keys().Contains(imageCreation.id), L"ImageCreate::imageDataOmitted should be false for the first appearance.");
				return L"{id:" + itow(imageCreation.id) + L", data:omitted}";
			}
		}

		template<typename T>
		WString ToString(const Nullable<T>& nullable)
		{
			if (nullable)
			{
				return ToString(nullable.Value());
			}
			else
			{
				return WString::Unmanaged(L"null");
			}
		}

		void RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments) override
		{
			eventLogs.Add(WString::Unmanaged(L"Begin()"));
		}

		void RequestRendererEndRendering(vint id) override
		{
			eventLogs.Add(WString::Unmanaged(L"End()"));
			events->RespondRendererEndRendering(id, measuringForNextRendering);
			measuringForNextRendering = {};
		}

		void RequestRendererBeginBoundary(const ElementBoundary& arguments) override
		{
			eventLogs.Add(
				L"BeginBoundary("
				+ ToString(arguments.hitTestResult)
				+ L", " + ToString(arguments.cursor)
				+ L", " + ToString(arguments.bounds)
				+ L", " + ToString(arguments.areaClippedBySelf)
				+ L")"
				);
		}

		void RequestRendererEndBoundary() override
		{
			eventLogs.Add(WString::Unmanaged(L"EndBoundary()"));
		}

		void RequestRendererRenderElement(const ElementRendering& arguments) override
		{
			eventLogs.Add(
				L"Render("
				+ itow(arguments.id)
				+ L", " + ToString(arguments.bounds)
				+ L", " + ToString(arguments.areaClippedByParent)
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

		void RequestRendererUpdateElement_SolidLabel(const ElementDesc_SolidLabel& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", " + arguments.textColor.ToString()
				+ L", " + ToString(arguments.horizontalAlignment)
				+ L", " + ToString(arguments.verticalAlignment)
				+ L", <flags:"
					+ (arguments.wrapLine ? L"[wl]" : L"")
					+ (arguments.wrapLineHeightCalculation ? L"[wlhc]" : L"")
					+ (arguments.ellipse ? L"[e]" : L"")
					+ (arguments.multiline ? L"[ml]" : L"")
					+ L">"
				+ L", " + (arguments.font ? L"<font:" + arguments.font.Value().fontFamily + L":" + itow(arguments.font.Value().size) + L">" : L"<nofont>")
				+ L", " + (arguments.text ? L"<text:" + arguments.text.Value() + L">" : L"<notext>")
				+ L", " + (arguments.measuringRequest ? L"<request:" + ToString(arguments.measuringRequest.Value()) + L">" : L"<norequest>")
				+ L")"
				);
		}

		void RequestImageCreated(vint id, const ImageCreation& arguments) override
		{
			eventLogs.Add(
				L"ImageCreated("
				+ ToString(arguments)
				+ L")"
				);
			if (!arguments.imageDataOmitted)
			{
				events->RespondImageCreated(id, imageMetadatas[arguments.id]);
			}
		}

		void RequestImageDestroyed(const vint& arguments) override
		{
			eventLogs.Add(
				L"ImageDestroyed("
				+ itow(arguments)
				+ L")"
				);
		}

		void RequestRendererUpdateElement_ImageFrame(const ElementDesc_ImageFrame& arguments) override
		{
			eventLogs.Add(
				L"Updated("
				+ itow(arguments.id)
				+ L", (" + (arguments.imageId ? itow(arguments.imageId.Value()) : L"null") + L":" + itow(arguments.imageFrame) + L")"
				+ L", " + ToString(arguments.horizontalAlignment)
				+ L", " + ToString(arguments.verticalAlignment)
				+ L", <flags:"
				+ (arguments.stretch ? L"[s]" : L"")
				+ (arguments.enabled ? L"[e]" : L"")
				+ L">"
				+ (arguments.imageCreation ? L", <imageCreation:" + ToString(arguments.imageCreation.Value()) + L">" : L"")
				+ L")"
				);
			if (arguments.imageCreation && !arguments.imageCreation.Value().imageDataOmitted)
			{
				if (!measuringForNextRendering.createdImages)
				{
					measuringForNextRendering.createdImages = Ptr(new List<ImageMetadata>);
				}
				measuringForNextRendering.createdImages->Add(imageMetadatas[arguments.imageCreation.Value().id]);
			}
		}
	};

	template<typename TProtocol, typename TActionAfterRenderingStops, typename ...TRenderingEvents>
	void AssertRenderingEventLogsNoLayout(TProtocol& protocol, List<WString>& eventLogs, TActionAfterRenderingStops&& actionAfterRenderingStops, TRenderingEvents&& ...renderingEvents)
	{
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

		AssertRenderingEventLogsNoLayout(
			protocol,
			eventLogs,
			std::forward<TActionAfterRenderingStops&&>(actionAfterRenderingStops),
			std::forward<TRenderingEvents&&>(renderingEvents)...
			);
	}
}

#endif