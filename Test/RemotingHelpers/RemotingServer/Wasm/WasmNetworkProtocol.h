#ifndef VCZH_PRESENTATION_REMOTING_WASMNETWORKPROTOCOL
#define VCZH_PRESENTATION_REMOTING_WASMNETWORKPROTOCOL

#include <VlppOS.h>

#if defined VCZH_WASM

namespace vl::presentation::remoting
{
	class WasmNetworkProtocolConnection;

	class WasmNetworkProtocolServer : public Object, public virtual inter_process::INetworkProtocolServer
	{
	private:
		// All transport state and callbacks belong to the module's JavaScript worker.
		collections::Dictionary<vint, Ptr<WasmNetworkProtocolConnection>>	connections;
		bool																	stopped = true;

	public:
		void								Start() override;
		void								Stop() override;
		bool								IsStopped() override;

		void								Receive(vint connectionId, const WString& data);
	};

	/// <summary>Notify the browser that channel admission is ready, or that renderer admission is ready after RPC acquisition.</summary>
	extern void							NotifyWasmApplication(const WString& kind, vint connectionId = 0, const WString& data = WString::Empty);
}

/// <summary>Application entry implemented by each Wasm test application, running on its GacUI thread.</summary>
extern vl::vint WasmMain();

#endif
#endif
