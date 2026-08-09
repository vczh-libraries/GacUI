#include "StdioRedirection.h"
#include <cstdlib>
#include <limits>

namespace vl::presentation::remoting
{
	using namespace collections;
	using namespace console;
	using namespace inter_process;
	using namespace stream;

	WString EncodeStdioRedirectionMessage(const WString& message)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoting::EncodeStdioRedirectionMessage(const WString&)#"
		Array<vuint8_t> utf8;
		CHECK_ERROR(async_tcp_socket::EncodeStrictUtf8(message, utf8), ERROR_MESSAGE_PREFIX L"The message is not valid Unicode.");

		MemoryStream base64WStringStream;
		{
			UtfGeneralEncoder<wchar_t, char8_t> utf8ToWCharEncoder;
			EncoderStream utf8ToWCharStream(base64WStringStream, utf8ToWCharEncoder);
			Utf8Base64Encoder binaryToBase64Utf8Encoder;
			EncoderStream binaryToBase64Utf8Stream(utf8ToWCharStream, binaryToBase64Utf8Encoder);
			if (utf8.Count() > 0)
			{
				MemoryWrapperStream utf8Stream(&utf8[0], utf8.Count());
				CopyStream(utf8Stream, binaryToBase64Utf8Stream);
			}
		}
		base64WStringStream.SeekFromBegin(0);
		StreamReader reader(base64WStringStream);
		return reader.ReadToEnd();
#undef ERROR_MESSAGE_PREFIX
	}

	static vint GetStdioRedirectionBase64Value(wchar_t c)
	{
		if (L'A' <= c && c <= L'Z') return c - L'A';
		if (L'a' <= c && c <= L'z') return c - L'a' + 26;
		if (L'0' <= c && c <= L'9') return c - L'0' + 52;
		if (c == L'+') return 62;
		if (c == L'/') return 63;
		return -1;
	}

	static bool IsCanonicalStdioRedirectionBase64(const WString& base64)
	{
		auto lengthValue = base64.Length();
		if (lengthValue > (pos_t)std::numeric_limits<vint>::max())
		{
			return false;
		}
		auto length = (vint)lengthValue;
		if (length == 0)
		{
			return true;
		}
		if (length % 4 != 0)
		{
			return false;
		}

		vint padding = 0;
		if (base64[length - 1] == L'=') padding++;
		if (base64[length - 2] == L'=') padding++;
		for (vint i = 0; i < length - padding; i++)
		{
			if (GetStdioRedirectionBase64Value(base64[i]) == -1)
			{
				return false;
			}
		}
		for (vint i = length - padding; i < length; i++)
		{
			if (base64[i] != L'=')
			{
				return false;
			}
		}

		if (padding == 1)
		{
			return (GetStdioRedirectionBase64Value(base64[length - 2]) & 0x03) == 0;
		}
		if (padding == 2)
		{
			return (GetStdioRedirectionBase64Value(base64[length - 3]) & 0x0F) == 0;
		}
		return true;
	}

	bool DecodeStdioRedirectionMessage(const WString& base64, WString& message)
	{
		if (!IsCanonicalStdioRedirectionBase64(base64))
		{
			return false;
		}
		try
		{
			MemoryWrapperStream base64WStringStream((void*)base64.Buffer(), base64.Length() * sizeof(wchar_t));
			UtfGeneralDecoder<wchar_t, char8_t> wcharToUtf8Decoder;
			DecoderStream wcharToUtf8Stream(base64WStringStream, wcharToUtf8Decoder);
			Utf8Base64Decoder base64Utf8ToBinaryDecoder;
			DecoderStream base64Utf8ToBinaryStream(wcharToUtf8Stream, base64Utf8ToBinaryDecoder);
			MemoryStream utf8Stream;
			CopyStream(base64Utf8ToBinaryStream, utf8Stream);

			auto sizeValue = utf8Stream.Size();
			if (sizeValue > (pos_t)std::numeric_limits<vint>::max())
			{
				return false;
			}
			auto size = (vint)sizeValue;
			Array<vuint8_t> utf8(size);
			utf8Stream.SeekFromBegin(0);
			if (size > 0 && utf8Stream.Read(&utf8[0], size) != size)
			{
				return false;
			}
			return async_tcp_socket::DecodeStrictUtf8(size == 0 ? nullptr : &utf8[0], size, message);
		}
		catch (const Error&)
		{
			return false;
		}
		catch (const Exception&)
		{
			return false;
		}
	}

/***********************************************************************
StdioRedirectionConnection::Lifecycle
***********************************************************************/

	class StdioRedirectionConnection::Lifecycle : public Object
	{
	public:
		Ptr<IStdioRedirectionProcess>				process;

		CriticalSection							lockState;
		ConditionVariable						cvState;
		INetworkProtocolCallback*					callback = nullptr;
		bool									callbackInstalling = false;
		vint									activeCallbacks = 0;
		bool									readingStarted = false;
		bool									readerFinished = false;
		vint									readerThreadId = -1;
		bool									stopStarted = false;
		bool									disconnectDelivering = false;
		bool									disconnectFinished = false;

		CriticalSection							lockWrite;
	};

/***********************************************************************
StdioRedirectionConnection::CallbackFrame
***********************************************************************/

	struct StdioRedirectionConnection::CallbackFrame
	{
		Ptr<Lifecycle>								state;
		CallbackFrame*								previous = nullptr;

		CallbackFrame(Ptr<Lifecycle> _state)
			: state(_state)
			, previous(currentCallbackFrame)
		{
			currentCallbackFrame = this;
		}

		~CallbackFrame()
		{
			currentCallbackFrame = previous;
			CS_LOCK(state->lockState)
			{
				state->activeCallbacks--;
				state->cvState.WakeAllPendings();
			}
		}
	};

	thread_local StdioRedirectionConnection::CallbackFrame* StdioRedirectionConnection::currentCallbackFrame = nullptr;

/***********************************************************************
StdioRedirectionConnection
***********************************************************************/

	vint StdioRedirectionConnection::CurrentCallbackDepth(Ptr<Lifecycle> state)
	{
		vint depth = 0;
		for (auto frame = currentCallbackFrame; frame; frame = frame->previous)
		{
			if (frame->state.Obj() == state.Obj())
			{
				depth++;
			}
		}
		return depth;
	}

	void StdioRedirectionConnection::InvokeCallback(
		Ptr<Lifecycle> state,
		bool allowTerminal,
		const Func<void(INetworkProtocolCallback*)>& callback
		)
	{
		INetworkProtocolCallback* installed = nullptr;
		auto callbackDepth = CurrentCallbackDepth(state);
		state->lockState.Enter();
		while (state->callbackInstalling && callbackDepth == 0 && state->callback)
		{
			state->cvState.SleepWith(state->lockState);
		}
		if (state->callback && (allowTerminal || !state->stopStarted))
		{
			installed = state->callback;
			state->activeCallbacks++;
		}
		state->lockState.Leave();

		if (installed)
		{
			CallbackFrame frame(state);
			callback(installed);
		}
	}

	void StdioRedirectionConnection::NotifyDisconnected(Ptr<Lifecycle> state)
	{
		auto callbackDepth = CurrentCallbackDepth(state);
		state->lockState.Enter();
		if (state->disconnectFinished)
		{
			state->lockState.Leave();
			return;
		}
		if (state->disconnectDelivering)
		{
			if (callbackDepth == 0)
			{
				while (!state->disconnectFinished)
				{
					state->cvState.SleepWith(state->lockState);
				}
			}
			state->lockState.Leave();
			return;
		}
		if (callbackDepth == 0)
		{
			while (state->activeCallbacks > 0)
			{
				state->cvState.SleepWith(state->lockState);
			}
		}
		state->disconnectDelivering = true;
		while (state->activeCallbacks > callbackDepth)
		{
			state->cvState.SleepWith(state->lockState);
		}
		state->lockState.Leave();

		auto finishDisconnect = [&]()
		{
			CS_LOCK(state->lockState)
			{
				state->callback = nullptr;
				while (state->activeCallbacks > callbackDepth)
				{
					state->cvState.SleepWith(state->lockState);
				}
				state->disconnectFinished = true;
				state->cvState.WakeAllPendings();
			}
		};

		try
		{
			InvokeCallback(state, true, [](INetworkProtocolCallback* installed)
			{
				installed->OnDisconnected();
			});
		}
		catch (...)
		{
			finishDisconnect();
			throw;
		}
		finishDisconnect();
	}

	void StdioRedirectionConnection::ReportLocalError(Ptr<Lifecycle> state, const WString& errorMessage)
	{
		InvokeCallback(state, true, [&](INetworkProtocolCallback* installed)
		{
			installed->OnLocalError(errorMessage, true);
		});
	}

	void StdioRedirectionConnection::Disconnect(Ptr<Lifecycle> state, bool sendExit)
	{
		bool firstStop = false;
		CS_LOCK(state->lockWrite)
		{
			CS_LOCK(state->lockState)
			{
				if (!state->stopStarted)
				{
					state->stopStarted = true;
					firstStop = true;
				}
			}
			if (firstStop && sendExit && state->process)
			{
				const vuint8_t exitLine[] = { '!', 'E', 'x', 'i', 't', '\n' };
				WString errorMessage;
				state->process->Write(exitLine, sizeof(exitLine), errorMessage);
			}
		}

		if (firstStop && state->process)
		{
			state->process->CloseInput();
		}
		NotifyDisconnected(state);

		if (state->process && Thread::GetCurrentThreadId() != state->readerThreadId)
		{
			bool readerStarted = false;
			CS_LOCK(state->lockState)
			{
				readerStarted = state->readingStarted;
			}
			if (readerStarted)
			{
				CS_LOCK(state->lockState)
				{
					while (!state->readerFinished)
					{
						state->cvState.SleepWith(state->lockState);
					}
				}
			}
			else if (firstStop)
			{
				state->process->WaitForExit();
				state->process->CloseOutput();
				CS_LOCK(state->lockState)
				{
					state->readerFinished = true;
					state->cvState.WakeAllPendings();
				}
			}
			else
			{
				CS_LOCK(state->lockState)
				{
					while (!state->readerFinished)
					{
						state->cvState.SleepWith(state->lockState);
					}
				}
			}
		}
	}

	bool StdioRedirectionConnection::ProcessLine(Ptr<Lifecycle> state, const WString& line)
	{
		if (line == L"!Exit")
		{
			Disconnect(state, false);
			return false;
		}
		if (line.Length() > 0 && line[0] == L'!')
		{
			return true;
		}

		WString message;
		if (DecodeStdioRedirectionMessage(line, message))
		{
			InvokeCallback(state, false, [&](INetworkProtocolCallback* installed)
			{
				installed->OnReadString(message);
			});
		}
		return true;
	}

	void StdioRedirectionConnection::FinishReader(Ptr<Lifecycle> state)
	{
		if (state->process)
		{
			state->process->WaitForExit();
			state->process->CloseOutput();
		}
		CS_LOCK(state->lockState)
		{
			state->readerFinished = true;
			state->cvState.WakeAllPendings();
		}
	}

	void StdioRedirectionConnection::ReadLoop(Ptr<Lifecycle> state)
	{
		CS_LOCK(state->lockState)
		{
			state->readerThreadId = Thread::GetCurrentThreadId();
		}

		try
		{
			if (state->process)
			{
				List<vuint8_t> lineBytes;
				vuint8_t buffer[4096];
				bool continueReading = true;
				while (continueReading)
				{
					WString errorMessage;
					auto read = state->process->Read(buffer, sizeof(buffer), errorMessage);
					if (read < 0)
					{
						ReportLocalError(state, errorMessage);
						Disconnect(state, false);
						break;
					}
					if (read == 0)
					{
						Disconnect(state, false);
						break;
					}

					for (vint i = 0; i < read && continueReading; i++)
					{
						if (buffer[i] == '\n')
						{
							if (lineBytes.Count() > 0 && lineBytes[lineBytes.Count() - 1] == '\r')
							{
								lineBytes.RemoveAt(lineBytes.Count() - 1);
							}
							bool ascii = true;
							Array<wchar_t> characters(lineBytes.Count());
							for (vint j = 0; j < lineBytes.Count(); j++)
							{
								if (lineBytes[j] > 0x7F)
								{
									ascii = false;
									break;
								}
								characters[j] = (wchar_t)lineBytes[j];
							}
							if (ascii)
							{
								auto line = characters.Count() == 0
									? WString::Empty
									: WString::CopyFrom(&characters[0], characters.Count());
								continueReading = ProcessLine(state, line);
							}
							lineBytes.Clear();
						}
						else
						{
							lineBytes.Add(buffer[i]);
						}
					}
				}
			}
			else
			{
				for (;;)
				{
					auto line = Console::TryRead();
					if (!line)
					{
						Disconnect(state, false);
						break;
					}
					if (!ProcessLine(state, line.Value()))
					{
						break;
					}
				}
			}
			FinishReader(state);
		}
		catch (...)
		{
			std::_Exit(1);
		}
	}

	StdioRedirectionConnection::StdioRedirectionConnection()
		: lifecycle(Ptr(new Lifecycle))
	{
	}

	StdioRedirectionConnection::StdioRedirectionConnection(Ptr<IStdioRedirectionProcess> process)
		: lifecycle(Ptr(new Lifecycle))
	{
		CHECK_ERROR(process, L"StdioRedirectionConnection::StdioRedirectionConnection(...)#The process is null.");
		lifecycle->process = process;
	}

	StdioRedirectionConnection::~StdioRedirectionConnection()
	{
		Stop();
		InstallCallback(nullptr);
	}

	void StdioRedirectionConnection::InstallCallback(INetworkProtocolCallback* callback)
	{
		auto state = lifecycle;
		if (!callback)
		{
			auto callbackDepth = CurrentCallbackDepth(state);
			CS_LOCK(state->lockState)
			{
				state->callback = nullptr;
				while (state->activeCallbacks > callbackDepth)
				{
					state->cvState.SleepWith(state->lockState);
				}
			}
			return;
		}

		bool canInstall = false;
		CS_LOCK(state->lockState)
		{
			if (!state->callback && !state->callbackInstalling && !state->stopStarted)
			{
				state->callback = callback;
				state->callbackInstalling = true;
				state->activeCallbacks++;
				canInstall = true;
			}
		}
		CHECK_ERROR(canInstall, L"StdioRedirectionConnection::InstallCallback(...)#A callback cannot be replaced or installed after disconnection.");

		CallbackFrame frame(state);
		try
		{
			callback->OnInstalled(this);
		}
		catch (...)
		{
			CS_LOCK(state->lockState)
			{
				if (state->callback == callback)
				{
					state->callback = nullptr;
				}
				state->callbackInstalling = false;
				state->cvState.WakeAllPendings();
			}
			throw;
		}
		CS_LOCK(state->lockState)
		{
			state->callbackInstalling = false;
			state->cvState.WakeAllPendings();
		}
	}

	void StdioRedirectionConnection::BeginReadingLoopUnsafe()
	{
		auto state = lifecycle;
		CS_LOCK(state->lockState)
		{
			CHECK_ERROR(!state->stopStarted, L"StdioRedirectionConnection::BeginReadingLoopUnsafe()#The connection has disconnected.");
			CHECK_ERROR(state->callback, L"StdioRedirectionConnection::BeginReadingLoopUnsafe()#No callback is installed.");
			CHECK_ERROR(!state->readingStarted, L"StdioRedirectionConnection::BeginReadingLoopUnsafe()#The reading loop has already started.");
			state->readingStarted = true;
		}
		if (!ThreadPoolLite::Queue(Func<void()>([state]()
		{
			ReadLoop(state);
		})))
		{
			CS_LOCK(state->lockState)
			{
				state->readingStarted = false;
			}
			CHECK_ERROR(false, L"StdioRedirectionConnection::BeginReadingLoopUnsafe()#Failed to start the reading loop.");
		}
	}

	void StdioRedirectionConnection::SendString(const WString& str)
	{
		auto state = lifecycle;
		auto line = EncodeStdioRedirectionMessage(str);
		bool writeFailed = false;
		WString errorMessage;
		CS_LOCK(state->lockWrite)
		{
			bool canWrite = false;
			CS_LOCK(state->lockState)
			{
				canWrite = !state->stopStarted;
			}
			if (!canWrite)
			{
				return;
			}

			if (state->process)
			{
				Array<vuint8_t> bytes(line.Length() + 1);
				for (vint i = 0; i < line.Length(); i++)
				{
					bytes[i] = (vuint8_t)line[i];
				}
				bytes[line.Length()] = '\n';
				writeFailed = !state->process->Write(&bytes[0], bytes.Count(), errorMessage);
			}
			else
			{
				try
				{
					Console::WriteLine(line);
				}
				catch (const Error& error)
				{
					errorMessage = WString::Unmanaged(error.Description());
					writeFailed = true;
				}
				catch (const Exception& error)
				{
					errorMessage = error.Message();
					writeFailed = true;
				}
			}
		}
		if (writeFailed)
		{
			ReportLocalError(state, errorMessage);
			Disconnect(state, false);
		}
	}

	void StdioRedirectionConnection::Stop()
	{
		Disconnect(lifecycle, lifecycle->process != nullptr);
	}

	ClientStatus StdioRedirectionConnection::GetStatus()
	{
		bool stopped = false;
		CS_LOCK(lifecycle->lockState)
		{
			stopped = lifecycle->stopStarted;
		}
		return stopped ? ClientStatus::Disconnected : ClientStatus::Connected;
	}

	bool StdioRedirectionConnection::IsInCurrentCallback()
	{
		return CurrentCallbackDepth(lifecycle) > 0;
	}

/***********************************************************************
StdioRedirectionClient
***********************************************************************/

	StdioRedirectionClient::StdioRedirectionClient()
		: connection(Ptr(new StdioRedirectionConnection))
	{
	}

	INetworkProtocolConnection* StdioRedirectionClient::GetConnection()
	{
		return connection.Obj();
	}

	void StdioRedirectionClient::WaitForServer()
	{
	}

	ClientStatus StdioRedirectionClient::GetStatus()
	{
		return connection->GetStatus();
	}

/***********************************************************************
StdioRedirectionServer::Lifecycle
***********************************************************************/

	class StdioRedirectionServer::Lifecycle : public Object
	{
	public:
		enum class State
		{
			Ready,
			Started,
			Stopped,
		};

		CriticalSection							lockState;
		ConditionVariable						cvState;
		State									state = State::Ready;
		vint									activeConnects = 0;
		List<Ptr<StdioRedirectionConnection>>			connections;
		bool									stopCompleted = false;
	};

/***********************************************************************
StdioRedirectionServer
***********************************************************************/

	void StdioRedirectionServer::FinalizeStop(Ptr<Lifecycle> state)
	{
		List<Ptr<StdioRedirectionConnection>> connections;
		CS_LOCK(state->lockState)
		{
			while (state->activeConnects > 0)
			{
				state->cvState.SleepWith(state->lockState);
			}
			for (auto&& connection : state->connections)
			{
				connections.Add(connection);
			}
			state->connections.Clear();
		}

		try
		{
			for (auto&& connection : connections)
			{
				connection->Stop();
			}
		}
		catch (...)
		{
			std::_Exit(1);
		}

		CS_LOCK(state->lockState)
		{
			state->stopCompleted = true;
			state->cvState.WakeAllPendings();
		}
	}

	void StdioRedirectionServer::FinishConnect(
		Ptr<Lifecycle> state,
		Ptr<StdioRedirectionConnection> connection,
		bool keepConnection
		)
	{
		CS_LOCK(state->lockState)
		{
			if (!keepConnection)
			{
				for (vint i = 0; i < state->connections.Count(); i++)
				{
					if (state->connections[i].Obj() == connection.Obj())
					{
						state->connections.RemoveAt(i);
						break;
					}
				}
			}
			state->activeConnects--;
			state->cvState.WakeAllPendings();
		}
	}

	StdioRedirectionServer::StdioRedirectionServer()
		: lifecycle(Ptr(new Lifecycle))
	{
	}

	StdioRedirectionServer::~StdioRedirectionServer()
	{
		try
		{
			Stop();
		}
		catch (...)
		{
		}
	}

	WaitForClientResult StdioRedirectionServer::OnClientConnected(INetworkProtocolConnection*)
	{
		return WaitForClientResult::Accept;
	}

	void StdioRedirectionServer::Start()
	{
		CS_LOCK(lifecycle->lockState)
		{
			CHECK_ERROR(lifecycle->state == Lifecycle::State::Ready, L"StdioRedirectionServer::Start()#The server has already started or stopped.");
			lifecycle->state = Lifecycle::State::Started;
		}
	}

	void StdioRedirectionServer::Stop()
	{
		auto state = lifecycle;
		bool firstStop = false;
		bool reentrant = false;
		CS_LOCK(state->lockState)
		{
			if (state->state != Lifecycle::State::Stopped)
			{
				state->state = Lifecycle::State::Stopped;
				firstStop = true;
				for (auto&& connection : state->connections)
				{
					if (connection->IsInCurrentCallback())
					{
						reentrant = true;
						break;
					}
				}
			}
			else if (!state->stopCompleted)
			{
				for (auto&& connection : state->connections)
				{
					if (connection->IsInCurrentCallback())
					{
						reentrant = true;
						break;
					}
				}
			}
		}

		if (firstStop)
		{
			if (reentrant)
			{
				if (!ThreadPoolLite::Queue(Func<void()>([state]()
				{
					FinalizeStop(state);
				})))
				{
					std::_Exit(1);
				}
				return;
			}
			FinalizeStop(state);
			return;
		}

		if (!reentrant)
		{
			CS_LOCK(state->lockState)
			{
				while (!state->stopCompleted)
				{
					state->cvState.SleepWith(state->lockState);
				}
			}
		}
	}

	bool StdioRedirectionServer::IsStopped()
	{
		bool stopped = false;
		CS_LOCK(lifecycle->lockState)
		{
			stopped = lifecycle->state == Lifecycle::State::Stopped;
		}
		return stopped;
	}

	void StdioRedirectionServer::ConnectNewClient(const WString& command)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoting::StdioRedirectionServer::ConnectNewClient(const WString&)#"
		auto state = lifecycle;
		CHECK_ERROR(command != L"", ERROR_MESSAGE_PREFIX L"The command is empty.");
		CS_LOCK(state->lockState)
		{
			CHECK_ERROR(state->state == Lifecycle::State::Started, ERROR_MESSAGE_PREFIX L"The server has not started or has stopped.");
			state->activeConnects++;
		}

		Ptr<StdioRedirectionConnection> connection;
		bool connectPending = true;
		try
		{
			auto process = CreateStdioRedirectionProcess(command);
			connection = Ptr(new StdioRedirectionConnection(process));
			bool admitted = false;
			CS_LOCK(state->lockState)
			{
				if (state->state == Lifecycle::State::Started)
				{
					state->connections.Add(connection);
					admitted = true;
				}
			}
			if (!admitted)
			{
				connection->Stop();
				FinishConnect(state, connection, false);
				connectPending = false;
				CHECK_ERROR(false, ERROR_MESSAGE_PREFIX L"The server stopped while launching the client.");
			}

			auto accepted = OnClientConnected(connection.Obj()) == WaitForClientResult::Accept;
			if (!accepted)
			{
				connection->Stop();
			}
			FinishConnect(state, connection, accepted);
			connectPending = false;
		}
		catch (...)
		{
			if (connection)
			{
				connection->Stop();
			}
			if (connectPending)
			{
				FinishConnect(state, connection, false);
			}
			throw;
		}
#undef ERROR_MESSAGE_PREFIX
	}
}
