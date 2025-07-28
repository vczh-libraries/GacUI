#include "HttpServer.h"

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
	CHECK_FAIL(L"Not Implemented!");
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