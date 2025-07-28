#include "HttpServer.h"

using namespace vl::glr::json;

HttpServer::HttpServer()
{
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
}

void HttpServer::WaitForClient()
{
	ULONG result = NO_ERROR;
	ULONG bytesReturned = 0;

	Array<BYTE> httpRequest(sizeof(HTTP_REQUEST) + 4096);

	while (true)
	{
		ZeroMemory(&httpRequest[0], httpRequest.Count());
		PHTTP_REQUEST pRequest = (PHTTP_REQUEST)&httpRequest[0];

		result = HttpReceiveHttpRequest(
			httpRequestQueue,
			HTTP_NULL_ID,
			0,
			pRequest,
			(ULONG)httpRequest.Count(),
			&bytesReturned,
			NULL);

		if (result == ERROR_MORE_DATA)
		{
			CHECK_FAIL(L"HttpReceiveHttpRequest failed (ERROR_MORE_DATA).");
		}
		CHECK_ERROR(result == NO_ERROR, L"HttpReceiveHttpRequest failed.");

		if (pRequest->Verb == HttpVerbGET && pRequest->CookedUrl.pFullUrl == urlConnect)
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

			U8String body;
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

				JsonFormatting formatting;
				formatting.spaceAfterColon = true;
				formatting.spaceAfterComma = true;
				formatting.crlf = true;
				formatting.compact = false;
				formatting.indentation = L"  ";
				body = wtou8(JsonToString(jsonObject, formatting));
			}
			httpResponseBody.DataChunkType = HttpDataChunkFromMemory;
			httpResponseBody.FromMemory.pBuffer = (PVOID)body.Buffer();
			httpResponseBody.FromMemory.BufferLength = (ULONG)body.Length();

			char headerContentType[] = "application/json; charset=utf8";
			httpResponse.Headers.KnownHeaders[HttpHeaderContentType].pRawValue = headerContentType;
			httpResponse.Headers.KnownHeaders[HttpHeaderContentType].RawValueLength = sizeof(headerContentType) - 1;

			result = HttpSendHttpResponse(
				httpRequestQueue,
				pRequest->RequestId,
				0,
				&httpResponse,
				NULL,
				&bytesSent,
				NULL,
				0,
				NULL,
				NULL);
			CHECK_ERROR(result == NO_ERROR, L"HttpSendResponse failed (200).");

			break;
		}
		else
		{
			ULONG bytesSent = 0;
			HTTP_RESPONSE httpResponse;
			ZeroMemory(&httpResponse, sizeof(httpResponse));

			httpResponse.StatusCode = 404;
			httpResponse.pReason = "The first request must be /GacUIRemoting/Connect";

			result = HttpSendHttpResponse(
				httpRequestQueue,
				pRequest->RequestId,
				0,
				&httpResponse,
				NULL,
				&bytesSent,
				NULL,
				0,
				NULL,
				NULL);
			CHECK_ERROR(result == NO_ERROR, L"HttpSendResponse failed (404).");
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

void HttpServer::BeginReadingLoopUnsafe()
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpServer::SendStringArray(vint count, List<WString>& strs)
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpServer::SendSingleString(const WString& str)
{
	CHECK_FAIL(L"Not Implemented!");
}