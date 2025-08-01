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
	CHECK_FAIL(L"Not Implemented!");
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

HttpClient::HttpClient()
{
	httpSession = WinHttpOpen(
		L"RemotingTest_Rendering_Win32.exe",
		WINHTTP_ACCESS_TYPE_NO_PROXY,
		NULL,
		NULL,
		WINHTTP_FLAG_ASYNC);
	CHECK_ERROR(httpSession != NULL, L"WinHttpOpen failed.");

	httpConnection = WinHttpConnect(
		httpSession,
		L"localhost",
		8888,
		0);
	CHECK_ERROR(httpConnection != NULL, L"WinHttpConnect failed.");
}

HttpClient::~HttpClient()
{
	Stop();
}

void HttpClient::Stop()
{
	if (httpSession != NULL)
	{
		WinHttpCloseHandle(httpConnection);
		WinHttpCloseHandle(httpSession);

		httpConnection = NULL;
		httpSession = NULL;
	}
}

void HttpClient::InstallCallback(INetworkProtocolCallback* _callback)
{
	callback = _callback;
}