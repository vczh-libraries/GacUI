#ifndef VCZH_UIALIST_WINDOWCAPTURE_WINDOWS
#define VCZH_UIALIST_WINDOWCAPTURE_WINDOWS

#include "ProcessDiscovery.Windows.h"

namespace uialist::native
{
	enum class CaptureStatus { Ready, Unavailable, GeometryChanged, Canceled };

	struct CaptureSnapshot
	{
		CaptureStatus status = CaptureStatus::Unavailable;
		RECT bounds = {};
		UINT dpi = 96;
		vl::WString timestamp;
		vl::collections::Array<vl::vuint8_t> bitmap;
	};

	class CaptureRequest : public vl::Object
	{
	public:
		WindowIdentity target;
		vl::vint generation;
		HANDLE cancel;
		CaptureRequest(WindowIdentity window, vl::vint epoch);
		~CaptureRequest();
		void Cancel();
	};

	class CaptureWorker : public vl::Thread
	{
	protected:
		void Run() override;
	public:
		vl::TaskQueue queue;
	};

	extern vl::Ptr<CaptureSnapshot> CaptureWindow(vl::Ptr<CaptureRequest> request);
}

#endif
