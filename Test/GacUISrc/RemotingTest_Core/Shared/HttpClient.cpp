#include "HttpClient.h"

/***********************************************************************
HttpClient (Reading)
***********************************************************************/

void HttpClient::BeginReadingLoopUnsafe()
{
	if (state == State::Stopping) return;
	CHECK_ERROR(state == State::Running, L"BeginReadingLoopUnsafe can only be called when client is running.");
	DWORD lastError = 0;
	BOOL httpResult = FALSE;

	LPCWSTR acceptTypes[] = { L"application/json; charset=utf8", NULL };
	HINTERNET httpRequest = WinHttpOpenRequest(
		httpConnection,
		L"POST",
		urlRequest.Buffer(),
		NULL,
		WINHTTP_NO_REFERER,
		acceptTypes,
		WINHTTP_FLAG_REFRESH);
	lastError = GetLastError();
	if (lastError == ERROR_INVALID_HANDLE)
	{
		CHECK_ERROR(state == State::Stopping, L"WinHttpOpenRequest failed with ERROR_INVALID_HANDLE but client is not stopping.");
		return;
	}
	CHECK_ERROR(httpRequest != NULL, L"WinHttpOpenRequest failed.");
	{
		auto self = this;
		WINHTTP_STATUS_CALLBACK previousCallback = WinHttpSetStatusCallback(
			httpRequest,
			(WINHTTP_STATUS_CALLBACK)[](HINTERNET httpRequest, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) -> void
			{
				if (!dwContext) return;
				auto self = reinterpret_cast<HttpClient*>(dwContext);
				switch (dwInternetStatus)
				{
				case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
					{
						ThreadPoolLite::Queue([=]()
						{
							if (self->state == State::Stopping) return;
							DWORD lastError = 0;
							BOOL httpResult = WinHttpReceiveResponse(httpRequest, NULL);
							lastError = GetLastError();
							if (lastError == ERROR_INVALID_HANDLE)
							{
								CHECK_ERROR(self->state == State::Stopping, L"WinHttpReceiveResponse failed with ERROR_INVALID_HANDLE but client is not stopping.");
								WinHttpCloseHandle(httpRequest);
								return;
							}
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
							BOOL httpResult = FALSE;
							{
								httpResult = WinHttpQueryHeaders(
									httpRequest,
									WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
									WINHTTP_HEADER_NAME_BY_INDEX,
									&statusCode,
									&dwordLength,
									WINHTTP_NO_HEADER_INDEX);
								lastError = GetLastError();
								if (lastError == ERROR_INVALID_HANDLE)
								{
									CHECK_ERROR(self->state == State::Stopping, L"WinHttpQueryHeaders failed with ERROR_INVALID_HANDLE but client is not stopping.");
									return;
								}
								CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryHeaders failed to retrieve status code.");
								CHECK_ERROR(statusCode == 200, L"BeginReadingLoopUnsafe did not return status code: 200.");
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
								if (lastError == ERROR_INVALID_HANDLE)
								{
									CHECK_ERROR(self->state == State::Stopping, L"WinHttpQueryHeaders failed with ERROR_INVALID_HANDLE but client is not stopping.");
									return;
								}
								CHECK_ERROR(httpResult == FALSE && lastError == ERROR_INSUFFICIENT_BUFFER, L"WinHttpQueryHeaders failed to retrieve content type.");

								Array<wchar_t> headerBuffer(headerLength / 2 + 1);
								ZeroMemory(&headerBuffer[0], headerBuffer.Count() * sizeof(wchar_t));

								httpResult = WinHttpQueryHeaders(
									httpRequest,
									WINHTTP_QUERY_CONTENT_TYPE,
									WINHTTP_HEADER_NAME_BY_INDEX,
									&headerBuffer[0],
									&headerLength,
									WINHTTP_NO_HEADER_INDEX);
								lastError = GetLastError();
								if (lastError == ERROR_INVALID_HANDLE)
								{
									CHECK_ERROR(self->state == State::Stopping, L"WinHttpQueryHeaders failed with ERROR_INVALID_HANDLE but client is not stopping.");
									return;
								}
								CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryHeaders failed to retrieve content-type.");

								const wchar_t* header = &headerBuffer[0];
								CHECK_ERROR(wcscmp(header, L"application/json; charset=utf8") == 0, L"/Request did not return content type: application/json; charset=utf8.");
							}
							{
								httpResult = WinHttpQueryDataAvailable(
									httpRequest,
									NULL);
								lastError = GetLastError();
								if (lastError == ERROR_INVALID_HANDLE)
								{
									CHECK_ERROR(self->state == State::Stopping, L"WinHttpQueryDataAvailable failed with ERROR_INVALID_HANDLE but client is not stopping.");
									return;
								}
								CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryDataAvailable failed.");
							}
						});
					}
					break;
				case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
					{
						ThreadPoolLite::Queue([=]()
						{
							DWORD dataAvailable = *(PDWORD)lpvStatusInformation;
							if (self->httpRespondBodyBuffer.Count() < dataAvailable + 1)
							{
								self->httpRespondBodyBuffer.Resize(dataAvailable + 1);
							}
							ZeroMemory(&self->httpRespondBodyBuffer[0], self->httpRespondBodyBuffer.Count());

							DWORD lastError = 0;
							BOOL httpResult = WinHttpReadData(
								httpRequest,
								&self->httpRespondBodyBuffer[0],
								dataAvailable,
								NULL);
							lastError = GetLastError();
							if (lastError == ERROR_INVALID_HANDLE)
							{
								CHECK_ERROR(self->state == State::Stopping, L"WinHttpReadData failed with ERROR_INVALID_HANDLE but client is not stopping.");
								return;
							}
							CHECK_ERROR(httpResult == TRUE, L"WinHttpReadData failed.");
						});
					}
					break;
				case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
					{
						ThreadPoolLite::Queue([=]()
						{
							self->httpRespondBodyBuffer[dwStatusInformationLength] = 0;
							U8String bodyUtf8 = U8String::Unmanaged(&self->httpRespondBodyBuffer[0]);
							auto bodyJson = JsonParse(u8tow(bodyUtf8), self->jsonParser);
							auto bodyArray = bodyJson.Cast<JsonArray>();
							CHECK_ERROR(bodyArray, L"/Request response body must be a JSON array of strings.");

							auto strs = Ptr(new List<WString>);
							for (auto&& item : bodyArray->items)
							{
								auto itemString = item.Cast<JsonString>();
								CHECK_ERROR(itemString, L"/Request response body must be a JSON array of strings.");
								strs->Add(itemString->content.value);
							}
							self->callback->OnReadStringThreadUnsafe(strs);

							WinHttpCloseHandle(httpRequest);
							self->BeginReadingLoopUnsafe();
						});
					}
					break;
				case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
					{
						ThreadPoolLite::Queue([=]()
						{
							CHECK_ERROR(self->state == State::Stopping, L"/Request failed to complete.");
							WinHttpCloseHandle(httpRequest);
						});
					}
					break;
				}
			},
			WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
			NULL);
		lastError = GetLastError();
		if (previousCallback == WINHTTP_INVALID_STATUS_CALLBACK && lastError == ERROR_INVALID_HANDLE)
		{
			CHECK_ERROR(state == State::Stopping, L"WinHttpSetStatusCallback failed with ERROR_INVALID_HANDLE but client is not stopping.");
			WinHttpCloseHandle(httpRequest);
			return;
		}
		CHECK_ERROR(previousCallback != WINHTTP_INVALID_STATUS_CALLBACK, L"WinHttpSetStatusCallback failed.");
	}
	{
		httpResult = WinHttpSendRequest(
			httpRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0,
			WINHTTP_NO_REQUEST_DATA,
			0,
			0,
			reinterpret_cast<DWORD_PTR>(this));
		lastError = GetLastError();
		if (httpResult == FALSE && lastError == ERROR_INVALID_HANDLE)
		{
			CHECK_ERROR(state == State::Stopping, L"WinHttpSendRequest failed with ERROR_INVALID_HANDLE but client is not stopping.");
			WinHttpCloseHandle(httpRequest);
			return;
		}
		CHECK_ERROR(httpResult == TRUE, L"WinHttpSendRequest failed.");
	}
}

/***********************************************************************
HttpClient (WaitForServer)
***********************************************************************/

void HttpClient::WaitForServer()
{
	if (state == State::Stopping) return;
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
	if (state == State::Stopping) return;
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
	if (lastError == ERROR_INVALID_HANDLE)
	{
		CHECK_ERROR(state == State::Stopping, L"WinHttpOpenRequest failed with ERROR_INVALID_HANDLE.");
		return;
	}
	CHECK_ERROR(httpRequest != NULL, L"WinHttpOpenRequest failed.");

	WINHTTP_STATUS_CALLBACK previousCallback = WinHttpSetStatusCallback(
		httpRequest,
		(WINHTTP_STATUS_CALLBACK)[](HINTERNET httpRequest, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) -> void
		{
			if (!dwContext) return;
			auto self = reinterpret_cast<HttpClient*>(dwContext);
			switch (dwInternetStatus)
			{
			case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
				{
					SPIN_LOCK(self->httpRequestBodiesLock)
					{
						self->httpRequestBodies.Remove(httpRequest);
					}
					ThreadPoolLite::Queue([=]()
					{
						DWORD lastError = 0;
						BOOL httpResult = WinHttpReceiveResponse(httpRequest, NULL);
						lastError = GetLastError();
						if (lastError == ERROR_INVALID_HANDLE)
						{
							CHECK_ERROR(self->state == State::Stopping, L"WinHttpReceiveResponse failed with ERROR_INVALID_HANDLE but client is not stopping.");
							return;
						}
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
						if (lastError == ERROR_INVALID_HANDLE)
						{
							CHECK_ERROR(self->state == State::Stopping, L"WinHttpQueryHeaders failed with ERROR_INVALID_HANDLE but client is not stopping.");
							return;
						}
						CHECK_ERROR(httpResult == TRUE, L"WinHttpQueryHeaders failed to retrieve status code.");
						CHECK_ERROR(statusCode == 200, L"SendJsonRequest did not return status code: 200.");

						WinHttpCloseHandle(httpRequest);
					});
				}
				break;
			case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
				{
					SPIN_LOCK(self->httpRequestBodiesLock)
					{
						self->httpRequestBodies.Remove(httpRequest);
					}
					ThreadPoolLite::Queue([=]()
					{
						CHECK_ERROR(self->state == State::Stopping, L"/Response failed to complete.");
						WinHttpCloseHandle(httpRequest);
					});
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
	if (lastError == ERROR_INVALID_HANDLE)
	{
		CHECK_ERROR(state == State::Stopping, L"WinHttpAddRequestHeaders failed with ERROR_INVALID_HANDLE.");
		WinHttpCloseHandle(httpRequest);
		return;
	}
	CHECK_ERROR(httpResult == TRUE, L"WinHttpAddRequestHeaders failed.");

	U8String bodyUtf8 = wtou8(JsonToString(jsonBody));
	SPIN_LOCK(httpRequestBodiesLock)
	{
		httpRequestBodies.Add(httpRequest, bodyUtf8);
	}

	httpResult = WinHttpSendRequest(
		httpRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		(LPVOID)bodyUtf8.Buffer(),
		(DWORD)bodyUtf8.Length(),
		(DWORD)bodyUtf8.Length(),
		reinterpret_cast<DWORD_PTR>(this));
	lastError = GetLastError();
	if (lastError == ERROR_INVALID_HANDLE)
	{
		CHECK_ERROR(state == State::Stopping, L"WinHttpSendRequest failed with ERROR_INVALID_HANDLE.");
		WinHttpCloseHandle(httpRequest);
		return;
	}
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
		state = State::Stopping;

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