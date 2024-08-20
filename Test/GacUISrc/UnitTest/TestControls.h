#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif
#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::controls::list;
using namespace vl::presentation::controls::tree;
using namespace vl::presentation::unittest;

namespace gacui_unittest_template
{
	extern void GuiTextList_TestCases(
		const wchar_t* resourceXml,
		WString pathFragment,
		Func<Ptr<IValueList>(GuiWindow*)> getItems
	);
}