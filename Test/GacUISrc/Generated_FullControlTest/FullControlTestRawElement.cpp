#include "FullControlTestRawElement.h"
#include "../../../Source/PlatformProviders/Windows/Direct2D/Renderers/GuiGraphicsWindowsDirect2D.h"
#include "../../../Source/PlatformProviders/Windows/GDI/Renderers/GuiGraphicsWindowsGDI.h"
#include <cmath>

namespace demo
{
	using namespace vl;
	using namespace vl::presentation;
	using namespace vl::presentation::elements;
	using namespace vl::presentation::compositions;

	class RawTouchingAttachment : public Object
	{
	protected:
		Ptr<GuiRawElementBase> element;
		Ptr<IGuiGraphicsEventHandler> beforeHandler, afterHandler, renderingHandler;
		ComPtr<ID2D1SolidColorBrush> rectangleBrush, pointBrush;

		bool ReadState(Rect bounds, D2D1_POINT_2F (&polygon)[4], collections::List<Point>& points)
		{
			auto data = element->GetData();
			collections::List<double> values;
			vint start = 0;
			for (vint i = 0; i <= data.Length(); i++)
			{
				if (i != data.Length() && data[i] != L',') continue;
				bool success = false;
				auto value = wtof_test(data.Sub(start, i - start), success);
				if (!success || !std::isfinite(value)) return false;
				values.Add(value);
				start = i + 1;
			}
			if (values.Count() < 5 || values[4] < 0 || values[4] != (values.Count() - 5) / 3.0) return false;
			double x[] = { -80,80,80,-80 };
			double y[] = { -50,-50,50,50 };
			for (vint i = 0; i < 4; i++)
			{
				polygon[i] = D2D1::Point2F(
					(FLOAT)(bounds.x1 + values[0] + x[i] * values[2] - y[i] * values[3]),
					(FLOAT)(bounds.y1 + values[1] + x[i] * values[3] + y[i] * values[2]));
			}
			for (vint i = 5; i < values.Count(); i += 3)
			{
				points.Add(Point(bounds.x1 + (vint)values[i + 1], bounds.y1 + (vint)values[i + 2]));
			}
			return true;
		}

		void BeforeTarget(GuiGraphicsComposition*, GuiDirect2DElementEventArgs&)
		{
			rectangleBrush = nullptr;
			pointBrush = nullptr;
		}

		void AfterTarget(GuiGraphicsComposition*, GuiDirect2DElementEventArgs& arguments)
		{
			ID2D1SolidColorBrush* brush = nullptr;
			CHECK_ERROR(SUCCEEDED(arguments.rt->CreateSolidColorBrush(D2D1::ColorF(0x87CEEB), &brush)), L"demo::RawTouchingAttachment::AfterTarget(...)#Cannot create rectangle brush.");
			rectangleBrush = brush;
			CHECK_ERROR(SUCCEEDED(arguments.rt->CreateSolidColorBrush(D2D1::ColorF(0xFFFF00), &brush)), L"demo::RawTouchingAttachment::AfterTarget(...)#Cannot create point brush.");
			pointBrush = brush;
		}

		void RenderD2D(GuiGraphicsComposition*, GuiDirect2DElementEventArgs& arguments)
		{
			if (!rectangleBrush) AfterTarget(nullptr, arguments);
			D2D1_POINT_2F polygon[4];
			collections::List<Point> points;
			if (!ReadState(arguments.bounds, polygon, points)) return;
			ID2D1Factory* factory = nullptr;
			arguments.rt->GetFactory(&factory);
			ComPtr<ID2D1Factory> factoryHolder = factory;
			ID2D1PathGeometry* geometry = nullptr;
			CHECK_ERROR(SUCCEEDED(factory->CreatePathGeometry(&geometry)), L"demo::RawTouchingAttachment::RenderD2D(...)#Cannot create polygon.");
			ComPtr<ID2D1PathGeometry> geometryHolder = geometry;
			ID2D1GeometrySink* sink = nullptr;
			CHECK_ERROR(SUCCEEDED(geometry->Open(&sink)), L"demo::RawTouchingAttachment::RenderD2D(...)#Cannot open polygon.");
			ComPtr<ID2D1GeometrySink> sinkHolder = sink;
			sink->BeginFigure(polygon[0], D2D1_FIGURE_BEGIN_FILLED);
			sink->AddLines(polygon + 1, 3);
			sink->EndFigure(D2D1_FIGURE_END_CLOSED);
			CHECK_ERROR(SUCCEEDED(sink->Close()), L"demo::RawTouchingAttachment::RenderD2D(...)#Cannot finish polygon.");
			arguments.rt->FillGeometry(geometry, rectangleBrush.Obj());
			for (auto point : points)
			{
				arguments.rt->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((FLOAT)point.x, (FLOAT)point.y), 16, 16), pointBrush.Obj(), 2);
			}
		}

		void RenderGDI(GuiGraphicsComposition*, GuiGDIElementEventArgs& arguments)
		{
			D2D1_POINT_2F polygon[4];
			collections::List<Point> points;
			if (!ReadState(arguments.bounds, polygon, points)) return;
			POINT vertices[4];
			for (vint i = 0; i < 4; i++) vertices[i] = { (LONG)std::round(polygon[i].x),(LONG)std::round(polygon[i].y) };
			auto dc = arguments.dc->GetHandle();
			auto saved = SaveDC(dc);
			SelectObject(dc, GetStockObject(NULL_PEN));
			SelectObject(dc, GetStockObject(DC_BRUSH));
			SetDCBrushColor(dc, RGB(135,206,235));
			Polygon(dc, vertices, 4);
			SelectObject(dc, GetStockObject(DC_PEN));
			SelectObject(dc, GetStockObject(NULL_BRUSH));
			SetDCPenColor(dc, RGB(255,255,0));
			for (auto point : points) Ellipse(dc, (int)point.x - 16, (int)point.y - 16, (int)point.x + 16, (int)point.y + 16);
			RestoreDC(dc, saved);
		}

	public:
		RawTouchingAttachment(Ptr<GuiRawElementBase> _element) : element(_element)
		{
			if (auto d2d = element.Cast<GuiDirect2DElement>())
			{
				beforeHandler = d2d->BeforeRenderTargetChanged.AttachMethod(this, &RawTouchingAttachment::BeforeTarget);
				afterHandler = d2d->AfterRenderTargetChanged.AttachMethod(this, &RawTouchingAttachment::AfterTarget);
				renderingHandler = d2d->Rendering.AttachMethod(this, &RawTouchingAttachment::RenderD2D);
			}
			else if (auto gdi = element.Cast<GuiGDIElement>())
			{
				renderingHandler = gdi->Rendering.AttachMethod(this, &RawTouchingAttachment::RenderGDI);
			}
		}

		~RawTouchingAttachment()
		{
			if (auto d2d = element.Cast<GuiDirect2DElement>())
			{
				d2d->BeforeRenderTargetChanged.Detach(beforeHandler);
				d2d->AfterRenderTargetChanged.Detach(afterHandler);
				d2d->Rendering.Detach(renderingHandler);
			}
			else if (auto gdi = element.Cast<GuiGDIElement>())
			{
				gdi->Rendering.Detach(renderingHandler);
			}
		}
	};

	Ptr<Object> AttachFullControlTestTouchingElement(Ptr<GuiRawElementBase> element)
	{
		return Ptr(new RawTouchingAttachment(element));
	}
}
