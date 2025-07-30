#ifndef VCZH_REMOTINGTEST_SHARED_HTTPSERVER
#define VCZH_REMOTINGTEST_SHARED_HTTPSERVER

#include "ProtocolCallback.h"

constexpr const wchar_t* HttpServerUrl = L"localhost:8888";

/*
* GET: /Request
* To connect and initialize the server.
* Returns available URLs.
* 
* It can only be called once, all subsequence calls will be rejected.
*/
constexpr const wchar_t* HttpServerUrl_Connect = L"/GacUIRemoting/Connect";

/*
* POST: /Request/GUID
* Client should always maintain a living request on the server.
* 
* Returns only when a request is issued.
* It will be pending or timeout if no request is issued.
* If a request is issued but no living request available, it waits.
*/
constexpr const wchar_t* HttpServerUrl_Request = L"/GacUIRemoting/Request";

/*
* POST: /Response/GUID
* To send responses or events to the server.
* Returns nothing.
*/
constexpr const wchar_t* HttpServerUrl_Response = L"/GacUIRemoting/Response";

class HttpServer : public INetworkProtocol
{
	static constexpr vint32_t						HttpBodyInitSize = 1024;

protected:
	INetworkProtocolCallback*						callback = nullptr;
	WString											urlGuid;
	WString											urlConnect;
	WString											urlRequest;
	WString											urlResponse;

	HANDLE											httpRequestQueue = INVALID_HANDLE_VALUE;
	HTTP_SERVER_SESSION_ID							httpSessionId = HTTP_NULL_ID;
	HTTP_URL_GROUP_ID								httpUrlGroupId = HTTP_NULL_ID;

/***********************************************************************
HttpServer (Reading)
***********************************************************************/

protected:

	enum class State
	{
		Ready,
		WaitForClientConnection,
		Running,
		Stopping,
	};

	State											state = State::Ready;

	Array<BYTE>										bufferRequest;
	HANDLE											hWaitHandleRequest = INVALID_HANDLE_VALUE;
	OVERLAPPED										overlappedRequest;
	HANDLE											hEventRequest = INVALID_HANDLE_VALUE;

	void											OnHttpConnectionBrokenUnsafe();
	void											OnHttpRequestReceivedUnsafe(PHTTP_REQUEST pRequest);
	ULONG											ListenToHttpRequest_Init(OVERLAPPED* overlapped);
	ULONG											ListenToHttpRequest_MoreData(vint expectedBufferSize);
	void											ListenToHttpRequest();

/***********************************************************************
HttpServer (WaitForClient)
***********************************************************************/

protected:
	HANDLE											hEventWaitForClient = INVALID_HANDLE_VALUE;

	void											WaitForClient_OnHttpConnectionBrokenUnsafe();
	void											WaitForClient_OnHttpRequestReceivedUnsafe(PHTTP_REQUEST pRequest);

public:

	void											WaitForClient();

/***********************************************************************
HttpServer (BeginReadingLoopUnsafe)
***********************************************************************/

protected:

	void											BeginReadingLoopUnsafe_OnHttpConnectionBrokenUnsafe();
	void											BeginReadingLoopUnsafe_OnHttpRequestReceivedUnsafe(PHTTP_REQUEST pRequest);

public:
	void											BeginReadingLoopUnsafe() override;

/***********************************************************************
HttpServer (Writing)
***********************************************************************/

protected:
	HTTP_REQUEST_ID									httpRequestIdCurrent = HTTP_NULL_ID;

	HTTP_REQUEST_ID									WaitForRequest();
	void											Send404Response(HTTP_REQUEST_ID requestId, PCSTR reason);
	void											SendJsonResponse(HTTP_REQUEST_ID requestId, Ptr<JsonNode> jsonBody);

/***********************************************************************
HttpServer
***********************************************************************/

public:
	HttpServer();
	~HttpServer();

	void											Stop();

	void											InstallCallback(INetworkProtocolCallback* _callback) override;
	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;
};

#endif