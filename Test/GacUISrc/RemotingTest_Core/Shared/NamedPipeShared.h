#ifndef VCZH_REMOTINGTEST_SHARED_NAMEDPIPESHARED
#define VCZH_REMOTINGTEST_SHARED_NAMEDPIPESHARED

#include "ProtocolCallback.h"
#include <Windows.h>

constexpr const wchar_t* NamedPipeId = L"\\\\.\\pipe\\GacUIRemoteProtocol";

class NamedPipeSharedCommon : public INetworkProtocol
{
protected:
	INetworkProtocolCallback*						callback = nullptr;
	HANDLE											hPipe = INVALID_HANDLE_VALUE;

protected:
	void											InstallCallback(INetworkProtocolCallback* _callback);

	NamedPipeSharedCommon(HANDLE _hPipe);
	~NamedPipeSharedCommon();
};

class NamedPipeSharedReading : public virtual NamedPipeSharedCommon
{
private:
	bool											firstRead = true;
	Array<BYTE>										bufferReadFile;
	stream::MemoryStream							streamReadFile;
	HANDLE											hWaitHandleReadFile = INVALID_HANDLE_VALUE;
	OVERLAPPED										overlappedReadFile;
	HANDLE											hEventReadFile = INVALID_HANDLE_VALUE;

	void											BeginReadingUnsafe();
	void											SubmitReadBufferUnsafe(vint bytes);
	void											EndReadingUnsafe();

protected:
	void											BeginReadingLoopUnsafe();

	NamedPipeSharedReading(HANDLE _hPipe);
	~NamedPipeSharedReading();
};

class NamedPipeSharedWriting : public virtual NamedPipeSharedCommon
{
private:
	stream::MemoryStream							streamWriteFile;
	OVERLAPPED										overlappedWriteFile;
	HANDLE											hEventWriteFile = INVALID_HANDLE_VALUE;

	vint32_t										WriteInt32ToStream(vint32_t number);
	vint32_t										WriteStringToStream(const WString& str);
	void											BeginSendStream();
	void											EndSendStream(vint32_t bytes);

protected:
	void											SendStringArray(vint count, List<WString>& strs);
	void											SendSingleString(const WString& str);

	NamedPipeSharedWriting(HANDLE _hPipe);
	~NamedPipeSharedWriting();
};

class NamedPipeShared
	: public NamedPipeSharedReading
	, public NamedPipeSharedWriting
{
public:
	NamedPipeShared(HANDLE _hPipe);
	~NamedPipeShared();

	void											InstallCallback(INetworkProtocolCallback* _callback) override;
	void											BeginReadingLoopUnsafe() override;
	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;

	static HANDLE									ServerCreatePipe();
	static void										ServerWaitForClient(HANDLE hPipe);
	static HANDLE									ClientCreatePipe();
	static void										ClientWaitForServer(HANDLE hPipe);
};

#endif