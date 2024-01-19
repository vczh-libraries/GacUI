#include "TestRemote.h"

class EmptyWindowProtocol : public NotImplementedProtocolBase
{
public:
	static EmptyWindowProtocol*	instance;
	IGuiRemoteProtocolEvents*	events = nullptr;

	EmptyWindowProtocol()
	{
		CHECK_ERROR(instance == nullptr, L"EmptyWindowProtocol can only have one instance");
		instance = this;
	}

	~EmptyWindowProtocol()
	{
		instance = nullptr;
	}

	void Initialize(IGuiRemoteProtocolEvents* _events) override
	{
		events = _events;
	}

	void Submit() override
	{
		// respond to messages immediately in this test
		// no need to submit
	}

	WString GetExecutablePath() override
	{
		return L"/EmptyWindow/Protocol.exe";
	}

	void RequestControllerGetFontConfig(vint id) override
	{
	}

	void RequestControllerGetScreenConfig(vint id) override
	{
	}

	void RequestControllerConnectionEstablished() override
	{
	}

	void RequestWindowGetBounds(vint id) override
	{
	}
};
EmptyWindowProtocol* EmptyWindowProtocol::instance = nullptr;

TEST_FILE
{
	TEST_CATEGORY(L"Create one window and exit immediately")
	{
		EmptyWindowProtocol protocol;
		SetGuiMainProxy([]()
		{
		});
		SetupRemoteNativeController(&protocol);
		SetGuiMainProxy(nullptr);
	});
}