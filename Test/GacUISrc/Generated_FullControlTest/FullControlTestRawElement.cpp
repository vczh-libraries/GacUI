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

	class RawClockAttachment : public Object
	{
	protected:
		Ptr<GuiRawElementBase> element;
		Ptr<IGuiGraphicsEventHandler> beforeHandler, afterHandler, renderingHandler;
		ComPtr<ID2D1SolidColorBrush> brushes[4];

		bool ReadTime(vint& hour, vint& minute, vint& second)
		{
			auto data = element->GetData();
			if (data.Length() != 8 || data[2] != L':' || data[5] != L':') return false;
			for (vint i : { 0,1,3,4,6,7 })
			{
				if (data[i] < L'0' || data[i] > L'9') return false;
			}
			hour = (data[0] - L'0') * 10 + data[1] - L'0';
			minute = (data[3] - L'0') * 10 + data[4] - L'0';
			second = (data[6] - L'0') * 10 + data[7] - L'0';
			return hour < 24 && minute < 60 && second < 60;
		}

		template<typename TLine, typename TEllipse>
		void DrawClock(Rect bounds, TLine&& line, TEllipse&& ellipse)
		{
			vint hour, minute, second;
			if (!ReadTime(hour, minute, second)) return;
			double cx = bounds.x1 + bounds.Width() / 2.0;
			double cy = bounds.y1 + bounds.Height() / 2.0;
			double radius = (double)(bounds.Width() < bounds.Height() ? bounds.Width() : bounds.Height()) / 2.0 - 10.0;
			if (radius <= 0) return;
			constexpr double pi = 3.14159265358979323846;
			ellipse(cx, cy, radius);
			for (vint i = 0; i < 12; i++)
			{
				auto angle = i * pi / 6;
				line(0, cx + cos(angle) * radius * 0.94, cy + sin(angle) * radius * 0.94,
					cx + cos(angle) * radius, cy + sin(angle) * radius);
			}
			double angles[] = { (hour + minute / 60.0 + second / 3600.0) * pi / 6 - pi / 2,
				(minute + second / 60.0) * pi / 30 - pi / 2, second * pi / 30 - pi / 2 };
			double lengths[] = { 90.0 / 160, 110.0 / 160, 130.0 / 160 };
			for (vint i = 0; i < 3; i++)
			{
				line(i + 1, cx, cy, cx + cos(angles[i]) * radius * lengths[i], cy + sin(angles[i]) * radius * lengths[i]);
			}
		}

		void BeforeTarget(GuiGraphicsComposition*, GuiDirect2DElementEventArgs&)
		{
			for (auto& brush : brushes) brush = nullptr;
		}

		void AfterTarget(GuiGraphicsComposition*, GuiDirect2DElementEventArgs& arguments)
		{
			D2D1_COLOR_F colors[] = { D2D1::ColorF(1,1,1), D2D1::ColorF(0,1,1), D2D1::ColorF(0,1,0), D2D1::ColorF(1,1,0) };
			for (vint i = 0; i < 4; i++)
			{
				ID2D1SolidColorBrush* brush = nullptr;
				CHECK_ERROR(SUCCEEDED(arguments.rt->CreateSolidColorBrush(colors[i], &brush)), L"Cannot create clock brush.");
				brushes[i] = brush;
			}
		}

		void RenderD2D(GuiGraphicsComposition*, GuiDirect2DElementEventArgs& arguments)
		{
			// Late subscribers may have missed the initial target assignment.
			if (!brushes[0]) AfterTarget(nullptr, arguments);
			DrawClock(arguments.bounds,
				[&](vint color, double x1, double y1, double x2, double y2)
				{
					arguments.rt->DrawLine(D2D1::Point2F((FLOAT)x1, (FLOAT)y1), D2D1::Point2F((FLOAT)x2, (FLOAT)y2), brushes[color].Obj(), 2);
				},
				[&](double cx, double cy, double radius)
				{
					arguments.rt->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((FLOAT)cx, (FLOAT)cy), (FLOAT)radius, (FLOAT)radius), brushes[0].Obj(), 2);
				});
		}

		void RenderGDI(GuiGraphicsComposition*, GuiGDIElementEventArgs& arguments)
		{
			auto dc = arguments.dc->GetHandle();
			auto saved = SaveDC(dc);
			SelectObject(dc, GetStockObject(DC_PEN));
			SelectObject(dc, GetStockObject(NULL_BRUSH));
			COLORREF colors[] = { RGB(255,255,255), RGB(0,255,255), RGB(0,255,0), RGB(255,255,0) };
			DrawClock(arguments.bounds,
				[&](vint color, double x1, double y1, double x2, double y2)
				{
					SetDCPenColor(dc, colors[color]);
					MoveToEx(dc, (int)std::round(x1), (int)std::round(y1), nullptr);
					LineTo(dc, (int)std::round(x2), (int)std::round(y2));
				},
				[&](double cx, double cy, double radius)
				{
					SetDCPenColor(dc, colors[0]);
					Ellipse(dc, (int)(cx - radius), (int)(cy - radius), (int)(cx + radius), (int)(cy + radius));
				});
			RestoreDC(dc, saved);
		}
	public:
		RawClockAttachment(Ptr<GuiRawElementBase> _element) : element(_element)
		{
			if (auto d2d = element.Cast<GuiDirect2DElement>())
			{
				beforeHandler = d2d->BeforeRenderTargetChanged.AttachMethod(this, &RawClockAttachment::BeforeTarget);
				afterHandler = d2d->AfterRenderTargetChanged.AttachMethod(this, &RawClockAttachment::AfterTarget);
				renderingHandler = d2d->Rendering.AttachMethod(this, &RawClockAttachment::RenderD2D);
			}
			else if (auto gdi = element.Cast<GuiGDIElement>())
			{
				renderingHandler = gdi->Rendering.AttachMethod(this, &RawClockAttachment::RenderGDI);
			}
		}

		~RawClockAttachment()
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

	Ptr<Object> AttachFullControlTestRawElement(Ptr<GuiRawElementBase> element)
	{
		return Ptr(new RawClockAttachment(element));
	}
}
