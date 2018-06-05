#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"

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

#define PROCESS_TYPE(TYPE) types.Remove(GetTypeDescriptor<TYPE>());
#define GUIREFLECTIONTEMPLATES_PROCESS_TYPE(NAME, BASE) types.Remove(GetTypeDescriptor<presentation::templates::NAME>());

TEST_CASE(Reflection_TypeList_Elements)
{
	Reflection_TypeList<IGuiGraphicsElement>([](auto& types)
	{
		types.Remove(GetTypeDescriptor<IGuiGraphicsElement>());
		GUIREFLECTIONELEMENT_CLASS_TYPELIST(PROCESS_TYPE);
	});
}

TEST_CASE(Reflection_TypeList_Compositions)
{
	Reflection_TypeList<GuiGraphicsComposition>([](auto& types)
	{
		GUIREFLECTIONCOMPOSITION_CLASS_TYPELIST(PROCESS_TYPE);
		GUIREFLECTIONTEMPLATES_CLASS_TYPELIST(PROCESS_TYPE);
	});
}

TEST_CASE(Reflection_TypeList_Controls)
{
	Reflection_TypeList<GuiControl>([](auto& types)
	{
		GUIREFLECTIONCONTROLS_CLASS_TYPELIST(PROCESS_TYPE);
	});
}

#undef GUIREFLECTIONTEMPLATES_PROCESS_TYPE
#undef PROCESS_TYPE