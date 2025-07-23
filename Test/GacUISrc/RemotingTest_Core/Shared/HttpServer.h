#ifndef VCZH_REMOTINGTEST_SHARED_HTTPSERVERSHARED
#define VCZH_REMOTINGTEST_SHARED_HTTPSERVERSHARED

#include "ProtocolCallback.h"

constexpr const wchar_t* HttpServerUrl = L"localhost:8888/GacUIRemoting";

/*
* URL: /Request
* To connect and initialize the server.
* Returns a GUID.
* 
* It can only be called once, all subsequence calls will be rejected.
*/
constexpr const wchar_t* HttpServerUrl_Connect = L"/Connect";

/*
* URL: /Request/GUID
* Client should always maintain a living request on the server.
* 
* Returns only when a request is issued.
* It will be pending or timeout if no request is issued.
* If a request is issued but no living request available, it waits.
*/
constexpr const wchar_t* HttpServerUrl_Request = L"/Request";

/*
* URL: /Response/GUID
* To send responses or events to the server.
* Returns nothing.
*/
constexpr const wchar_t* HttpServerUrl_Response = L"/Response";

class HttpServer : public INetworkProtocol
{
protected:
	INetworkProtocolCallback*						callback = nullptr;

public:
	HttpServer();
	~HttpServer();

	void											WaitForClient();
	void											Stop();

	void											InstallCallback(INetworkProtocolCallback* _callback) override;
	void											BeginReadingLoopUnsafe() override;
	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;
};

#endif