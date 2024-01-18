#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

extern void SetGuiMainProxy(void(*proxy)());

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

class StartUpProtocol : public Object, public IGuiRemoteProtocol
{
public:
	IGuiRemoteProtocolEvents* events = nullptr;

	void Initialize(IGuiRemoteProtocolEvents* _events) override
	{
		events = _events;
	}

	void Submit() override
	{
	}

	WString GetExecutablePath() override
	{
		return L"/StartUp/Protocol.exe";
	}

	void RequestGetFontConfig(vint id) override
	{
	}

	void RequestGetScreenConfig(vint id) override
	{
	}

	void RequestConnectionEstablished() override
	{
	}
};

TEST_FILE
{
	TEST_CASE(L"Start and Stop")
	{
		StartUpProtocol protocol;
		SetGuiMainProxy([]()
		{
		});
		SetupRemoteNativeController(&protocol);
		SetGuiMainProxy(nullptr);
	});
}