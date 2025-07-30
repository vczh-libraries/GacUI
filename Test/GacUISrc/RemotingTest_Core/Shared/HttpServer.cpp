#include "HttpServer.h"

/***********************************************************************
HttpServer (Reading)
***********************************************************************/

void HttpServer::OnHttpConnectionBrokenUnsafe()
{
	switch (state)
	{
	case State::WaitForClientConnection:
		WaitForClient_OnHttpConnectionBrokenUnsafe();
		break;
	case State::Running:
		BeginReadingLoopUnsafe_OnHttpConnectionBrokenUnsafe();
		break;
	default:
		CHECK_FAIL(L"Unexpected HTTP request.");
	}
}

void HttpServer::OnHttpRequestReceivedUnsafe(PHTTP_REQUEST pRequest)
{
	switch (state)
	{
	case State::WaitForClientConnection:
		WaitForClient_OnHttpRequestReceivedUnsafe(pRequest);
		break;
	case State::Running:
		BeginReadingLoopUnsafe_OnHttpRequestReceivedUnsafe(pRequest);
		break;
	default:
		CHECK_FAIL(L"Unexpected HTTP request.");
	}
}

void HttpServer::ListenToHttpRequest()
{
	ResetEvent(hEventRequest);
	ZeroMemory(&overlappedRequest, sizeof(overlappedRequest));
	overlappedRequest.hEvent = hEventRequest;

	ZeroMemory(&bufferRequest[0], sizeof(HTTP_REQUEST));
	PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&bufferRequest[0];

	ULONG result = HttpReceiveHttpRequest(
		httpRequestQueue,
		HTTP_NULL_ID,
		0,
		pRequest,
		(ULONG)bufferRequest.Count(),
		NULL,
		&overlappedRequest);

	if (result == ERROR_CONNECTION_INVALID)
	{
		OnHttpConnectionBrokenUnsafe();
		return;
	}
	CHECK_ERROR(result == ERROR_IO_PENDING, L"HttpReceiveHttpRequest(Overlapped) failed on unexpected result.");

	RegisterWaitForSingleObject(
		&hWaitHandleRequest,
		hEventRequest,
		[](PVOID lpParameter, BOOLEAN TimerOrWaitFired)
		{
			auto self = (HttpServer*)lpParameter;
			UnregisterWait(self->hWaitHandleRequest);
			self->hWaitHandleRequest = INVALID_HANDLE_VALUE;

			DWORD read = 0;
			BOOL result = GetOverlappedResult(self->httpRequestQueue, &self->overlappedRequest, &read, FALSE);
			if (result == TRUE)
			{
				PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&self->bufferRequest[0];
				self->OnHttpRequestReceivedUnsafe(pRequest);
			}
			else
			{
				DWORD error = GetLastError();
				if (error == ERROR_CONNECTION_INVALID)
				{
					self->OnHttpConnectionBrokenUnsafe();
					return;
				}
				CHECK_ERROR(error == ERROR_MORE_DATA, L"GetOverlappedResult(Request) failed on unexpected GetLastError.");
				CHECK_ERROR(self->bufferRequest.Count() < (vint)read, L"GetOverlappedResult(Request) failed on unexpected read size.");

				PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&self->bufferRequest[0];
				HTTP_REQUEST_ID httpRequestIdReading = pRequest->RequestId;
				self->bufferRequest.Resize((vint)read);
				ZeroMemory(&self->bufferRequest[0], sizeof(HTTP_REQUEST));

				ULONG bytesReturned = 0;
				ULONG result = HttpReceiveHttpRequest(
					self->httpRequestQueue,
					httpRequestIdReading,
					0,
					pRequest,
					(ULONG)self->bufferRequest.Count(),
					&bytesReturned,
					NULL);

				if (result == ERROR_CONNECTION_INVALID)
				{
					self->OnHttpConnectionBrokenUnsafe();
					return;
				}
				CHECK_ERROR(result == NO_ERROR, L"HttpReceiveHttpRequest(Request) failed on unexpected result.");
				self->OnHttpRequestReceivedUnsafe(pRequest);
			}
			self->BeginReadingLoopUnsafe();
		},
		this,
		INFINITE,
		WT_EXECUTEONLYONCE);
}

/***********************************************************************
HttpServer (WaitForClient)
***********************************************************************/

void HttpServer::WaitForClient_OnHttpConnectionBrokenUnsafe()
{
	CHECK_FAIL(L"HTTP server stopped while waiting for client connection.");
}

void HttpServer::WaitForClient_OnHttpRequestReceivedUnsafe(PHTTP_REQUEST pRequest)
{
	if (pRequest->Verb == HttpVerbGET && pRequest->CookedUrl.pFullUrl == urlConnect)
	{
		Send404Response(pRequest->RequestId, "The first request must be /GacUIRemoting/Connect");
	}
	else
	{
		Send404Response(pRequest->RequestId, "The first request must be /GacUIRemoting/Connect");
		Console::WriteLine(L"Unexpected request received: " + WString::Unmanaged(pRequest->CookedUrl.pFullUrl));
		ListenToHttpRequest();
	}
}

void HttpServer::WaitForClient()
{
	CHECK_ERROR(state == State::Ready, L"WaitForClient() can only be called for once.");

	state = State::WaitForClientConnection;
	ResetEvent(hEventWaitForClient);
	ListenToHttpRequest();
	WaitForSingleObject(hEventWaitForClient, INFINITE);

	CHECK_ERROR(state == State::Running, L"WaitForClient() failed to connect to a client.");
}

/***********************************************************************
HttpServer (BeginReadingLoopUnsafe)
***********************************************************************/

void HttpServer::BeginReadingLoopUnsafe_OnHttpConnectionBrokenUnsafe()
{
	state = State::Stopping;
	callback->OnReadStoppedThreadUnsafe();
}

void HttpServer::BeginReadingLoopUnsafe_OnHttpRequestReceivedUnsafe(PHTTP_REQUEST pRequest)
{
	if (pRequest->Verb == HttpVerbGET && pRequest->CookedUrl.pFullUrl == urlConnect)
	{
		Send404Response(pRequest->RequestId, "Subsequential requests to /GacUIRemoting/Connect is denied");
	}
	else if (pRequest->Verb == HttpVerbPOST && pRequest->CookedUrl.pFullUrl == urlRequest)
	{
		Send404Response(pRequest->RequestId, "Not Implemented");
	}
	else if (pRequest->Verb == HttpVerbPOST && pRequest->CookedUrl.pFullUrl == urlResponse)
	{
		Send404Response(pRequest->RequestId, "Not Implemented");
	}
	else
	{
		Send404Response(pRequest->RequestId, "Unknown URL");
	}
	ListenToHttpRequest();
}

void HttpServer::BeginReadingLoopUnsafe()
{
	ListenToHttpRequest();
}

/***********************************************************************
HttpServer (Writing)
***********************************************************************/

HTTP_REQUEST_ID HttpServer::WaitForRequest()
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpServer::Send404Response(HTTP_REQUEST_ID requestId, PCSTR reason)
{
	ULONG bytesSent = 0;
	HTTP_RESPONSE httpResponse;
	ZeroMemory(&httpResponse, sizeof(httpResponse));

	httpResponse.StatusCode = 404;
	httpResponse.pReason = reason;

	ULONG result = NO_ERROR;
	result = HttpSendHttpResponse(
		httpRequestQueue,
		requestId,
		0,
		&httpResponse,
		NULL,
		&bytesSent,
		NULL,
		0,
		NULL,
		NULL);
	CHECK_ERROR(result == NO_ERROR, L"HttpSendResponse failed (404).");
}

void HttpServer::SendJsonResponse(HTTP_REQUEST_ID requestId, Ptr<JsonNode> jsonBody)
{
	ULONG bytesSent = 0;
	HTTP_RESPONSE httpResponse;
	HTTP_DATA_CHUNK httpResponseBody;
	ZeroMemory(&httpResponse, sizeof(httpResponse));
	ZeroMemory(&httpResponseBody, sizeof(httpResponseBody));

	httpResponse.StatusCode = 200;
	httpResponse.pReason = "OK";
	httpResponse.EntityChunkCount = 1;
	httpResponse.pEntityChunks = &httpResponseBody;

	U8String body = wtou8(JsonToString(jsonBody));
	httpResponseBody.DataChunkType = HttpDataChunkFromMemory;
	httpResponseBody.FromMemory.pBuffer = (PVOID)body.Buffer();
	httpResponseBody.FromMemory.BufferLength = (ULONG)body.Length();

	char headerContentType[] = "application/json; charset=utf8";
	httpResponse.Headers.KnownHeaders[HttpHeaderContentType].pRawValue = headerContentType;
	httpResponse.Headers.KnownHeaders[HttpHeaderContentType].RawValueLength = sizeof(headerContentType) - 1;

	ULONG result = NO_ERROR;
	result = HttpSendHttpResponse(
		httpRequestQueue,
		requestId,
		0,
		&httpResponse,
		NULL,
		&bytesSent,
		NULL,
		0,
		NULL,
		NULL);
	CHECK_ERROR(result == NO_ERROR, L"HttpSendResponse failed (200).");
}

void HttpServer::SendStringArray(vint count, List<WString>& strs)
{
	auto requestId = WaitForRequest();
	if (requestId != HTTP_NULL_ID)
	{
		auto jsonArray = Ptr(new JsonArray);
		for (vint i = 0; i < count; i++)
		{
			auto jsonValue = Ptr(new JsonString);
			jsonValue->content.value = strs[i];
			jsonArray->items.Add(jsonValue);
		}

		SendJsonResponse(requestId, jsonArray);
	}
}

void HttpServer::SendSingleString(const WString& str)
{
	auto requestId = WaitForRequest();
	if (requestId != HTTP_NULL_ID)
	{
		auto jsonValue = Ptr(new JsonString);
		jsonValue->content.value = str;

		auto jsonArray = Ptr(new JsonArray);
		jsonArray->items.Add(jsonValue);

		SendJsonResponse(requestId, jsonArray);
	}
}

/***********************************************************************
HttpServer
***********************************************************************/

HttpServer::HttpServer()
	: bufferRequest(HttpBodyInitSize)
{
	hEventRequest = CreateEvent(NULL, TRUE, TRUE, NULL);
	CHECK_ERROR(hEventRequest != NULL, L"HttpServer initialization failed on CreateEvent(hEventRequest).");

	hEventWaitForClient = CreateEvent(NULL, TRUE, TRUE, NULL);
	CHECK_ERROR(hEventWaitForClient != NULL, L"HttpServer initialization failed on CreateEvent(hEventWaitForClient).");

	{
		RPC_STATUS status = -1;
		UUID guid;
		status = UuidCreate(&guid);
		CHECK_ERROR(status == RPC_S_OK, L"UuidCreate failed.");

		RPC_WSTR guidString = nullptr;
		status = UuidToString(&guid, &guidString);
		CHECK_ERROR(status == RPC_S_OK, L"UuidToString failed.");

		urlGuid = guidString;
		urlConnect = WString::Unmanaged(L"http://") + WString::Unmanaged(HttpServerUrl) + WString::Unmanaged(HttpServerUrl_Connect);
		urlRequest = WString::Unmanaged(L"http://") + WString::Unmanaged(HttpServerUrl) + WString::Unmanaged(HttpServerUrl_Request) + L"/" + urlGuid;
		urlResponse = WString::Unmanaged(L"http://") + WString::Unmanaged(HttpServerUrl) + WString::Unmanaged(HttpServerUrl_Response) + L"/" + urlGuid;

		status = RpcStringFree(&guidString);
		CHECK_ERROR(status == RPC_S_OK, L"RpcStringFree failed.");
	}
	{
		ULONG result = NO_ERROR;

		result = HttpInitialize(
			HTTPAPI_VERSION_2,
			HTTP_INITIALIZE_SERVER,
			NULL);
		CHECK_ERROR(result == NO_ERROR, L"HttpInitialize failed.");

		result = HttpCreateRequestQueue(
			HTTPAPI_VERSION_2,
			NULL,
			NULL,
			0,
			&httpRequestQueue);
		CHECK_ERROR(result == NO_ERROR, L"HttpCreateRequestQueue failed.");

		result = HttpCreateServerSession(
			HTTPAPI_VERSION_2,
			&httpSessionId,
			0);
		CHECK_ERROR(result == NO_ERROR, L"HttpCreateServerSession failed.");

		result = HttpCreateUrlGroup(
			httpSessionId,
			&httpUrlGroupId,
			0);
		CHECK_ERROR(result == NO_ERROR, L"HttpCreateUrlGroup failed.");
	}
	{
		ULONG result = NO_ERROR;

		result = HttpAddUrlToUrlGroup(
			httpUrlGroupId,
			urlConnect.Buffer(),
			0,
			0);
		CHECK_ERROR(result == NO_ERROR, L"HttpAddUrlToUrlGroup failed (urlConnect).");

		result = HttpAddUrlToUrlGroup(
			httpUrlGroupId,
			urlRequest.Buffer(),
			0,
			0);
		CHECK_ERROR(result == NO_ERROR, L"HttpAddUrlToUrlGroup failed (urlRequest).");

		result = HttpAddUrlToUrlGroup(
			httpUrlGroupId,
			urlResponse.Buffer(),
			0,
			0);
		CHECK_ERROR(result == NO_ERROR, L"HttpAddUrlToUrlGroup failed (urlResponse).");
	}
	{
		ULONG result = NO_ERROR;

		HTTP_BINDING_INFO bindingInfo;
		ZeroMemory(&bindingInfo, sizeof(bindingInfo));
		bindingInfo.Flags.Present = 1;
		bindingInfo.RequestQueueHandle = httpRequestQueue;

		result = HttpSetUrlGroupProperty(
			httpUrlGroupId,
			HttpServerBindingProperty,
			&bindingInfo,
			sizeof(bindingInfo));
		CHECK_ERROR(result == NO_ERROR, L"HttpSetUrlGroupProperty failed (HttpServerBindingProperty).");
	}
}

HttpServer::~HttpServer()
{
	Stop();
	CloseHandle(hEventRequest);
	CloseHandle(hEventWaitForClient);
}

void HttpServer::Stop()
{
	if (httpRequestQueue != INVALID_HANDLE_VALUE)
	{
		HttpCloseUrlGroup(httpUrlGroupId);
		HttpCloseServerSession(httpSessionId);
		HttpCloseRequestQueue(httpRequestQueue);
		httpRequestQueue = INVALID_HANDLE_VALUE;

		HttpTerminate(
			HTTP_INITIALIZE_SERVER,
			NULL);
	}
}

void HttpServer::InstallCallback(INetworkProtocolCallback* _callback)
{
	callback = _callback;
}