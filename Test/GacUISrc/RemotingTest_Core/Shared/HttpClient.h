#ifndef VCZH_REMOTINGTEST_SHARED_HttpClient
#define VCZH_REMOTINGTEST_SHARED_HttpClient

#include "ProtocolCallback.h"

class HttpClient : public INetworkProtocol
{
public:
	HttpClient();
	~HttpClient();

	void											WaitForClient();
	void											StopHttpClient();

	void											InstallCallback(INetworkProtocolCallback* callback) override;
	void											BeginReadingLoopUnsafe() override;
	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;
};

#endif