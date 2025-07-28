#ifndef VCZH_REMOTINGTEST_SHARED_PROTOCOLCALLBACK
#define VCZH_REMOTINGTEST_SHARED_PROTOCOLCALLBACK

#include <VlppGlrParser.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <http.h>
#define RPC_USE_NATIVE_WCHAR
#include <rpc.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;
using namespace vl::glr::json;

class INetworkProtocolCallback : public virtual Interface
{
public:
	virtual void				OnReadStringThreadUnsafe(Ptr<List<WString>> strs) = 0;
	virtual void				OnReadStoppedThreadUnsafe() = 0;
};

class INetworkProtocol : public virtual Interface
{
public:
	virtual void				InstallCallback(INetworkProtocolCallback* callback) = 0;
	virtual void				BeginReadingLoopUnsafe() = 0;
	virtual void				SendStringArray(vint count, List<WString>& strs) = 0;
	virtual void				SendSingleString(const WString& str) = 0;
};

#endif