#include "../../../Source/NativeWindow/GuiNativeWindow.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;

class LoggingWindowListener : public Object, public virtual INativeWindowListener
{
public:
	List<WString>				callbacks;
	bool						logMoved = false;

	template<typename ...TArgs>
	void AssertCallbacks(TArgs&& ...args)
	{
		const wchar_t* expected[] = {args...};
		TEST_ASSERT(CompareEnumerable(callbacks, From(expected).Select(WString::Unmanaged)) == 0);
		callbacks.Clear();
	}

	void AssertCallbacks()
	{
		TEST_ASSERT(callbacks.Count() == 0);
	}

	WString PrintArguments(const NativeWindowMouseInfo& info)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	WString PrintArguments(const NativeWindowKeyInfo& info)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	WString PrintArguments(const NativeWindowCharInfo& info)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

#define SHOULD_NOT_BE_CALLED(NAME)\
		CHECK_FAIL(L"INativeWindowListener::" L ## #NAME L"() of GuiRemoteWindow should not be called.")

#define LOG_IO_CALLBACK(NAME, ARGUMENTS)\
	void NAME(const ARGUMENTS& info) override\
	{\
		callbacks.Add(L ## #NAME L"(" + PrintArguments(info) + L")");\
	}\

#define LOG_CALLBACK(NAME)\
	void NAME() override\
	{\
		callbacks.Add(L ## #NAME L"()");\
	}\

	HitTestResult HitTest(NativePoint location)
	{
		return INativeWindowListener::NoDecision;
	}

	void Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder) override
	{

	}

	void Moved() override
	{
		if (logMoved)
		{
			callbacks.Add(L"Moved()");
		}
	}

	void DpiChanged(bool preparing) override
	{
		callbacks.Add(L"BeforeClosing(" + WString::Unmanaged(preparing ? L"true" : L"false") + L")");
	}

	LOG_CALLBACK(Enabled)
	LOG_CALLBACK(Disabled)
	LOG_CALLBACK(GotFocus)
	LOG_CALLBACK(LostFocus)
	LOG_CALLBACK(RenderingAsActivated)
	LOG_CALLBACK(RenderingAsDeactivated)
	LOG_CALLBACK(Opened)
	void BeforeClosing(bool& cancel) override
	{
		callbacks.Add(L"BeforeClosing()");
	}
	LOG_CALLBACK(AfterClosing)
	LOG_CALLBACK(Closed)
	LOG_CALLBACK(Paint)
	LOG_CALLBACK(Destroying)
	LOG_CALLBACK(Destroyed)

	LOG_IO_CALLBACK(LeftButtonDown, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(LeftButtonUp, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(LeftButtonDoubleClick, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(RightButtonDown, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(RightButtonUp, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(RightButtonDoubleClick, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(MiddleButtonDown, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(MiddleButtonUp, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(MiddleButtonDoubleClick, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(HorizontalWheel, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(VerticalWheel, NativeWindowMouseInfo)
	LOG_IO_CALLBACK(MouseMoving, NativeWindowMouseInfo)
	LOG_CALLBACK(MouseEntered)
	LOG_CALLBACK(MouseLeaved)
	
	LOG_IO_CALLBACK(KeyDown, NativeWindowKeyInfo)
	LOG_IO_CALLBACK(KeyUp, NativeWindowKeyInfo)
	LOG_IO_CALLBACK(Char, NativeWindowCharInfo)

	bool NeedRefresh() override
	{
		SHOULD_NOT_BE_CALLED(NeedRefresh);
	}

	void ForceRefresh(bool handleFailure, bool& updated, bool& failureByResized, bool& failureByLostDevice)
	{
		SHOULD_NOT_BE_CALLED(ForceRefresh);
	}

	void AssignFrameConfig(const NativeWindowFrameConfig& config)
	{
	}

#undef LOG_IO_CALLBACK
#undef SHOULD_NOT_BE_CALLED
};