#include "GuiGraphicsHost_Point.h"
#include "GuiGraphicsHost.h"
#include <cmath>

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;

			constexpr vint StaticPointRange = 8;

/***********************************************************************
MomentumScrollingState
***********************************************************************/

			class MomentumScrollingState : public Object, public controls::IGuiAnimation
			{
			public:
				struct Sample
				{
					vuint64_t time;
					double position;
				};

				MomentumScrolling* owner;
				controls::GuiControl* control;
				Ptr<controls::GuiDisposedFlag> disposed;
				GuiGraphicsComposition* bounds;
				GuiGraphicsHost* host = nullptr;
				Ptr<IGuiGraphicsEventHandler> startedHandler, actionHandler, gestureHandler, endedHandler;
				List<Sample> samples;
				vuint64_t sequence = 0, started = 0, timestamp = 0, playbackStart = 0, version = 0;
				double position = 0, velocity = 0, duration = 0, elapsed = 0, residue = 0;
				vint boundary = 1;
				bool vertical = false, scrolling = false, stopped = true, generating = false;

				MomentumScrollingState(MomentumScrolling* _owner, controls::GuiControl* _control)
					:owner(_owner), control(_control), disposed(_control->GetDisposedFlag()), bounds(_control->GetBoundsComposition())
				{
					auto receiver = bounds->GetEventReceiver();
					startedHandler = receiver->pointSequenceStarted.AttachMethod(this, &MomentumScrollingState::OnStarted);
					actionHandler = receiver->pointAction.AttachMethod(this, &MomentumScrollingState::OnAction);
					gestureHandler = receiver->pointGesture.AttachMethod(this, &MomentumScrollingState::OnGesture);
					endedHandler = receiver->pointSequenceEnded.AttachMethod(this, &MomentumScrollingState::OnEnded);
				}

				void Stop()
				{
					version++;
					stopped = true;
					sequence = 0;
					samples.Clear();
				}

				void Detach()
				{
					Stop();
					owner = nullptr;
					if (!disposed->IsDisposed())
					{
						auto receiver = bounds->GetEventReceiver();
						receiver->pointSequenceStarted.Detach(startedHandler);
						receiver->pointAction.Detach(actionHandler);
						receiver->pointGesture.Detach(gestureHandler);
						receiver->pointSequenceEnded.Detach(endedHandler);
					}
				}

				void Record(vuint64_t time)
				{
					if (samples.Count() && samples[samples.Count() - 1].time == time)
					{
						samples[samples.Count() - 1].position = position;
					}
					else
					{
						samples.Add({ time,position });
					}
					auto left = time > 500000 ? time - 500000 : 0;
					while (samples.Count() > 1 && samples[1].time <= left) samples.RemoveAt(0);
				}

				void OnStarted(GuiGraphicsComposition*, GuiPointSequenceStartedEventArgs& arguments)
				{
					Stop();
					if (!owner || disposed->IsDisposed()) return;
					// A later Started handler can still accept this bounds. Rejected queries receive no actions.
					if (arguments.acceptedComposition && arguments.acceptedComposition != bounds) return;
					sequence = arguments.sequenceId;
					started = timestamp = arguments.timestamp;
					position = 0;
					scrolling = false;
					host = bounds->GetRelatedGraphicsHost();
					Record(started);
				}

				void OnAction(GuiGraphicsComposition*, GuiPointEventArgs& arguments)
				{
					if (!sequence || arguments.sequenceId != sequence) return;
					if (arguments.compositionSource != bounds || arguments.eventSource != bounds) return;
					if (arguments.eventType == GuiPointEventsType::Down && arguments.pointId != 0)
					{
						Stop();
						return;
					}
					timestamp = arguments.timestamp;
				}

				void OnGesture(GuiGraphicsComposition*, GuiPointGestureEventArgs& arguments)
				{
					if (!owner || disposed->IsDisposed() || generating || !sequence) return;
					if (arguments.compositionSource != bounds || arguments.eventSource != bounds) return;
					if (auto gesture = arguments.gesture.TryGet<GuiPointGesture_Scrolling>())
					{
						arguments.selected = true;
						scrolling = true;
						vertical = gesture->isVertical;
						position += gesture->distance;
						Record(timestamp);
					}
				}

				void OnEnded(GuiGraphicsComposition*, GuiPointSequenceEndedEventArgs& arguments)
				{
					if (!owner || disposed->IsDisposed() || !sequence || arguments.sequenceId != sequence) return;
					sequence = 0;
					if (arguments.reason != PointSequenceEndReason::Succeeded || !scrolling || !owner->rootObject) return;
					if (bounds->GetRelatedGraphicsHost() != host) return;
					Record(arguments.timestamp);
					auto left = arguments.timestamp - started > 500000 ? arguments.timestamp - 500000 : started;
					if (left == arguments.timestamp || samples.Count() < 2) return;
					auto first = samples[0];
					auto next = samples[1];
					auto initial = first.position + (next.position - first.position) * ((double)(left - first.time) / (next.time - first.time));
					velocity = (position - initial) * 1000000 / (arguments.timestamp - left);
					if (velocity == 0) return;
					duration = std::abs(velocity) / 320 / owner->deceleration;
					if (duration == 0) return;
					stopped = false;
					// Start also resets an animation still awaiting retirement from the previous sequence.
					Start();
					for (auto state : owner->states)
					{
						if (state.Obj() == this)
						{
							owner->rootObject->AddAnimation(state);
							break;
						}
					}
				}

				void Start()override
				{
					if (stopped) return;
					playbackStart = DateTime::UtcTime().osMilliseconds;
					elapsed = residue = 0;
					boundary = 1;
				}

				void Pause()override { Stop(); }
				void Resume()override {}
				bool GetStopped()override { return stopped; }

				void Run()override
				{
					if (!owner) return;
					if (disposed->IsDisposed())
					{
						owner->RetireDisposedControls();
						return;
					}
					if (stopped) return;
					if (bounds->GetRelatedGraphicsHost() != host)
					{
						Stop();
						return;
					}
					auto now = DateTime::UtcTime().osMilliseconds;
					auto time = (double)(now >= playbackStart ? now - playbackStart : 0) / 1000;
					auto speed = std::abs(velocity);
					auto playbackVersion = version;
					while (!stopped)
					{
						auto end = std::min(boundary / 10.0, duration);
						if (time < end) break;
						auto distance = speed * (1 - (elapsed / duration + end / duration) / 2) * (end - elapsed);
						residue += velocity < 0 ? -distance : distance;
						auto rounded = std::round(residue);
						// A carried half-pixel must not reverse direction on a zero-distance tail.
						auto increment = (vint)(velocity < 0 ? std::min(0.0, rounded) : std::max(0.0, rounded));
						residue -= increment;
						elapsed = end;
						boundary++;
						if (end == duration) stopped = true;
						if (increment != 0)
						{
							GuiPointGestureEventArgs arguments(bounds);
							arguments.gesture = GuiPointGesture_Scrolling{ vertical,increment };
							generating = true;
							bounds->GetEventReceiver()->pointGesture.Execute(arguments);
							generating = false;
							if (!owner || disposed->IsDisposed() || version != playbackVersion) return;
							if (bounds->GetRelatedGraphicsHost() != host) Stop();
						}
					}
				}
			};

/***********************************************************************
MomentumScrolling
***********************************************************************/

			MomentumScrolling::MomentumScrolling() {}

			MomentumScrolling::~MomentumScrolling()
			{
				Detach(rootObject);
			}

			void MomentumScrolling::Attach(controls::GuiInstanceRootObject* value)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::MomentumScrolling::Attach#"
				CHECK_ERROR(!rootObject, ERROR_MESSAGE_PREFIX L"The component already has an owner.");
				rootObject = value;
				RetireDisposedControls();
#undef ERROR_MESSAGE_PREFIX
			}

			void MomentumScrolling::Detach(controls::GuiInstanceRootObject*)
			{
				for (auto state : states) state->Detach();
				states.Clear();
				rootObject = nullptr;
			}

			void MomentumScrolling::EnvironmentChanged()
			{
				RetireDisposedControls();
				for (auto state : states) state->Stop();
			}

			void MomentumScrolling::RetireDisposedControls()
			{
				for (vint i = states.Count() - 1; i >= 0; i--)
				{
					if (states[i]->disposed->IsDisposed())
					{
						states[i]->Detach();
						states.RemoveAt(i);
					}
				}
			}

			void MomentumScrolling::AttachControl(controls::GuiControl* control)
			{
				RetireDisposedControls();
				for (auto state : states) if (state->control == control) return;
				states.Add(Ptr(new MomentumScrollingState(this, control)));
			}

			void MomentumScrolling::DetachControl(controls::GuiControl* control)
			{
				RetireDisposedControls();
				for (auto [state, index] : indexed(states))
				{
					if (state->control == control)
					{
						state->Detach();
						states.RemoveAt(index);
						return;
					}
				}
			}

			double MomentumScrolling::GetDeceleration()
			{
				RetireDisposedControls();
				return deceleration;
			}

			void MomentumScrolling::SetDeceleration(double value)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::MomentumScrolling::SetDeceleration#"
				CHECK_ERROR(std::isfinite(value) && value > 0, ERROR_MESSAGE_PREFIX L"Deceleration must be finite and positive.");
				RetireDisposedControls();
				deceleration = value;
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
Predefined Gestures
***********************************************************************/

			bool IsOutsideStaticPointRange(Point first, Point last)
			{
				auto dx = (vint64_t)last.x - first.x;
				auto dy = (vint64_t)last.y - first.y;
				return dx < -StaticPointRange || dx > StaticPointRange
					|| dy < -StaticPointRange || dy > StaticPointRange
					|| dx * dx + dy * dy > StaticPointRange * StaticPointRange;
			}

			class GuiPointGestureBase : public Object, public IGuiPointGesture
			{
			protected:
				GuiPointActionManager* manager;
				bool selected = false;

				void RaiseGesture(const Variant<GuiPointGesture_Scrolling, GuiPointGesture_DraggingAndRotation>& payload)
				{
					auto target = manager->GetPointCapture();
					GuiPointGestureEventArgs arguments(target);
					arguments.gesture = payload;
					arguments.selected = selected;
					if (target->HasEventReceiver()) target->GetEventReceiver()->pointGesture.Execute(arguments);
					selected = selected || arguments.selected;
				}

			public:
				GuiPointGestureBase(GuiPointActionManager* _manager) : manager(_manager) {}
				bool IsGestureSelected()override { return selected; }
				Nullable<NativeMouseButton> GetFallbackClick()override { return {}; }
				void OnPointSequenceEnded(const GuiPointSequenceEndedEventArgs&)override {}
			};

			namespace predefined_gestures
			{
				class RightClickGesture : public GuiPointGestureBase
				{
				protected:
					NativePoint first;
					vuint64_t timestamp = 0;
					vint downCount = 0;
					bool eligible = false;
					Nullable<NativeMouseButton> fallback;

				public:
					RightClickGesture(GuiPointActionManager* manager) : GuiPointGestureBase(manager) {}

					void OnPointSequenceStarted(const GuiPointSequenceStartedEventArgs& info)override
					{
						selected = false;
						first = manager->GetCurrentScreenPoint();
						timestamp = info.timestamp;
						downCount = 0;
						eligible = info.generateRightButtonClick;
						fallback.Reset();
					}

					void OnPointAction(const GuiPointEventArgs& info)override
					{
						if (info.eventType == GuiPointEventsType::Down && ++downCount != 1) eligible = false;
						if (eligible && info.pointId == 0)
						{
							auto start = manager->ConvertPoint(first, nullptr);
							auto current = manager->ConvertPoint(manager->GetCurrentScreenPoint(), nullptr);
							if (IsOutsideStaticPointRange(start, current)) eligible = false;
						}
					}

					void OnPointSequenceEnded(const GuiPointSequenceEndedEventArgs& info)override
					{
						if (eligible && downCount == 1 && info.reason == PointSequenceEndReason::Succeeded
							&& info.timestamp - timestamp >= 500000)
						{
							fallback = NativeMouseButton::Right;
						}
					}

					Nullable<NativeMouseButton> GetFallbackClick()override { return fallback; }
				};

				class LeftClickGesture : public GuiPointGestureBase
				{
				protected:
					NativePoint first;
					vint downCount = 0;
					bool eligible = false;
					Nullable<NativeMouseButton> fallback;

				public:
					LeftClickGesture(GuiPointActionManager* manager) : GuiPointGestureBase(manager) {}

					void OnPointSequenceStarted(const GuiPointSequenceStartedEventArgs& info)override
					{
						selected = false;
						first = manager->GetCurrentScreenPoint();
						downCount = 0;
						eligible = info.generateLeftButtonClick;
						fallback.Reset();
					}

					void OnPointAction(const GuiPointEventArgs& info)override
					{
						if (info.eventType == GuiPointEventsType::Down && ++downCount != 1) eligible = false;
						if (eligible && info.pointId == 0)
						{
							auto start = manager->ConvertPoint(first, nullptr);
							auto current = manager->ConvertPoint(manager->GetCurrentScreenPoint(), nullptr);
							if (IsOutsideStaticPointRange(start, current)) eligible = false;
						}
					}

					void OnPointSequenceEnded(const GuiPointSequenceEndedEventArgs& info)override
					{
						if (eligible && downCount == 1 && info.reason == PointSequenceEndReason::Succeeded)
						{
							fallback = NativeMouseButton::Left;
						}
					}

					Nullable<NativeMouseButton> GetFallbackClick()override { return fallback; }
				};

				class ScrollingGesture : public GuiPointGestureBase
				{
				protected:
					NativePoint previous;
					vint axis = 0;
					bool stopped = false;

				public:
					ScrollingGesture(GuiPointActionManager* manager) : GuiPointGestureBase(manager) {}

					void OnPointSequenceStarted(const GuiPointSequenceStartedEventArgs& info)override
					{
						selected = false;
						previous = manager->GetCurrentScreenPoint();
						axis = 0;
						stopped = false;
					}

					void OnPointAction(const GuiPointEventArgs& info)override
					{
						if (info.eventType == GuiPointEventsType::Down && info.pointId != 0) stopped = true;
						if (stopped || info.pointId != 0) return;
						auto screen = manager->GetCurrentScreenPoint();
						auto start = manager->ConvertPoint(previous, nullptr);
						auto current = manager->ConvertPoint(screen, nullptr);
						auto dx = (vint64_t)current.x - start.x;
						auto dy = (vint64_t)current.y - start.y;
						if (axis == 0)
						{
							if (!IsOutsideStaticPointRange(start, current)) return;
							axis = (dx < 0 ? -dx : dx) >= (dy < 0 ? -dy : dy) ? 1 : 2;
						}
						previous = screen;
						RaiseGesture(GuiPointGesture_Scrolling{ .isVertical = axis == 2, .distance = (vint)(axis == 1 ? dx : dy) });
					}
				};

				class DraggingAndRotationGesture : public GuiPointGestureBase
				{
				protected:
					List<Nullable<NativePoint>> baseline;
					double residualX = 0;
					double residualY = 0;

					void UpdateTransform(const List<Nullable<NativePoint>>& current)
					{
						vint ids[2] = { -1,-1 };
						vint count = 0;
						bool same = baseline.Count() == current.Count();
						for (auto [point, i] : indexed(current))
						{
							if (i >= baseline.Count() || (bool)point != (bool)baseline[i]) same = false;
							if (point)
							{
								if (count < 2) ids[count] = i;
								count++;
							}
						}
						if (same && count > 0 && count <= 2)
						{
							auto target = manager->GetPointCapture();
							auto oldFirst = manager->ConvertPoint(baseline[ids[0]].Value(), target);
							auto newFirst = manager->ConvertPoint(current[ids[0]].Value(), target);
							double oldX = (double)oldFirst.x, oldY = (double)oldFirst.y;
							double newX = (double)newFirst.x, newY = (double)newFirst.y;
							double real = 1, imaginary = 0;
							bool separated = true;
							if (count == 2)
							{
								auto oldSecond = manager->ConvertPoint(baseline[ids[1]].Value(), target);
								auto newSecond = manager->ConvertPoint(current[ids[1]].Value(), target);
								auto oldDX = (double)oldSecond.x - oldX;
								auto oldDY = (double)oldSecond.y - oldY;
								auto newDX = (double)newSecond.x - newX;
								auto newDY = (double)newSecond.y - newY;
								auto lengthSquared = oldDX * oldDX + oldDY * oldDY;
								separated = lengthSquared > 0 && newDX * newDX + newDY * newDY > 0;
								if (separated)
								{
									real = (newDX * oldDX + newDY * oldDY) / lengthSquared;
									imaginary = (newDY * oldDX - newDX * oldDY) / lengthSquared;
									oldX += oldDX / 2; oldY += oldDY / 2;
									newX += newDX / 2; newY += newDY / 2;
								}
							}
							if (separated)
							{
								auto dx = newX - real * oldX + imaginary * oldY + real * residualX - imaginary * residualY;
								auto dy = newY - imaginary * oldX - real * oldY + imaginary * residualX + real * residualY;
								GuiPointGesture_DraggingAndRotation payload;
								payload.rotation = std::atan2(imaginary, real);
								payload.scale = std::sqrt(real * real + imaginary * imaginary);
								// Ties round towards positive infinity, keeping residuals in [-0.5, 0.5).
								payload.distance = Point((vint)std::floor(dx + 0.5), (vint)std::floor(dy + 0.5));
								residualX = dx - payload.distance.x;
								residualY = dy - payload.distance.y;
								RaiseGesture(payload);
							}
						}
						CopyFrom(baseline, current);
					}

				public:
					DraggingAndRotationGesture(GuiPointActionManager* manager) : GuiPointGestureBase(manager) {}

					void OnPointSequenceStarted(const GuiPointSequenceStartedEventArgs&)override
					{
						selected = false;
						residualX = residualY = 0;
						CopyFrom(baseline, manager->GetNativePoints());
						RaiseGesture(GuiPointGesture_DraggingAndRotation{});
					}

					void OnPointAction(const GuiPointEventArgs& info)override
					{
						if (!selected) return;
						auto&& current = manager->GetNativePoints();
						if (info.eventType == GuiPointEventsType::Up)
						{
							List<Nullable<NativePoint>> finalSample;
							CopyFrom(finalSample, current);
							finalSample[info.pointId] = manager->GetCurrentScreenPoint();
							UpdateTransform(finalSample);
							CopyFrom(baseline, current);
						}
						else UpdateTransform(current);
					}
				};
			}

/***********************************************************************
GuiPointActionManager
***********************************************************************/

			GuiPointActionManager::GuiPointActionManager(GuiGraphicsHost* _host)
				:host(_host)
			{
				gestures.Add(Ptr(new predefined_gestures::RightClickGesture(this)));
				gestures.Add(Ptr(new predefined_gestures::LeftClickGesture(this)));
				gestures.Add(Ptr(new predefined_gestures::ScrollingGesture(this)));
				gestures.Add(Ptr(new predefined_gestures::DraggingAndRotationGesture(this)));
			}

			void GuiPointActionManager::OnPointSequenceStarted(const NativeWindowPointInfo& info, const GuiPointSequenceStartedEventArgs& query)
			{
				ResetPointCapture();
				firstPoint = info;
				firstPointScreen = GetScreenPoint(info);
				currentPointScreen = firstPointScreen;
				nativePoints.Add(firstPointScreen);
				PointCapture(query.acceptedComposition);
				GuiPointSequenceStartedEventArgs arguments(captureComposition, GetPoints(captureComposition));
				static_cast<WindowPointInfo&>(arguments) = ConvertSample(info, captureComposition);
				arguments.acceptedComposition = captureComposition;
				arguments.generateLeftButtonClick = query.generateLeftButtonClick;
				arguments.generateRightButtonClick = query.generateRightButtonClick;
				for (auto gesture : gestures)
				{
					gesture->OnPointSequenceStarted(arguments);
					if (gesture->IsGestureSelected())
					{
						selectedGesture = gesture.Obj();
						break;
					}
				}
			}

			void GuiPointActionManager::OnPointAction(const NativeWindowPointInfo& info, GuiPointEventsType eventType)
			{
				if (!captureComposition || ended || firstPoint.sequenceId != info.sequenceId) return;
				currentPointScreen = GetScreenPoint(info);
				while (nativePoints.Count() <= info.pointId) nativePoints.Add({});
				nativePoints[info.pointId] = eventType == GuiPointEventsType::Up ? Nullable<NativePoint>() : Nullable<NativePoint>(currentPointScreen);
				GuiPointEventArgs arguments(captureComposition, eventType, GetPoints(captureComposition));
				static_cast<WindowPointInfo&>(arguments) = ConvertSample(info, captureComposition);
				if (captureComposition->HasEventReceiver()) captureComposition->GetEventReceiver()->pointAction.Execute(arguments);
				if (selectedGesture) selectedGesture->OnPointAction(arguments);
				else for (auto gesture : gestures)
				{
					gesture->OnPointAction(arguments);
					if (gesture->IsGestureSelected())
					{
						selectedGesture = gesture.Obj();
						break;
					}
				}
			}

			bool GuiPointActionManager::OnPointSequenceEnded(const PointSequenceEndInfo& info)
			{
				if (!captureComposition || ended || firstPoint.sequenceId != info.sequenceId) return false;
				ended = true;
				GuiPointSequenceEndedEventArgs arguments(captureComposition, GetPoints(captureComposition));
				static_cast<PointSequenceEndInfo&>(arguments) = info;
				arguments.accepted = true;
				if (selectedGesture) selectedGesture->OnPointSequenceEnded(arguments);
				else for (auto gesture : gestures)
				{
					gesture->OnPointSequenceEnded(arguments);
					if (gesture->IsGestureSelected())
					{
						selectedGesture = gesture.Obj();
						break;
					}
				}
				if (captureComposition->HasEventReceiver()) captureComposition->GetEventReceiver()->pointSequenceEnded.Execute(arguments);
				return true;
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
				firstPointScreen = {};
				currentPointScreen = {};
				nativePoints.Clear();
				points.Clear();
				selectedGesture = nullptr;
				ended = false;
			}

			GuiGraphicsComposition* GuiPointActionManager::GetPointCapture()
			{
				return captureComposition;
			}

			const List<Nullable<NativePoint>>& GuiPointActionManager::GetNativePoints()
			{
				return nativePoints;
			}

			const List<Nullable<Point>>& GuiPointActionManager::GetPoints(GuiGraphicsComposition* relativeTo)
			{
				points.Clear();
				for (auto point : nativePoints)
				{
					points.Add(point ? Nullable<Point>(ConvertPoint(point.Value(), relativeTo)) : Nullable<Point>());
				}
				return points;
			}

			NativePoint GuiPointActionManager::GetScreenPoint(const NativeWindowPointInfo& info)
			{
				auto origin = host->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
				return NativePoint(info.position.x + origin.x, info.position.y + origin.y);
			}

			NativePoint GuiPointActionManager::GetCurrentScreenPoint()
			{
				return currentPointScreen;
			}

			Point GuiPointActionManager::ConvertPoint(NativePoint screen, GuiGraphicsComposition* relativeTo)
			{
				auto window = host->GetNativeWindow();
				auto origin = window->GetClientBoundsInScreen().LeftTop();
				auto bounds = relativeTo ? relativeTo->GetGlobalBounds() : Rect();
				auto local = window->Convert(NativePoint(screen.x - origin.x, screen.y - origin.y));
				return Point(local.x - bounds.x1, local.y - bounds.y1);
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


			NativeWindowPointInfo GuiPointActionManager::GetFirstPoint()
			{
				auto result = firstPoint;
				auto origin = host->GetNativeWindow()->GetClientBoundsInScreen().LeftTop();
				result.position = NativePoint(firstPointScreen.x - origin.x, firstPointScreen.y - origin.y);
				return result;
			}

			Nullable<NativeMouseButton> GuiPointActionManager::GetFallbackClick()
			{
				if (!captureComposition || !ended) return {};
				if (selectedGesture) return selectedGesture->GetFallbackClick();
				for (auto gesture : gestures)
				{
					if (auto button = gesture->GetFallbackClick()) return button;
				}
				return {};
			}
		}
	}
}
