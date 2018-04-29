#include "GuiBasicControls.h"
#include "GuiApplication.h"
#include "Templates/GuiThemeStyleFactory.h"

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

			void GuiControl::BeforeControlTemplateUninstalled()
			{
			}

			void GuiControl::AfterControlTemplateInstalled(bool initialize)
			{
				controlTemplateObject->SetText(text);
				controlTemplateObject->SetFont(font);
				controlTemplateObject->SetContext(context);
				controlTemplateObject->SetVisuallyEnabled(isVisuallyEnabled);
				controlTemplateObject->SetFocusableComposition(focusableComposition);
			}

			void GuiControl::CheckAndStoreControlTemplate(templates::GuiControlTemplate* value)
			{
				controlTemplateObject = value;
			}

			void GuiControl::EnsureControlTemplateExists()
			{
				if (!controlTemplateObject)
				{
					RebuildControlTemplate();
				}
			}

			void GuiControl::RebuildControlTemplate()
			{
				bool initialize = controlTemplateObject == nullptr;
				if (controlTemplateObject)
				{
					BeforeControlTemplateUninstalled();
					containerComposition->GetParent()->RemoveChild(containerComposition);
					boundsComposition->AddChild(containerComposition);
					SafeDeleteComposition(controlTemplateObject);
					controlTemplateObject = nullptr;
				}

				if (controlTemplate)
				{
					CheckAndStoreControlTemplate(controlTemplate({}));
				}
				else
				{
					CheckAndStoreControlTemplate(theme::GetCurrentTheme()->CreateStyle(controlThemeName)({}));
				}

				if (controlTemplateObject)
				{
					controlTemplateObject->SetAlignmentToParent(Margin(0, 0, 0, 0));

					containerComposition->GetParent()->RemoveChild(containerComposition);
					boundsComposition->AddChild(controlTemplateObject);
					controlTemplateObject->GetContainerComposition()->AddChild(containerComposition);
					AfterControlTemplateInstalled(initialize);
				}
			}

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
				bool newValue = isEnabled && (parent == 0 ? true : parent->GetVisuallyEnabled());
				if (isVisuallyEnabled != newValue)
				{
					isVisuallyEnabled = newValue;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetVisuallyEnabled(isVisuallyEnabled);
					}
					VisuallyEnabledChanged.Execute(GetNotifyEventArguments());

					for (vint i = 0; i < children.Count(); i++)
					{
						children[i]->UpdateVisuallyEnabled();
					}
				}
			}

			void GuiControl::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				if (focusableComposition != value)
				{
					focusableComposition = value;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetFocusableComposition(focusableComposition);
					}
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
				return boundsComposition;
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

			GuiControl::GuiControl(theme::ThemeName themeName)
				:controlThemeName(themeName)
			{
				{
					boundsComposition = new GuiBoundsComposition;
					boundsComposition->SetAssociatedControl(this);
					boundsComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					containerComposition = new GuiBoundsComposition;
					containerComposition->SetTransparentToMouse(true);
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					containerComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));

					boundsComposition->AddChild(containerComposition);
				}
				{
					ControlThemeNameChanged.SetAssociatedComposition(boundsComposition);
					ControlTemplateChanged.SetAssociatedComposition(boundsComposition);
					RenderTargetChanged.SetAssociatedComposition(boundsComposition);
					VisibleChanged.SetAssociatedComposition(boundsComposition);
					EnabledChanged.SetAssociatedComposition(boundsComposition);
					VisuallyEnabledChanged.SetAssociatedComposition(boundsComposition);
					AltChanged.SetAssociatedComposition(boundsComposition);
					TextChanged.SetAssociatedComposition(boundsComposition);
					FontChanged.SetAssociatedComposition(boundsComposition);
					ContextChanged.SetAssociatedComposition(boundsComposition);
				}
				font = GetCurrentController()->ResourceService()->GetDefaultFont();
				sharedPtrDestructorProc = &GuiControl::SharedPtrDestructorProc;
			}

			GuiControl::~GuiControl()
			{
				// prevent a root bounds composition from notifying its dead controls
				if (!parent)
				{
					NotifyFinalizeInstance(boundsComposition);
				}

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
				if (!parent)
				{
					delete boundsComposition;
				}
			}

			compositions::GuiEventArgs GuiControl::GetNotifyEventArguments()
			{
				return GuiEventArgs(boundsComposition);
			}

			theme::ThemeName GuiControl::GetControlThemeName()
			{
				return controlThemeName;
			}

			void GuiControl::SetControlThemeName(theme::ThemeName value)
			{
				if (controlThemeName != value)
				{
					controlThemeName = value;
					if (!controlTemplate)
					{
						RebuildControlTemplate();
					}
					ControlThemeNameChanged.Execute(GetNotifyEventArguments());
				}
			}

			GuiControl::ControlTemplatePropertyType GuiControl::GetControlTemplate()
			{
				return controlTemplate;
			}

			void GuiControl::SetControlTemplate(const ControlTemplatePropertyType& value)
			{
				controlTemplate = value;
				RebuildControlTemplate();
				ControlTemplateChanged.Execute(GetNotifyEventArguments());
			}

			void GuiControl::SetControlThemeNameAndTemplate(theme::ThemeName themeNameValue, const ControlTemplatePropertyType& controlTemplateValue)
			{
				if (controlThemeName == themeNameValue && !controlTemplate && !controlTemplateValue)
				{
					return;
				}

				controlThemeName = themeNameValue;
				controlTemplate = controlTemplateValue;
				RebuildControlTemplate();
				ControlThemeNameChanged.Execute(GetNotifyEventArguments());
				ControlTemplateChanged.Execute(GetNotifyEventArguments());
			}

			templates::GuiControlTemplate* GuiControl::GetControlTemplateObject()
			{
				EnsureControlTemplateExists();
				return controlTemplateObject;
			}

			compositions::GuiBoundsComposition* GuiControl::GetBoundsComposition()
			{
				EnsureControlTemplateExists();
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* GuiControl::GetContainerComposition()
			{
				EnsureControlTemplateExists();
				return containerComposition;
			}

			compositions::GuiGraphicsComposition* GuiControl::GetFocusableComposition()
			{
				EnsureControlTemplateExists();
				return focusableComposition;
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
				if (text != value)
				{
					text = value;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetText(text);
					}
					TextChanged.Execute(GetNotifyEventArguments());
				}
			}

			const FontProperties& GuiControl::GetFont()
			{
				return font;
			}

			void GuiControl::SetFont(const FontProperties& value)
			{
				if (font != value)
				{
					font = value;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetFont(font);
					}
					FontChanged.Execute(GetNotifyEventArguments());
				}
			}
			
			description::Value GuiControl::GetContext()
			{
				return context;
			}

			void GuiControl::SetContext(const description::Value& value)
			{
				if (context != value)
				{
					context = value;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetContext(context);
					}
					ContextChanged.Execute(GetNotifyEventArguments());
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

			controls::GuiControlHost* GuiCustomControl::GetControlHostForInstance()
			{
				return GetRelatedControlHost();
			}

			void GuiCustomControl::OnParentLineChanged()
			{
				GuiControl::OnParentLineChanged();
				OnControlHostForInstanceChanged();
			}

			GuiCustomControl::GuiCustomControl(theme::ThemeName themeName)
				:GuiControl(themeName)
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