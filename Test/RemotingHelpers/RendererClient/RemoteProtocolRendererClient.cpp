#include "RemoteProtocolRendererClient.h"

namespace vl::presentation::remoting
{
	using namespace remoteprotocol::channeling;
	using namespace remote_renderer;

	RemoteProtocolRendererClient::RemoteProtocolRendererClient(
		Ptr<inter_process::INetworkProtocolClient> client,
		Ptr<glr::json::Parser> parser,
		const WString& _fatalTitle
		)
		: Base(client, parser)
		, fatalTitle(_fatalTitle)
	{
		CHECK_ERROR(fatalTitle != L"", L"RemoteProtocolRendererClient::RemoteProtocolRendererClient(...)#The fatal title is empty.");
	}

	void RemoteProtocolRendererClient::QueueMainThreadTask(const Func<void()>& task)
	{
		GuiRemoteProtocolAsyncJsonChannelRenderer* targetChannel = nullptr;
		SPIN_LOCK(lockState)
		{
			if (!stopping)
			{
				targetChannel = asyncRendererChannel;
			}
		}
		if (targetChannel)
		{
			targetChannel->QueueMainThreadTask(task);
		}
	}

	void RemoteProtocolRendererClient::ProcessFatalError(const WString& errorMessage)
	{
		GuiRemoteRendererSingle* targetRenderer = nullptr;
		AutomationServiceRenderer* targetAutomationService = nullptr;
		SPIN_LOCK(lockState)
		{
			if (
				!stopping &&
				triggeredFatalError &&
				!retainedFatalError &&
				renderer &&
				rendererAutomationService &&
				!renderer->IsDisconnectedFromCore()
				)
			{
				targetRenderer = renderer;
				targetAutomationService = rendererAutomationService;
			}
		}
		if (!targetRenderer)
		{
			return;
		}

		bool retainRenderer = true;
#if !defined VCZH_GCC || defined VCZH_APPLE
		auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
		auto result = GetCurrentController()->DialogService()->ShowMessageBox(
			mainWindow,
			errorMessage + WString::Unmanaged(L"\r\n\r\nDo you want to close the renderer?"),
			fatalTitle,
			INativeDialogService::DisplayYesNo,
			INativeDialogService::DefaultFirst,
			INativeDialogService::IconError
			);
		retainRenderer = result != INativeDialogService::SelectYes;
#endif
		if (retainRenderer)
		{
			SPIN_LOCK(lockState)
			{
				if (
					!stopping &&
					renderer == targetRenderer &&
					rendererAutomationService == targetAutomationService
					)
				{
					retainedFatalError = true;
				}
			}
			targetRenderer->RetainByFatalError(errorMessage);
			targetAutomationService->SetFatalError(Nullable<WString>(errorMessage));
		}
		else
		{
			targetRenderer->ForceExitByFatelError();
		}
	}

	void RemoteProtocolRendererClient::ProcessDisconnected()
	{
		GuiRemoteProtocolAsyncJsonChannelRenderer* targetChannel = nullptr;
		GuiRemoteRendererSingle* targetRenderer = nullptr;
		bool forceRendererToExit = false;
		SPIN_LOCK(lockState)
		{
			if (stopping || triggeredFatalError || (renderer && renderer->IsDisconnectedFromCore()))
			{
				return;
			}
			targetChannel = asyncRendererChannel;
			targetRenderer = renderer;
			forceRendererToExit = targetRenderer && !targetRenderer->IsDisconnectedFromCore();
		}
		if (targetChannel)
		{
			targetChannel->Detach();
		}
		if (forceRendererToExit)
		{
			targetRenderer->ForceExitByFatelError();
		}
	}

	void RemoteProtocolRendererClient::SetRenderer(GuiRemoteRendererSingle* value)
	{
		SPIN_LOCK(lockState)
		{
			renderer = value;
		}
	}

	void RemoteProtocolRendererClient::SetAsyncRendererChannel(GuiRemoteProtocolAsyncJsonChannelRenderer* value)
	{
		SPIN_LOCK(lockState)
		{
			asyncRendererChannel = value;
		}
	}

	void RemoteProtocolRendererClient::SetRendererAutomationService(AutomationServiceRenderer* value)
	{
		SPIN_LOCK(lockState)
		{
			rendererAutomationService = value;
		}
	}

	void RemoteProtocolRendererClient::BeginStopping()
	{
		GuiRemoteProtocolAsyncJsonChannelRenderer* targetChannel = nullptr;
		SPIN_LOCK(lockState)
		{
			if (!stopping)
			{
				stopping = true;
				targetChannel = asyncRendererChannel;
			}
		}
		if (targetChannel)
		{
			targetChannel->Detach();
		}
	}

	bool RemoteProtocolRendererClient::IsFatalErrorRetained()
	{
		bool retained = false;
		SPIN_LOCK(lockState)
		{
			retained = retainedFatalError;
		}
		return retained;
	}

	void RemoteProtocolRendererClient::OnReadError(const WString& errorMessage)
	{
		GuiRemoteProtocolAsyncJsonChannelRenderer* targetChannel = nullptr;
		SPIN_LOCK(lockState)
		{
			if (!stopping && !triggeredFatalError && asyncRendererChannel)
			{
				triggeredFatalError = true;
				targetChannel = asyncRendererChannel;
			}
		}
		if (targetChannel)
		{
			targetChannel->QueueMainThreadTask(Func<void()>([this, errorMessage]()
			{
				ProcessFatalError(errorMessage);
			}));
		}
	}

	void RemoteProtocolRendererClient::OnLocalError(const WString&, bool fatal)
	{
		if (fatal)
		{
			QueueMainThreadTask(Func<void()>([this]()
			{
				ProcessDisconnected();
			}));
		}
	}

	void RemoteProtocolRendererClient::OnDisconnected()
	{
		Base::OnDisconnected();
		QueueMainThreadTask(Func<void()>([this]()
		{
			ProcessDisconnected();
		}));
	}
}
