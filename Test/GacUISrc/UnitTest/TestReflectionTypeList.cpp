#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#include "../../../Source/GacUI.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::templates;

template<typename T, typename U>
void Reflection_TypeList(U&& callback)
{
	SortedList<ITypeDescriptor*> types;
	auto manager = GetGlobalTypeManager();
	vint count = manager->GetTypeDescriptorCount();
	auto tdBase = GetTypeDescriptor<T>();

	for (vint i = 0; i < count; i++)
	{
		auto td = manager->GetTypeDescriptor(i);
		if (td->CanConvertTo(tdBase))
		{
			types.Add(td);
		}
	}
	
	callback(types);
	TEST_ASSERT(types.Count() == 0);
}

#define PROCESS_TYPE(TYPE) TEST_ASSERT(types.Remove(GetTypeDescriptor<TYPE>()));
#define GUIREFLECTIONTEMPLATES_PROCESS_TYPE(NAME, BASE) TEST_ASSERT(types.Remove(GetTypeDescriptor<presentation::templates::NAME>()));

extern void SetGuiMainProxy(const Func<void()>& proxy);

TEST_FILE
{
	SetGuiMainProxy([]()
	{
		TEST_CASE(L"Raw elements expose shared Id and Data without property events")
		{
			auto rawType = GetTypeDescriptor(L"presentation::elements::GuiRawElementBase");
			TEST_ASSERT(rawType);
			TEST_ASSERT(rawType->CanConvertTo(GetTypeDescriptor<IGuiGraphicsElement>()));
			for (auto name : { L"Id", L"Data" })
			{
				auto property = rawType->GetPropertyByName(name, true);
				TEST_ASSERT(property);
				TEST_ASSERT(property->GetGetter());
				TEST_ASSERT(property->GetSetter());
				TEST_ASSERT(!property->GetValueChangedEvent());
			}
		});

		TEST_CASE(L"Ensure GUIREFLECTIONELEMENT_CLASS_TYPELIST has all IGuiGraphicsElement sub classes")
		{
			Reflection_TypeList<IGuiGraphicsElement>([](auto& types)
			{
				types.Remove(GetTypeDescriptor<IGuiGraphicsElement>());
				GUIREFLECTIONELEMENT_CLASS_TYPELIST(PROCESS_TYPE);
			});
		});

		TEST_CASE(L"Ensure GUIREFLECTION(COMPOSITION|TEMPLATES)_CLASS_TYPELIST has all GuiGraphicsComposition sub classes")
		{
			Reflection_TypeList<GuiGraphicsComposition>([](auto& types)
			{
				GUIREFLECTIONCOMPOSITION_CLASS_TYPELIST(PROCESS_TYPE);
				GUIREFLECTIONTEMPLATES_CLASS_TYPELIST(PROCESS_TYPE);
			});
		});

		TEST_CASE(L"Ensure GUIREFLECTIONCONTROLS_CLASS_TYPELIST has all GuiControl sub classes")
		{
			Reflection_TypeList<GuiControl>([](auto& types)
			{
				GUIREFLECTIONCONTROLS_CLASS_TYPELIST(PROCESS_TYPE);
			});
		});

		TEST_CASE(L"Construct and access an embedded document object through reflection")
		{
			auto run = Value::Create(L"presentation::DocumentEmbeddedObjectRun");
			run.SetProperty(L"name", BoxValue(WString(L"embedded-control")));
			auto embedded = UnboxValue<Ptr<DocumentEmbeddedObjectRun>>(run);
			TEST_ASSERT(embedded->name == L"embedded-control");
		});

		TEST_CASE(L"Unified point actions expose their kind and borrow the sequence list")
		{
			auto pointEventType = GetTypeDescriptor(L"presentation::compositions::GuiPointEventsType");
			TEST_ASSERT(pointEventType);
			auto pointEventEnum = pointEventType->GetEnumType();
			TEST_ASSERT(pointEventEnum);
			for (auto name : { L"Started", L"Down", L"Update", L"Up", L"Enter", L"Leave" })
			{
				TEST_ASSERT(pointEventEnum->IndexOfItem(name) != -1);
			}

			auto compositionType = GetTypeDescriptor<GuiGraphicsComposition>();
			for (auto name : { L"pointSequenceStarted", L"pointAction", L"pointSequenceEnded" })
			{
				TEST_ASSERT(compositionType->GetEventByName(name, true));
			}
			for (auto name : { L"pointDown", L"pointUpdate", L"pointUp", L"pointEnter", L"pointLeave" })
			{
				TEST_ASSERT(!compositionType->GetEventByName(name, true));
			}

			GuiBoundsComposition composition;
			List<Nullable<Point>> points;
			points.Add(Point(10, 20));
			points.Add({});
			GuiPointSequenceStartedEventArgs started(points);
			GuiPointSequenceStartedEventArgs sourcedStarted(&composition, points);
			TEST_ASSERT(started.eventType == GuiPointEventsType::Started);
			TEST_ASSERT(sourcedStarted.eventType == GuiPointEventsType::Started);
			TEST_ASSERT(sourcedStarted.compositionSource == &composition);
			TEST_ASSERT(&started.GetPoints() == &points);
			TEST_ASSERT(&sourcedStarted.GetPoints() == &points);

			for (auto eventType : { GuiPointEventsType::Down, GuiPointEventsType::Update, GuiPointEventsType::Up, GuiPointEventsType::Enter, GuiPointEventsType::Leave })
			{
				GuiPointEventArgs arguments(eventType, points);
				GuiPointEventArgs sourcedArguments(&composition, eventType, points);
				TEST_ASSERT(arguments.eventType == eventType);
				TEST_ASSERT(sourcedArguments.eventType == eventType);
				TEST_ASSERT(sourcedArguments.compositionSource == &composition);
				TEST_ASSERT(&sourcedArguments.GetPoints() == &points);
				auto boxed = BoxValue(&arguments);
				TEST_ASSERT(UnboxValue<GuiPointEventsType>(boxed.GetProperty(L"eventType")) == eventType);
				auto reflectedPoints = UnboxValue<Ptr<IValueReadonlyList>>(boxed.GetProperty(L"Points"));
				TEST_ASSERT(reflectedPoints->GetCount() == 2);
				TEST_ASSERT(UnboxValue<Nullable<Point>>(reflectedPoints->Get(0)).Value() == Point(10, 20));
				TEST_ASSERT(!UnboxValue<Nullable<Point>>(reflectedPoints->Get(1)));
			}
		});

		TEST_CASE(L"Ensure mouse button and Super modifier reflection surface")
		{
			auto mouseButtonType = GetTypeDescriptor(L"presentation::NativeMouseButton");
			TEST_ASSERT(mouseButtonType);
			auto mouseButtonEnum = mouseButtonType->GetEnumType();
			TEST_ASSERT(mouseButtonEnum);
			TEST_ASSERT(mouseButtonEnum->IndexOfItem(L"Left") != -1);
			TEST_ASSERT(mouseButtonEnum->IndexOfItem(L"Middle") != -1);
			TEST_ASSERT(mouseButtonEnum->IndexOfItem(L"Right") != -1);
			TEST_ASSERT(mouseButtonEnum->IndexOfItem(L"Mouse4") != -1);
			TEST_ASSERT(mouseButtonEnum->IndexOfItem(L"Mouse5") != -1);

			auto mouseEventArgsType = GetTypeDescriptor(L"presentation::compositions::GuiMouseEventArgs");
			TEST_ASSERT(mouseEventArgsType->GetPropertyByName(L"button", true));
			TEST_ASSERT(mouseEventArgsType->GetPropertyByName(L"osSuper", true));
			TEST_ASSERT(mouseEventArgsType->GetPropertyByName(L"alt", true));
			TEST_ASSERT((vint)VKEY::KEY_LEFT_BRACKET == 0xDB && (vint)VKEY::KEY_RIGHT_BRACKET == 0xDD);
			TEST_ASSERT(GetTypeDescriptor(L"presentation::compositions::GuiKeyEventArgs")->GetPropertyByName(L"osSuper", true));
			TEST_ASSERT(GetTypeDescriptor(L"presentation::compositions::GuiCharEventArgs")->GetPropertyByName(L"osSuper", true));

			auto compositionType = GetTypeDescriptor(L"presentation::compositions::GuiGraphicsComposition");
			TEST_ASSERT(compositionType->GetEventByName(L"mouseDown", true));
			TEST_ASSERT(compositionType->GetEventByName(L"mouseUp", true));
			TEST_ASSERT(compositionType->GetEventByName(L"mouseDoubleClick", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"leftButtonDown", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"leftButtonUp", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"leftButtonDoubleClick", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"middleButtonDown", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"middleButtonUp", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"middleButtonDoubleClick", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"rightButtonDown", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"rightButtonUp", true));
			TEST_ASSERT(!compositionType->GetEventByName(L"rightButtonDoubleClick", true));
		});
	});
	SetupGacGenNativeController();
	SetGuiMainProxy({});
}

#undef GUIREFLECTIONTEMPLATES_PROCESS_TYPE
#undef PROCESS_TYPE
