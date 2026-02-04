# Remote Protocol Core Application

Unlike working in the unit test framework, we usually needs to start a GacUI application with remote protocol asynchronously. So that we can give a dedicated thread for handling actual communitation with the remote client. A typical example would be:
```c++
class CoreChannel : public Object, public IGuiRemoteProtocolChannel<WString>
{
public:
    void Initialize(IGuiRemoteProtocolChannelReceiver<WString>* receiver) override
    {
        // when you get any string later you need to call receiver->OnReceive(str)
        // receiver->OnReceive must be called with asyncChannel->ExecuteInChannelThread
        // asyncChannel is the argument of WaitForConnected
    }

    IGuiRemoteProtocolChannelReceiver<WString>* GetReceiver() override
    {
        // return the argument passed to Initialize
    }
    
    void Write(const WString& package) override
    {
        // send the string away
    }
    
    WString GetExecutablePath() override
    {
        // anything that you want GetCurrentController()->GetExecutablePath() to return
    }
    
    void Submit(bool& disconnected) override
    {
        // you can also save all Write calls and send them in batch here
    }
    
    IGuiRemoteEventProcessor* GetRemoteEventProcessor() override
    {
        return nullptr;
    }
    
    void WaitForConnected(GuiRemoteProtocolAsyncJsonChannelSerializer* asyncChannel)
    {
        // block until a remote client is connected
    }

    void WaitForDisconnected()
    {
        // tell the remote client that all work is done
        // block until the remote client is disconnected
    }

    void WriteError(const WString& error)
    {
        // send an error message to the remote client
        // such error will be a fetal error and the remote client is supposed to stop
    }
};

CoreChannel* coreChannel = nullptr;

void GuiMain()
{
    theme::RegisterTheme(Ptr(new darkskin::Theme));
    {
        rptest::MainWindow window;
        window.ForceCalculateSizeImmediately();
        window.MoveToScreenCenter();
        try
        {
            GetApplication()->Run(&window);
        }
        catch (const Exception& e)
        {
            coreChannel->WriteError(e.Message());
        }
        catch (const Error& e)
        {
            coreChannel->WriteError(WString::Unmanaged(e.Description()));
        }
    }
}

int main(int argc, char* argv[])
{
    CoreChannel serverCoreChannel(&server);

    auto jsonParser = Ptr(new glr::json::Parser);
    GuiRemoteJsonChannelStringSerializer channelJsonSerializer(&serverCoreChannel, jsonParser);

    GuiRemoteProtocolAsyncJsonChannelSerializer asyncChannelSender;
    asyncChannelSender.Start(
        &channelJsonSerializer,
        [&serverCoreChannel](GuiRemoteProtocolAsyncJsonChannelSerializer* channel)
        {
            GuiRemoteProtocolFromJsonChannel channelSender(channel);
            GuiRemoteProtocolFilter filteredProtocol(&channelSender);
            GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);
            coreChannel = &serverCoreChannel;
            SetupRemoteNativeController(&diffConverterProtocol);
            coreChannel = nullptr;
        },
        [&serverCoreChannel](
            GuiRemoteProtocolAsyncJsonChannelSerializer::TChannelThreadProc channelThreadProc,
            GuiRemoteProtocolAsyncJsonChannelSerializer::TUIThreadProc uiThreadProc
            )
        {
            Thread::CreateAndStart(channelThreadProc);
            Thread::CreateAndStart(uiThreadProc);
        });

    Console::WriteLine(L"> Waiting for a renderer ...");
    serverCoreChannel.WaitForConnected(&asyncChannelSender);
    asyncChannelSender.WaitForStopped();
    serverCoreChannel.WaitForDisconnected();

    return 0;
}
```


This sample simplifies all the details leaving the sending and receiving of strings to you. Check out[this test project](https://github.com/vczh-libraries/GacUI/tree/master/Test/GacUISrc/RemotingTest_Core)for a complete example.

