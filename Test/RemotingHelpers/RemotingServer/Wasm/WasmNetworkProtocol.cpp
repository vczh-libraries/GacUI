#include "WasmNetworkProtocol.h"

#if defined VCZH_WASM
#include <emscripten/bind.h>
#include <emscripten/threading.h>

namespace vl::presentation::remoting
{
	using namespace inter_process;

	struct WasmThreadCall
	{
		Func<void()>								callback;
		std::exception_ptr						error;
	};

	void RunWasmThreadCall(WasmThreadCall* call)
	{
		try
		{
			call->callback();
		}
		catch (...)
		{
			call->error = std::current_exception();
		}
	}

	void InvokeOnWasmThread(const Func<void()>& callback)
	{
		WasmThreadCall call{ callback };
		emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, &RunWasmThreadCall, &call);
		if (call.error) std::rethrow_exception(call.error);
	}

	WString CurrentWasmException()
	{
		try
		{
			throw;
		}
		catch (const Error& error)
		{
			return error.Description();
		}
		catch (const Exception& error)
		{
			return error.Message();
		}
		catch (const std::exception& error)
		{
			return atow(error.what());
		}
		catch (...)
		{
			return L"Unknown C++ exception.";
		}
	}

	class WasmApplicationThread : public Thread
	{
	protected:
		void Run() override
		{
			vint result = 1;
			try
			{
				result = WasmMain();
			}
			catch (...)
			{
				NotifyWasmApplication(L"error", 0, CurrentWasmException());
			}
			NotifyWasmApplication(L"exit", 0, itow(result));
		}
	};

	struct WasmApplicationContext
	{
		Func<void(const WString&, vint, const WString&)>	receiver;
		WasmApplicationThread							thread;
		WasmNetworkProtocolServer*						server = nullptr;
		vint											connectionCount = 0;
	};

	WasmApplicationContext* wasmApplication = nullptr;

	void NotifyWasmApplication(const WString& kind, vint connectionId, const WString& data)
	{
		InvokeOnWasmThread([&]()
		{
			wasmApplication->receiver(kind, connectionId, data);
		});
	}

/***********************************************************************
WasmNetworkProtocolConnection
***********************************************************************/

	class WasmNetworkProtocolConnection : public Object, public INetworkProtocolConnection
	{
	private:
		vint									connectionId;
		INetworkProtocolCallback*			callback = nullptr;
		bool									reading = false;
		bool									stopped = false;

	public:
		WasmNetworkProtocolConnection(vint _connectionId);
		void									InstallCallback(INetworkProtocolCallback* value) override;
		void									BeginReadingLoopUnsafe() override;
		void									SendString(const WString& str) override;
		void									Stop() override;
		void									Receive(const WString& data);
	};

	WasmNetworkProtocolConnection::WasmNetworkProtocolConnection(vint _connectionId)
		: connectionId(_connectionId)
	{
	}

	void WasmNetworkProtocolConnection::InstallCallback(INetworkProtocolCallback* value)
	{
		InvokeOnWasmThread([&]()
		{
			callback = value;
			if (callback) callback->OnInstalled(this);
		});
	}

	void WasmNetworkProtocolConnection::BeginReadingLoopUnsafe()
	{
		InvokeOnWasmThread([&]() { reading = true; });
	}

	void WasmNetworkProtocolConnection::SendString(const WString& str)
	{
		InvokeOnWasmThread([&]()
		{
			if (!stopped) NotifyWasmApplication(L"data", connectionId, str);
		});
	}

	void WasmNetworkProtocolConnection::Stop()
	{
		InvokeOnWasmThread([&]()
		{
			if (stopped) return;
			stopped = true;
			NotifyWasmApplication(L"closed", connectionId);
			auto oldCallback = callback;
			callback = nullptr;
			if (oldCallback) oldCallback->OnDisconnected();
		});
	}

	void WasmNetworkProtocolConnection::Receive(const WString& data)
	{
		if (!stopped && reading) callback->OnReadString(data);
	}

/***********************************************************************
WasmNetworkProtocolServer
***********************************************************************/

	void WasmNetworkProtocolServer::Start()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoting::WasmNetworkProtocolServer::Start()#"
		InvokeOnWasmThread([&]()
		{
			CHECK_ERROR(!wasmApplication->server, ERROR_MESSAGE_PREFIX L"A Wasm server is already running.");
			stopped = false;
			wasmApplication->server = this;
			for (vint connectionId = 1; connectionId <= wasmApplication->connectionCount; connectionId++)
			{
				auto connection = Ptr(new WasmNetworkProtocolConnection(connectionId));
				connections.Add(connectionId, connection);
				if (OnClientConnected(connection) == WaitForClientResult::Reject)
				{
					connections.Remove(connectionId);
					connection->Stop();
				}
			}
		});
#undef ERROR_MESSAGE_PREFIX
	}

	void WasmNetworkProtocolServer::Stop()
	{
		InvokeOnWasmThread([&]()
		{
			if (stopped) return;
			stopped = true;
			wasmApplication->server = nullptr;
			auto stoppingConnections = std::move(connections);
			for (auto connection : stoppingConnections.Values()) connection->Stop();
		});
	}

	bool WasmNetworkProtocolServer::IsStopped()
	{
		bool result = false;
		InvokeOnWasmThread([&]() { result = stopped; });
		return result;
	}

	void WasmNetworkProtocolServer::Receive(vint connectionId, const WString& data)
	{
		auto index = connections.Keys().IndexOf(connectionId);
		if (index == -1) return;
		auto connection = connections.Values()[index];
		connection->Receive(data);
	}

/***********************************************************************
JavaScript Boundary
***********************************************************************/

	void StartApplication(const Func<void(const WString&, vint, const WString&)>& receiver, vint connectionCount)
	{
		if (wasmApplication) return;
		CHECK_ERROR(connectionCount > 0, L"StartApplication()#At least one connection is required.");
		wasmApplication = new WasmApplicationContext;
		wasmApplication->receiver = receiver;
		wasmApplication->connectionCount = connectionCount;
		CHECK_ERROR(wasmApplication->thread.Start(), L"StartApplication()#Failed to start the application thread.");
	}

	void SendDataToWasmCore(vint connectionId, const WString& data)
	{
		if (wasmApplication && wasmApplication->server) wasmApplication->server->Receive(connectionId, data);
	}

	std::u16string ExecuteWasmCall(const Func<void()>& callback)
	{
		try
		{
			callback();
			return {};
		}
		catch (...)
		{
			auto message = wtou16(CurrentWasmException());
			return std::u16string(message.Buffer(), message.Length());
		}
	}

	std::u16string wasm_StartApplication(emscripten::val receiver, vint connectionCount)
	{
		return ExecuteWasmCall([&]()
		{
			StartApplication([receiver](const WString& kind, vint connectionId, const WString& data)
			{
				auto encodedKind = wtou16(kind);
				auto encodedData = wtou16(data);
				auto error = receiver(
					std::u16string(encodedKind.Buffer(), encodedKind.Length()),
					connectionId,
					std::u16string(encodedData.Buffer(), encodedData.Length())
					).as<std::u16string>();
				if (!error.empty()) throw Exception(u16tow(U16String::CopyFrom(error.data(), error.size())));
			}, connectionCount);
		});
	}

	std::u16string wasm_SendDataToWasmCore(vint connectionId, const std::u16string& data)
	{
		return ExecuteWasmCall([&]() { SendDataToWasmCore(connectionId, u16tow(U16String::CopyFrom(data.data(), data.size()))); });
	}

}

EMSCRIPTEN_BINDINGS(GacUIWasmApplication)
{
	using namespace vl::presentation::remoting;
	emscripten::function("StartApplication", &wasm_StartApplication);
	emscripten::function("SendDataToWasmCore", &wasm_SendDataToWasmCore);
}
#endif
