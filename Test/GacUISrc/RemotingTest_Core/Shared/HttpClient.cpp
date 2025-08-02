#include "HttpClient.h"

/***********************************************************************
HttpClient (Reading)
***********************************************************************/

void HttpClient::BeginReadingLoopUnsafe()
{
	CHECK_FAIL(L"Not Implemented!");
}

/***********************************************************************
HttpClient (WaitForServer)
***********************************************************************/

void HttpClient::WaitForServer()
{
	CHECK_ERROR(state == State::Ready, L"WaitForServer can only be called once.");
	DWORD lastError = 0;
	state = State::WaitForServerConnection;
	LPCWSTR acceptTypes[] = { L"application/json; charset=utf8", NULL };
	BOOL httpResult = FALSE;

	HINTERNET httpRequest = WinHttpOpenRequest(
		httpConnection,
		L"GET",
		L"/GacUIRemoting/Connect",
		NULL,
		WINHTTP_NO_REFERER,
		acceptTypes,
		WINHTTP_FLAG_REFRESH);
	lastError = GetLastError();
	CHECK_ERROR(httpRequest != NULL, L"WinHttpOpenRequest failed.");
	{
		WINHTTP_STATUS_CALLBACK previousCallback = WinHttpSetStatusCallback(
			httpRequest,
			(WINHTTP_STATUS_CALLBACK)[](HINTERNET httpRequest, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) -> void
			{
				if (!dwContext) return;
				auto self = reinterpret_cast<HttpClient*>(dwContext);
				switch (dwInternetStatus)
				{
				case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
				case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
				case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
				case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
					{
						self->dwInternetStatus_WaitForServer = dwInternetStatus;
						self->dwStatusInformationLength_WaitForServer = dwStatusInformationLength;
						SetEvent(self->hEventWaitForServer);
					}
					break;
				}
			},
			WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
			NULL);
		lastError = GetLastError();
		CHECK_ERROR(previousCallback != WINHTTP_INVALID_STATUS_CALLBACK, L"WinHttpSetStatusCallback failed.");
	}
	{
		ResetEvent(hEventWaitForServer);
		httpResult = WinHttpSendRequest(
			httpRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			reinterpret_cast<DWORD_PTR>(this));
		lastError = GetLastError();
		CHECK_ERROR(httpResult == TRUE, L"WinHttpSendRequest failed.");
		WaitForSingleObject(hEventWaitForServer, INFINITE);
		CHECK_ERROR(dwInternetStatus_WaitForServer == WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE, L"WinHttpSendRequest failed to complete.");
	}
	{
		ResetEvent(hEventWaitForServer);
		httpResult = WinHttpReceiveResponse(httpRequest, NULL);
		lastError = GetLastError();
		CHECK_ERROR(httpResult == TRUE, L"WinHttpReceiveResponse failed.");
		WaitForSingleObject(hEventWaitForServer, INFINITE);
		CHECK_ERROR(dwInternetStatus_WaitForServer == WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE, L"WinHttpSendRequest failed to complete.");
	}

	DWORD statusCode = 0;
	DWORD dataLength = 0;
	DWORD dwordLength = sizeof(DWORD);
	{
		httpResult = WinHttpQueryHeaders(
			httpRequest,
			WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&statusCode,
			&dwordLength,
			WINHTTP_NO_HEADER_INDEX);
		lastError = GetLastError();
		CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryHeaders failed to retrieve status code.");
		CHECK_ERROR(statusCode == 200, L"/Connect did not return status code: 200.");
	}
	{
		DWORD headerLength = 0;
		httpResult = WinHttpQueryHeaders(
			httpRequest,
			WINHTTP_QUERY_CONTENT_TYPE,
			WINHTTP_HEADER_NAME_BY_INDEX,
			NULL,
			&headerLength,
			WINHTTP_NO_HEADER_INDEX);
		lastError = GetLastError();
		CHECK_ERROR(httpResult == FALSE && lastError == ERROR_INSUFFICIENT_BUFFER, L"WinHttpQueryHeaders failed to retrieve content type.");

		Array<wchar_t> headerBuffer(headerLength + 1);
		ZeroMemory(&headerBuffer[0], headerBuffer.Count() * sizeof(wchar_t));

		httpResult = WinHttpQueryHeaders(
			httpRequest,
			WINHTTP_QUERY_CONTENT_TYPE,
			WINHTTP_HEADER_NAME_BY_INDEX,
			&headerBuffer[0],
			&headerLength,
			WINHTTP_NO_HEADER_INDEX);
		lastError = GetLastError();
		CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryHeaders failed to retrieve content-type.");

		const wchar_t* header = &headerBuffer[0];
		CHECK_ERROR(wcscmp(header, L"application/json; charset=utf8") == 0, L"/Content did not return content type: application/json; charset=utf8.");
	}
	{
		httpResult = WinHttpQueryDataAvailable(
			httpRequest,
			&dataLength);
		lastError = GetLastError();
		CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryDataAvailable failed.");
	}
	{
		Array<char8_t> bodyBuffer(dataLength + 1);
		ZeroMemory(&bodyBuffer[0], bodyBuffer.Count() * sizeof(char8_t));

		ResetEvent(hEventWaitForServer);
		httpResult = WinHttpReadData(
			httpRequest,
			&bodyBuffer[0],
			dataLength,
			NULL);
		lastError = GetLastError();
		CHECK_ERROR(httpResult == TRUE, L"WinHttpReadData failed.");
		WaitForSingleObject(hEventWaitForServer, INFINITE);
		CHECK_ERROR(dwInternetStatus_WaitForServer == WINHTTP_CALLBACK_STATUS_READ_COMPLETE, L"WinHttpReadData failed to complete.");
		CHECK_ERROR(dwStatusInformationLength_WaitForServer == dataLength, L"WinHttpReadData failed to read full data.");

		U8String bodyUtf8 = U8String::Unmanaged(&bodyBuffer[0]);
		auto bodyJson = JsonParse(u8tow(bodyUtf8), jsonParser);
		auto bodyObject = bodyJson.Cast<JsonObject>();
		CHECK_ERROR(bodyObject, L"/Connect response body must be a JSON object.");

		for (auto field : bodyObject->fields)
		{
			if (field->name.value == L"request")
			{
				auto value = field->value.Cast<JsonString>();
				CHECK_ERROR(value, L"/Connect response body must contain a \"request\" field with a string value.");
				urlRequest = value->content.value;
			}
			else if (field->name.value == L"response")
			{
				auto value = field->value.Cast<JsonString>();
				CHECK_ERROR(value, L"/Connect response body must contain a \"response\" field with a string value.");
				urlResponse = value->content.value;
			}
		}

		CHECK_ERROR(urlRequest != L"", L"/Connect response body missing the \"request\" field.");
		CHECK_ERROR(urlResponse != L"", L"/Connect response body missing the \"response\" field.");
	}
	WinHttpCloseHandle(httpRequest);
	state = State::Running;
}

/***********************************************************************
HttpClient (Writing)
***********************************************************************/

void HttpClient::SendJsonRequest(Ptr<JsonNode> jsonBody)
{
	CHECK_ERROR(state == State::Running, L"SendJsonRequest can only be called when client is running.");
	DWORD lastError = 0;
	BOOL httpResult = FALSE;

	HINTERNET httpRequest = WinHttpOpenRequest(
		httpConnection,
		L"POST",
		urlResponse.Buffer(),
		NULL,
		WINHTTP_NO_REFERER,
		NULL,
		WINHTTP_FLAG_REFRESH);
	lastError = GetLastError();
	CHECK_ERROR(httpRequest != NULL, L"WinHttpOpenRequest failed.");

	WINHTTP_STATUS_CALLBACK previousCallback = WinHttpSetStatusCallback(
		httpRequest,
		(WINHTTP_STATUS_CALLBACK)[](HINTERNET httpRequest, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) -> void
		{
			if (!dwContext) return;
			switch (dwInternetStatus)
			{
			case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
				{
					ThreadPoolLite::Queue([=]()
					{
						DWORD lastError = 0;
						BOOL httpResult = WinHttpReceiveResponse(httpRequest, NULL);
						lastError = GetLastError();
						CHECK_ERROR(httpResult == TRUE, L"WinHttpReceiveResponse failed.");
					});
				}
				break;
			case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
				{
					ThreadPoolLite::Queue([=]()
					{
						DWORD lastError = 0;
						DWORD statusCode = 0;
						DWORD dwordLength = sizeof(DWORD);
						BOOL httpResult = WinHttpQueryHeaders(
							httpRequest,
							WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
							WINHTTP_HEADER_NAME_BY_INDEX,
							&statusCode,
							&dwordLength,
							WINHTTP_NO_HEADER_INDEX);
						lastError = GetLastError();
						CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryHeaders failed to retrieve status code.");
						CHECK_ERROR(statusCode == 200, L"SendJsonRequest did not return status code: 200.");

						WinHttpCloseHandle(httpRequest);
					});
				}
				break;
			case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
				{
					CHECK_FAIL(L"/Response failed to complete.");
				}
				break;
			}
		},
		WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
		NULL);
	lastError = GetLastError();
	CHECK_ERROR(previousCallback != WINHTTP_INVALID_STATUS_CALLBACK, L"WinHttpSetStatusCallback failed.");

	httpResult = WinHttpAddRequestHeaders(
		httpRequest,
		L"Content-Type: application/json; charset=utf8",
		-1,
		WINHTTP_ADDREQ_FLAG_ADD);
	lastError = GetLastError();
	CHECK_ERROR(httpResult == TRUE, L"WinHttpAddRequestHeaders failed.");

	U8String body = wtou8(JsonToString(jsonBody));
	httpResult = WinHttpSendRequest(
		httpRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		(LPVOID)body.Buffer(),
		(DWORD)body.Length(),
		(DWORD)body.Length(),
		reinterpret_cast<DWORD_PTR>(httpRequest));
	lastError = GetLastError();
	CHECK_ERROR(httpResult == TRUE, L"WinHttpSendRequest failed.");
}

void HttpClient::SendStringArray(vint count, List<WString>& strs)
{
	auto jsonArray = Ptr(new JsonArray);
	for (vint i = 0; i < count; i++)
	{
		auto jsonValue = Ptr(new JsonString);
		jsonValue->content.value = strs[i];
		jsonArray->items.Add(jsonValue);
	}
	SendJsonRequest(jsonArray);
}

void HttpClient::SendSingleString(const WString& str)
{
	auto jsonValue = Ptr(new JsonString);
	jsonValue->content.value = str;
	auto jsonArray = Ptr(new JsonArray);
	jsonArray->items.Add(jsonValue);
	SendJsonRequest(jsonArray);
}

/***********************************************************************
HttpClient
***********************************************************************/

HttpClient::HttpClient()
{
	DWORD lastError = 0;
	hEventWaitForServer = CreateEvent(NULL, FALSE, TRUE, NULL);
	CHECK_ERROR(hEventWaitForServer != NULL, L"HttpClient initialization failed on CreateEvent(hEventWaitForServer).");

	httpSession = WinHttpOpen(
		L"RemotingTest_Rendering_Win32.exe",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		WINHTTP_FLAG_ASYNC);
	lastError = GetLastError();
	CHECK_ERROR(httpSession != NULL, L"WinHttpOpen failed.");

	httpConnection = WinHttpConnect(
		httpSession,
		L"localhost",
		8888,
		0);
	lastError = GetLastError();
	CHECK_ERROR(httpConnection != NULL, L"WinHttpConnect failed.");
}

HttpClient::~HttpClient()
{
	Stop();
	CloseHandle(hEventWaitForServer);
}

void HttpClient::Stop()
{
	if (httpSession != NULL)
	{
		WinHttpCloseHandle(httpConnection);
		WinHttpSetStatusCallback(
			httpSession,
			NULL,
			WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
			NULL);
		WinHttpCloseHandle(httpSession);

		httpConnection = NULL;
		httpSession = NULL;
	}
}

void HttpClient::InstallCallback(INetworkProtocolCallback* _callback)
{
	callback = _callback;
}