#include <VlppOS.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;

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