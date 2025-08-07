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
						if (!displayedError)
						{
							displayedError = true;
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
	List<WString> packagesToSend;
	bool shouldSendBatch = false;
	
	SPIN_LOCK(cacheLock)
	{
		if (isCaching)
		{
			cachedPackages.Add(package);
			return;
		}

		if (cachedPackages.Count() > 0)
		{
			packagesToSend = std::move(cachedPackages);
			packagesToSend.Add(package);
			shouldSendBatch = true;
		}
	}
	
	if (shouldSendBatch)
	{
		networkProtocol->SendStringArray(packagesToSend.Count(), packagesToSend);
	}
	else
	{
		networkProtocol->SendSingleString(package);
	}
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

void RendererChannel::BeforeWrite(const ChannelPackageInfo& info)
{
	if (info.semantic == ChannelPackageSemantic::Request)
	{
		SPIN_LOCK(cacheLock)
		{
			isCaching = true;
		}
	}
}

void RendererChannel::BeforeOnReceive(const ChannelPackageInfo& info)
{
	if (info.semantic == ChannelPackageSemantic::Response)
	{
		SPIN_LOCK(cacheLock)
		{
			isCaching = false;
		}
	}
}