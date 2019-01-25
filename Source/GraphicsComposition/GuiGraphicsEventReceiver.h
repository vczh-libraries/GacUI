/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Event Model

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSEVENTRECEIVER
#define VCZH_PRESENTATION_COMPOSITION_GUIGRAPHICSEVENTRECEIVER

#include "../NativeWindow/GuiNativeWindow.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace tree
			{
				class INodeProvider;
			}
		}
	}
}

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class GuiGraphicsComposition;

/***********************************************************************
Event
***********************************************************************/

			class IGuiGraphicsEventHandler : public virtual IDescriptable, public Description<IGuiGraphicsEventHandler>
			{
			public:
				class Container
				{
				public:
					Ptr<IGuiGraphicsEventHandler>	handler;
				};

				virtual bool IsAttached() = 0;
			};

			template<typename T>
			class GuiGraphicsEvent : public Object, public Description<GuiGraphicsEvent<T>>
			{
			public:
				typedef void(RawFunctionType)(GuiGraphicsComposition*, T&);
				typedef Func<RawFunctionType>						FunctionType;
				typedef T											ArgumentType;
				
				class FunctionHandler : public Object, public IGuiGraphicsEventHandler
				{
				public:
					bool					isAttached = true;
					FunctionType			handler;

					FunctionHandler(const FunctionType& _handler)
						:handler(_handler)
					{
					}

					bool IsAttached()override
					{
						return isAttached;
					}

					void Execute(GuiGraphicsComposition* sender, T& argument)
					{
						handler(sender, argument);
					}
				};
			protected:
				struct HandlerNode
				{
					Ptr<FunctionHandler>								handler;
					Ptr<HandlerNode>									next;
				};

				GuiGraphicsComposition*									sender;
				Ptr<HandlerNode>										handlers;

				bool Attach(Ptr<FunctionHandler> handler)
				{
					Ptr<HandlerNode>* currentHandler = &handlers;
					while (*currentHandler)
					{
						if ((*currentHandler)->handler == handler)
						{
							return false;
						}
						else
						{
							currentHandler = &(*currentHandler)->next;
						}
					}
					(*currentHandler) = new HandlerNode;
					(*currentHandler)->handler = handler;
					return true;
				}
			public:
				GuiGraphicsEvent(GuiGraphicsComposition* _sender=0)
					:sender(_sender)
				{
				}

				~GuiGraphicsEvent()
				{
				}

				GuiGraphicsComposition* GetAssociatedComposition()
				{
					return sender;
				}

				void SetAssociatedComposition(GuiGraphicsComposition* _sender)
				{
					sender=_sender;
				}

				template<typename TClass, typename TMethod>
				Ptr<IGuiGraphicsEventHandler> AttachMethod(TClass* receiver, TMethod TClass::* method)
				{
					auto handler=MakePtr<FunctionHandler>(FunctionType(receiver, method));
					Attach(handler);
					return handler;
				}

				Ptr<IGuiGraphicsEventHandler> AttachFunction(RawFunctionType* function)
				{
					auto handler = MakePtr<FunctionHandler>(FunctionType(function));
					Attach(handler);
					return handler;
				}

				Ptr<IGuiGraphicsEventHandler> AttachFunction(const FunctionType& function)
				{
					auto handler = MakePtr<FunctionHandler>(function);
					Attach(handler);
					return handler;
				}

				template<typename TLambda>
				Ptr<IGuiGraphicsEventHandler> AttachLambda(const TLambda& lambda)
				{
					auto handler = MakePtr<FunctionHandler>(FunctionType(lambda));
					Attach(handler);
					return handler;
				}

				bool Detach(Ptr<IGuiGraphicsEventHandler> handler)
				{
					auto typedHandler = handler.Cast<FunctionHandler>();
					if (!typedHandler)
					{
						return false;
					}

					auto currentHandler=&handlers;
					while(*currentHandler)
					{
						if((*currentHandler)->handler == typedHandler)
						{
							(*currentHandler)->handler->isAttached = false;

							auto next=(*currentHandler)->next;
							(*currentHandler)=next;
							return true;
						}
						else
						{
							currentHandler=&(*currentHandler)->next;
						}
					}
					return false;
				}

				void ExecuteWithNewSender(T& argument, GuiGraphicsComposition* newSender)
				{
					auto currentHandler=&handlers;
					while(*currentHandler)
					{
						(*currentHandler)->handler->Execute(newSender?newSender:sender, argument);
						currentHandler=&(*currentHandler)->next;
					}
				}

				void Execute(T& argument)
				{
					ExecuteWithNewSender(argument, 0);
				}

				void Execute(const T& argument)
				{
					auto t = argument;
					ExecuteWithNewSender(t, 0);
				}
			};

/***********************************************************************
Predefined Events
***********************************************************************/

			/// <summary>Notify event arguments.</summary>
			struct GuiEventArgs : public Object, public AggregatableDescription<GuiEventArgs>
			{
				/// <summary>The event raiser composition.</summary>
				GuiGraphicsComposition*		compositionSource;
				/// <summary>The nearest parent of the event raiser composition that contains an event receiver. If the event raiser composition contains an event receiver, it will be the event raiser composition.</summary>
				GuiGraphicsComposition*		eventSource;
				/// <summary>Set this field to true will stop the event routing. This is a signal that the event is properly handeled, and the event handler want to override the default behavior.</summary>
				bool						handled;

				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to null.</summary>
				GuiEventArgs()
					:compositionSource(0)
					,eventSource(0)
					,handled(false)
				{
				}

				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiEventArgs(GuiGraphicsComposition* composition)
					:compositionSource(composition)
					,eventSource(composition)
					,handled(false)
				{
				}

				~GuiEventArgs()
				{
					FinalizeAggregation();
				}
			};
			
			/// <summary>Request event arguments.</summary>
			struct GuiRequestEventArgs : public GuiEventArgs, public Description<GuiRequestEventArgs>
			{
				/// <summary>Set this field to false in event handlers will stop the corresponding action.</summary>
				bool		cancel;
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to null.</summary>
				GuiRequestEventArgs()
					:cancel(false)
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiRequestEventArgs(GuiGraphicsComposition* composition)
					:GuiEventArgs(composition)
					,cancel(false)
				{
				}
			};
			
			/// <summary>Keyboard event arguments.</summary>
			struct GuiKeyEventArgs : public GuiEventArgs, public WindowKeyInfo, public Description<GuiKeyEventArgs>
			{
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to null.</summary>
				GuiKeyEventArgs()
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiKeyEventArgs(GuiGraphicsComposition* composition)
					:GuiEventArgs(composition)
				{
				}
			};
			
			/// <summary>Char input event arguments.</summary>
			struct GuiCharEventArgs : public GuiEventArgs, public WindowCharInfo, public Description<GuiCharEventArgs>
			{
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to null.</summary>
				GuiCharEventArgs()
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiCharEventArgs(GuiGraphicsComposition* composition)
					:GuiEventArgs(composition)
				{
				}
			};
			
			/// <summary>Mouse event arguments.</summary>
			struct GuiMouseEventArgs : public GuiEventArgs, public WindowMouseInfo, public Description<GuiMouseEventArgs>
			{
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to null.</summary>
				GuiMouseEventArgs()
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiMouseEventArgs(GuiGraphicsComposition* composition)
					:GuiEventArgs(composition)
				{
				}
			};

			/// <summary>Control signal.</summary>
			enum class ControlSignal
			{
				/// <summary>Render target changed.</summary>
				RenderTargetChanged,
				/// <summary>Render target changed.</summary>
				ParentLineChanged,
				/// <summary>Service added changed.</summary>
				ServiceAdded,
			};

			/// <summary>Control signal event arguments.</summary>
			struct GuiControlSignalEventArgs : public GuiEventArgs, public Description<GuiControlSignalEventArgs>
			{
				/// <summary>The event raiser composition.</summary>
				ControlSignal				controlSignal = ControlSignal::ParentLineChanged;

				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to null.</summary>
				GuiControlSignalEventArgs()
				{
				}

				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiControlSignalEventArgs(GuiGraphicsComposition* composition)
					:GuiEventArgs(composition)
				{
				}
			};

			typedef GuiGraphicsEvent<GuiEventArgs>				GuiNotifyEvent;
			typedef GuiGraphicsEvent<GuiRequestEventArgs>		GuiRequestEvent;
			typedef GuiGraphicsEvent<GuiKeyEventArgs>			GuiKeyEvent;
			typedef GuiGraphicsEvent<GuiCharEventArgs>			GuiCharEvent;
			typedef GuiGraphicsEvent<GuiMouseEventArgs>			GuiMouseEvent;
			typedef GuiGraphicsEvent<GuiControlSignalEventArgs>	GuiControlSignalEvent;

/***********************************************************************
Predefined Item Events
***********************************************************************/
			
			/// <summary>Item event arguments.</summary>
			struct GuiItemEventArgs : public GuiEventArgs, public Description<GuiItemEventArgs>
			{
				/// <summary>Item index.</summary>
				vint			itemIndex;

				GuiItemEventArgs()
					:itemIndex(-1)
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiItemEventArgs(GuiGraphicsComposition* composition)
					:GuiEventArgs(composition)
					,itemIndex(-1)
				{
				}
			};
			
			/// <summary>Item mouse event arguments.</summary>
			struct GuiItemMouseEventArgs : public GuiMouseEventArgs, public Description<GuiItemMouseEventArgs>
			{
				/// <summary>Item index.</summary>
				vint			itemIndex;

				GuiItemMouseEventArgs()
					:itemIndex(-1)
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiItemMouseEventArgs(GuiGraphicsComposition* composition)
					:GuiMouseEventArgs(composition)
					,itemIndex(-1)
				{
				}
			};

			typedef GuiGraphicsEvent<GuiItemEventArgs>			GuiItemNotifyEvent;
			typedef GuiGraphicsEvent<GuiItemMouseEventArgs>		GuiItemMouseEvent;

/***********************************************************************
Predefined Node Events
***********************************************************************/
			
			/// <summary>Node event arguments.</summary>
			struct GuiNodeEventArgs : public GuiEventArgs, public Description<GuiNodeEventArgs>
			{
				/// <summary>Tree node.</summary>
				controls::tree::INodeProvider*		node;

				GuiNodeEventArgs()
					:node(0)
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiNodeEventArgs(GuiGraphicsComposition* composition)
					:GuiEventArgs(composition)
					,node(0)
				{
				}
			};
			
			/// <summary>Node mouse event arguments.</summary>
			struct GuiNodeMouseEventArgs : public GuiMouseEventArgs, public Description<GuiNodeMouseEventArgs>
			{
				/// <summary>Tree node.</summary>
				controls::tree::INodeProvider*		node;

				GuiNodeMouseEventArgs()
					:node(0)
				{
				}
				
				/// <summary>Create an event arguments with <see cref="compositionSource"/> and <see cref="eventSource"/> set to a specified value.</summary>
				/// <param name="composition">The speciied value to set <see cref="compositionSource"/> and <see cref="eventSource"/>.</param>
				GuiNodeMouseEventArgs(GuiGraphicsComposition* composition)
					:GuiMouseEventArgs(composition)
					,node(0)
				{
				}
			};

			typedef GuiGraphicsEvent<GuiNodeEventArgs>			GuiNodeNotifyEvent;
			typedef GuiGraphicsEvent<GuiNodeMouseEventArgs>		GuiNodeMouseEvent;

/***********************************************************************
Event Receiver
***********************************************************************/

			/// <summary>
			/// Contains all available user input events for a <see cref="GuiGraphicsComposition"/>. Almost all events are routed events. Routed events means, not only the activated composition receives the event, all it direct or indirect parents receives the event. The argument(all derives from <see cref="GuiEventArgs"/>) for the event will store the original event raiser composition.
			/// </summary>
			class GuiGraphicsEventReceiver : public Object
			{
			protected:
				GuiGraphicsComposition*			sender;
			public:
				GuiGraphicsEventReceiver(GuiGraphicsComposition* _sender);
				~GuiGraphicsEventReceiver();

				GuiGraphicsComposition*			GetAssociatedComposition();

				/// <summary>Left mouse button down event.</summary>
				GuiMouseEvent					leftButtonDown;
				/// <summary>Left mouse button up event.</summary>
				GuiMouseEvent					leftButtonUp;
				/// <summary>Left mouse button double click event.</summary>
				GuiMouseEvent					leftButtonDoubleClick;
				/// <summary>Middle mouse button down event.</summary>
				GuiMouseEvent					middleButtonDown;
				/// <summary>Middle mouse button up event.</summary>
				GuiMouseEvent					middleButtonUp;
				/// <summary>Middle mouse button double click event.</summary>
				GuiMouseEvent					middleButtonDoubleClick;
				/// <summary>Right mouse button down event.</summary>
				GuiMouseEvent					rightButtonDown;
				/// <summary>Right mouse button up event.</summary>
				GuiMouseEvent					rightButtonUp;
				/// <summary>Right mouse button double click event.</summary>
				GuiMouseEvent					rightButtonDoubleClick;
				/// <summary>Horizontal wheel scrolling event.</summary>
				GuiMouseEvent					horizontalWheel;
				/// <summary>Vertical wheel scrolling event.</summary>
				GuiMouseEvent					verticalWheel;
				/// <summary>Mouse move event.</summary>
				GuiMouseEvent					mouseMove;
				/// <summary>Mouse enter event.</summary>
				GuiNotifyEvent					mouseEnter;
				/// <summary>Mouse leave event.</summary>
				GuiNotifyEvent					mouseLeave;
				
				/// <summary>Preview key event.</summary>
				GuiKeyEvent						previewKey;
				/// <summary>Key down event.</summary>
				GuiKeyEvent						keyDown;
				/// <summary>Key up event.</summary>
				GuiKeyEvent						keyUp;
				/// <summary>System key down event.</summary>
				GuiKeyEvent						systemKeyDown;
				/// <summary>System key up event.</summary>
				GuiKeyEvent						systemKeyUp;
				/// <summary>Preview char input event.</summary>
				GuiCharEvent					previewCharInput;
				/// <summary>Char input event.</summary>
				GuiCharEvent					charInput;
				/// <summary>Got focus event.</summary>
				GuiNotifyEvent					gotFocus;
				/// <summary>Lost focus event.</summary>
				GuiNotifyEvent					lostFocus;
				/// <summary>Caret notify event. This event is raised when a caret graph need to change the visibility state.</summary>
				GuiNotifyEvent					caretNotify;
				/// <summary>Clipboard notify event. This event is raised when the content in the system clipboard is changed.</summary>
				GuiNotifyEvent					clipboardNotify;
				/// <summary>Render target changed event. This event is raised when the render target of this composition is changed.</summary>
				GuiNotifyEvent					renderTargetChanged;
			};
		}
	}

/***********************************************************************
Workflow to C++ Codegen Helpers
***********************************************************************/

	namespace __vwsn
	{
		template<typename T>
		struct EventHelper<presentation::compositions::GuiGraphicsEvent<T>>
		{
			using Event = presentation::compositions::GuiGraphicsEvent<T>;
			using Sender = presentation::compositions::GuiGraphicsComposition;
			using IGuiGraphicsEventHandler = presentation::compositions::IGuiGraphicsEventHandler;
			using Handler = Func<void(Sender*, T*)>;

			class EventHandlerImpl : public Object, public reflection::description::IEventHandler
			{
			public:
				Ptr<IGuiGraphicsEventHandler> handler;

				EventHandlerImpl(Ptr<IGuiGraphicsEventHandler> _handler)
					:handler(_handler)
				{
				}

				bool IsAttached()override
				{
					return handler->IsAttached();
				}
			};

			static Ptr<reflection::description::IEventHandler> Attach(Event& e, Handler handler)
			{
				return MakePtr<EventHandlerImpl>(e.AttachLambda([=](Sender* sender, T& args)
				{
					handler(sender, &args);
				}));
			}

			static bool Detach(Event& e, Ptr<reflection::description::IEventHandler> handler)
			{
				auto impl = handler.Cast<EventHandlerImpl>();
				if (!impl) return false;
				return e.Detach(impl->handler);
			}

			static auto Invoke(Event& e)
			{
				return [&](Sender* sender, T* args)
				{
					e.ExecuteWithNewSender(*args, sender);
				};
			}
		};
	}
}

#endif