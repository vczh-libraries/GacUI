#include "GacUI.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::presentation;

class GacGenNativeController
	: public Object
	, public INativeController
	, protected INativeImageService
	, protected INativeInputService
{
public:
	INativeCallbackService* CallbackService() override
	{
		return this;
	}

	INativeResourceService* ResourceService() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	INativeAsyncService* AsyncService() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	INativeClipboardService* ClipboardService() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	INativeImageService* ImageService() override
	{
		return this;
	}

	INativeScreenService* ScreenService() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	INativeWindowService* WindowService() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	INativeInputService* InputService() override
	{
		return this;
	}

	INativeDialogService* DialogService() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	WString GetExecutablePath() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	////////////////////////////////////////////////////////////////////
	// INativeCallbackService
	////////////////////////////////////////////////////////////////////

	bool InstallListener(INativeControllerListener* listener) override
	{
		return true;
	}

	bool UninstallListener(INativeControllerListener* listener) override
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////
	// INativeImageService
	////////////////////////////////////////////////////////////////////

	class NativeImage : public Object, public INativeImage
	{
	protected:
		INativeImageService*		imageService;
		MemoryStream				memoryStream;

	public:
		NativeImage(INativeImageService* _imageService, IStream& inputStream)
			: imageService(_imageService)
		{
			CopyStream(inputStream, memoryStream);
		}

		INativeImageService* GetImageService() override
		{
			return imageService;
		}

		FormatType GetFormat() override
		{
			CHECK_FAIL(L"Not implemented!");
		}

		vint GetFrameCount() override
		{
			return 0;
		}

		INativeImageFrame* GetFrame(vint index) override
		{
			CHECK_FAIL(L"Not implemented!");
		}

		void SaveToStream(stream::IStream& imageStream, FormatType formatType) override
		{
			CHECK_ERROR(formatType == FormatType::Unknown, L"Not Implemented!");
			CopyStream(imageStream, memoryStream);
		}
	};

	Ptr<INativeImage> CreateImageFromFile(const WString& path) override
	{
		FileStream imageStream(path, FileStream::ReadOnly);
		return new NativeImage(this, imageStream);
	}

	Ptr<INativeImage> CreateImageFromMemory(void* buffer, vint length) override
	{
		MemoryWrapperStream imageStream(buffer, length);
		return new NativeImage(this, imageStream);
	}

	Ptr<INativeImage> CreateImageFromStream(stream::IStream& imageStream) override
	{
		return new NativeImage(this, imageStream);
	}

	////////////////////////////////////////////////////////////////////
	// INativeInputService
	////////////////////////////////////////////////////////////////////

	void StartTimer() override
	{
	}

	void StopTimer() override
	{
	}

	bool IsTimerEnabled() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	bool IsKeyPressing(VKEY code) override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	bool IsKeyToggled(VKEY code) override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	WString GetKeyName(VKEY code) override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	VKEY GetKey(const WString& name) override
	{
		CHECK_FAIL(L"Not implemented!");
	}

};

int SetupGacGenNativeController()
{
	GacGenNativeController controller;
	SetCurrentController(&controller);
	GuiApplicationMain();
	return 0;
}