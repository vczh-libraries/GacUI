/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Graphics Composition Host
***********************************************************************/

#ifndef VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_POINT
#define VCZH_PRESENTATION_HOST_GUIGRAPHICSHOST_POINT

#include "../GraphicsCompositions/GuiGraphicsEventReceiver.h"
#include "../Controls/GuiInstanceRootObject.h"
#include "../Controls/GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class GuiGraphicsHost;
			class MomentumScrollingState;

			/// <summary>Optional inertial scrolling for controls accepting point sequences. Own this component with GuiInstanceRootObject::AddComponent.</summary>
			class MomentumScrolling : public controls::GuiComponent, public Description<MomentumScrolling>
			{
				friend class MomentumScrollingState;
			protected:
				using StateMap = collections::Dictionary<controls::GuiControl*, Ptr<MomentumScrollingState>>;

				controls::GuiInstanceRootObject*                 rootObject = nullptr;
				StateMap                                        states;
				double                                          deceleration = 2.0;

				void                                            RetireDisposedControls();
			public:
				MomentumScrolling();
				~MomentumScrolling();

				void                                            Attach(controls::GuiInstanceRootObject* rootObject)override;
				void                                            Detach(controls::GuiInstanceRootObject* rootObject)override;
				void                                            EnvironmentChanged()override;
				/// <summary>Observe a control's persistent bounds without changing AcceptPointEvents. Repeated attachment is a no-op.</summary>
				/// <param name="control">The control to observe; attachments do not own controls.</param>
				void                                            AttachControl(controls::GuiControl* control);
				/// <summary>Stop continuation and remove listeners. An unattached control is a no-op.</summary>
				/// <param name="control">The control to detach.</param>
				void                                            DetachControl(controls::GuiControl* control);
				/// <summary>Get the positive deceleration multiplier, initially 2.0 (640 logical pixels per second squared).</summary>
				/// <returns>The multiplier.</returns>
				double                                          GetDeceleration();
				/// <summary>Set a finite positive multiplier for subsequent playbacks.</summary>
				/// <param name="value">The multiplier; twice the value halves the stopping time.</param>
				void                                            SetDeceleration(double value);
			};

			class IGuiPointGesture : public virtual Interface
			{
			public:
				virtual void								OnPointSequenceStarted(const GuiPointSequenceStartedEventArgs& arguments) = 0;
				virtual void								OnPointAction(const GuiPointEventArgs& arguments) = 0;
				virtual void								OnPointSequenceEnded(const GuiPointSequenceEndedEventArgs& arguments) = 0;
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
				NativePoint									currentPointScreen;
				collections::List<Nullable<NativePoint>>	nativePoints;
				collections::List<Nullable<Point>>			points;
				collections::List<Ptr<IGuiPointGesture>>	gestures;
				IGuiPointGesture*							selectedGesture = nullptr;
				bool										ended = false;

			public:
				GuiPointActionManager(GuiGraphicsHost* _host);
				void										OnPointSequenceStarted(const NativeWindowPointInfo& info, const GuiPointSequenceStartedEventArgs& query);
				void										OnPointAction(const NativeWindowPointInfo& info, GuiPointEventsType eventType);
				bool										OnPointSequenceEnded(const PointSequenceEndInfo& info);

				void										PointCapture(GuiGraphicsComposition* composition);
				void										ResetPointCapture();
				GuiGraphicsComposition*						GetPointCapture();

				const collections::List<Nullable<NativePoint>>& GetNativePoints();
				const collections::List<Nullable<Point>>&	GetPoints(GuiGraphicsComposition* relativeTo);
				NativePoint									GetScreenPoint(const NativeWindowPointInfo& info);
				NativePoint									GetCurrentScreenPoint();
				Point										ConvertPoint(NativePoint screen, GuiGraphicsComposition* relativeTo);
				WindowPointInfo								ConvertSample(const NativeWindowPointInfo& info, GuiGraphicsComposition* relativeTo);
				NativeWindowPointInfo						GetFirstPoint();
				Nullable<NativeMouseButton>					GetFallbackClick();
			};
		}
	}
}

#endif
