#include "FullControlTestRawClockInterpretor.h"
#include <cmath>

#if defined VCZH_MSVC
#include "../../../Source/PlatformProviders/Windows/Direct2D/Renderers/GuiGraphicsWindowsDirect2D.h"
#include "../../../Source/PlatformProviders/Windows/GDI/Renderers/GuiGraphicsWindowsGDI.h"
#endif

namespace demo
{
	using namespace vl;
	using namespace vl::presentation;
	using namespace vl::presentation::elements;
	using namespace vl::presentation::compositions;

	bool RawClockInterpretorBase::ReadTime(vint& hour, vint& minute, vint& second)
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

	Color RawClockInterpretorBase::GetStrokeColor(Stroke stroke)
	{
		const Color colors[] = { Color(0, 0, 0), Color(255, 255, 255), Color(0, 255, 255), Color(0, 255, 0), Color(255, 255, 0) };
		return colors[(vint)stroke];
	}

	vint RawClockInterpretorBase::GetStrokeWidth(Stroke stroke)
	{
		return stroke == Stroke::Outline ? 3 : 2;
	}

	void RawClockInterpretorBase::DrawClock(Rect bounds)
	{
		vint hour, minute, second;
		if (!ReadTime(hour, minute, second)) return;
		double cx = bounds.x1 + bounds.Width() / 2.0;
		double cy = bounds.y1 + bounds.Height() / 2.0;
		double radius = (double)(bounds.Width() < bounds.Height() ? bounds.Width() : bounds.Height()) / 2.0 - 10.0;
		if (radius <= 0) return;
		constexpr double pi = 3.14159265358979323846;
		double angles[] = { (hour + minute / 60.0 + second / 3600.0) * pi / 6 - pi / 2,
			(minute + second / 60.0) * pi / 30 - pi / 2, second * pi / 30 - pi / 2 };
		double lengths[] = { 90.0 / 160, 110.0 / 160, 130.0 / 160 };
		const Stroke hands[] = { Stroke::Hour, Stroke::Minute, Stroke::Second };
		for (bool outline : { true, false })
		{
			SelectStroke(outline ? Stroke::Outline : Stroke::Face);
			DrawEllipse(cx, cy, radius);
			for (vint i = 0; i < 12; i++)
			{
				auto angle = i * pi / 6;
				DrawLine(cx + cos(angle) * radius * 0.94, cy + sin(angle) * radius * 0.94,
					cx + cos(angle) * radius, cy + sin(angle) * radius);
			}
			for (vint i = 0; i < 3; i++)
			{
				if (!outline) SelectStroke(hands[i]);
				DrawLine(cx, cy, cx + cos(angles[i]) * radius * lengths[i], cy + sin(angles[i]) * radius * lengths[i]);
			}
		}
	}

#if defined VCZH_MSVC
	class RawClockD2DInterpretor : public RawClockInterpretorBase
	{
	protected:
		Ptr<IGuiGraphicsEventHandler> beforeHandler, afterHandler, renderingHandler;
		ComPtr<ID2D1SolidColorBrush> brush;
		FLOAT strokeWidth = 0;
		ID2D1RenderTarget* renderTarget = nullptr;

		void SelectStroke(Stroke stroke) override
		{
			brush->SetColor(elements_windows_d2d::GetD2DColor(GetStrokeColor(stroke)));
			strokeWidth = (FLOAT)GetStrokeWidth(stroke);
		}

		void DrawLine(double x1, double y1, double x2, double y2) override
		{
			renderTarget->DrawLine(D2D1::Point2F((FLOAT)x1, (FLOAT)y1), D2D1::Point2F((FLOAT)x2, (FLOAT)y2), brush.Obj(), strokeWidth);
		}

		void DrawEllipse(double cx, double cy, double radius) override
		{
			renderTarget->DrawEllipse(D2D1::Ellipse(D2D1::Point2F((FLOAT)cx, (FLOAT)cy), (FLOAT)radius, (FLOAT)radius), brush.Obj(), strokeWidth);
		}

		void BeforeTarget(GuiGraphicsComposition*, GuiDirect2DElementEventArgs&)
		{
			brush = nullptr;
		}

		void AfterTarget(GuiGraphicsComposition*, GuiDirect2DElementEventArgs& arguments)
		{
			ID2D1SolidColorBrush* newBrush = nullptr;
			CHECK_ERROR(SUCCEEDED(arguments.rt->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &newBrush)), L"demo::RawClockD2DInterpretor::AfterTarget(...)#Cannot create clock brush.");
			brush = newBrush;
		}

		void Render(GuiGraphicsComposition*, GuiDirect2DElementEventArgs& arguments)
		{
			// Late subscribers may have missed the initial target assignment.
			if (!brush) AfterTarget(nullptr, arguments);
			renderTarget = arguments.rt;
			DrawClock(arguments.bounds);
			renderTarget = nullptr;
		}

	public:
		void Initialize(IGuiGraphicsElement* _element) override
		{
			auto d2d = dynamic_cast<GuiDirect2DElement*>(_element);
			element = d2d;
			beforeHandler = d2d->BeforeRenderTargetChanged.AttachMethod(this, &RawClockD2DInterpretor::BeforeTarget);
			afterHandler = d2d->AfterRenderTargetChanged.AttachMethod(this, &RawClockD2DInterpretor::AfterTarget);
			renderingHandler = d2d->Rendering.AttachMethod(this, &RawClockD2DInterpretor::Render);
		}

		void Finalize() override
		{
			auto d2d = static_cast<GuiDirect2DElement*>(element);
			d2d->BeforeRenderTargetChanged.Detach(beforeHandler);
			d2d->AfterRenderTargetChanged.Detach(afterHandler);
			d2d->Rendering.Detach(renderingHandler);
			brush = nullptr;
			beforeHandler = nullptr;
			afterHandler = nullptr;
			renderingHandler = nullptr;
		}
	};

	class RawClockGDIInterpretor : public RawClockInterpretorBase
	{
	protected:
		Ptr<IGuiGraphicsEventHandler> renderingHandler;
		HDC dc = nullptr;
		windows::WinPen outlinePen{ PS_SOLID, GetStrokeWidth(Stroke::Outline), RGB(0, 0, 0) };

		void SelectStroke(Stroke stroke) override
		{
			if (stroke == Stroke::Outline)
			{
				SelectObject(dc, outlinePen.GetHandle());
			}
			else
			{
				auto color = GetStrokeColor(stroke);
				SelectObject(dc, GetStockObject(DC_PEN));
				SetDCPenColor(dc, RGB(color.r, color.g, color.b));
			}
		}

		void DrawLine(double x1, double y1, double x2, double y2) override
		{
			MoveToEx(dc, (int)std::round(x1), (int)std::round(y1), nullptr);
			LineTo(dc, (int)std::round(x2), (int)std::round(y2));
		}

		void DrawEllipse(double cx, double cy, double radius) override
		{
			Ellipse(dc, (int)(cx - radius), (int)(cy - radius), (int)(cx + radius), (int)(cy + radius));
		}

		void Render(GuiGraphicsComposition*, GuiGDIElementEventArgs& arguments)
		{
			dc = arguments.dc->GetHandle();
			auto saved = SaveDC(dc);
			SelectObject(dc, GetStockObject(NULL_BRUSH));
			DrawClock(arguments.bounds);
			RestoreDC(dc, saved);
			dc = nullptr;
		}

	public:
		void Initialize(IGuiGraphicsElement* _element) override
		{
			auto gdi = dynamic_cast<GuiGDIElement*>(_element);
			element = gdi;
			renderingHandler = gdi->Rendering.AttachMethod(this, &RawClockGDIInterpretor::Render);
		}

		void Finalize() override
		{
			auto gdi = static_cast<GuiGDIElement*>(element);
			gdi->Rendering.Detach(renderingHandler);
			renderingHandler = nullptr;
		}
	};

	class RawClockD2DInterpretorFactory : public Object, public IGuiGraphicsRawInterpretorFactory
	{
	public:
		IGuiGraphicsRawInterpretor* Create() override
		{
			return new RawClockD2DInterpretor;
		}
	};

	class RawClockGDIInterpretorFactory : public Object, public IGuiGraphicsRawInterpretorFactory
	{
	public:
		IGuiGraphicsRawInterpretor* Create() override
		{
			return new RawClockGDIInterpretor;
		}
	};

	class FullControlTestClockInterpretorPlugin : public Object, public IGuiPlugin
	{
	public:
		GUI_PLUGIN_NAME(FullControlTest_ClockInterpretor)
		{
		}

		void Load(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins) override
		{
			if (controllerRelatedPlugins)
			{
				if (elements_windows_d2d::GetWindowsDirect2DResourceManager())
				{
					GetGuiGraphicsResourceManager()->RegisterRawInterpretorFactory(L"FullControlTest::Clock", Ptr(new RawClockD2DInterpretorFactory));
				}
				else if (elements_windows_gdi::GetWindowsGDIResourceManager())
				{
					GetGuiGraphicsResourceManager()->RegisterRawInterpretorFactory(L"FullControlTest::Clock", Ptr(new RawClockGDIInterpretorFactory));
				}
			}
		}

		void Unload(bool controllerUnrelatedPlugins, bool controllerRelatedPlugins) override
		{
		}
	};
	GUI_REGISTER_PLUGIN(FullControlTestClockInterpretorPlugin)
#endif
}
