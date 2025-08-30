#include "TestControls.h"

using namespace vl::collections;
using namespace gacui_unittest_template;

TEST_FILE
{
	const auto resourceDifferentButtons = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <ref.Members><![CDATA[
        func UpdateWindowTitle(result : INativeDialogService::MessageBoxButtonsOutput) : void
        {
          switch(result)
          {
            case SelectOK: { self.Text = "SelectOK"; }
            case SelectCancel: { self.Text = "SelectCancel"; }
            case SelectYes: { self.Text = "SelectYes"; }
            case SelectNo: { self.Text = "SelectNo"; }
            case SelectRetry: { self.Text = "SelectRetry"; }
            case SelectAbort: { self.Text = "SelectAbort"; }
            case SelectIgnore: { self.Text = "SelectIgnore"; }
            case SelectTryAgain: { self.Text = "SelectTryAgain"; }
            case SelectContinue: { self.Text = "SelectContinue"; }
          }
        }
      ]]></ref.Members>

      <Window ref.Name="self" Text="MessageDialog" ClientSize="x:480 y:320">
        <MessageDialog ref.Name="dialogDisplayOK" 
                       Input="DisplayOK" 
                       DefaultButton="DefaultFirst"
                       Text="Select a button!" 
                       Title="DisplayOK"/>

        <MessageDialog ref.Name="dialogDisplayOKCancel" 
                       Icon="IconError"
                       Input="DisplayOKCancel" 
                       DefaultButton="DefaultSecond"
                       Text="Select a button!" 
                       Title="DisplayOKCancel"/>

        <MessageDialog ref.Name="dialogDisplayYesNo" 
                       Icon="IconWarning"
                       Input="DisplayYesNo" 
                       DefaultButton="DefaultSecond"
                       Text="Select a button!" 
                       Title="DisplayYesNo"/>

        <MessageDialog ref.Name="dialogDisplayYesNoCancel" 
                       Icon="IconQuestion"
                       Input="DisplayYesNoCancel" 
                       DefaultButton="DefaultSecond"
                       Text="Select a button!" 
                       Title="DisplayYesNoCancel"/>

        <MessageDialog ref.Name="dialogDisplayRetryCancel" 
                       Icon="IconInformation"
                       Input="DisplayRetryCancel" 
                       DefaultButton="DefaultSecond"
                       Text="Select a button!" 
                       Title="DisplayRetryCancel"/>

        <MessageDialog ref.Name="dialogDisplayAbortRetryIgnore" 
                       Input="DisplayAbortRetryIgnore" 
                       DefaultButton="DefaultSecond"
                       Text="Select a button!" 
                       Title="DisplayAbortRetryIgnore"/>

        <MessageDialog ref.Name="dialogDisplayCancelTryAgainContinue" 
                       Input="DisplayCancelTryAgainContinue" 
                       DefaultButton="DefaultSecond"
                       Text="Select a button!" 
                       Title="DisplayCancelTryAgainContinue"/>

        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <Button Text="DisplayOK">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var result = dialogDisplayOK.ShowDialog();
                self.UpdateWindowTitle(result);
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:1 column:0">
            <Button Text="DisplayOKCancel">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var result = dialogDisplayOKCancel.ShowDialog();
                self.UpdateWindowTitle(result);
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:2 column:0">
            <Button Text="DisplayYesNo">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var result = dialogDisplayYesNo.ShowDialog();
                self.UpdateWindowTitle(result);
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:3 column:0">
            <Button Text="DisplayYesNoCancel">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var result = dialogDisplayYesNoCancel.ShowDialog();
                self.UpdateWindowTitle(result);
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:4 column:0">
            <Button Text="DisplayRetryCancel">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var result = dialogDisplayRetryCancel.ShowDialog();
                self.UpdateWindowTitle(result);
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:5 column:0">
            <Button Text="DisplayAbortRetryIgnore">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var result = dialogDisplayAbortRetryIgnore.ShowDialog();
                self.UpdateWindowTitle(result);
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>

          <Cell Site="row:6 column:0">
            <Button Text="DisplayCancelTryAgainContinue">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                var result = dialogDisplayCancelTryAgainContinue.ShowDialog();
                self.UpdateWindowTitle(result);
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"Dialog Message")
	{
		TEST_CASE(L"Open and Close")
		{
			auto showMessageDialogs = []<typename TLastCallback>(UnitTestRemoteProtocol* protocol, const WString& firstFrame, TLastCallback && lastCallback, auto ...dialogNameArgs)
			{
				const wchar_t* dialogNames[] = { dialogNameArgs... };
				for (auto [dialogName, index] : indexed(From(dialogNames)))
				{
					protocol->OnNextIdleFrame(index == 0 ? firstFrame : WString::Unmanaged(L"[ENTER]"), [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto button = FindControlByText<GuiButton>(window, WString::Unmanaged(dialogName));
						auto location = protocol->LocationOf(button);
						GetApplication()->InvokeInMainThread(window, [=]()
						{
							protocol->LClick(location);
						});
					});
					protocol->OnNextIdleFrame(WString::Unmanaged(L"Show ") + WString::Unmanaged(dialogName), [=]()
					{
						protocol->KeyPress(VKEY::KEY_RETURN);
					});
				}
				protocol->OnNextIdleFrame(L"[ENTER]", std::forward<TLastCallback&&>(lastCallback));
			};

			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				showMessageDialogs(protocol, L"Ready", []()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				}, L"DisplayOK", L"DisplayOKCancel", L"DisplayYesNo", L"DisplayYesNoCancel", L"DisplayRetryCancel", L"DisplayAbortRetryIgnore", L"DisplayCancelTryAgainContinue");
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_Message/OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceDifferentButtons
			);
		});
	});
}