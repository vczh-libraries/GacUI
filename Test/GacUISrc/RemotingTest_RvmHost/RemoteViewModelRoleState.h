#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_ROLESTATE
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_ROLESTATE

#include "RemoteViewModelTestShared.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol_Channel_Json.h"

namespace vl::presentation::remote_view_model_test
{
	inline constexpr vint InvalidRemoteViewModelClientId = -1;

	enum class RemoteViewModelChannelRole
	{
		Unknown,
		ViewModelHost,
		Renderer,
	};

	enum class RemoteViewModelApplicationPhase
	{
		Starting,
		Running,
		Stopping,
	};

	class RemoteViewModelLeaseState
	{
	private:
		bool											active = false;
		vint64_t										expiration = -1;
		vint64_t										lastSequence = -1;

	public:
		void											Start(vint64_t now, vint timeout)
		{
			active = true;
			expiration = now + timeout;
			lastSequence = -1;
		}

		bool											Renew(vint64_t sequence, vint64_t now, vint timeout)
		{
			if (
				!active ||
				now >= expiration ||
				sequence <= lastSequence
				)
			{
				return false;
			}
			lastSequence = sequence;
			expiration = now + timeout;
			return true;
		}

		bool											IsExpired(vint64_t now)
		{
			return active && now >= expiration;
		}

		void											Stop()
		{
			active = false;
		}

		bool											IsActive()
		{
			return active;
		}

		vint64_t										GetLastSequence()
		{
			return lastSequence;
		}
	};

	class RemoteViewModelTerminalState
	{
	private:
		bool											stopping = false;
		bool											claimed = false;
		bool											normal = false;
		bool											taken = false;
		WString											message;

	public:
		bool											TryClaimFailure(const WString& errorMessage)
		{
			if (stopping || claimed)
			{
				return false;
			}
			claimed = true;
			normal = false;
			message = errorMessage;
			return true;
		}

		bool											TryClaimNormal()
		{
			if (stopping || claimed)
			{
				return false;
			}
			claimed = true;
			normal = true;
			return true;
		}

		bool											BeginStopping()
		{
			if (stopping || claimed)
			{
				return false;
			}
			stopping = true;
			return true;
		}

		bool											TryTake(bool& isNormal, WString& terminalMessage)
		{
			if (!claimed || taken)
			{
				return false;
			}
			taken = true;
			isNormal = normal;
			terminalMessage = message;
			return true;
		}

		bool											IsStopping()
		{
			return stopping;
		}

		bool											HasFailure()
		{
			return claimed && !normal;
		}

		const WString&									GetMessage()
		{
			return message;
		}
	};

	template<typename TChannelNameList>
	RemoteViewModelChannelRole ClassifyRemoteViewModelChannel(const TChannelNameList& availableChannels)
	{
		if (
			availableChannels.Count() == 2 &&
			availableChannels.IndexOf(WString::Unmanaged(ViewModelChannelName)) != -1 &&
			availableChannels.IndexOf(WString::Unmanaged(ViewModelReadyChannelName)) != -1
			)
		{
			return RemoteViewModelChannelRole::ViewModelHost;
		}

		if (
			availableChannels.Count() == 1 &&
			availableChannels[0] == WString::Unmanaged(remoteprotocol::channeling::GacUIRemoteProtocolChannelName)
			)
		{
			return RemoteViewModelChannelRole::Renderer;
		}
		return RemoteViewModelChannelRole::Unknown;
	}

	class RemoteViewModelRoleState
	{
	private:
		bool								remoteViewModelEnabled = false;
		RemoteViewModelApplicationPhase		phase = RemoteViewModelApplicationPhase::Running;
		bool								viewModelHostAccepted = false;
		bool								fatalError = false;
		vint								viewModelHostId = InvalidRemoteViewModelClientId;
		vint								requesterClientId = InvalidRemoteViewModelClientId;
		vint								rendererClientId = InvalidRemoteViewModelClientId;

		bool IsDistinctClientId(vint clientId)
		{
			return
				clientId != InvalidRemoteViewModelClientId &&
				clientId != viewModelHostId &&
				clientId != requesterClientId &&
				clientId != rendererClientId;
		}

	public:
		bool EnableRemoteViewModel()
		{
			if (
				remoteViewModelEnabled ||
				phase != RemoteViewModelApplicationPhase::Running ||
				viewModelHostAccepted ||
				fatalError ||
				viewModelHostId != InvalidRemoteViewModelClientId ||
				requesterClientId != InvalidRemoteViewModelClientId ||
				rendererClientId != InvalidRemoteViewModelClientId
				)
			{
				return false;
			}

			remoteViewModelEnabled = true;
			phase = RemoteViewModelApplicationPhase::Starting;
			return true;
		}

		bool IsRemoteViewModelEnabled()
		{
			return remoteViewModelEnabled;
		}

		RemoteViewModelApplicationPhase GetPhase()
		{
			return phase;
		}

		bool HasFatalError()
		{
			return fatalError;
		}

		bool HasAcceptedViewModelHost()
		{
			return viewModelHostAccepted;
		}

		vint GetViewModelHostId()
		{
			return viewModelHostId;
		}

		vint GetRequesterClientId()
		{
			return requesterClientId;
		}

		vint GetRendererClientId()
		{
			return rendererClientId;
		}

		bool TryAcceptViewModelHost(vint clientId)
		{
			if (
				!remoteViewModelEnabled ||
				phase != RemoteViewModelApplicationPhase::Starting ||
				viewModelHostAccepted ||
				fatalError ||
				!IsDistinctClientId(clientId)
				)
			{
				return false;
			}

			viewModelHostAccepted = true;
			viewModelHostId = clientId;
			return true;
		}

		bool FailViewModelHost(vint clientId)
		{
			if (
				!remoteViewModelEnabled ||
				viewModelHostId != clientId
				)
			{
				return false;
			}

			viewModelHostId = InvalidRemoteViewModelClientId;
			if (phase == RemoteViewModelApplicationPhase::Stopping)
			{
				return false;
			}
			fatalError = true;
			return true;
		}

		bool FailRemoteViewModel()
		{
			if (
				!remoteViewModelEnabled ||
				phase == RemoteViewModelApplicationPhase::Stopping ||
				fatalError
				)
			{
				return false;
			}

			fatalError = true;
			return true;
		}

		bool RegisterRequester(vint clientId)
		{
			if (
				!remoteViewModelEnabled ||
				phase != RemoteViewModelApplicationPhase::Starting ||
				requesterClientId != InvalidRemoteViewModelClientId ||
				!IsDistinctClientId(clientId)
				)
			{
				return false;
			}

			requesterClientId = clientId;
			return true;
		}

		bool DisconnectRequester(vint clientId)
		{
			if (requesterClientId != clientId)
			{
				return false;
			}

			requesterClientId = InvalidRemoteViewModelClientId;
			return true;
		}

		bool CanAdmitRenderer()
		{
			return
				phase == RemoteViewModelApplicationPhase::Running &&
				!fatalError;
		}

		bool TryAcceptRenderer(vint clientId, vint& replacedClientId)
		{
			replacedClientId = InvalidRemoteViewModelClientId;
			if (
				!CanAdmitRenderer() ||
				clientId == rendererClientId ||
				!IsDistinctClientId(clientId)
				)
			{
				return false;
			}

			replacedClientId = rendererClientId;
			rendererClientId = clientId;
			return true;
		}

		bool DisconnectRenderer(vint clientId)
		{
			if (rendererClientId != clientId)
			{
				return false;
			}

			rendererClientId = InvalidRemoteViewModelClientId;
			return true;
		}

		bool BeginRunning()
		{
			if (
				!remoteViewModelEnabled ||
				phase != RemoteViewModelApplicationPhase::Starting ||
				fatalError ||
				viewModelHostId == InvalidRemoteViewModelClientId
				)
			{
				return false;
			}

			phase = RemoteViewModelApplicationPhase::Running;
			return true;
		}

		void BeginStopping()
		{
			phase = RemoteViewModelApplicationPhase::Stopping;
		}
	};
}

#endif
