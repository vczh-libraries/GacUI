#include "RemotingChannelClient.h"

namespace vl::presentation::remoting
{
	using namespace remoteprotocol::channeling;
	using namespace remote_renderer;

	RemotingChannelClient::RemotingChannelClient(
		Ptr<inter_process::INetworkProtocolClient> client,
		Ptr<glr::json::Parser> parser,
		const RemotingChannelClientConfiguration& _configuration
		)
		: Base(client, parser)
		, configuration(_configuration)
	{
		CHECK_ERROR(configuration.fatalTitle != L"", L"RemotingChannelClient::RemotingChannelClient(...)#The fatal title is empty.");
	}

	void RemotingChannelClient::QueueMainThreadTask(const Func<void()>& task)
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

	void RemotingChannelClient::ProcessFatalError(const WString& errorMessage)
	{
		GuiRemoteRendererSingle* targetRenderer = nullptr;
		AutomationServiceRenderer* targetAutomationService = nullptr;
		SPIN_LOCK(lockState)
		{
			if (
				!stopping &&
				!triggeredFatalError &&
				renderer &&
				rendererAutomationService &&
				!renderer->IsDisconnectedFromCore()
				)
			{
				triggeredFatalError = true;
				targetRenderer = renderer;
				targetAutomationService = rendererAutomationService;
			}
		}
		if (!targetRenderer)
		{
			return;
		}

		auto retainRenderer =
			configuration.retainFatalError
			? configuration.retainFatalError(configuration.fatalTitle, errorMessage)
			: true;
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

	void RemotingChannelClient::ProcessDisconnected()
	{
		GuiRemoteProtocolAsyncJsonChannelRenderer* targetChannel = nullptr;
		GuiRemoteRendererSingle* targetRenderer = nullptr;
		bool forceRendererToExit = false;
		SPIN_LOCK(lockState)
		{
			if (stopping || disconnectedProcessed)
			{
				return;
			}
			disconnectedProcessed = true;
			targetChannel = asyncRendererChannel;
			targetRenderer = renderer;
			forceRendererToExit =
				!triggeredFatalError &&
				targetRenderer &&
				!targetRenderer->IsDisconnectedFromCore();
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

	void RemotingChannelClient::SetRenderer(GuiRemoteRendererSingle* value)
	{
		SPIN_LOCK(lockState)
		{
			renderer = value;
		}
	}

	void RemotingChannelClient::SetAsyncRendererChannel(GuiRemoteProtocolAsyncJsonChannelRenderer* value)
	{
		SPIN_LOCK(lockState)
		{
			asyncRendererChannel = value;
		}
	}

	void RemotingChannelClient::SetRendererAutomationService(AutomationServiceRenderer* value)
	{
		SPIN_LOCK(lockState)
		{
			rendererAutomationService = value;
		}
	}

	void RemotingChannelClient::BeginStopping()
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

	bool RemotingChannelClient::IsFatalErrorRetained()
	{
		bool retained = false;
		SPIN_LOCK(lockState)
		{
			retained = retainedFatalError;
		}
		return retained;
	}

	void RemotingChannelClient::OnReadError(const WString& errorMessage)
	{
		QueueMainThreadTask(Func<void()>([this, errorMessage]()
		{
			ProcessFatalError(errorMessage);
		}));
	}

	void RemotingChannelClient::OnLocalError(const WString&, bool fatal)
	{
		if (fatal)
		{
			QueueMainThreadTask(Func<void()>([this]()
			{
				ProcessDisconnected();
			}));
		}
	}

	void RemotingChannelClient::OnDisconnected()
	{
		Base::OnDisconnected();
		QueueMainThreadTask(Func<void()>([this]()
		{
			ProcessDisconnected();
		}));
	}
}
