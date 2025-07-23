#include "HttpClient.h"

HttpClient::HttpClient()
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::WaitForClient()
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpClient::Stop()
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpClient::InstallCallback(INetworkProtocolCallback* _callback)
{
	callback = _callback;
}

void HttpClient::BeginReadingLoopUnsafe()
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpClient::SendStringArray(vint count, List<WString>& strs)
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpClient::SendSingleString(const WString& str)
{
	CHECK_FAIL(L"Not Implemented!");
}