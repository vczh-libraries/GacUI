#include "GuiGraphicsComposition.h"
#include "../GraphicsHost/GuiGraphicsHost.h"
#include "../Controls/GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace elements;

			void InvokeOnCompositionStateChanged(compositions::GuiGraphicsComposition* composition)
			{
				composition->InvokeOnCompositionStateChanged();
			}

/***********************************************************************
GuiGraphicsComposition
***********************************************************************/

			void GuiGraphicsComposition::OnControlParentChanged(controls::GuiControl* control)
			{
				if (associatedControl && associatedControl != control)
				{
					if (associatedControl->GetParent())
					{
						associatedControl->GetParent()->OnChildRemoved(associatedControl);
					}
					if (control)
					{
						control->OnChildInserted(associatedControl);
					}
				}
				else
				{
					for (auto child : children)
					{
						child->OnControlParentChanged(control);
					}
				}
			}

			void GuiGraphicsComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				child->OnControlParentChanged(GetRelatedControl());
			}

			void GuiGraphicsComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				child->OnControlParentChanged(nullptr);
			}

			void GuiGraphicsComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				OnParentLineChanged();
			}

			void GuiGraphicsComposition::OnParentLineChanged()
			{
				for (auto child : children)
				{
					child->OnParentLineChanged();
				}
			}

			void GuiGraphicsComposition::OnCompositionStateChanged()
			{
			}

			void GuiGraphicsComposition::OnRenderContextChanged()
			{
			}

			void GuiGraphicsComposition::UpdateRelatedHostRecord(GraphicsHostRecord* record)
			{
				relatedHostRecord = record;
				auto renderTarget = GetRenderTarget();

				if (ownedElement)
				{
					if (auto renderer = ownedElement->GetRenderer())
					{
						renderer->SetRenderTarget(renderTarget);
					}
				}

				for (auto child : children)
				{
					child->UpdateRelatedHostRecord(record);
				}

				if (HasEventReceiver())
				{
					GetEventReceiver()->renderTargetChanged.Execute(GuiEventArgs(this));
				}
				if (associatedControl)
				{
					associatedControl->OnRenderTargetChanged(renderTarget);
				}

				OnRenderContextChanged();
			}

			void GuiGraphicsComposition::SetAssociatedControl(controls::GuiControl* control)
			{
				if (associatedControl)
				{
					for (auto child : children)
					{
						child->OnControlParentChanged(nullptr);
					}
				}
				associatedControl = control;
				if (associatedControl)
				{
					for (auto child : children)
					{
						child->OnControlParentChanged(associatedControl);
					}
				}
			}

			void GuiGraphicsComposition::InvokeOnCompositionStateChanged(bool forceRequestRender)
			{
				OnCompositionStateChanged();
				if (relatedHostRecord && (forceRequestRender || GetEventuallyVisible()))
				{
					relatedHostRecord->host->RequestRender();
				}
			}

			bool GuiGraphicsComposition::SharedPtrDestructorProc(DescriptableObject* obj, bool forceDisposing)
			{
				GuiGraphicsComposition* value=dynamic_cast<GuiGraphicsComposition*>(obj);
				if(value->parent)
				{
					if (!forceDisposing) return false;
				}
				SafeDeleteComposition(value);
				return true;
			}

			GuiGraphicsComposition::GuiGraphicsComposition()
			{
				sharedPtrDestructorProc = &GuiGraphicsComposition::SharedPtrDestructorProc;
				CachedMinSizeChanged.SetAssociatedComposition(this);
				CachedBoundsChanged.SetAssociatedComposition(this);
			}

			GuiGraphicsComposition::~GuiGraphicsComposition()
			{
				for (auto child : children)
				{
					delete child;
				}
			}

			bool GuiGraphicsComposition::IsRendering()
			{
				return isRendering;
			}

			GuiGraphicsComposition* GuiGraphicsComposition::GetParent()
			{
				return parent;
			}

			const GuiGraphicsComposition::CompositionList& GuiGraphicsComposition::Children()
			{
				return children;
			}

			bool GuiGraphicsComposition::AddChild(GuiGraphicsComposition* child)
			{
				return InsertChild(children.Count(), child);
			}

			bool GuiGraphicsComposition::InsertChild(vint index, GuiGraphicsComposition* child)
			{
				CHECK_ERROR(!isRendering, L"GuiGraphicsComposition::InsertChild(vint, GuiGraphicsComposition*)#Cannot modify composition tree during rendering.");
				if (!child) return false;
				if (child->GetParent()) return false;
				children.Insert(index, child);

				// composition parent changed -> control parent changed -> related host changed
				child->parent = this;
				child->OnParentChanged(nullptr, this);
				OnChildInserted(child);
				child->UpdateRelatedHostRecord(relatedHostRecord);

				InvokeOnCompositionStateChanged();
				return true;
			}

			bool GuiGraphicsComposition::RemoveChild(GuiGraphicsComposition* child)
			{
				CHECK_ERROR(!isRendering, L"GuiGraphicsComposition::InsertChild(vint, GuiGraphicsComposition*)#Cannot modify composition tree during rendering.");
				if (!child) return false;
				vint index = children.IndexOf(child);
				if (index == -1) return false;

				// composition parent changed -> control parent changed -> related host changed
				child->parent = nullptr;
				child->OnParentChanged(this, nullptr);
				OnChildRemoved(child);
				child->UpdateRelatedHostRecord(nullptr);

				GuiGraphicsHost* host = GetRelatedGraphicsHost();
				if (host)
				{
					host->DisconnectComposition(child);
				}
				children.RemoveAt(index);
				InvokeOnCompositionStateChanged();
				return true;
			}

			bool GuiGraphicsComposition::MoveChild(GuiGraphicsComposition* child, vint newIndex)
			{
				if(!child) return false;
				vint index=children.IndexOf(child);
				if(index==-1) return false;
				if(index==newIndex) return true;
				children.RemoveAt(index);
				children.Insert(newIndex, child);
				InvokeOnCompositionStateChanged();
				return true;
			}

			Ptr<IGuiGraphicsElement> GuiGraphicsComposition::GetOwnedElement()
			{
				return ownedElement;
			}

			void GuiGraphicsComposition::SetOwnedElement(Ptr<IGuiGraphicsElement> element)
			{
				if (ownedElement != element)
				{
					if (ownedElement)
					{
						if (auto renderer = ownedElement->GetRenderer())
						{
							renderer->SetRenderTarget(nullptr);
						}
						ownedElement->SetOwnerComposition(nullptr);
					}
					ownedElement = element;
					if (ownedElement)
					{
						if (auto renderer = ownedElement->GetRenderer())
						{
							renderer->SetRenderTarget(GetRenderTarget());
						}
						ownedElement->SetOwnerComposition(this);
					}
					InvokeOnCompositionStateChanged();
				}
			}

			bool GuiGraphicsComposition::GetVisible()
			{
				return visible;
			}

			void GuiGraphicsComposition::SetVisible(bool value)
			{
				if (visible != value)
				{
					visible = value;
					InvokeOnCompositionStateChanged(true);
				}
			}

			bool GuiGraphicsComposition::GetEventuallyVisible()
			{
				auto current = this;
				while (current)
				{
					if (!current->visible) return false;
					current = current->parent;
				}
				return true;
			}

			GuiGraphicsComposition::MinSizeLimitation GuiGraphicsComposition::GetMinSizeLimitation()
			{
				return minSizeLimitation;
			}

			void GuiGraphicsComposition::SetMinSizeLimitation(MinSizeLimitation value)
			{
				if (minSizeLimitation != value)
				{
					minSizeLimitation = value;
					InvokeOnCompositionStateChanged();
				}
			}

			elements::IGuiGraphicsRenderTarget* GuiGraphicsComposition::GetRenderTarget()
			{
				return relatedHostRecord ? relatedHostRecord->renderTarget : nullptr;
			}

			void GuiGraphicsComposition::Render(Size offset)
			{
				auto renderTarget = GetRenderTarget();
				if (visible && renderTarget && !renderTarget->IsClipperCoverWholeTarget())
				{
					Rect bounds = GetCachedBounds();
					if (bounds.x1 <= bounds.x2 && bounds.y1 <= bounds.y2)
					{
						bounds.x1 += offset.x;
						bounds.x2 += offset.x;
						bounds.y1 += offset.y;
						bounds.y2 += offset.y;

						isRendering = true;
						if (ownedElement)
						{
							IGuiGraphicsRenderer* renderer = ownedElement->GetRenderer();
							if (renderer)
							{
								renderer->Render(bounds);
							}
						}

						if (children.Count() > 0 || associatedHitTestResult != INativeWindowListener::NoDecision || associatedCursor)
						{
							renderTarget->PushClipper(bounds, this);
							if (!renderTarget->IsClipperCoverWholeTarget())
							{
								for (auto child : children)
								{
									child->Render(Size(bounds.x1, bounds.y1));
								}
							}
							renderTarget->PopClipper(this);
						}
						isRendering = false;
					}
				}
			}

			GuiGraphicsEventReceiver* GuiGraphicsComposition::GetEventReceiver()
			{
				if(!eventReceiver)
				{
					eventReceiver=Ptr(new GuiGraphicsEventReceiver(this));
				}
				return eventReceiver.Obj();
			}

			bool GuiGraphicsComposition::HasEventReceiver()
			{
				return eventReceiver;
			}

			GuiGraphicsComposition* GuiGraphicsComposition::FindVisibleComposition(Point location, bool forMouseEvent)
			{
				if (!visible) return 0;
				Rect bounds = GetCachedBounds();
				Rect relativeBounds = Rect(Point(0, 0), bounds.GetSize());
				if (relativeBounds.Contains(location))
				{
					// TODO: (enumerable) foreach:reversed
					for (vint i = children.Count() - 1; i >= 0; i--)
					{
						GuiGraphicsComposition* child = children[i];
						Rect childBounds = child->GetCachedBounds();
						Point newLocation = location - Size(childBounds.x1, childBounds.y1);
						GuiGraphicsComposition* childResult = child->FindVisibleComposition(newLocation, forMouseEvent);
						if (childResult)
						{
							return childResult;
						}
					}

					if (!forMouseEvent || !transparentToMouse)
					{
						return this;
					}
				}
				return nullptr;
			}

			bool GuiGraphicsComposition::GetTransparentToMouse()
			{
				return transparentToMouse;
			}

			void GuiGraphicsComposition::SetTransparentToMouse(bool value)
			{
				transparentToMouse = value;
			}

			controls::GuiControl* GuiGraphicsComposition::GetAssociatedControl()
			{
				return associatedControl;
			}

			GuiGraphicsHost* GuiGraphicsComposition::GetAssociatedHost()
			{
				if (relatedHostRecord && relatedHostRecord->host->GetMainComposition() == this)
				{
					return relatedHostRecord->host;
				}
				else
				{
					return nullptr;
				}
			}

			INativeCursor* GuiGraphicsComposition::GetAssociatedCursor()
			{
				return associatedCursor;
			}

			void GuiGraphicsComposition::SetAssociatedCursor(INativeCursor* cursor)
			{
				associatedCursor = cursor;
			}

			INativeWindowListener::HitTestResult GuiGraphicsComposition::GetAssociatedHitTestResult()
			{
				return associatedHitTestResult;
			}

			void GuiGraphicsComposition::SetAssociatedHitTestResult(INativeWindowListener::HitTestResult value)
			{
				associatedHitTestResult = value;
			}

			controls::GuiControl* GuiGraphicsComposition::GetRelatedControl()
			{
				GuiGraphicsComposition* composition = this;
				while (composition)
				{
					if (composition->GetAssociatedControl())
					{
						return composition->GetAssociatedControl();
					}
					else
					{
						composition = composition->GetParent();
					}
				}
				return nullptr;
			}

			GuiGraphicsHost* GuiGraphicsComposition::GetRelatedGraphicsHost()
			{
				return relatedHostRecord ? relatedHostRecord->host : nullptr;
			}

			controls::GuiControlHost* GuiGraphicsComposition::GetRelatedControlHost()
			{
				if (auto control = GetRelatedControl())
				{
					return control->GetRelatedControlHost();
				}
				return nullptr;
			}

			INativeCursor* GuiGraphicsComposition::GetRelatedCursor()
			{
				GuiGraphicsComposition* composition = this;
				while (composition)
				{
					if (composition->GetAssociatedCursor())
					{
						return composition->GetAssociatedCursor();
					}
					else
					{
						composition = composition->GetParent();
					}
				}
				return nullptr;
			}

			INativeWindowListener::HitTestResult GuiGraphicsComposition::GetRelatedHitTestResult()
			{
				GuiGraphicsComposition* composition = this;
				while (composition)
				{
					INativeWindowListener::HitTestResult result = composition->GetAssociatedHitTestResult();
					if (result == INativeWindowListener::NoDecision)
					{
						composition = composition->GetParent();
					}
					else
					{
						return result;
					}
				}
				return INativeWindowListener::NoDecision;
			}

			Margin GuiGraphicsComposition::GetInternalMargin()
			{
				return internalMargin;
			}

			void GuiGraphicsComposition::SetInternalMargin(Margin value)
			{
				if (value.left < 0) value.left = 0;
				if (value.top < 0) value.top = 0;
				if (value.right < 0) value.right = 0;
				if (value.bottom < 0) value.bottom = 0;

				if (internalMargin != value)
				{
					internalMargin = value;
					InvokeOnCompositionStateChanged();
				}
			}

			Size GuiGraphicsComposition::GetPreferredMinSize()
			{
				return preferredMinSize;
			}

			void GuiGraphicsComposition::SetPreferredMinSize(Size value)
			{
				if (value.x < 0) value.x = 0;
				if (value.y < 0) value.y = 0;

				if (preferredMinSize != value)
				{
					preferredMinSize = value;
					InvokeOnCompositionStateChanged();
				}
			}
		}
	}
}