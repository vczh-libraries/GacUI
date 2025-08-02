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
	static constexpr vint32_t						HttpRespondBodyStep = 65536;
	Array<char8_t>									httpRespondBodyBuffer;
	DWORD											httpRespondBodyBufferWriting = 0;
	DWORD											httpRespondBodyBufferWritingAvailable = 0;

public:

	void											BeginReadingLoopUnsafe() override;

/***********************************************************************
HttpClient (WaitForServer)
***********************************************************************/

protected:

	HANDLE											hEventWaitForServer = INVALID_HANDLE_VALUE;
	DWORD											dwInternetStatus_WaitForServer = 0;
	DWORD											dwStatusInformationLength_WaitForServer = 0;

public:

	void											WaitForServer();

/***********************************************************************
HttpClient (Writing)
***********************************************************************/

protected:
	SpinLock										httpRequestBodiesLock;
	Dictionary<HINTERNET, U8String>					httpRequestBodies;

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