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

			class IGuiPointGesture : public virtual Interface
			{
			public:
				virtual void								OnPointSequenceStarted(const NativeWindowPointInfo& info) = 0;
				virtual void								OnPointAction(const NativeWindowPointInfo& info, GuiPointEventsType eventType) = 0;
				virtual void								OnPointSequenceEnded(const PointSequenceEndInfo& info) = 0;
				virtual bool								IsGestureSelected() = 0;
				virtual Nullable<NativeMouseButton>			GetFallbackClick() = 0;
			};

			class GuiPointActionManager : public Object
			{
			protected:
				GuiGraphicsHost*							host;
				GuiGraphicsComposition*						captureComposition = nullptr;
				NativeWindowPointInfo						firstPoint;
				NativePoint									firstPointScreen;
				collections::List<Nullable<NativePoint>>	nativePoints;
				collections::List<Nullable<Point>>			points;
				collections::List<Ptr<IGuiPointGesture>>	gestures;
				IGuiPointGesture*							selectedGesture = nullptr;
				bool										ended = false;

			public:
				GuiPointActionManager(GuiGraphicsHost* _host);
				void										OnPointSequenceStarted(const NativeWindowPointInfo& info, GuiGraphicsComposition* composition);
				void										OnPointAction(const NativeWindowPointInfo& info, GuiPointEventsType eventType);
				bool										OnPointSequenceEnded(const PointSequenceEndInfo& info);

				void										PointCapture(GuiGraphicsComposition* composition);
				void										ResetPointCapture();
				GuiGraphicsComposition*						GetPointCapture();

				const collections::List<Nullable<NativePoint>>& GetNativePoints();
				const collections::List<Nullable<Point>>&	GetPoints(GuiGraphicsComposition* relativeTo);
				NativePoint									GetScreenPoint(const NativeWindowPointInfo& info);
				Point										ConvertPoint(NativePoint screen, GuiGraphicsComposition* relativeTo);
				WindowPointInfo								ConvertSample(const NativeWindowPointInfo& info, GuiGraphicsComposition* relativeTo);
				NativeWindowPointInfo						GetFirstPoint();
				Nullable<NativeMouseButton>					GetFallbackClick();
			};
		}
	}
}

#endif
