#include <VlppOS.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;

class INetworkProtocol : public virtual Interface
{
public:
	virtual void				BeginReadingLoopUnsafe() = 0;
	virtual void				SendStringArray(vint count, List<WString>& strs) = 0;
	virtual void				SendSingleString(const WString& str) = 0;
};