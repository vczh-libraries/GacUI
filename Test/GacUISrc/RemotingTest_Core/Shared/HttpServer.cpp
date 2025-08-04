#include "HttpServer.h"

/***********************************************************************
HttpServer (ListenToHttpRequest)
***********************************************************************/

void HttpServer::OnHttpConnectionBrokenUnsafe()
{
	switch (state)
	{
	case State::WaitForClientConnection:
		CHECK_FAIL(L"HTTP server stopped while waiting for client connection.");
		break;
	case State::Running:
		state = State::Stopping;
		callback->OnReadStoppedThreadUnsafe();
		break;
	default:
		CHECK_FAIL(L"Unexpected HTTP request.");
	}
}

void HttpServer::OnHttpRequestReceivedUnsafe(PHTTP_REQUEST pRequest)
{
	if (pRequest->Verb == HttpVerbGET && wcscmp(pRequest->CookedUrl.pAbsPath, HttpServerUrl_Connect) == 0)
	{
		GenerateNewUrls();
		SendConnectResponse(pRequest);

		if (state == State::WaitForClientConnection)
		{
			state = State::Running;
			SetEvent(hEventWaitForClient);
		}
	}
	else if (pRequest->Verb == HttpVerbPOST && pRequest->CookedUrl.pAbsPath == urlRequest)
	{
		SPIN_LOCK(pendingRequestLock)
		{
			OnNewHttpRequestForPendingRequest(pRequest->RequestId);
		}
	}
	else if (pRequest->Verb == HttpVerbPOST && pRequest->CookedUrl.pAbsPath == urlResponse)
	{
		SubmitResponse(pRequest);
		ULONG result = SendJsonResponse(httpRequestQueue, pRequest->RequestId, Ptr(new JsonObject));
		CHECK_ERROR(result == NO_ERROR, L"HttpSendHttpResponse failed for responding /Response.");
	}
	else
	{
		Send404Response(httpRequestQueue, pRequest->RequestId, "Unknown URL");
	}
	ListenToHttpRequest();
}

ULONG HttpServer::ListenToHttpRequest_Init(OVERLAPPED* overlapped)
{
	ZeroMemory(&bufferRequest[0], bufferRequest.Count());

	ULONG result = HttpReceiveHttpRequest(
		httpRequestQueue,
		HTTP_NULL_ID,
		0,
		(PHTTP_REQUEST)&bufferRequest[0],
		(ULONG)bufferRequest.Count(),
		NULL,
		overlapped);

	return result;
}

ULONG HttpServer::ListenToHttpRequest_InitMoreData(ULONG* bytesReturned)
{
	HTTP_REQUEST_ID httpRequestIdReading = ((PHTTP_REQUEST)&bufferRequest[0])->RequestId;
	ZeroMemory(&bufferRequest[0], bufferRequest.Count());

	ULONG result = HttpReceiveHttpRequest(
		httpRequestQueue,
		httpRequestIdReading,
		0,
		(PHTTP_REQUEST)&bufferRequest[0],
		(ULONG)bufferRequest.Count(),
		bytesReturned,
		NULL);

	return result;
}

ULONG HttpServer::ListenToHttpRequest_OverlappedMoreData(vint expectedBufferSize)
{
	HTTP_REQUEST_ID httpRequestIdReading = ((PHTTP_REQUEST)&bufferRequest[0])->RequestId;
	bufferRequest.Resize(expectedBufferSize);
	ZeroMemory(&bufferRequest[0], bufferRequest.Count());

	ULONG bytesReturned = 0;
	ULONG result = HttpReceiveHttpRequest(
		httpRequestQueue,
		httpRequestIdReading,
		0,
		(PHTTP_REQUEST)&bufferRequest[0],
		(ULONG)bufferRequest.Count(),
		&bytesReturned,
		NULL);

	return result;
}

void HttpServer::ListenToHttpRequest()
{
	ResetEvent(hEventRequest);
	ZeroMemory(&overlappedRequest, sizeof(overlappedRequest));
	overlappedRequest.hEvent = hEventRequest;

	ZeroMemory(&bufferRequest[0], sizeof(HTTP_REQUEST));
	PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&bufferRequest[0];

	ULONG result = ListenToHttpRequest_Init(&overlappedRequest);
	if (result == ERROR_CONNECTION_INVALID || result == ERROR_OPERATION_ABORTED)
	{
		OnHttpConnectionBrokenUnsafe();
		return;
	}

	if (result == NO_ERROR)
	{
		OnHttpRequestReceivedUnsafe(pRequest);
		return;
	}

	if (result == ERROR_MORE_DATA)
	{
		ULONG bytesReturned = 0;
		result = ListenToHttpRequest_InitMoreData(&bytesReturned);
		if (result == ERROR_CONNECTION_INVALID || result == ERROR_OPERATION_ABORTED)
		{
			OnHttpConnectionBrokenUnsafe();
			return;
		}
		CHECK_ERROR(result == ERROR_MORE_DATA, L"HttpReceiveHttpRequest(#1) failed on unexpected result.");

		result = ListenToHttpRequest_OverlappedMoreData((vint)bytesReturned);
		if (result == ERROR_CONNECTION_INVALID || result == ERROR_OPERATION_ABORTED)
		{
			OnHttpConnectionBrokenUnsafe();
			return;
		}
		CHECK_ERROR(result == NO_ERROR, L"HttpReceiveHttpRequest(#2) failed on unexpected result.");

		PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&bufferRequest[0];
		OnHttpRequestReceivedUnsafe(pRequest);
		return;
	}

	CHECK_ERROR(result == ERROR_IO_PENDING, L"HttpReceiveHttpRequest(#3) failed on unexpected result.");

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
				if (error == ERROR_CONNECTION_INVALID || error == ERROR_OPERATION_ABORTED)
				{
					self->OnHttpConnectionBrokenUnsafe();
					return;
				}
				CHECK_ERROR(error == ERROR_MORE_DATA, L"GetOverlappedResult(#4) failed on unexpected GetLastError.");
				CHECK_ERROR(self->bufferRequest.Count() < (vint)read, L"GetOverlappedResult(#5) failed on unexpected read size.");

				ULONG result = self->ListenToHttpRequest_OverlappedMoreData((vint)read);
				if (result == ERROR_CONNECTION_INVALID || result == ERROR_OPERATION_ABORTED)
				{
					self->OnHttpConnectionBrokenUnsafe();
					return;
				}
				CHECK_ERROR(result == NO_ERROR, L"HttpReceiveHttpRequest(#6) failed on unexpected result.");

				PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&self->bufferRequest[0];
				self->OnHttpRequestReceivedUnsafe(pRequest);
			}
		},
		this,
		INFINITE,
		WT_EXECUTEONLYONCE);
}

/***********************************************************************
HttpServer (WaitForClient)
***********************************************************************/

void HttpServer::GenerateNewUrls()
{
	RPC_STATUS status = -1;
	UUID guid;
	status = UuidCreate(&guid);
	CHECK_ERROR(status == RPC_S_OK, L"UuidCreate failed.");

	RPC_WSTR guidString = nullptr;
	status = UuidToString(&guid, &guidString);
	CHECK_ERROR(status == RPC_S_OK, L"UuidToString failed.");

	urlRequest = WString::Unmanaged(HttpServerUrl_Request) + L"/" + WString::Unmanaged(guidString);
	urlResponse = WString::Unmanaged(HttpServerUrl_Response) + L"/" + WString::Unmanaged(guidString);

	status = RpcStringFree(&guidString);
	CHECK_ERROR(status == RPC_S_OK, L"RpcStringFree failed.");
}

void HttpServer::SendConnectResponse(PHTTP_REQUEST pRequest)
{
	auto jsonObject = Ptr(new JsonObject);
	{
		auto jsonValue = Ptr(new JsonString);
		jsonValue->content.value = urlRequest;

		auto jsonField = Ptr(new JsonObjectField);
		jsonField->name.value = WString::Unmanaged(L"request");
		jsonField->value = jsonValue;

		jsonObject->fields.Add(jsonField);
	}
	{
		auto jsonValue = Ptr(new JsonString);
		jsonValue->content.value = urlResponse;

		auto jsonField = Ptr(new JsonObjectField);
		jsonField->name.value = WString::Unmanaged(L"response");
		jsonField->value = jsonValue;

		jsonObject->fields.Add(jsonField);
	}
	{
		auto jsonValue = Ptr(new JsonString);
		jsonValue->content.value = WString::Unmanaged(L"request to wait for next request; response to send events with one optional response.");

		auto jsonField = Ptr(new JsonObjectField);
		jsonField->name.value = WString::Unmanaged(L"comments");
		jsonField->value = jsonValue;

		jsonObject->fields.Add(jsonField);
	}

	ULONG result = SendJsonResponse(httpRequestQueue, pRequest->RequestId, jsonObject);
	CHECK_ERROR(result == NO_ERROR, L"HttpSendHttpResponse failed for establishing a connection.");
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

void HttpServer::SubmitResponse(PHTTP_REQUEST pRequest)
{
	ULONG bodyLength = 0;
	ULONG bodyReceived = 0;
	{
		auto& headerContentType = pRequest->Headers.KnownHeaders[HttpHeaderContentType];
		CHECK_ERROR(headerContentType.pRawValue != NULL, L"/Response missing Content-Type header.");
		CHECK_ERROR(
			strncmp((const char*)headerContentType.pRawValue, "application/json; charset=utf8", headerContentType.RawValueLength) == 0,
			L"/Response Content-Type header must be \"application/json; charset=utf8\".");
	}
	{
		auto& headerContentLength = pRequest->Headers.KnownHeaders[HttpHeaderContentLength];
		CHECK_ERROR(headerContentLength.pRawValue != NULL, L"/Response missing Content-Type header.");
		bodyLength = (ULONG)atoi(headerContentLength.pRawValue);
	}
	CHECK_ERROR(pRequest->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS, L"/Response must contain body data.");

	Array<char8_t> bodyBuffer(bodyLength + 1);
	ZeroMemory(&bodyBuffer[0], bodyBuffer.Count() * sizeof(char8_t));
	{
		ULONG result = NO_ERROR;
		result = HttpReceiveRequestEntityBody(
			httpRequestQueue,
			pRequest->RequestId,
			HTTP_RECEIVE_REQUEST_ENTITY_BODY_FLAG_FILL_BUFFER,
			&bodyBuffer[0],
			bodyLength,
			&bodyReceived,
			NULL);
		CHECK_ERROR(result == NO_ERROR, L"HttpReceiveRequestEntityBody.");
	}
	{
		U8String bodyUtf8 = U8String::Unmanaged(&bodyBuffer[0]);
		auto bodyJson = JsonParse(u8tow(bodyUtf8), jsonParser);
		auto bodyArray = bodyJson.Cast<JsonArray>();
		CHECK_ERROR(bodyArray, L"/Response body must be a JSON array of strings.");

		auto strs = Ptr(new List<WString>);
		for (auto&& item : bodyArray->items)
		{
			auto itemString = item.Cast<JsonString>();
			CHECK_ERROR(itemString, L"/Response body must be a JSON array of strings.");
			strs->Add(itemString->content.value);
		}
		callback->OnReadStringThreadUnsafe(strs);
	}
}

void HttpServer::BeginReadingLoopUnsafe()
{
	// Does nothing since ListenToHttpRequest is looping after WaitForClient is called
}

/***********************************************************************
HttpServer (Writing)
***********************************************************************/

void HttpServer::Send404Response(HANDLE httpRequestQueue, HTTP_REQUEST_ID requestId, PCSTR reason)
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
	CHECK_ERROR(result == NO_ERROR, L"HttpSendHttpResponse failed (404).");
}

ULONG HttpServer::SendJsonResponse(HANDLE httpRequestQueue, HTTP_REQUEST_ID requestId, Ptr<JsonNode> jsonBody)
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
	return result;
}

void HttpServer::OnNewHttpRequestForPendingRequest(HTTP_REQUEST_ID httpRequestId)
{
	if (httpPendingRequestId != HTTP_NULL_ID)
	{
		ULONG result = HttpCancelHttpRequest(
			httpRequestQueue,
			httpPendingRequestId,
			NULL);
		CHECK_ERROR(
			result == NO_ERROR || result == ERROR_CONNECTION_INVALID || result == ERROR_OPERATION_ABORTED,
			L"HttpCancelHttpRequest failed for canceling outdated /Request.");
	}
	httpPendingRequestId = httpRequestId;
	if (pendingRequestToSend)
	{
		ULONG result = SendJsonResponse(httpRequestQueue, httpPendingRequestId, pendingRequestToSend);
		CHECK_ERROR(result == NO_ERROR, L"HttpSendHttpResponse failed for responding /Request.");
		pendingRequestToSend = nullptr;
	}
}

void HttpServer::BeginSubmitPendingRequest()
{
	if (!pendingRequestToSend)
	{
		pendingRequestToSend = Ptr(new JsonArray);
	}
}

void HttpServer::EndSubmitPendingRequest()
{
	if (httpPendingRequestId != HTTP_NULL_ID && pendingRequestToSend)
	{
		ULONG result = SendJsonResponse(httpRequestQueue, httpPendingRequestId, pendingRequestToSend);
		if (result == NO_ERROR)
		{
			httpPendingRequestId = HTTP_NULL_ID;
			pendingRequestToSend = nullptr;
		}
		else if (result == ERROR_CONNECTION_INVALID || result == ERROR_OPERATION_ABORTED)
		{
			httpPendingRequestId = HTTP_NULL_ID;
		}
		else
		{
			CHECK_FAIL(L"HttpSendHttpResponse failed for responding /Request.");
		}
	}
}

void HttpServer::SendStringArray(vint count, List<WString>& strs)
{
	SPIN_LOCK(pendingRequestLock)
	{
		BeginSubmitPendingRequest();
		for (vint i = 0; i < count; i++)
		{
			auto jsonValue = Ptr(new JsonString);
			jsonValue->content.value = strs[i];
			pendingRequestToSend->items.Add(jsonValue);
		}
		EndSubmitPendingRequest();
	}
}

void HttpServer::SendSingleString(const WString& str)
{
	SPIN_LOCK(pendingRequestLock)
	{
		BeginSubmitPendingRequest();
		auto jsonValue = Ptr(new JsonString);
		jsonValue->content.value = str;
		pendingRequestToSend->items.Add(jsonValue);
		EndSubmitPendingRequest();
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
			(WString::Unmanaged(L"http://") + WString::Unmanaged(HttpServerUrl) + WString::Unmanaged(HttpServerUrl_Connect)).Buffer(),
			0,
			0);
		CHECK_ERROR(result == NO_ERROR, L"HttpAddUrlToUrlGroup failed (urlConnect).");

		result = HttpAddUrlToUrlGroup(
			httpUrlGroupId,
			(WString::Unmanaged(L"http://") + WString::Unmanaged(HttpServerUrl) + WString::Unmanaged(HttpServerUrl_Request)).Buffer(),
			0,
			0);
		CHECK_ERROR(result == NO_ERROR, L"HttpAddUrlToUrlGroup failed (urlRequest).");

		result = HttpAddUrlToUrlGroup(
			httpUrlGroupId,
			(WString::Unmanaged(L"http://") + WString::Unmanaged(HttpServerUrl) + WString::Unmanaged(HttpServerUrl_Response)).Buffer(),
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