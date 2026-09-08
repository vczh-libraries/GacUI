#ifndef VCZH_PRESENTATION_WINDOWS_TUIWINDOWSCONTROLLER
#define VCZH_PRESENTATION_WINDOWS_TUIWINDOWSCONTROLLER

#include "../../TUI/TuiController.h"

#ifdef VCZH_MSVC
namespace vl::presentation::windows
{
	class TuiWindowsController : public TuiControllerBase
	{
	protected:
		void	PumpPlatformEvents() override;
	public:
		TuiWindowsController(INativeController* services);
		void	ApplyTitle(const WString& title) override;
	};
}
#endif

#endif
