#ifndef VCZH_REMOTINGTEST_SHARED_HTTPSERVERSHARED
#define VCZH_REMOTINGTEST_SHARED_HTTPSERVERSHARED

#include "ProtocolCallback.h"

constexpr const wchar_t* HttpServerUrl = L"localhost:8888/GacUIRemoting";
constexpr const wchar_t* HttpServerUrl_Request = L"Request";
constexpr const wchar_t* HttpServerUrl_Response = L"Response";

class HttpServer : public INetworkProtocol
{
public:
	HttpServer();
	~HttpServer();

	void											WaitForClient();
	void											StopHttpServer();

	void											InstallCallback(INetworkProtocolCallback* callback) override;
	void											BeginReadingLoopUnsafe() override;
	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;
};

#endif