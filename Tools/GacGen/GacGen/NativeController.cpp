#include "GacUI.h"

using namespace vl;
using namespace vl::presentation;

class GacGenNativeController : public Object, public INativeController
{
public:
	virtual INativeCallbackService* CallbackService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeResourceService* ResourceService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeAsyncService* AsyncService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeClipboardService* ClipboardService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeImageService* ImageService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeScreenService* ScreenService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeWindowService* WindowService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeInputService* InputService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual INativeDialogService* DialogService() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

	virtual WString GetExecutablePath() override
	{
		CHECK_FAIL(L"GacGenNativeController::CallbackService()#Not implemented for GacGen.exe");
	}

};

int SetupGacGenNativeController()
{
	GacGenNativeController controller;
	SetCurrentController(&controller);
	GuiApplicationMain();
	return 0;
}