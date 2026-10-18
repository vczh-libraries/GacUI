/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Graphics Composition Host
***********************************************************************/

#ifndef VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_POINT
#define VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_POINT

#include "../GraphicsCompositions/GuiGraphicsEventReceiver.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class GuiGraphicsHost;

			class GuiPointActionManager : public Object
			{
			protected:
				GuiGraphicsHost*							host;
				GuiGraphicsComposition*						captureComposition = nullptr;
				NativeWindowPointInfo						firstPoint;
				NativePoint									firstPointScreen;
				NativePoint									latestFirstPointScreen;
				collections::List<Nullable<NativePoint>>	nativePoints;
				collections::List<Nullable<Point>>			points;
				vint										downCount = 0;
				vint										axis = 0;
				Nullable<NativeMouseButton>					fallbackClick;

			public:
				GuiPointActionManager(GuiGraphicsHost* _host);
				void										Initialize(const NativeWindowPointInfo& info);
				void										PointCapture(GuiGraphicsComposition* composition);
				void										ResetPointCapture();
				GuiGraphicsComposition*						GetPointCapture();
				const collections::List<Nullable<Point>>&	GetPoints(GuiGraphicsComposition* relativeTo);
				WindowPointInfo								ConvertSample(const NativeWindowPointInfo& info, GuiGraphicsComposition* relativeTo);
				void										OnPointInput(const NativeWindowPointInfo& info, GuiPointEventsType eventType);
				bool										PointSequenceEnded(const PointSequenceEndInfo& info);
				vint										GetAxis();
				Point										GetDisplacement();
				NativeWindowPointInfo						GetFirstPoint();
				Nullable<NativeMouseButton>					GetFallbackClick();
			};
		}
	}
}

#endif
