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

void HttpClient::WinHttpStatusCallback_WaitForServer(DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	switch (dwInternetStatus)
	{
	case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
	case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
		{
			dwWaitForServerInternetStatus = dwInternetStatus;
			SetEvent(hEventWaitForServer);
		}
		break;
	}
}

void HttpClient::WaitForServer()
{
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

	dwWaitForServerInternetStatus = 0;
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
	CHECK_ERROR(dwWaitForServerInternetStatus == WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE, L"WinHttpSendRequest failed.");

	WinHttpCloseHandle(httpRequest);
}

/***********************************************************************
HttpClient (Writing)
***********************************************************************/

void HttpClient::SendJsonRequest(Ptr<JsonNode> jsonBody)
{
	CHECK_FAIL(L"Not Implemented!");
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

void HttpClient::WinHttpStatusCallback(DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	if (state == State::WaitForServerConnection)
	{
		WinHttpStatusCallback_WaitForServer(dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
	}
}

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

	WINHTTP_STATUS_CALLBACK previousCallback = WinHttpSetStatusCallback(
		httpSession,
		(WINHTTP_STATUS_CALLBACK)[](HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) -> void
		{
			if (dwContext)
			{
				auto self = reinterpret_cast<HttpClient*>(dwContext);
				self->WinHttpStatusCallback(dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
			}
		},
		WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
		NULL);
	lastError = GetLastError();
	CHECK_ERROR(previousCallback != WINHTTP_INVALID_STATUS_CALLBACK, L"WinHttpSetStatusCallback failed.");

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