/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Events

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONEVENTS
#define VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONEVENTS

#include "GuiReflectionControls.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
Type List
***********************************************************************/

#define GUIREFLECTIONEVENT_TYPELIST(F)\
			F(presentation::compositions::GuiEventArgs)\
			F(presentation::compositions::GuiRequestEventArgs)\
			F(presentation::compositions::GuiKeyEventArgs)\
			F(presentation::compositions::GuiCharEventArgs)\
			F(presentation::compositions::GuiMouseEventArgs)\
			F(presentation::compositions::GuiItemEventArgs)\
			F(presentation::compositions::GuiItemMouseEventArgs)\
			F(presentation::compositions::GuiNodeEventArgs)\
			F(presentation::compositions::GuiNodeMouseEventArgs)\

			GUIREFLECTIONEVENT_TYPELIST(DECL_TYPE_INFO)

/***********************************************************************
GuiEventInfoImpl
***********************************************************************/

			template<typename T>
			class GuiEventInfoImpl : public EventInfoImpl
			{
				using GuiGraphicsComposition = presentation::compositions::GuiGraphicsComposition;

				template<typename U>
				using GuiGraphicsEvent = presentation::compositions::GuiGraphicsEvent<U>;
			protected:
				typedef Func<GuiGraphicsEvent<T>*(DescriptableObject*, bool)>		EventRetriverFunction;

				EventRetriverFunction				eventRetriver;

				void AttachInternal(DescriptableObject* thisObject, IEventHandler* eventHandler)override
				{
					if(thisObject)
					{
						if(EventHandlerImpl* handlerImpl=dynamic_cast<EventHandlerImpl*>(eventHandler))
						{
							GuiGraphicsEvent<T>* eventObject=eventRetriver(thisObject, true);
							if(eventObject)
							{
								auto handler=eventObject->AttachLambda(
									[=](GuiGraphicsComposition* sender, T& arguments)
									{
										Value senderObject = BoxValue<GuiGraphicsComposition*>(sender, Description<GuiGraphicsComposition>::GetAssociatedTypeDescriptor());
										Value argumentsObject = BoxValue<T*>(&arguments, Description<T>::GetAssociatedTypeDescriptor());

										collections::Array<Value> eventArgs(2);
										eventArgs[0] = senderObject;
										eventArgs[1] = argumentsObject;
										eventHandler->Invoke(Value::From(thisObject), eventArgs);
									});
								handlerImpl->SetDescriptableTag(handler);
							}
						}
					}
				}

				void DetachInternal(DescriptableObject* thisObject, IEventHandler* eventHandler)override
				{
					if(thisObject)
					{
						if(EventHandlerImpl* handlerImpl=dynamic_cast<EventHandlerImpl*>(eventHandler))
						{
							GuiGraphicsEvent<T>* eventObject=eventRetriver(thisObject, false);
							if(eventObject)
							{
								auto handler=handlerImpl->GetDescriptableTag().Cast<typename GuiGraphicsEvent<T>::IHandler>();
								if(handler)
								{
									eventObject->Detach(handler);
								}
							}
						}
					}
				}

				void InvokeInternal(DescriptableObject* thisObject, collections::Array<Value>& arguments)override
				{
					if(thisObject)
					{
						GuiGraphicsEvent<T>* eventObject=eventRetriver(thisObject, false);
						if(eventObject)
						{
							T* value=UnboxValue<T*>(arguments[1], Description<T>::GetAssociatedTypeDescriptor());
							eventObject->Execute(*value);
						}
					}
				}

				Ptr<ITypeInfo> GetHandlerTypeInternal()override
				{
					return TypeInfoRetriver<Func<void(GuiGraphicsComposition*, T*)>>::CreateTypeInfo();
				}
			public:
				GuiEventInfoImpl(ITypeDescriptor* _ownerTypeDescriptor, const WString& _name, const EventRetriverFunction& _eventRetriver)
					:EventInfoImpl(_ownerTypeDescriptor, _name)
					,eventRetriver(_eventRetriver)
				{
				}

				~GuiEventInfoImpl()
				{
				}

				ICpp* GetCpp()override
				{
					throw 0;
				}
			};

			template<typename T>
			struct GuiEventArgumentTypeRetriver
			{
				typedef vint								Type;
			};

			template<typename TClass, typename TEvent>
			struct GuiEventArgumentTypeRetriver<TEvent TClass::*>
			{
				typedef typename TEvent::ArgumentType		Type;
			};

/***********************************************************************
Macros
***********************************************************************/

#define CLASS_MEMBER_GUIEVENT(EVENTNAME)\
			AddEvent(\
				new GuiEventInfoImpl<GuiEventArgumentTypeRetriver<decltype(&ClassType::EVENTNAME)>::Type>(\
					this,\
					L ## #EVENTNAME,\
					[](DescriptableObject* thisObject, bool addEventHandler){\
						return &thisObject->SafeAggregationCast<ClassType>()->EVENTNAME;\
					}\
				)\
			);\

#define CLASS_MEMBER_GUIEVENT_COMPOSITION(EVENTNAME)\
			AddEvent(\
				new GuiEventInfoImpl<GuiEventArgumentTypeRetriver<decltype(&GuiGraphicsEventReceiver::EVENTNAME)>::Type>(\
					this,\
					L ## #EVENTNAME,\
					[](DescriptableObject* thisObject, bool addEventHandler){\
						GuiGraphicsComposition* composition=thisObject->SafeAggregationCast<GuiGraphicsComposition>();\
						if(!addEventHandler && !composition->HasEventReceiver())\
						{\
							return (GuiGraphicsEvent<GuiEventArgumentTypeRetriver<decltype(&GuiGraphicsEventReceiver::EVENTNAME)>::Type>*)0;\
						}\
						return &composition->GetEventReceiver()->EVENTNAME;\
					}\
				)\
			);\

#define CLASS_MEMBER_PROPERTY_GUIEVENT_FAST(PROPERTYNAME)\
			CLASS_MEMBER_GUIEVENT(PROPERTYNAME##Changed)\
			CLASS_MEMBER_PROPERTY_EVENT_FAST(PROPERTYNAME, PROPERTYNAME##Changed)\

#define CLASS_MEMBER_PROPERTY_GUIEVENT_READONLY_FAST(PROPERTYNAME)\
			CLASS_MEMBER_GUIEVENT(PROPERTYNAME##Changed)\
			CLASS_MEMBER_PROPERTY_EVENT_READONLY_FAST(PROPERTYNAME, PROPERTYNAME##Changed)\

#define GUIEVENT_HANDLER_PARAMETERS {L"sender", L"arguments"}

#define CLASS_MEMBER_GUIEVENT_HANDLER(FUNCTIONNAME, ARGUMENTTYPE)\
			CLASS_MEMBER_EXTERNALMETHOD_INVOKETEMPLATE(\
				FUNCTIONNAME,\
				GUIEVENT_HANDLER_PARAMETERS,\
				void(ClassType::*)(vl::presentation::compositions::GuiGraphicsComposition*, ARGUMENTTYPE*),\
				[](ClassType* owner, vl::presentation::compositions::GuiGraphicsComposition* sender, ARGUMENTTYPE* arguments)\
				{\
					owner->FUNCTIONNAME(sender, *arguments);\
				},\
				L"[](auto owner, auto sender, auto arguments){ return owner->$Name(sender, *arguments); }($This, $Arguments)")\

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiEventTypes();
		}
	}
}

#endif