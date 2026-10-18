#include "GuiGraphicsHost_Point.h"
#include "GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;

			GuiPointActionManager::GuiPointActionManager(GuiGraphicsHost* _host)
				:host(_host)
			{
			}

			void GuiPointActionManager::Initialize(const NativeWindowPointInfo& info)
			{
				ResetPointCapture();
				firstPoint = info;
				auto origin = host->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
				firstPointScreen = NativePoint(info.position.x + origin.x, info.position.y + origin.y);
				latestFirstPointScreen = firstPointScreen;
				nativePoints.Add(firstPointScreen);
			}

			void GuiPointActionManager::PointCapture(GuiGraphicsComposition* composition)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::GuiPointActionManager::PointCapture#"
				CHECK_ERROR(!composition || composition->GetRelatedGraphicsHost() == host, ERROR_MESSAGE_PREFIX L"The selected composition must belong to this graphics host.");
				captureComposition = composition;
				if (!composition) ResetPointCapture();
#undef ERROR_MESSAGE_PREFIX
			}

			void GuiPointActionManager::ResetPointCapture()
			{
				captureComposition = nullptr;
				firstPoint = {};
				firstPointScreen = latestFirstPointScreen = {};
				nativePoints.Clear();
				points.Clear();
				downCount = axis = 0;
				fallbackClick.Reset();
			}

			GuiGraphicsComposition* GuiPointActionManager::GetPointCapture()
			{
				return captureComposition;
			}

			const List<Nullable<Point>>& GuiPointActionManager::GetPoints(GuiGraphicsComposition* relativeTo)
			{
				auto window = host->GetNativeWindow();
				auto origin = window->GetClientBoundsInScreen().LeftTop();
				auto bounds = relativeTo ? relativeTo->GetGlobalBounds() : Rect();
				points.Clear();
				for (auto point : nativePoints)
				{
					if (point)
					{
						auto native = point.Value();
						auto local = window->Convert(NativePoint(native.x - origin.x, native.y - origin.y));
						points.Add(Point(local.x - bounds.x1, local.y - bounds.y1));
					}
					else points.Add({});
				}
				return points;
			}

			WindowPointInfo GuiPointActionManager::ConvertSample(const NativeWindowPointInfo& info, GuiGraphicsComposition* relativeTo)
			{
				auto window = host->GetNativeWindow();
				auto bounds = relativeTo ? relativeTo->GetGlobalBounds() : Rect();
				WindowPointInfo result;
				result.sequenceId = info.sequenceId;
				result.pointId = info.pointId;
				result.timestamp = info.timestamp;
				result.activePointCount = info.activePointCount;
				auto position = window->Convert(info.position);
				result.position = Point(position.x - bounds.x1, position.y - bounds.y1);
				result.primary = info.primary;
				result.ctrl = info.ctrl;
				result.shift = info.shift;
				result.alt = info.alt;
				result.osSuper = info.osSuper;
				if (info.contactBounds)
				{
					auto contact = info.contactBounds.Value();
					auto lt = window->Convert(contact.LeftTop());
					auto rb = window->Convert(contact.RightBottom());
					result.contactBounds = Rect(lt.x - bounds.x1, lt.y - bounds.y1, rb.x - bounds.x1, rb.y - bounds.y1);
				}
				result.pressure = info.pressure;
				result.orientation = info.orientation;
				result.platformFrameId = info.platformFrameId;
				return result;
			}

			void GuiPointActionManager::OnPointInput(const NativeWindowPointInfo& info, GuiPointEventsType eventType)
			{
				if (!captureComposition || firstPoint.sequenceId != info.sequenceId) return;
				auto origin = host->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
				auto screen = NativePoint(info.position.x + origin.x, info.position.y + origin.y);
				if (eventType == GuiPointEventsType::Down) downCount++;
				if (info.pointId == 0)
				{
					latestFirstPointScreen = screen;
					if (axis == 0)
					{
						auto displacement = GetDisplacement();
						auto dx = (vint64_t)displacement.x;
						auto dy = (vint64_t)displacement.y;
						if (dx * dx + dy * dy > 64)
						{
							axis = (dx < 0 ? -dx : dx) >= (dy < 0 ? -dy : dy) ? 1 : 2;
						}
					}
				}
				while (nativePoints.Count() <= info.pointId) nativePoints.Add({});
				nativePoints[info.pointId] = eventType == GuiPointEventsType::Up ? Nullable<NativePoint>() : Nullable<NativePoint>(screen);
				GuiPointEventArgs arguments(captureComposition, eventType, GetPoints(captureComposition));
				static_cast<WindowPointInfo&>(arguments) = ConvertSample(info, captureComposition);
				if (captureComposition->HasEventReceiver()) captureComposition->GetEventReceiver()->pointAction.Execute(arguments);
			}

			bool GuiPointActionManager::PointSequenceEnded(const PointSequenceEndInfo& info)
			{
				fallbackClick.Reset();
				if (!captureComposition || firstPoint.sequenceId != info.sequenceId) return false;
				GuiPointSequenceEndedEventArgs arguments(captureComposition, GetPoints(captureComposition));
				static_cast<PointSequenceEndInfo&>(arguments) = info;
				arguments.accepted = true;
				if (captureComposition->HasEventReceiver()) captureComposition->GetEventReceiver()->pointSequenceEnded.Execute(arguments);
				if (arguments.fallbackClick && info.reason == PointSequenceEndReason::Succeeded && downCount == 1 && axis == 0)
				{
					fallbackClick = info.timestamp - firstPoint.timestamp < 500000 ? NativeMouseButton::Left : NativeMouseButton::Right;
				}
				return true;
			}

			vint GuiPointActionManager::GetAxis()
			{
				return axis;
			}

			Point GuiPointActionManager::GetDisplacement()
			{
				auto window = host->GetNativeWindow();
				auto origin = window->GetClientBoundsInScreen().LeftTop();
				auto start = window->Convert(NativePoint(firstPointScreen.x - origin.x, firstPointScreen.y - origin.y));
				auto latest = window->Convert(NativePoint(latestFirstPointScreen.x - origin.x, latestFirstPointScreen.y - origin.y));
				return Point(latest.x - start.x, latest.y - start.y);
			}

			NativeWindowPointInfo GuiPointActionManager::GetFirstPoint()
			{
				auto result = firstPoint;
				auto origin = host->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
				result.position = NativePoint(firstPointScreen.x - origin.x, firstPointScreen.y - origin.y);
				return result;
			}

			Nullable<NativeMouseButton> GuiPointActionManager::GetFallbackClick()
			{
				return fallbackClick;
			}
		}
	}
}
