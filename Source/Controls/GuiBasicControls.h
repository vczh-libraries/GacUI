/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIBASICCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIBASICCONTROLS

#include "../GraphicsElement/GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		template<typename T>
		using ItemProperty = Func<T(const reflection::description::Value&)>;

		template<typename T>
		using WritableItemProperty = Func<T(const reflection::description::Value&, T, bool)>;

		template<typename T>
		using TemplateProperty = Func<T*(const reflection::description::Value&)>;

		namespace controls
		{

/***********************************************************************
Basic Construction
***********************************************************************/

			/// <summary>
			/// The base class of all controls.
			/// When the control is destroyed, it automatically destroys sub controls, and the bounds composition from the style controller.
			/// If you want to manually destroy a control, you should first remove it from its parent.
			/// The only way to remove a control from a parent control, is to remove the bounds composition from its parent composition. The same to inserting a control.
			/// </summary>
			class GuiControl : public Object, protected compositions::IGuiAltAction, public Description<GuiControl>
			{
				friend class compositions::GuiGraphicsComposition;
				typedef collections::List<GuiControl*>		ControlList;
			public:
				/// <summary>
				/// Represents a style for a control. A style is something like a skin, but contains some default action handlers.
				/// </summary>
				class IStyleController : public virtual IDescriptable, public Description<IStyleController>
				{
				public:
					/// <summary>Get the bounds composition. A bounds composition represents all visible contents of a control.</summary>
					/// <returns>The bounds composition.</returns>
					virtual compositions::GuiBoundsComposition*		GetBoundsComposition()=0;
					/// <summary>Get the container composition. A container composition is where to place all bounds compositions for child controls.</summary>
					/// <returns>The container composition.</returns>
					virtual compositions::GuiGraphicsComposition*	GetContainerComposition()=0;
					/// <summary>Set the focusable composition. A focusable composition is the composition to be focused when the control is focused.</summary>
					/// <param name="value">The focusable composition.</param>
					virtual void									SetFocusableComposition(compositions::GuiGraphicsComposition* value)=0;
					/// <summary>Set the text to display on the control.</summary>
					/// <param name="value">The text to display.</param>
					virtual void									SetText(const WString& value)=0;
					/// <summary>Set the font to render the text.</summary>
					/// <param name="value">The font to render the text.</param>
					virtual void									SetFont(const FontProperties& value)=0;
					/// <summary>Set the enableing state to affect the rendering of the control.</summary>
					/// <param name="value">The enableing state.</param>
					virtual void									SetVisuallyEnabled(bool value)=0;
				};

				/// <summary>
				/// An empty style for <see cref="GuiControl"/>.
				/// </summary>
				class EmptyStyleController : public Object, public virtual IStyleController, public Description<EmptyStyleController>
				{
				protected:
					compositions::GuiBoundsComposition*				boundsComposition;
				public:
					EmptyStyleController();
					~EmptyStyleController();

					compositions::GuiBoundsComposition*				GetBoundsComposition()override;
					compositions::GuiGraphicsComposition*			GetContainerComposition()override;
					void											SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
					void											SetText(const WString& value)override;
					void											SetFont(const FontProperties& value)override;
					void											SetVisuallyEnabled(bool value)override;
				};

				/// <summary>
				/// A style provider is a callback interface for some control that already provides a style controller, but the controller need callbacks to create sub compositions or handle actions.
				/// </summary>
				class IStyleProvider : public virtual IDescriptable, public Description<IStyleProvider>
				{
				public:
					/// <summary>Called when a style provider is associated with a style controller.</summary>
					/// <param name="controller">The style controller that is associated.</param>
					virtual void								AssociateStyleController(IStyleController* controller)=0;
					/// <summary>Set the focusable composition. A focusable composition is the composition to be focused when the control is focused.</summary>
					/// <param name="value">The focusable composition.</param>
					virtual void								SetFocusableComposition(compositions::GuiGraphicsComposition* value)=0;
					/// <summary>Set the text to display on the control.</summary>
					/// <param name="value">The text to display.</param>
					virtual void								SetText(const WString& value)=0;
					/// <summary>Set the font to render the text.</summary>
					/// <param name="value">The font to render the text.</param>
					virtual void								SetFont(const FontProperties& value)=0;
					/// <summary>Set the enableing state to affect the rendering of the control.</summary>
					/// <param name="value">The enableing state.</param>
					virtual void								SetVisuallyEnabled(bool value)=0;
				};
			protected:
				Ptr<IStyleController>							styleController;
				compositions::GuiBoundsComposition*				boundsComposition;
				compositions::GuiGraphicsComposition*			focusableComposition;
				compositions::GuiGraphicsEventReceiver*			eventReceiver;

				bool									isEnabled;
				bool									isVisuallyEnabled;
				bool									isVisible;
				WString									alt;
				WString									text;
				FontProperties							font;
				compositions::IGuiAltActionHost*		activatingAltHost;

				GuiControl*								parent;
				ControlList								children;
				description::Value						tag;
				GuiControl*								tooltipControl;
				vint									tooltipWidth;

				virtual void							OnChildInserted(GuiControl* control);
				virtual void							OnChildRemoved(GuiControl* control);
				virtual void							OnParentChanged(GuiControl* oldParent, GuiControl* newParent);
				virtual void							OnParentLineChanged();
				virtual void							OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget);
				virtual void							OnBeforeReleaseGraphicsHost();
				virtual void							UpdateVisuallyEnabled();
				void									SetFocusableComposition(compositions::GuiGraphicsComposition* value);

				bool									IsAltEnabled()override;
				bool									IsAltAvailable()override;
				compositions::GuiGraphicsComposition*	GetAltComposition()override;
				compositions::IGuiAltActionHost*		GetActivatingAltHost()override;
				void									OnActiveAlt()override;

				static bool								SharedPtrDestructorProc(DescriptableObject* obj, bool forceDisposing);

			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiControl(IStyleController* _styleController);
				~GuiControl();

				/// <summary>Visible event. This event will be raised when the visibility state of the control is changed.</summary>
				compositions::GuiNotifyEvent			VisibleChanged;
				/// <summary>Enabled event. This event will be raised when the enabling state of the control is changed.</summary>
				compositions::GuiNotifyEvent			EnabledChanged;
				/// <summary>
				/// Enabled event. This event will be raised when the visually enabling state of the control is changed. A visually enabling is combined by the enabling state and the parent's visually enabling state.
				/// A control is rendered as disabled, not only when the control itself is disabled, but also when the parent control is rendered as disabled.
				/// </summary>
				compositions::GuiNotifyEvent			VisuallyEnabledChanged;
				/// <summary>Alt changed event. This event will be raised when the associated Alt-combined shortcut key of the control is changed.</summary>
				compositions::GuiNotifyEvent			AltChanged;
				/// <summary>Text changed event. This event will be raised when the text of the control is changed.</summary>
				compositions::GuiNotifyEvent			TextChanged;
				/// <summary>Font changed event. This event will be raised when the font of the control is changed.</summary>
				compositions::GuiNotifyEvent			FontChanged;

				/// <summary>A function to create the argument for notify events that raised by itself.</summary>
				/// <returns>The created argument.</returns>
				compositions::GuiEventArgs				GetNotifyEventArguments();
				/// <summary>Get the associated style controller.</summary>
				/// <returns>The associated style controller.</returns>
				IStyleController*						GetStyleController();
				/// <summary>Get the bounds composition for the control. The value is from <see cref="IStyleController::GetBoundsComposition"/>.</summary>
				/// <returns>The bounds composition.</returns>
				compositions::GuiBoundsComposition*		GetBoundsComposition();
				/// <summary>Get the container composition for the control. The value is from <see cref="IStyleController::GetContainerComposition"/>.</summary>
				/// <returns>The container composition.</returns>
				compositions::GuiGraphicsComposition*	GetContainerComposition();
				/// <summary>Get the focusable composition for the control. A focusable composition is the composition to be focused when the control is focused.</summary>
				/// <returns>The focusable composition.</returns>
				compositions::GuiGraphicsComposition*	GetFocusableComposition();
				/// <summary>Get the event receiver from the bounds composition.</summary>
				/// <returns>The event receiver.</returns>
				compositions::GuiGraphicsEventReceiver*	GetEventReceiver();
				/// <summary>Get the parent control.</summary>
				/// <returns>The parent control.</returns>
				GuiControl*								GetParent();
				/// <summary>Get the number of child controls.</summary>
				/// <returns>The number of child controls.</returns>
				vint									GetChildrenCount();
				/// <summary>Get the child control using a specified index.</summary>
				/// <returns>The child control.</returns>
				/// <param name="index">The specified index.</param>
				GuiControl*								GetChild(vint index);
				/// <summary>Put another control in the container composition of this control.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="control">The control to put in this control.</param>
				bool									AddChild(GuiControl* control);
				/// <summary>Test if a control owned by this control.</summary>
				/// <returns>Returns true if the control is owned by this control.</returns>
				/// <param name="control">The control to test.</param>
				bool									HasChild(GuiControl* control);
				
				/// <summary>Get the <see cref="GuiControlHost"/> that contains this control.</summary>
				/// <returns>The <see cref="GuiControlHost"/> that contains this control.</returns>
				virtual GuiControlHost*					GetRelatedControlHost();
				/// <summary>Test if this control is rendered as enabled.</summary>
				/// <returns>Returns true if this control is rendered as enabled.</returns>
				virtual bool							GetVisuallyEnabled();
				/// <summary>Test if this control is enabled.</summary>
				/// <returns>Returns true if this control is enabled.</returns>
				virtual bool							GetEnabled();
				/// <summary>Make the control enabled or disabled.</summary>
				/// <param name="value">Set to true to make the control enabled.</param>
				virtual void							SetEnabled(bool value);
				/// <summary>Test if this visible or invisible.</summary>
				/// <returns>Returns true if this control is visible.</returns>
				virtual bool							GetVisible();
				/// <summary>Make the control visible or invisible.</summary>
				/// <param name="value">Set to true to make the visible enabled.</param>
				virtual void							SetVisible(bool value);
				/// <summary>Get the Alt-combined shortcut key associated with this control.</summary>
				/// <returns>The Alt-combined shortcut key associated with this control.</returns>
				virtual const WString&					GetAlt()override;
				/// <summary>Associate a Alt-combined shortcut key with this control.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="value">The Alt-combined shortcut key to associate. Only zero, sigle or multiple upper case letters are legal.</param>
				virtual bool							SetAlt(const WString& value);
				/// <summary>Make the control as the parent of multiple Alt-combined shortcut key activatable controls.</summary>
				/// <param name="host">The alt action host object.</param>
				void									SetActivatingAltHost(compositions::IGuiAltActionHost* host);
				/// <summary>Get the text to display on the control.</summary>
				/// <returns>The text to display on the control.</returns>
				virtual const WString&					GetText();
				/// <summary>Set the text to display on the control.</summary>
				/// <param name="value">The text to display on the control.</param>
				virtual void							SetText(const WString& value);
				/// <summary>Get the font to render the text.</summary>
				/// <returns>The font to render the text.</returns>
				virtual const FontProperties&			GetFont();
				/// <summary>Set the font to render the text.</summary>
				/// <param name="value">The font to render the text.</param>
				virtual void							SetFont(const FontProperties& value);
				/// <summary>Focus this control.</summary>
				virtual void							SetFocus();

				/// <summary>Get the tag object of the control.</summary>
				/// <returns>The tag object of the control.</returns>
				description::Value						GetTag();
				/// <summary>Set the tag object of the control.</summary>
				/// <param name="value">The tag object of the control.</param>
				void									SetTag(const description::Value& value);
				/// <summary>Get the tooltip control of the control.</summary>
				/// <returns>The tooltip control of the control.</returns>
				GuiControl*								GetTooltipControl();
				/// <summary>Set the tooltip control of the control. The tooltip control will be released when this control is released. If you set a new tooltip control to replace the old one, the old one will not be owned by this control anymore, therefore user should release the old tooltip control manually.</summary>
				/// <returns>The old tooltip control.</returns>
				/// <param name="value">The tooltip control of the control.</param>
				GuiControl*								SetTooltipControl(GuiControl* value);
				/// <summary>Get the tooltip width of the control.</summary>
				/// <returns>The tooltip width of the control.</returns>
				vint									GetTooltipWidth();
				/// <summary>Set the tooltip width of the control.</summary>
				/// <param name="value">The tooltip width of the control.</param>
				void									SetTooltipWidth(vint value);
				/// <summary>Display the tooltip.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="location">The relative location to specify the left-top position of the tooltip.</param>
				bool									DisplayTooltip(Point location);
				/// <summary>Close the tooltip that owned by this control.</summary>
				void									CloseTooltip();

				/// <summary>Query a service using an identifier. If you want to get a service of type IXXX, use IXXX::Identifier as the identifier.</summary>
				/// <returns>The requested service. If the control doesn't support this service, it will be null.</returns>
				/// <param name="identifier">The identifier.</param>
				virtual IDescriptable*					QueryService(const WString& identifier);

				template<typename T>
				T* QueryTypedService()
				{
					return dynamic_cast<T*>(QueryService(T::Identifier));
				}
			};

			class GuiInstanceRootObject;

			/// <summary>
			/// Represnets a component.
			/// </summary>
			class GuiComponent : public Object, public Description<GuiComponent>
			{
			public:
				GuiComponent();
				~GuiComponent();

				virtual void							Attach(GuiInstanceRootObject* rootObject);
				virtual void							Detach(GuiInstanceRootObject* rootObject);
			};
			
			/// <summary>Represnets a root GUI object.</summary>
			class GuiInstanceRootObject abstract : public Description<GuiInstanceRootObject>
			{
				typedef collections::List<Ptr<description::IValueSubscription>>		SubscriptionList;
			protected:
				Ptr<GuiResourcePathResolver>					resourceResolver;
				SubscriptionList								subscriptions;
				collections::SortedList<GuiComponent*>			components;

				void											FinalizeInstance();
			public:
				GuiInstanceRootObject();
				~GuiInstanceRootObject();

				/// <summary>Set the resource resolver to connect the current root object to the resource creating it.</summary>
				/// <param name="resolver">The resource resolver</param>
				void											SetResourceResolver(Ptr<GuiResourcePathResolver> resolver);
				/// <summary>Resolve a resource using the current resource resolver.</summary>
				/// <returns>The loaded resource. Returns null if failed to load.</returns>
				/// <param name="protocol">The protocol.</param>
				/// <param name="path">The path.</param>
				/// <param name="ensureExist">Set to true and it will throw an exception if the resource doesn't exist.</param>
				Ptr<DescriptableObject>							ResolveResource(const WString& protocol, const WString& path, bool ensureExist);

				/// <summary>Add a subscription. When this control host is disposing, all attached subscriptions will be deleted.</summary>
				/// <returns>Returns null if this operation failed.</returns>
				/// <param name="subscription">The subscription to test.</param>
				Ptr<description::IValueSubscription>			AddSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Remove a subscription.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="subscription">The subscription to test.</param>
				bool											RemoveSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Test does the window contain the subscription.</summary>
				/// <returns>Returns true if the window contains the subscription.</returns>
				/// <param name="subscription">The subscription to test.</param>
				bool											ContainsSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Clear all subscriptions.</summary>
				void											ClearSubscriptions();

				/// <summary>Add a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to add.</param>
				bool											AddComponent(GuiComponent* component);

				/// <summary>Add a control host as a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="controlHost">The controlHost to add.</param>
				bool											AddControlHostComponent(GuiControlHost* controlHost);
				/// <summary>Remove a component.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to remove.</param>
				bool											RemoveComponent(GuiComponent* component);
				/// <summary>Test does the window contain the component.</summary>
				/// <returns>Returns true if the window contains the component.</returns>
				/// <param name="component">The component to test.</param>
				bool											ContainsComponent(GuiComponent* component);
				/// <summary>Clear all components.</summary>
				void											ClearComponents();
			};

			/// <summary>Represnets a user customizable control.</summary>
			class GuiCustomControl : public GuiControl, public GuiInstanceRootObject, public AggregatableDescription<GuiCustomControl>
			{
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiCustomControl(IStyleController* _styleController);
				~GuiCustomControl();
			};

			template<typename T>
			class GuiObjectComponent : public GuiComponent
			{
			public:
				Ptr<T>				object;

				GuiObjectComponent()
				{
				}

				GuiObjectComponent(Ptr<T> _object)
					:object(_object)
				{
				}
			};
			
			namespace list
			{
/***********************************************************************
List interface common implementation
***********************************************************************/

				template<typename T, typename K=typename KeyType<T>::Type>
				class ItemsBase : public Object, public virtual collections::IEnumerable<T>
				{
				protected:
					collections::List<T, K>					items;

					virtual void NotifyUpdateInternal(vint start, vint count, vint newCount)
					{
					}

					virtual bool QueryInsert(vint index, const T& value)
					{
						return true;
					}

					virtual void BeforeInsert(vint index, const T& value)
					{
					}

					virtual void AfterInsert(vint index, const T& value)
					{
					}

					virtual bool QueryRemove(vint index, const T& value)
					{
						return true;
					}

					virtual void BeforeRemove(vint index, const T& value)
					{
					}

					virtual void AfterRemove(vint index, vint count)
					{
					}
					
				public:
					ItemsBase()
					{
					}

					~ItemsBase()
					{
					}

					collections::IEnumerator<T>* CreateEnumerator()const
					{
						return items.CreateEnumerator();
					}

					bool NotifyUpdate(vint start, vint count=1)
					{
						if(start<0 || start>=items.Count() || count<=0 || start+count>items.Count())
						{
							return false;
						}
						else
						{
							NotifyUpdateInternal(start, count, count);
							return true;
						}
					}

					bool Contains(const K& item)const
					{
						return items.Contains(item);
					}

					vint Count()const
					{
						return items.Count();
					}

					vint Count()
					{
						return items.Count();
					}

					const T& Get(vint index)const
					{
						return items.Get(index);
					}

					const T& operator[](vint index)const
					{
						return items.Get(index);
					}

					vint IndexOf(const K& item)const
					{
						return items.IndexOf(item);
					}

					vint Add(const T& item)
					{
						return Insert(items.Count(), item);
					}

					bool Remove(const K& item)
					{
						vint index=items.IndexOf(item);
						if(index==-1) return false;
						return RemoveAt(index);
					}

					bool RemoveAt(vint index)
					{
						if (0 <= index && index < items.Count() && QueryRemove(index, items[index]))
						{
							BeforeRemove(index, items[index]);
							T item = items[index];
							items.RemoveAt(index);
							AfterRemove(index, 1);
							NotifyUpdateInternal(index, 1, 0);
							return true;
						}
						return false;
					}

					bool RemoveRange(vint index, vint count)
					{
						if(count<=0) return false;
						if (0 <= index && index<items.Count() && index + count <= items.Count())
						{
							for (vint i = 0; i < count; i++)
							{
								if (!QueryRemove(index + 1, items[index + i])) return false;
							}
							for (vint i = 0; i < count; i++)
							{
								BeforeRemove(index + i, items[index + i]);
							}
							items.RemoveRange(index, count);
							AfterRemove(index, count);
							NotifyUpdateInternal(index, count, 0);
							return true;
						}
						return false;
					}

					bool Clear()
					{
						vint count = items.Count();
						for (vint i = 0; i < count; i++)
						{
							if (!QueryRemove(i, items[i])) return false;
						}
						for (vint i = 0; i < count; i++)
						{
							BeforeRemove(i, items[i]);
						}
						items.Clear();
						AfterRemove(0, count);
						NotifyUpdateInternal(0, count, 0);
						return true;
					}

					vint Insert(vint index, const T& item)
					{
						if (0 <= index && index <= items.Count() && QueryInsert(index, item))
						{
							BeforeInsert(index, item);
							items.Insert(index, item);
							AfterInsert(index, item);
							NotifyUpdateInternal(index, 0, 1);
							return index;
						}
						else
						{
							return -1;
						}
					}

					bool Set(vint index, const T& item)
					{
						if (0 <= index && index < items.Count())
						{
							if (QueryRemove(index, items[index]) && QueryInsert(index, item))
							{
								BeforeRemove(index, items[index]);
								items.RemoveAt(index);
								AfterRemove(index, 1);

								BeforeInsert(index, item);
								items.Insert(index, item);
								AfterInsert(index, item);

								NotifyUpdateInternal(index, 1, 1);
								return true;
							}
						}
						return false;
					}
				};

				template<typename T>
				class ObservableList : public ItemsBase<T>
				{
				protected:
					Ptr<description::IValueObservableList>		observableList;

					void NotifyUpdateInternal(vint start, vint count, vint newCount)override
					{
						if (observableList)
						{
							observableList->ItemChanged(start, count, newCount);
						}
					}
				public:

					Ptr<description::IValueObservableList> GetWrapper()
					{
						if (!observableList)
						{
							observableList = new description::ValueObservableListWrapper<ObservableList<T>*>(this);
						}
						return observableList;
					}
				};
			}
		}
	}

	namespace collections
	{
		namespace randomaccess_internal
		{
			template<typename T>
			struct RandomAccessable<presentation::controls::list::ItemsBase<T>>
			{
				static const bool							CanRead = true;
				static const bool							CanResize = false;
			};
		}
	}
}

#endif
