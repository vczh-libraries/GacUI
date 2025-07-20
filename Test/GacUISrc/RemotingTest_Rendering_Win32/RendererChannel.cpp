#include "RendererChannel.h"
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"

void RendererChannel::OnReadStringThreadUnsafe(Ptr<List<WString>> strs)
{
	GetCurrentController()->AsyncService()->InvokeInMainThread(
		GetCurrentController()->WindowService()->GetMainWindow(),
		[this, strs]()
		{
			for (auto str : *strs.Obj())
			{
				if (str.Length() > 0)
				{
					if (str[0] == L'!')
					{
						auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
						auto title = mainWindow->GetTitle();
						auto errorMessage = str.Right(str.Length() - 1);
						MessageBox(
							windows::GetWindowsForm(mainWindow)->GetWindowHandle(),
							errorMessage.Buffer(),
							title.Buffer(),
							MB_OK | MB_ICONERROR | MB_APPLMODAL
							);
						renderer->ForceExitByFatelError();
					}
					else
					{
						channel->Write(str);
					}
				}
			}
		});
}

void RendererChannel::OnReadStoppedThreadUnsafe()
{
	eventDisconnected.Signal();
}

void RendererChannel::OnReceive(const WString& package)
{
	networkProtocol->SendSingleString(package);
}

RendererChannel::RendererChannel(GuiRemoteRendererSingle* _renderer, INetworkProtocol* _networkProtocol, IGuiRemoteProtocolChannel<WString>* _channel)
	: networkProtocol(_networkProtocol)
	, renderer(_renderer)
	, channel(_channel)
{
	networkProtocol->InstallCallback(this);
	eventDisconnected.CreateManualUnsignal(false);
	_channel->Initialize(this);
}

RendererChannel::~RendererChannel()
{
}

void RendererChannel::RegisterMainWindow(INativeWindow* _window)
{
	networkProtocol->BeginReadingLoopUnsafe();
}

void RendererChannel::UnregisterMainWindow()
{
}

void RendererChannel::WaitForDisconnected()
{
	eventDisconnected.Wait();
}