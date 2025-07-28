#ifndef VCZH_REMOTINGTEST_SHARED_HTTPCLIENT
#define VCZH_REMOTINGTEST_SHARED_HTTPCLIENT

#include "ProtocolCallback.h"

class HttpClient : public INetworkProtocol
{
protected:
	INetworkProtocolCallback*						callback = nullptr;

public:
	HttpClient();
	~HttpClient();

	void											WaitForClient();
	void											Stop();

	void											InstallCallback(INetworkProtocolCallback* _callback) override;
	void											BeginReadingLoopUnsafe() override;
	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;
};

#endif