#ifndef VCZH_PRESENTATION_WINDOWS_TUIWINDOWSCONTROLLER
#define VCZH_PRESENTATION_WINDOWS_TUIWINDOWSCONTROLLER

#include "../../TUI/TuiController.h"

#ifdef VCZH_MSVC
#include "../ServicesImpl/WindowsResourceService.h"
#include "../ServicesImpl/WindowsInputService.h"
#include "../ServicesImpl/WindowsClipboardService.h"
#include "../ServicesImpl/WindowsImageService.h"

namespace vl::presentation::windows
{
	class TuiWindowsResourceService : public WindowsResourceService
	{
	public:
		FontProperties	GetDefaultFont() override;
		void			SetDefaultFont(const FontProperties& value) override;
		void			EnumerateFonts(collections::List<WString>& fonts) override;
	};

	class TuiWindowsInputService : public WindowsInputService
	{
	public:
		void	StartTimer() override;
		void	StopTimer() override;
	};

	class TuiWindowsController : public TuiControllerBase
	{
	protected:
		TuiWindowsResourceService	resourceService;
		TuiWindowsInputService		inputService;
		WindowsClipboardService		clipboardService;
		WindowsImageService			imageService;
		HINSTANCE					instance;
		HWND						serviceWindow = nullptr;

		static LRESULT CALLBACK		ServiceWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		void						PumpPlatformEvents() override;
	public:
		TuiWindowsController(HINSTANCE instance, const TuiConfiguration& configuration);
		~TuiWindowsController();
		INativeResourceService*		ResourceService() override;
		INativeInputService*			InputService() override;
		INativeClipboardService*		ClipboardService() override;
		INativeImageService*			ImageService() override;
		WString						GetExecutablePath() override;
		void						ApplyTitle(const WString& title) override;
	};
}
#endif

#endif
