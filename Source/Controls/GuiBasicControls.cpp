#include "GuiBasicControls.h"
#include "GuiApplication.h"
#include "Templates/GuiThemeStyleFactory.h"
#include "../GraphicsHost/GuiGraphicsHost.h"

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
GuiDisposedFlag
***********************************************************************/

			void GuiDisposedFlag::SetDisposed()
			{
				disposed = true;
			}

			GuiDisposedFlag::GuiDisposedFlag(GuiControl* _owner)
				:owner(_owner)
			{
			}

			GuiDisposedFlag::~GuiDisposedFlag()
			{
			}

			bool GuiDisposedFlag::IsDisposed()
			{
				return disposed;
			}

/***********************************************************************
GuiControl
***********************************************************************/

			Ptr<GuiDisposedFlag> GuiControl::GetDisposedFlag()
			{
				if (!disposedFlag)
				{
					disposedFlag = new GuiDisposedFlag(this);
				}
				return disposedFlag;
			}

			void GuiControl::BeforeControlTemplateUninstalled()
			{
			}

			void GuiControl::AfterControlTemplateInstalled(bool initialize)
			{
				controlTemplateObject->SetText(text);
				controlTemplateObject->SetFont(displayFont);
				controlTemplateObject->SetContext(context);
				controlTemplateObject->SetVisuallyEnabled(isVisuallyEnabled);
				controlTemplateObject->SetFocusableComposition(focusableComposition);
				controlTemplateObject->SetFocused(isFocused);
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
				control->UpdateDisplayFont();

				if (auto host = boundsComposition->GetRelatedGraphicsHost())
				{
					host->InvalidateTabOrderCache();
				}
			}

			void GuiControl::OnChildRemoved(GuiControl* control)
			{
				GuiControl* oldParent=control->parent;
				control->parent=0;
				children.Remove(control);
				control->OnParentChanged(oldParent, control->parent);

				if (auto host = boundsComposition->GetRelatedGraphicsHost())
				{
					host->InvalidateTabOrderCache();
				}
			}

			void GuiControl::OnParentChanged(GuiControl* oldParent, GuiControl* newParent)
			{
				OnParentLineChanged();
			}

			void GuiControl::OnParentLineChanged()
			{
				{
					GuiControlSignalEventArgs arguments(boundsComposition);
					arguments.controlSignal = ControlSignal::ParentLineChanged;
					ControlSignalTrigerred.Execute(arguments);
				}
				for(vint i=0;i<children.Count();i++)
				{
					children[i]->OnParentLineChanged();
				}
			}

			void GuiControl::OnServiceAdded()
			{
				{
					GuiControlSignalEventArgs arguments(boundsComposition);
					arguments.controlSignal = ControlSignal::ServiceAdded;
					ControlSignalTrigerred.Execute(arguments);
				}
				for(vint i=0;i<children.Count();i++)
				{
					children[i]->OnParentLineChanged();
				}
			}

			void GuiControl::OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget)
			{
				GuiControlSignalEventArgs arguments(boundsComposition);
				arguments.controlSignal = ControlSignal::RenderTargetChanged;
				ControlSignalTrigerred.Execute(arguments);
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

			void GuiControl::UpdateDisplayFont()
			{
				auto newValue =
					font ? font.Value() :
					parent ? parent->GetDisplayFont() :
					GetCurrentController()->ResourceService()->GetDefaultFont();

				if (displayFont != newValue)
				{
					displayFont = newValue;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetFont(displayFont);
					}
					DisplayFontChanged.Execute(GetNotifyEventArguments());

					for (vint i = 0; i < children.Count(); i++)
					{
						children[i]->UpdateDisplayFont();
					}
				}
			}

			void GuiControl::OnGotFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (!isFocused)
				{
					isFocused = true;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetFocused(true);
					}
					FocusedChanged.Execute(GetNotifyEventArguments());
				}
			}

			void GuiControl::OnLostFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (isFocused)
				{
					isFocused = false;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetFocused(false);
					}
					FocusedChanged.Execute(GetNotifyEventArguments());
				}
			}

			void GuiControl::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				if (focusableComposition != value)
				{
					if (focusableComposition)
					{
						focusableComposition->GetEventReceiver()->gotFocus.Detach(gotFocusHandler);
						focusableComposition->GetEventReceiver()->lostFocus.Detach(lostFocusHandler);

						gotFocusHandler = nullptr;
						lostFocusHandler = nullptr;
					}

					focusableComposition = value;
					if (controlTemplateObject)
					{
						controlTemplateObject->SetFocusableComposition(focusableComposition);
					}

					if (focusableComposition)
					{
						gotFocusHandler = focusableComposition->GetEventReceiver()->gotFocus.AttachMethod(this, &GuiControl::OnGotFocus);
						lostFocusHandler = focusableComposition->GetEventReceiver()->lostFocus.AttachMethod(this, &GuiControl::OnLostFocus);
					}
					else
					{
						GuiEventArgs arguments(boundsComposition);
						OnLostFocus(boundsComposition, arguments);
					}
				}
			}

			bool GuiControl::IsControlVisibleAndEnabled()
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

			bool GuiControl::IsAltEnabled()
			{
				return IsControlVisibleAndEnabled();
			}

			bool GuiControl::IsAltAvailable()
			{
				return focusableComposition != nullptr && alt != L"";
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

			bool GuiControl::IsTabEnabled()
			{
				return IsControlVisibleAndEnabled();
			}

			bool GuiControl::IsTabAvailable()
			{
				return focusableComposition != nullptr;
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
				, displayFont(GetCurrentController()->ResourceService()->GetDefaultFont())
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
					ControlSignalTrigerred.SetAssociatedComposition(boundsComposition);
					VisibleChanged.SetAssociatedComposition(boundsComposition);
					EnabledChanged.SetAssociatedComposition(boundsComposition);
					FocusedChanged.SetAssociatedComposition(boundsComposition);
					VisuallyEnabledChanged.SetAssociatedComposition(boundsComposition);
					DisplayFontChanged.SetAssociatedComposition(boundsComposition);
					AltChanged.SetAssociatedComposition(boundsComposition);
					TextChanged.SetAssociatedComposition(boundsComposition);
					FontChanged.SetAssociatedComposition(boundsComposition);
					ContextChanged.SetAssociatedComposition(boundsComposition);
				}
				sharedPtrDestructorProc = &GuiControl::SharedPtrDestructorProc;
			}

			GuiControl::~GuiControl()
			{
				if (disposedFlag)
				{
					disposedFlag->SetDisposed();
				}
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

			void GuiControl::InvokeOrDelayIfRendering(Func<void()> proc)
			{
				auto controlHost = GetRelatedControlHost();
				if (controlHost && boundsComposition->IsRendering())
				{
					auto flag = GetDisposedFlag();
					GetApplication()->InvokeInMainThread(controlHost, [=]()
					{
						if (!flag->IsDisposed())
						{
							proc();
						}
					});
				}
				else
				{
					proc();
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
				SetControlThemeNameAndTemplate(value, controlTemplate);
			}

			GuiControl::ControlTemplatePropertyType GuiControl::GetControlTemplate()
			{
				return controlTemplate;
			}

			void GuiControl::SetControlTemplate(const ControlTemplatePropertyType& value)
			{
				SetControlThemeNameAndTemplate(controlThemeName, value);
			}

			void GuiControl::SetControlThemeNameAndTemplate(theme::ThemeName themeNameValue, const ControlTemplatePropertyType& controlTemplateValue)
			{
				bool themeChanged = (controlThemeName != themeNameValue);
				bool templateChanged = (controlTemplate || controlTemplateValue);
				if (themeChanged || templateChanged)
				{
					controlThemeName = themeNameValue;
					controlTemplate = controlTemplateValue;
					RebuildControlTemplate();
					
					if (themeChanged)
					{
						ControlThemeNameChanged.Execute(GetNotifyEventArguments());
					}
					if (templateChanged)
					{
						ControlTemplateChanged.Execute(GetNotifyEventArguments());
					}
				}
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

			bool GuiControl::GetFocused()
			{
				return isFocused;
			}

			bool GuiControl::GetAcceptTabInput()
			{
				return acceptTabInput;
			}

			void GuiControl::SetAcceptTabInput(bool value)
			{
				acceptTabInput = value;
			}

			vint GuiControl::GetTabPriority()
			{
				return tabPriority;
			}

			void GuiControl::SetTabPriority(vint value)
			{
				vint newTabPriority = value < 0 ? -1 : value;
				if (tabPriority != newTabPriority)
				{
					tabPriority = newTabPriority;
					if (auto host = boundsComposition->GetRelatedGraphicsHost())
					{
						host->InvalidateTabOrderCache();
					}
				}
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

			const Nullable<FontProperties>& GuiControl::GetFont()
			{
				return font;
			}

			void GuiControl::SetFont(const Nullable<FontProperties>& value)
			{
				if (font != value)
				{
					font = value;
					FontChanged.Execute(GetNotifyEventArguments());
					UpdateDisplayFont();
				}
			}

			const FontProperties& GuiControl::GetDisplayFont()
			{
				return displayFont;
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
				if (focusableComposition)
				{
					if (auto host = focusableComposition->GetRelatedGraphicsHost())
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
					return nullptr;
				}
				else if (identifier == IGuiTabAction::Identifier)
				{
					return (IGuiTabAction*)this;
				}
				else
				{
					vint index = controlServices.Keys().IndexOf(identifier);
					if (index != -1)
					{
						return controlServices.Values()[index].Obj();
					}

					if (parent)
					{
						return parent->QueryService(identifier);
					}
				}
				return nullptr;
			}

			bool GuiControl::AddService(const WString& identifier, Ptr<IDescriptable> value)
			{
				if (controlServices.Keys().Contains(identifier))
				{
					return false;
				}

				controlServices.Add(identifier, value);
				OnServiceAdded();
				return true;
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