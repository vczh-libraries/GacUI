#include "GuiReflectionPlugin.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{
			using namespace presentation::compositions;
			using namespace presentation::controls;

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

/***********************************************************************
Type Declaration
***********************************************************************/

#define _ ,

#define EVENTARGS_CONSTRUCTOR(EVENTARGS)\
	CLASS_MEMBER_CONSTRUCTOR(Ptr<EVENTARGS>(), NO_PARAMETER)\
	CLASS_MEMBER_CONSTRUCTOR(Ptr<EVENTARGS>(GuiGraphicsComposition*), {L"composition"})

#define POINTARGS_POINTS(EVENTARGS)\
	CLASS_MEMBER_EXTERNALMETHOD_TEMPLATE(GetPoints, NO_PARAMETER, const collections::List<Nullable<presentation::Point>>& (EVENTARGS::*)(),\
		[](EVENTARGS* arguments) -> const collections::List<Nullable<presentation::Point>>& { return arguments->GetPoints(); },\
		L"$This->GetPoints()", L"[=]() -> const ::vl::collections::List<::vl::Nullable<::vl::presentation::Point>>& { return $This->GetPoints(); }")\
	CLASS_MEMBER_PROPERTY_READONLY(Points, GetPoints)

			BEGIN_CLASS_MEMBER(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiEventArgs)

				CLASS_MEMBER_FIELD(compositionSource)
				CLASS_MEMBER_FIELD(eventSource)
				CLASS_MEMBER_FIELD(handled)
			END_CLASS_MEMBER(GuiEventArgs)

			BEGIN_CLASS_MEMBER(GuiRequestEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiRequestEventArgs)

				CLASS_MEMBER_FIELD(cancel)
			END_CLASS_MEMBER(GuiRequestEventArgs)

			BEGIN_ENUM_ITEM(CompositionUpdateType)
				ENUM_CLASS_ITEM(Inserted)
				ENUM_CLASS_ITEM(Removed)
				ENUM_CLASS_ITEM(Moved)
			END_ENUM_ITEM(CompositionUpdateType)

			BEGIN_CLASS_MEMBER(GuiCompositionUpdateEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiCompositionUpdateEventArgs)
				CLASS_MEMBER_FIELD(updateType)
				CLASS_MEMBER_FIELD(parent)
				CLASS_MEMBER_FIELD(child)
			END_CLASS_MEMBER(GuiCompositionUpdateEventArgs)

			BEGIN_CLASS_MEMBER(GuiKeyEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiKeyEventArgs)
				
				CLASS_MEMBER_FIELD(code)
				CLASS_MEMBER_FIELD(ctrl)
				CLASS_MEMBER_FIELD(shift)
				CLASS_MEMBER_FIELD(alt)
				CLASS_MEMBER_FIELD(osSuper)
				CLASS_MEMBER_FIELD(capslock)
				CLASS_MEMBER_FIELD(autoRepeatKeyDown)
			END_CLASS_MEMBER(GuiKeyEventArgs)

			BEGIN_CLASS_MEMBER(GuiCharEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiCharEventArgs)
				
				CLASS_MEMBER_FIELD(code)
				CLASS_MEMBER_FIELD(ctrl)
				CLASS_MEMBER_FIELD(shift)
				CLASS_MEMBER_FIELD(alt)
				CLASS_MEMBER_FIELD(osSuper)
				CLASS_MEMBER_FIELD(capslock)
			END_CLASS_MEMBER(GuiCharEventArgs)

			BEGIN_CLASS_MEMBER(GuiMouseEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiMouseEventArgs)
				
				CLASS_MEMBER_FIELD(button)
				CLASS_MEMBER_FIELD(alt)
				CLASS_MEMBER_FIELD(osSuper)
				CLASS_MEMBER_FIELD(ctrl)
				CLASS_MEMBER_FIELD(shift)
				CLASS_MEMBER_FIELD(left)
				CLASS_MEMBER_FIELD(middle)
				CLASS_MEMBER_FIELD(right)
				CLASS_MEMBER_FIELD(x)
				CLASS_MEMBER_FIELD(y)
				CLASS_MEMBER_FIELD(wheel)
				CLASS_MEMBER_FIELD(nonClient)
			END_CLASS_MEMBER(GuiMouseEventArgs)

			BEGIN_CLASS_MEMBER(GuiPointEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				CLASS_MEMBER_FIELD(sequenceId)
				CLASS_MEMBER_FIELD(pointId)
				CLASS_MEMBER_FIELD(timestamp)
				CLASS_MEMBER_FIELD(activePointCount)
				CLASS_MEMBER_FIELD(position)
				CLASS_MEMBER_FIELD(primary)
				CLASS_MEMBER_FIELD(ctrl)
				CLASS_MEMBER_FIELD(shift)
				CLASS_MEMBER_FIELD(alt)
				CLASS_MEMBER_FIELD(osSuper)
				CLASS_MEMBER_FIELD(contactBounds)
				CLASS_MEMBER_FIELD(pressure)
				CLASS_MEMBER_FIELD(orientation)
				CLASS_MEMBER_FIELD(platformFrameId)
				POINTARGS_POINTS(GuiPointEventArgs)
			END_CLASS_MEMBER(GuiPointEventArgs)

			BEGIN_CLASS_MEMBER(GuiPointSequenceStartedEventArgs)
				CLASS_MEMBER_BASE(GuiPointEventArgs)
				CLASS_MEMBER_FIELD(accept)
			END_CLASS_MEMBER(GuiPointSequenceStartedEventArgs)

			BEGIN_CLASS_MEMBER(GuiPointSequenceEndedEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				CLASS_MEMBER_FIELD(sequenceId)
				CLASS_MEMBER_FIELD(timestamp)
				CLASS_MEMBER_FIELD(accepted)
				CLASS_MEMBER_FIELD(reason)
				POINTARGS_POINTS(GuiPointSequenceEndedEventArgs)
			END_CLASS_MEMBER(GuiPointSequenceEndedEventArgs)

			BEGIN_ENUM_ITEM(ControlSignal)
				ENUM_CLASS_ITEM(RenderTargetChanged)
				ENUM_CLASS_ITEM(ParentLineChanged)
				ENUM_CLASS_ITEM(ServiceAdded)
				ENUM_CLASS_ITEM(UpdateRequested)
				ENUM_CLASS_ITEM(UpdateFullfilled)
			END_ENUM_ITEM(ControlSignal)

			BEGIN_CLASS_MEMBER(GuiControlSignalEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				
				CLASS_MEMBER_FIELD(controlSignal)
			END_CLASS_MEMBER(GuiControlSignalEventArgs)

			BEGIN_CLASS_MEMBER(GuiItemEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiItemEventArgs)

				CLASS_MEMBER_FIELD(itemIndex)
			END_CLASS_MEMBER(GuiItemEventArgs)

			BEGIN_CLASS_MEMBER(GuiItemMouseEventArgs)
				CLASS_MEMBER_BASE(GuiMouseEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiItemMouseEventArgs)

				CLASS_MEMBER_FIELD(itemIndex)
			END_CLASS_MEMBER(GuiItemMouseEventArgs)

			BEGIN_CLASS_MEMBER(GuiNodeEventArgs)
				CLASS_MEMBER_BASE(GuiEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiNodeEventArgs)

				CLASS_MEMBER_FIELD(node)
			END_CLASS_MEMBER(GuiNodeEventArgs)

			BEGIN_CLASS_MEMBER(GuiNodeMouseEventArgs)
				CLASS_MEMBER_BASE(GuiMouseEventArgs)
				EVENTARGS_CONSTRUCTOR(GuiNodeMouseEventArgs)

				CLASS_MEMBER_FIELD(node)
			END_CLASS_MEMBER(GuiNodeMouseEventArgs)

#undef EVENTARGS_CONSTRUCTOR
#undef POINTARGS_POINTS
#undef _

/***********************************************************************
Type Loader
***********************************************************************/

			class GuiEventTypeLoader : public Object, public ITypeLoader
			{
			public:
				void Load(ITypeManager* manager)
				{
					GUIREFLECTIONEVENT_TYPELIST(ADD_TYPE_INFO)
				}

				void Unload(ITypeManager* manager)
				{
				}
			};

#endif

			bool LoadGuiEventTypes()
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				ITypeManager* manager=GetGlobalTypeManager();
				if(manager)
				{
					auto loader=Ptr(new GuiEventTypeLoader);
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}
