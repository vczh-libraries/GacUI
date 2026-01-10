#ifndef GACUI_UNITTEST_TESTCONTROLS
#define GACUI_UNITTEST_TESTCONTROLS

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
	class TooltipTimer : protected vl::feature_injection::FeatureImpl<vl::IDateTimeImpl>
	{
	private:
		vl::DateTime			currentTime;

	public:
		TooltipTimer()
		{
			currentTime = vl::DateTime::FromDateTime(2000, 1, 1, 0, 0, 0, 0);
			vl::InjectDateTimeImpl(this);
		}

		~TooltipTimer()
		{
			try
			{
				vl::EjectDateTimeImpl(nullptr);
			}
			catch (...)
			{
			}
		}

		void Tooltip()
		{
			currentTime = currentTime.Forward(600);
		}

		void WaitForClosing()
		{
			currentTime = currentTime.Forward(5100);
		}

	protected:
		vl::DateTime FromDateTime(vint _year, vint _month, vint _day, vint _hour, vint _minute, vint _second, vint _milliseconds) override
		{
			return Previous()->FromDateTime(_year, _month, _day, _hour, _minute, _second, _milliseconds);
		}

		vl::DateTime FromOSInternal(vuint64_t osInternal) override
		{
			return Previous()->FromOSInternal(osInternal);
		}

		vuint64_t LocalTime() override
		{
			return currentTime.osMilliseconds;
		}

		vuint64_t UtcTime() override
		{
			return Previous()->LocalToUtcTime(currentTime.osInternal);
		}

		vuint64_t LocalToUtcTime(vuint64_t osInternal) override
		{
			return Previous()->LocalToUtcTime(osInternal);
		}

		vuint64_t UtcToLocalTime(vuint64_t osInternal) override
		{
			return Previous()->UtcToLocalTime(osInternal);
		}

		vuint64_t Forward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Forward(osInternal, milliseconds);
		}

		vuint64_t Backward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Backward(osInternal, milliseconds);
		}
	};
}

#endif