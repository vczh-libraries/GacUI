#include "HttpServer.h"

/***********************************************************************
HttpServer (Reading)
***********************************************************************/

void HttpServer::BeginReadingUnsafe()
{
	if (httpRequestIdReading == HTTP_NULL_ID)
	{
		streamReadFile.SeekFromBegin(0);
	}
}

void HttpServer::SubmitReadBufferUnsafe(vint bytes)
{
	if(httpRequestIdReading == HTTP_NULL_ID)
	{
		PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&bufferReadFile[0];
		httpRequestIdReading = pRequest->RequestId;
	}
	streamReadFile.Write(&bufferReadFile[0], bytes);
}

void HttpServer::EndReadingUnsafe()
{
	httpRequestIdReading = HTTP_NULL_ID;
	PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&bufferReadFile[0];

	if (pRequest->Verb == HttpVerbGET && pRequest->CookedUrl.pFullUrl == urlConnect)
	{
		Send404Response(pRequest->RequestId, "Subsequent connection to GacUI Core is denied.");
		return;
	}

	if (pRequest->Verb == HttpVerbPOST && pRequest->CookedUrl.pFullUrl == urlRequest)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	if (pRequest->Verb == HttpVerbPOST && pRequest->CookedUrl.pFullUrl == urlResponse)
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}

void HttpServer::BeginReadingLoopUnsafe()
{
RESTART_LOOP:
	{
		BeginReadingUnsafe();
		ResetEvent(hEventReadFile);
		ZeroMemory(&overlappedReadFile, sizeof(overlappedReadFile));
		overlappedReadFile.hEvent = hEventReadFile;

		ZeroMemory(&bufferReadFile[0], sizeof(HTTP_REQUEST));
		PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&bufferReadFile[0];

		ULONG result = HttpReceiveHttpRequest(
			httpRequestQueue,
			httpRequestIdReading,
			0,
			pRequest,
			(ULONG)bufferReadFile.Count(),
			NULL,
			&overlappedReadFile);

		if (result == ERROR_CONNECTION_INVALID)
		{
			callback->OnReadStoppedThreadUnsafe();
			return;
		}
		CHECK_ERROR(result == ERROR_IO_PENDING, L"HttpReceiveHttpRequest failed on unexpected result.");

		RegisterWaitForSingleObject(
			&hWaitHandleReadFile,
			hEventReadFile,
			[](PVOID lpParameter, BOOLEAN TimerOrWaitFired)
			{
				auto self = (HttpServer*)lpParameter;
				UnregisterWait(self->hWaitHandleReadFile);
				self->hWaitHandleReadFile = INVALID_HANDLE_VALUE;

				DWORD read = 0;
				BOOL result = GetOverlappedResult(self->httpRequestQueue, &self->overlappedReadFile, &read, FALSE);
				if (result == TRUE)
				{
					self->SubmitReadBufferUnsafe((vint)read);
					self->EndReadingUnsafe();
				}
				else
				{
					DWORD error = GetLastError();
					if (error == ERROR_CONNECTION_INVALID)
					{
						self->callback->OnReadStoppedThreadUnsafe();
						return;
					}
					CHECK_ERROR(error == ERROR_MORE_DATA, L"GetOverlappedResult(ReadFile) failed on unexpected GetLastError.");
					self->SubmitReadBufferUnsafe((vint)read);
				}
				self->BeginReadingLoopUnsafe();
			},
			this,
			INFINITE,
			WT_EXECUTEONLYONCE);
	}
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
	: bufferReadFile(HttpBodyBlockSize)
{
	hEventReadFile = CreateEvent(NULL, TRUE, TRUE, NULL);
	CHECK_ERROR(hEventReadFile != NULL, L"HttpServer initialization failed on CreateEvent(hEventReadFile).");

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
	CloseHandle(hEventReadFile);
}

void HttpServer::WaitForClient()
{
	ULONG result = NO_ERROR;
	ULONG bytesReturned = 0;

	while (true)
	{
		ZeroMemory(&bufferReadFile[0], sizeof(HTTP_REQUEST));
		PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&bufferReadFile[0];

		result = HttpReceiveHttpRequest(
			httpRequestQueue,
			HTTP_NULL_ID,
			0,
			pRequest,
			(ULONG)bufferReadFile.Count(),
			&bytesReturned,
			NULL);

		if (result == ERROR_MORE_DATA)
		{
			CHECK_FAIL(L"HttpReceiveHttpRequest failed (ERROR_MORE_DATA).");
		}
		CHECK_ERROR(result == NO_ERROR, L"HttpReceiveHttpRequest failed.");

		if (pRequest->Verb == HttpVerbGET && pRequest->CookedUrl.pFullUrl == urlConnect)
		{
			auto jsonObject = Ptr(new JsonObject);
			{
				auto jsonValue = Ptr(new JsonString);
				jsonValue->content.value = urlRequest.Right(urlRequest.Length() - wcslen(HttpServerUrl) - 7);

				auto jsonField = Ptr(new JsonObjectField);
				jsonField->name.value = WString::Unmanaged(L"request");
				jsonField->value = jsonValue;

				jsonObject->fields.Add(jsonField);
			}
			{
				auto jsonValue = Ptr(new JsonString);
				jsonValue->content.value = urlResponse.Right(urlResponse.Length() - wcslen(HttpServerUrl) - 7);

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

			SendJsonResponse(pRequest->RequestId, jsonObject);
			break;
		}
		else
		{
			Send404Response(pRequest->RequestId, "The first request must be /GacUIRemoting/Connect");
			Console::WriteLine(L"Unexpected request received: " + WString::Unmanaged(pRequest->CookedUrl.pFullUrl));
		}
	}
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