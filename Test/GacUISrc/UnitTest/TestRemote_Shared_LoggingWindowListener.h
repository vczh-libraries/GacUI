#ifndef GACUISRC_REMOTE_LOGGINGWINDOWLISTENER
#define GACUISRC_REMOTE_LOGGINGWINDOWLISTENER

#include "../../../Source/NativeWindow/GuiNativeWindow.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;

namespace remote_protocol_tests
{
	template<typename ...TArgs>
	void AssertEventLogs(List<WString>& eventLogs, TArgs&& ...args)
	{
		const wchar_t* expected[] = { args... };
		if (eventLogs.Count() != sizeof...(args))
		{
			TEST_PRINT(L"Actual event logs (" + itow(eventLogs.Count()) + L"):");
			for (auto&& eventLog : eventLogs)
			{
				TEST_PRINT(L"  " + eventLog);
			}
			TEST_PRINT(L"Expected event logs (" + itow(sizeof...(args)) + L"):");
			for (auto eventLog : expected)
			{
				TEST_PRINT(L"  " + WString::Unmanaged(eventLog));
			}
		}
		TEST_ASSERT(eventLogs.Count() == sizeof...(args));
		for (vint i = 0; i < eventLogs.Count(); i++)
		{
			TEST_ASSERT(eventLogs[i] == expected[i]);
		}
		eventLogs.Clear();
	}

	inline void AssertEventLogs(List<WString>& eventLogs)
	{
		TEST_ASSERT(eventLogs.Count() == 0);
	}

	class LoggingWindowListener : public Object, public virtual INativeWindowListener
	{
	public:
		List<WString>				callbacks;
		bool						logMoved = false;
	
		template<typename ...TArgs>
		void AssertCallbacks(TArgs&& ...args)
		{
			AssertEventLogs(callbacks, std::forward<TArgs&&>(args)...);
		}
	
		void AssertCallbacks()
		{
			AssertEventLogs(callbacks);
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
	
		HitTestResult HitTest(NativePoint location) override
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

		WString PrintButton(NativeMouseButton button)
		{
			switch (button)
			{
			case NativeMouseButton::Left: return WString::Unmanaged(L"Left");
			case NativeMouseButton::Middle: return WString::Unmanaged(L"Middle");
			case NativeMouseButton::Right: return WString::Unmanaged(L"Right");
			case NativeMouseButton::Mouse4: return WString::Unmanaged(L"Mouse4");
			case NativeMouseButton::Mouse5: return WString::Unmanaged(L"Mouse5");
			default: return WString::Unmanaged(L"Unknown");
			}
		}

#define LOG_MOUSE_BUTTON_CALLBACK(NAME)\
		void NAME(NativeMouseButton button, const NativeWindowMouseInfo& info) override\
		{\
			callbacks.Add(L ## #NAME L"(" + PrintButton(button) + L"," + PrintArguments(info) + (info.osSuper ? L",Super)" : L")"));\
		}\

#define LOG_MOUSE_CALLBACK(NAME)\
		void NAME(const NativeWindowMouseInfo& info) override\
		{\
			callbacks.Add(L ## #NAME L"(" + PrintArguments(info) + (info.osSuper ? L",Super)" : L")"));\
		}\

		LOG_MOUSE_BUTTON_CALLBACK(MouseDown)
		LOG_MOUSE_BUTTON_CALLBACK(MouseUp)
		LOG_MOUSE_BUTTON_CALLBACK(MouseDoubleClick)
		LOG_MOUSE_CALLBACK(HorizontalWheel)
		LOG_MOUSE_CALLBACK(VerticalWheel)
		LOG_MOUSE_CALLBACK(MouseMoving)
		LOG_CALLBACK(MouseEntered)
		LOG_CALLBACK(MouseLeaved)
		
		LOG_IO_CALLBACK(KeyDown, NativeWindowKeyInfo)
		LOG_IO_CALLBACK(KeyUp, NativeWindowKeyInfo)
		LOG_IO_CALLBACK(Char, NativeWindowCharInfo)
	
		bool NeedRefresh() override
		{
			return false;
		}
	
		void ForceRefresh(bool handleFailure, bool& updated, bool& failureByResized, bool& failureByLostDevice) override
		{
		}
	
		void AssignFrameConfig(const NativeWindowFrameConfig& config) override
		{
		}
	
#undef LOG_IO_CALLBACK
#undef LOG_MOUSE_BUTTON_CALLBACK
#undef LOG_MOUSE_CALLBACK
#undef SHOULD_NOT_BE_CALLED
	};
}

#endif
