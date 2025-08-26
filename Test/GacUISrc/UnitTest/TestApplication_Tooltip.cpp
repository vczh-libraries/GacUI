#include "TestControls.h"

using namespace gacui_unittest_template;

namespace vl
{
	extern IDateTimeImpl* GetOSDateTimeImpl();
}

class TooltipTimer : protected vl::IDateTimeImpl
{
private:
	vl::IDateTimeImpl*		osImpl = nullptr;
	vl::DateTime			currentTime;
	
public:
	TooltipTimer()
	{
		// Store the current DateTime implementation before injection
		osImpl = vl::GetOSDateTimeImpl();
		
		// Initialize controlled time to 2000-1-1 0:0:0:000
		currentTime = vl::DateTime::FromDateTime(2000, 1, 1, 0, 0, 0, 0);
		
		// Inject ourselves as the DateTime implementation
		vl::InjectDateTimeImpl(this);
	}
	
	~TooltipTimer()
	{
		try
		{
			// Restore original DateTime implementation
			vl::InjectDateTimeImpl(nullptr);
		}
		catch (...)
		{
			// Don't let exceptions escape destructor
		}
	}
	
	void Tooltip()
	{
		currentTime = currentTime.Forward(600); // 600ms forward
	}
	
	void WaitForClosing()
	{
		currentTime = currentTime.Forward(5100); // 5100ms forward
	}
	
protected:
	// IDateTimeImpl interface implementation
	vl::DateTime FromDateTime(vint _year, vint _month, vint _day, vint _hour, vint _minute, vint _second, vint _milliseconds) override
	{
		// Delegate to original implementation for proper DateTime construction
		return osImpl->FromDateTime(_year, _month, _day, _hour, _minute, _second, _milliseconds);
	}
	
	vl::DateTime FromOSInternal(vuint64_t osInternal) override
	{
		return osImpl->FromOSInternal(osInternal);
	}
	
	vuint64_t LocalTime() override
	{
		return currentTime.osMilliseconds; // Return our controlled time
	}
	
	vuint64_t UtcTime() override
	{
		return osImpl->LocalToUtcTime(currentTime.osInternal);
	}
	
	vuint64_t LocalToUtcTime(vuint64_t osInternal) override
	{
		return osImpl->LocalToUtcTime(osInternal);
	}
	
	vuint64_t UtcToLocalTime(vuint64_t osInternal) override
	{
		return osImpl->UtcToLocalTime(osInternal);
	}
	
	vuint64_t Forward(vuint64_t osInternal, vuint64_t milliseconds) override
	{
		return osImpl->Forward(osInternal, milliseconds);
	}
	
	vuint64_t Backward(vuint64_t osInternal, vuint64_t milliseconds) override
	{
		return osImpl->Backward(osInternal, milliseconds);
	}
};

TEST_FILE
{
	const auto resourceTooltipWindows = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Tooltip" ClientSize="x:480 y:320">
        <Table CellPadding="5" AlignmentToParent="left:0 top:0 right:0 bottom:0">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>
          <Cell Site="row:0 column:0">
            <Button ref.Name="buttonTooltipA" Text="TooltipA">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <att.TooltipControl>
                <Label Text="This is Tooltip A"/>
              </att.TooltipControl>
            </Button>
          </Cell>
          <Cell Site="row:1 column:0">
            <Button ref.Name="buttonTooltipB" Text="TooltipB">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <att.TooltipControl>
                <Label Text="This is Tooltip B"/>
              </att.TooltipControl>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Tooltip")
	{
		TEST_CASE(L"ShowTooltipAndWaitForClosing")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					timer.WaitForClosing();
				});
				protocol->OnNextIdleFrame(L"After timer.WaitForClosing", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndWaitForClosing"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
		
		TEST_CASE(L"ShowTooltipAndLeave")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					protocol->MouseMove({0, 0});
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After leaving tooltip", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndLeave"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
		
		TEST_CASE(L"ShowTooltipAndSwitchToAnother")
		{
			TooltipTimer timer;
			
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipB = FindObjectByName<GuiButton>(window, L"buttonTooltipB");
					auto location = protocol->LocationOf(buttonTooltipB);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipB", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto buttonTooltipA = FindObjectByName<GuiButton>(window, L"buttonTooltipA");
					auto location = protocol->LocationOf(buttonTooltipA);
					protocol->MouseMove(location);
				});
				protocol->OnNextIdleFrame(L"Mouse Hover on TooltipA", [&, protocol]()
				{
					timer.Tooltip();
				});
				protocol->OnNextIdleFrame(L"After timer.Tooltip for A", [&, protocol]()
				{
					timer.WaitForClosing();
				});
				protocol->OnNextIdleFrame(L"After timer.WaitForClosing", [&, protocol]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Tooltip/ShowTooltipAndSwitchToAnother"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceTooltipWindows
			);
		});
	});
}
