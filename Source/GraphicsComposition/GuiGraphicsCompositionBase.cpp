#include "GuiGraphicsCompositionBase.h"
#include "../Controls/GuiWindowControls.h"
#include "../GraphicsHost/GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace controls;
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
				if(associatedControl && associatedControl!=control)
				{
					if(associatedControl->GetParent())
					{
						associatedControl->GetParent()->OnChildRemoved(associatedControl);
					}
					if(control)
					{
						control->OnChildInserted(associatedControl);
					}
				}
				else
				{
					for(vint i=0;i<children.Count();i++)
					{
						children[i]->OnControlParentChanged(control);
					}
				}
			}

			void GuiGraphicsComposition::OnChildInserted(GuiGraphicsComposition* child)
			{
				child->OnControlParentChanged(GetRelatedControl());
			}

			void GuiGraphicsComposition::OnChildRemoved(GuiGraphicsComposition* child)
			{
				child->OnControlParentChanged(0);
			}

			void GuiGraphicsComposition::OnParentChanged(GuiGraphicsComposition* oldParent, GuiGraphicsComposition* newParent)
			{
				OnParentLineChanged();
			}

			void GuiGraphicsComposition::OnParentLineChanged()
			{
				for (vint i = 0; i < children.Count(); i++)
				{
					children[i]->OnParentLineChanged();
				}
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

				for (vint i = 0; i < children.Count(); i++)
				{
					children[i]->UpdateRelatedHostRecord(record);
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
					for (vint i = 0; i < children.Count(); i++)
					{
						children[i]->OnControlParentChanged(0);
					}
				}
				associatedControl = control;
				if (associatedControl)
				{
					for (vint i = 0; i < children.Count(); i++)
					{
						children[i]->OnControlParentChanged(associatedControl);
					}
				}
			}

			void GuiGraphicsComposition::InvokeOnCompositionStateChanged()
			{
				if (relatedHostRecord)
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
			}

			GuiGraphicsComposition::~GuiGraphicsComposition()
			{
				for(vint i=0;i<children.Count();i++)
				{
					delete children[i];
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
				visible = value;
				InvokeOnCompositionStateChanged();
			}

			GuiGraphicsComposition::MinSizeLimitation GuiGraphicsComposition::GetMinSizeLimitation()
			{
				return minSizeLimitation;
			}

			void GuiGraphicsComposition::SetMinSizeLimitation(MinSizeLimitation value)
			{
				minSizeLimitation = value;
				InvokeOnCompositionStateChanged();
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
					Rect bounds = GetBounds();
					bounds.x1 += margin.left;
					bounds.y1 += margin.top;
					bounds.x2 -= margin.right;
					bounds.y2 -= margin.bottom;

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
						if (children.Count() > 0)
						{
							bounds.x1 += internalMargin.left;
							bounds.y1 += internalMargin.top;
							bounds.x2 -= internalMargin.right;
							bounds.y2 -= internalMargin.bottom;
							if (bounds.x1 <= bounds.x2 && bounds.y1 <= bounds.y2)
							{
								offset = bounds.GetSize();
								renderTarget->PushClipper(bounds);
								if (!renderTarget->IsClipperCoverWholeTarget())
								{
									for (vint i = 0; i < children.Count(); i++)
									{
										children[i]->Render(Size(bounds.x1, bounds.y1));
									}
								}
								renderTarget->PopClipper();
							}
						}
						isRendering = false;
					}
				}
			}

			GuiGraphicsEventReceiver* GuiGraphicsComposition::GetEventReceiver()
			{
				if(!eventReceiver)
				{
					eventReceiver=new GuiGraphicsEventReceiver(this);
				}
				return eventReceiver.Obj();
			}

			bool GuiGraphicsComposition::HasEventReceiver()
			{
				return eventReceiver;
			}

			GuiGraphicsComposition* GuiGraphicsComposition::FindComposition(Point location, bool forMouseEvent)
			{
				if (!visible) return 0;
				Rect bounds = GetBounds();
				Rect relativeBounds = Rect(Point(0, 0), bounds.GetSize());
				if (relativeBounds.Contains(location))
				{
					Rect clientArea = GetClientArea();
					for (vint i = children.Count() - 1; i >= 0; i--)
					{
						GuiGraphicsComposition* child = children[i];
						Rect childBounds = child->GetBounds();
						vint offsetX = childBounds.x1 + (clientArea.x1 - bounds.x1);
						vint offsetY = childBounds.y1 + (clientArea.y1 - bounds.y1);
						Point newLocation = location - Size(offsetX, offsetY);
						GuiGraphicsComposition* childResult = child->FindComposition(newLocation, forMouseEvent);
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

			Rect GuiGraphicsComposition::GetGlobalBounds()
			{
				Rect bounds = GetBounds();
				GuiGraphicsComposition* composition = parent;
				while (composition)
				{
					Rect clientArea = composition->GetClientArea();
					Rect parentBounds = composition->GetBounds();
					bounds.x1 += clientArea.x1;
					bounds.x2 += clientArea.x1;
					bounds.y1 += clientArea.y1;
					bounds.y2 += clientArea.y1;
					composition = composition->parent;
				}
				return bounds;
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

			Margin GuiGraphicsComposition::GetMargin()
			{
				return margin;
			}

			void GuiGraphicsComposition::SetMargin(Margin value)
			{
				margin = value;
				InvokeOnCompositionStateChanged();
			}

			Margin GuiGraphicsComposition::GetInternalMargin()
			{
				return internalMargin;
			}

			void GuiGraphicsComposition::SetInternalMargin(Margin value)
			{
				internalMargin = value;
				InvokeOnCompositionStateChanged();
			}

			Size GuiGraphicsComposition::GetPreferredMinSize()
			{
				return preferredMinSize;
			}

			void GuiGraphicsComposition::SetPreferredMinSize(Size value)
			{
				preferredMinSize = value;
				InvokeOnCompositionStateChanged();
			}

			Rect GuiGraphicsComposition::GetClientArea()
			{
				Rect bounds=GetBounds();
				bounds.x1+=margin.left+internalMargin.left;
				bounds.y1+=margin.top+internalMargin.top;
				bounds.x2-=margin.right+internalMargin.right;
				bounds.y2-=margin.bottom+internalMargin.bottom;
				return bounds;
			}

			void GuiGraphicsComposition::ForceCalculateSizeImmediately()
			{
				isRendering = true;
				for (vint i = 0; i < children.Count(); i++)
				{
					children[i]->ForceCalculateSizeImmediately();
				}
				isRendering = false;
				InvokeOnCompositionStateChanged();
			}

/***********************************************************************
GuiGraphicsSite
***********************************************************************/

			Rect GuiGraphicsSite::GetBoundsInternal(Rect expectedBounds)
			{
				Size minSize = GetMinPreferredClientSize();
				if (minSize.x < preferredMinSize.x) minSize.x = preferredMinSize.x;
				if (minSize.y < preferredMinSize.y) minSize.y = preferredMinSize.y;

				minSize.x += margin.left + margin.right + internalMargin.left + internalMargin.right;
				minSize.y += margin.top + margin.bottom + internalMargin.top + internalMargin.bottom;
				vint w = expectedBounds.Width();
				vint h = expectedBounds.Height();
				if (minSize.x < w) minSize.x = w;
				if (minSize.y < h) minSize.y = h;
				return Rect(expectedBounds.LeftTop(), minSize);
			}

			void GuiGraphicsSite::UpdatePreviousBounds(Rect bounds)
			{
				if (previousBounds != bounds)
				{
					previousBounds = bounds;
					BoundsChanged.Execute(GuiEventArgs(this));
					InvokeOnCompositionStateChanged();
				}
			}

			GuiGraphicsSite::GuiGraphicsSite()
			{
				BoundsChanged.SetAssociatedComposition(this);
			}

			GuiGraphicsSite::~GuiGraphicsSite()
			{
			}

			bool GuiGraphicsSite::IsSizeAffectParent()
			{
				return true;
			}

			Size GuiGraphicsSite::GetMinPreferredClientSize()
			{
				Size minSize;
				if (minSizeLimitation != GuiGraphicsComposition::NoLimit)
				{
					if (ownedElement)
					{
						IGuiGraphicsRenderer* renderer = ownedElement->GetRenderer();
						if (renderer)
						{
							minSize = renderer->GetMinSize();
						}
					}
				}
				if (minSizeLimitation == GuiGraphicsComposition::LimitToElementAndChildren)
				{
					vint childCount = Children().Count();
					for (vint i = 0; i < childCount; i++)
					{
						GuiGraphicsComposition* child = children[i];
						if (child->IsSizeAffectParent())
						{
							Rect childBounds = child->GetPreferredBounds();
							if (minSize.x < childBounds.x2) minSize.x = childBounds.x2;
							if (minSize.y < childBounds.y2) minSize.y = childBounds.y2;
						}
					}
				}
				return minSize;
			}

			Rect GuiGraphicsSite::GetPreferredBounds()
			{
				return GetBoundsInternal(Rect(Point(0, 0), GetMinPreferredClientSize()));
			}

/***********************************************************************
Helper Functions
***********************************************************************/

			void NotifyFinalizeInstance(controls::GuiControl* value)
			{
				if (value)
				{
					NotifyFinalizeInstance(value->GetBoundsComposition());
				}
			}

			void NotifyFinalizeInstance(GuiGraphicsComposition* value)
			{
				if (value)
				{
					bool finalized = false;
					if (auto root = dynamic_cast<GuiInstanceRootObject*>(value))
					{
						if (root->IsFinalized())
						{
							finalized = true;
						}
						else
						{
							root->FinalizeInstance();
						}
					}

					if (auto control = value->GetAssociatedControl())
					{
						if (auto root = dynamic_cast<GuiInstanceRootObject*>(control))
						{
							if (root->IsFinalized())
							{
								finalized = true;
							}
							else
							{
								root->FinalizeInstance();
							}
						}
					}

					if (!finalized)
					{
						vint count = value->Children().Count();
						for (vint i = 0; i < count; i++)
						{
							NotifyFinalizeInstance(value->Children()[i]);
						}
					}
				}
			}

			void SafeDeleteControlInternal(controls::GuiControl* value)
			{
				if(value)
				{
					if (value->GetRelatedControlHost() != value)
					{
						GuiGraphicsComposition* bounds = value->GetBoundsComposition();
						if (bounds->GetParent())
						{
							bounds->GetParent()->RemoveChild(bounds);
						}
					}
					delete value;
				}
			}

			void SafeDeleteCompositionInternal(GuiGraphicsComposition* value)
			{
				if (value)
				{
					if (value->GetParent())
					{
						value->GetParent()->RemoveChild(value);
					}

					if (value->GetAssociatedControl())
					{
						SafeDeleteControlInternal(value->GetAssociatedControl());
					}
					else
					{
						for (vint i = value->Children().Count() - 1; i >= 0; i--)
						{
							SafeDeleteCompositionInternal(value->Children().Get(i));
						}
						delete value;
					}
				}
			}

			void SafeDeleteControl(controls::GuiControl* value)
			{
				if (auto controlHost = dynamic_cast<controls::GuiControlHost*>(value))
				{
					controlHost->DeleteAfterProcessingAllEvents();
				}
				else
				{
					NotifyFinalizeInstance(value);
					SafeDeleteControlInternal(value);
				}
			}

			void SafeDeleteComposition(GuiGraphicsComposition* value)
			{
				NotifyFinalizeInstance(value);
				SafeDeleteCompositionInternal(value);
			}
		}
	}
}