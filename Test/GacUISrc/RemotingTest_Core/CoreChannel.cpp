#include "CoreChannel.h"

void CoreChannel::OnReadStringThreadUnsafe(Ptr<List<WString>> strs)
{
	static WString filteredStrings[] = {
		WString::Unmanaged(LR"JSON({"semantic":"Event","name":"IOMouseMoving")JSON"),
		WString::Unmanaged(LR"JSON({"semantic":"Event","name":"IOMouseEntered")JSON"),
		WString::Unmanaged(LR"JSON({"semantic":"Event","name":"IOMouseLeaved")JSON"),
		WString::Unmanaged(LR"JSON({"semantic":"Event","name":"WindowBoundsUpdated")JSON")
	};
	for (auto str : *strs.Obj())
	{
		for (auto&& filtered : filteredStrings)
		{
			if (str.Length() > filtered.Length() && str.Left(filtered.Length()) == filtered)
			{
				goto ON_RECEIVE;
			}
		}
#ifdef _DEBUG
		Console::WriteLine(L"Received: " + str);
#endif
	ON_RECEIVE:
		receiver->OnReceive(str);
	}
}

void CoreChannel::OnReadStoppedThreadUnsafe()
{
	eventDisconnected.Signal();
}

void CoreChannel::SendPendingMessages()
{
	networkProtocol->SendStringArray(pendingMessageCount, pendingMessages);
	pendingMessageCount = 0;
}

CoreChannel::CoreChannel(INetworkProtocol* _networkProtocol)
	: networkProtocol(_networkProtocol)
{
	networkProtocol->InstallCallback(this);
	eventDisconnected.CreateManualUnsignal(false);
}

CoreChannel::~CoreChannel()
{
}

void CoreChannel::RendererConnectedThreadUnsafe(GuiRemoteProtocolAsyncJsonChannelSerializer* asyncChannel)
{
	Console::WriteLine(L"> Renderer connected");
	networkProtocol->BeginReadingLoopUnsafe();
	asyncChannel->ExecuteInChannelThread([this]()
	{
		connected = true;
	});
}

void CoreChannel::WaitForDisconnected()
{
	eventDisconnected.Wait();
}

void CoreChannel::WriteErrorThreadUnsafe(const WString& error)
{
	Console::WriteLine(L"Error: " + error);
	networkProtocol->SendSingleString(L"!" + error);
}

void CoreChannel::Initialize(IGuiRemoteProtocolChannelReceiver<WString>* _receiver)
{
	receiver = _receiver;
}

IGuiRemoteProtocolChannelReceiver<WString>* CoreChannel::GetReceiver()
{
	return receiver;
}

void CoreChannel::Write(const WString& package)
{
#ifdef _DEBUG
	Console::WriteLine(L"Sent: " + package);
#endif
	if (pendingMessageCount < pendingMessages.Count())
	{
		pendingMessages[pendingMessageCount] = package;
	}
	else
	{
		pendingMessages.Add(package);
	}
	pendingMessageCount++;
}

WString CoreChannel::GetExecutablePath()
{
	return WString::Unmanaged(L"RemotingTest_Core.vcxproj");
}

void CoreChannel::Submit(bool& disconnected)
{
	if (connected)
	{
		if (pendingMessageCount > 0)
		{
#ifdef _DEBUG
			Console::WriteLine(L"Submit");
#endif
			SendPendingMessages();
		}
	}
	else
	{
		if (pendingMessageCount > 0)
		{
#ifdef _DEBUG
			Console::WriteLine(L"Submit (unconnected)");
#endif
			pendingMessageCount = 0;
		}
		disconnected = true;
	}
}

void CoreChannel::ProcessRemoteEvents()
{
}