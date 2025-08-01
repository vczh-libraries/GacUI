#ifndef VCZH_REMOTINGTEST_SHARED_HTTPCLIENT
#define VCZH_REMOTINGTEST_SHARED_HTTPCLIENT

#include "ProtocolCallback.h"

class HttpClient : public INetworkProtocol
{
protected:
	INetworkProtocolCallback*						callback = nullptr;

	HINTERNET										httpSession = NULL;
	HINTERNET										httpConnection = NULL;
	WString											urlRequest;
	WString											urlResponse;

/***********************************************************************
HttpClient (Reading)
***********************************************************************/

protected:

public:

	void											BeginReadingLoopUnsafe() override;

/***********************************************************************
HttpClient (WaitForServer)
***********************************************************************/

	void											WaitForServer();

/***********************************************************************
HttpClient (Writing)
***********************************************************************/

protected:

	void											SendJsonRequest(Ptr<JsonNode> jsonBody);

public:

	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;

/***********************************************************************
HttpClient
***********************************************************************/

public:
	HttpClient();
	~HttpClient();
	void											Stop();

	void											InstallCallback(INetworkProtocolCallback* _callback) override;
};

#endif