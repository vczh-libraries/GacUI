#include "GuiBasicControls.h"
#include "GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;
			using namespace reflection::description;

/***********************************************************************
GuiControl
***********************************************************************/

			void GuiControl::OnChildInserted(GuiControl* control)
			{
				GuiControl* oldParent=control->parent;
				children.Add(control);
				control->parent=this;
				control->OnParentChanged(oldParent, control->parent);
				control->UpdateVisuallyEnabled();
			}

			void GuiControl::OnChildRemoved(GuiControl* control)
			{
				GuiControl* oldParent=control->parent;
				control->parent=0;
				children.Remove(control);
				control->OnParentChanged(oldParent, control->parent);
			}

			void GuiControl::OnParentChanged(GuiControl* oldParent, GuiControl* newParent)
			{
				OnParentLineChanged();
			}

			void GuiControl::OnParentLineChanged()
			{
				for(vint i=0;i<children.Count();i++)
				{
					children[i]->OnParentLineChanged();
				}
			}

			void GuiControl::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				RenderTargetChanged.Execute(GetNotifyEventArguments());
			}

			void GuiControl::OnBeforeReleaseGraphicsHost()
			{
				for(vint i=0;i<children.Count();i++)
				{
					children[i]->OnBeforeReleaseGraphicsHost();
				}
			}

			void GuiControl::UpdateVisuallyEnabled()
			{
				bool newValue=isEnabled && (parent==0?true:parent->GetVisuallyEnabled());
				if(isVisuallyEnabled!=newValue)
				{
					isVisuallyEnabled=newValue;
					controlTemplate->SetVisuallyEnabled(isVisuallyEnabled);
					VisuallyEnabledChanged.Execute(GetNotifyEventArguments());

					for(vint i=0;i<children.Count();i++)
					{
						children[i]->UpdateVisuallyEnabled();
					}
				}
			}

			void GuiControl::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				if(focusableComposition!=value)
				{
					focusableComposition=value;
					controlTemplate->SetFocusableComposition(focusableComposition);
				}
			}

			bool GuiControl::IsAltEnabled()
			{
				GuiControl* control = this;
				while (control)
				{
					if (!control->GetVisible() || !control->GetEnabled())
					{
						return false;
					}
					control = control->GetParent();
				}

				return true;
			}

			bool GuiControl::IsAltAvailable()
			{
				return focusableComposition != 0 && alt != L"";
			}

			compositions::GuiGraphicsComposition* GuiControl::GetAltComposition()
			{
				return controlTemplate;
			}

			compositions::IGuiAltActionHost* GuiControl::GetActivatingAltHost()
			{
				return activatingAltHost;
			}

			void GuiControl::OnActiveAlt()
			{
				SetFocus();
			}

			bool GuiControl::SharedPtrDestructorProc(DescriptableObject* obj, bool forceDisposing)
			{
				GuiControl* value=dynamic_cast<GuiControl*>(obj);
				if(value->GetBoundsComposition()->GetParent())
				{
					if (!forceDisposing) return false;
				}
				SafeDeleteControl(value);
				return true;
			}

			GuiControl::GuiControl(templates::GuiControlTemplate* _controlTemplate)
				:controlTemplate(_controlTemplate)
				, boundsComposition(controlTemplate)
				, eventReceiver(controlTemplate->GetEventReceiver())
			{
				controlTemplate->Initialize();
				boundsComposition->SetAssociatedControl(this);
				containerComposition = controlTemplate->GetContainerComposition();

				RenderTargetChanged.SetAssociatedComposition(boundsComposition);
				VisibleChanged.SetAssociatedComposition(boundsComposition);
				EnabledChanged.SetAssociatedComposition(boundsComposition);
				VisuallyEnabledChanged.SetAssociatedComposition(boundsComposition);
				AltChanged.SetAssociatedComposition(boundsComposition);
				TextChanged.SetAssociatedComposition(boundsComposition);
				FontChanged.SetAssociatedComposition(boundsComposition);

				font=GetCurrentController()->ResourceService()->GetDefaultFont();
				controlTemplate->SetFont(font);
				controlTemplate->SetText(text);
				controlTemplate->SetVisuallyEnabled(isVisuallyEnabled);
				
				sharedPtrDestructorProc = &GuiControl::SharedPtrDestructorProc;
			}

			GuiControl::~GuiControl()
			{
				if (tooltipControl)
				{
					// the only legal parent is the GuiApplication::sharedTooltipWindow
					if (tooltipControl->GetBoundsComposition()->GetParent())
					{
						tooltipControl->GetBoundsComposition()->GetParent()->RemoveChild(tooltipControl->GetBoundsComposition());
					}
					delete tooltipControl;
				}

				for (vint i = 0; i < children.Count(); i++)
				{
					delete children[i];
				}
				children.Clear();

				// let the root control of a control tree delete the whole composition tree
				if (controlTemplate)
				{
					controlTemplate->SetAssociatedControl(nullptr);

					if (!parent)
					{
						delete controlTemplate;
					}
				}
			}

			compositions::GuiEventArgs GuiControl::GetNotifyEventArguments()
			{
				return GuiEventArgs(controlTemplate);
			}

			templates::GuiControlTemplate* GuiControl::GetControlTemplate()
			{
				return controlTemplate;
			}

			compositions::GuiBoundsComposition* GuiControl::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* GuiControl::GetContainerComposition()
			{
				return containerComposition;
			}

			compositions::GuiGraphicsComposition* GuiControl::GetFocusableComposition()
			{
				return focusableComposition;
			}

			compositions::GuiGraphicsEventReceiver* GuiControl::GetEventReceiver()
			{
				return eventReceiver;
			}

			GuiControl* GuiControl::GetParent()
			{
				return parent;
			}

			vint GuiControl::GetChildrenCount()
			{
				return children.Count();
			}

			GuiControl* GuiControl::GetChild(vint index)
			{
				return children[index];
			}

			bool GuiControl::AddChild(GuiControl* control)
			{
				return GetContainerComposition()->AddChild(control->GetBoundsComposition());
			}

			bool GuiControl::HasChild(GuiControl* control)
			{
				return children.Contains(control);
			}

			GuiControlHost* GuiControl::GetRelatedControlHost()
			{
				return parent?parent->GetRelatedControlHost():0;
			}

			bool GuiControl::GetVisuallyEnabled()
			{
				return isVisuallyEnabled;
			}

			bool GuiControl::GetEnabled()
			{
				return isEnabled;
			}

			void GuiControl::SetEnabled(bool value)
			{
				if(isEnabled!=value)
				{
					isEnabled=value;
					EnabledChanged.Execute(GetNotifyEventArguments());
					UpdateVisuallyEnabled();
				}
			}

			bool GuiControl::GetVisible()
			{
				return isVisible;
			}

			void GuiControl::SetVisible(bool value)
			{
				boundsComposition->SetVisible(value);
				if(isVisible!=value)
				{
					isVisible=value;
					VisibleChanged.Execute(GetNotifyEventArguments());
				}
			}

			const WString& GuiControl::GetAlt()
			{
				return alt;
			}

			bool GuiControl::SetAlt(const WString& value)
			{
				if (!IGuiAltAction::IsLegalAlt(value)) return false;
				if (alt != value)
				{
					alt = value;
					AltChanged.Execute(GetNotifyEventArguments());
				}
				return true;
			}

			void GuiControl::SetActivatingAltHost(compositions::IGuiAltActionHost* host)
			{
				activatingAltHost = host;
			}

			const WString& GuiControl::GetText()
			{
				return text;
			}

			void GuiControl::SetText(const WString& value)
			{
				if(text!=value)
				{
					text=value;
					controlTemplate->SetText(text);
					TextChanged.Execute(GetNotifyEventArguments());
				}
			}

			const FontProperties& GuiControl::GetFont()
			{
				return font;
			}

			void GuiControl::SetFont(const FontProperties& value)
			{
				if(font!=value)
				{
					font=value;
					controlTemplate->SetFont(font);
					FontChanged.Execute(GetNotifyEventArguments());
				}
			}

			void GuiControl::SetFocus()
			{
				if(focusableComposition)
				{
					GuiGraphicsHost* host=focusableComposition->GetRelatedGraphicsHost();
					if(host)
					{
						host->SetFocus(focusableComposition);
					}
				}
			}

			description::Value GuiControl::GetTag()
			{
				return tag;
			}

			void GuiControl::SetTag(const description::Value& value)
			{
				tag=value;
			}

			GuiControl* GuiControl::GetTooltipControl()
			{
				return tooltipControl;
			}

			GuiControl* GuiControl::SetTooltipControl(GuiControl* value)
			{
				GuiControl* oldTooltipControl=tooltipControl;
				tooltipControl=value;
				return oldTooltipControl;
			}

			vint GuiControl::GetTooltipWidth()
			{
				return tooltipWidth;
			}

			void GuiControl::SetTooltipWidth(vint value)
			{
				tooltipWidth=value;
			}

			bool GuiControl::DisplayTooltip(Point location)
			{
				if(!tooltipControl) return false;
				GetApplication()->ShowTooltip(this, tooltipControl, tooltipWidth, location);
				return true;
			}

			void GuiControl::CloseTooltip()
			{
				if(GetApplication()->GetTooltipOwner()==this)
				{
					GetApplication()->CloseTooltip();
				}
			}

			IDescriptable* GuiControl::QueryService(const WString& identifier)
			{
				if (identifier == IGuiAltAction::Identifier)
				{
					return (IGuiAltAction*)this;
				}
				else if (identifier == IGuiAltActionContainer::Identifier)
				{
					return 0;
				}
				else if(parent)
				{
					return parent->QueryService(identifier);
				}
				else
				{
					return 0;
				}
			}

/***********************************************************************
GuiCustomControl
***********************************************************************/

			GuiCustomControl::GuiCustomControl(templates::GuiControlTemplate* _controlTemplate)
				:GuiControl(_controlTemplate)
			{
			}

			GuiCustomControl::~GuiCustomControl()
			{
				FinalizeAggregation();
				FinalizeInstanceRecursively(this);
			}
		}
	}
}