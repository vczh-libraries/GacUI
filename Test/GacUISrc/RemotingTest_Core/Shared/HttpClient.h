#ifndef VCZH_REMOTINGTEST_SHARED_HTTPCLIENT
#define VCZH_REMOTINGTEST_SHARED_HTTPCLIENT

#include "ProtocolCallback.h"

class HttpClient : public INetworkProtocol
{
protected:

	enum class State
	{
		Ready,
		WaitForServerConnection,
		Running,
		Stopping,
	};

	State											state = State::Ready;
	glr::json::Parser								jsonParser;
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

protected:

	HANDLE											hEventWaitForServer = INVALID_HANDLE_VALUE;
	DWORD											dwInternetStatus_WaitForServer = 0;
	LPVOID											lpvStatusInformation_WaitForServer = NULL;
	DWORD											dwStatusInformationLength_WaitForServer = 0;

	void											WinHttpStatusCallback_WaitForServer(DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

public:

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

protected:

	void											WinHttpStatusCallback(DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

public:
	HttpClient();
	~HttpClient();
	void											Stop();

	void											InstallCallback(INetworkProtocolCallback* _callback) override;
};

#endif