#include "RendererChannel.h"

using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remote_renderer;

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
	SendSingleString(package);
}

RendererChannel::RendererChannel(GuiRemoteRendererSingle* _renderer, HANDLE _hPipe, IGuiRemoteProtocolChannel<WString>* _channel)
	: NamedPipeShared(_hPipe)
	, renderer(_renderer)
	, channel(_channel)
{
	eventDisconnected.CreateManualUnsignal(false);
	_channel->Initialize(this);
}

RendererChannel::~RendererChannel()
{
}

void RendererChannel::RegisterMainWindow(INativeWindow* _window)
{
	BeginReadingLoopUnsafe();
}

void RendererChannel::UnregisterMainWindow()
{
}

void RendererChannel::WaitForDisconnected()
{
	eventDisconnected.Wait();
}