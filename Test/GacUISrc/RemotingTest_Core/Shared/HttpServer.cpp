#include "HttpServer.h"

HttpServer::HttpServer()
{
}

HttpServer::~HttpServer()
{
}

void HttpServer::WaitForClient()
{
	CHECK_FAIL(L"Not Implemented!");
}

void HttpServer::Stop()
{
	CHECK_FAIL(L"Not Implemented!");
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