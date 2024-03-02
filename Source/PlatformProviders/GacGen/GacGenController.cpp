#include "../../NativeWindow/GuiNativeWindow.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;

class GacGenNativeController
	: public Object
	, public INativeController
	, protected INativeCallbackService
	, protected INativeResourceService
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
		return this;
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

	INativeCallbackInvoker* Invoker() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	////////////////////////////////////////////////////////////////////
	// INativeResourceService
	////////////////////////////////////////////////////////////////////

	INativeCursor* GetSystemCursor(INativeCursor::SystemCursorType type) override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	INativeCursor* GetDefaultSystemCursor() override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	FontProperties GetDefaultFont() override
	{
		FontProperties font;
		font.fontFamily = L"GacGen";
		font.size = 12;
		font.bold = false;
		font.italic = false;
		font.underline = false;
		font.strikeline = false;
		font.antialias = false;
		font.verticalAntialias = false;
		return font;
	}

	void SetDefaultFont(const FontProperties& value) override
	{
		CHECK_FAIL(L"Not implemented!");
	}

	void EnumerateFonts(collections::List<WString>& fonts) override
	{
		CHECK_FAIL(L"Not implemented!");
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
			memoryStream.SeekFromBegin(0);
			CopyStream(memoryStream, imageStream);
		}
	};

	Ptr<INativeImage> CreateImageFromFile(const WString& path) override
	{
		FileStream imageStream(path, FileStream::ReadOnly);
		if (!imageStream.IsAvailable()) return nullptr;
		return Ptr(new NativeImage(this, imageStream));
	}

	Ptr<INativeImage> CreateImageFromMemory(void* buffer, vint length) override
	{
		MemoryWrapperStream imageStream(buffer, length);
		return Ptr(new NativeImage(this, imageStream));
	}

	Ptr<INativeImage> CreateImageFromStream(stream::IStream& imageStream) override
	{
		return Ptr(new NativeImage(this, imageStream));
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

	vint RegisterGlobalShortcutKey(bool ctrl, bool shift, bool alt, VKEY key) override
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool UnregisterGlobalShortcutKey(vint id) override
	{
		CHECK_FAIL(L"Not Implemented!");
	}
};

extern void GuiApplicationMain();

int SetupGacGenNativeController()
{
	GacGenNativeController controller;
	SetNativeController(&controller);
	GuiApplicationMain();
	SetNativeController(nullptr);
	return 0;
}