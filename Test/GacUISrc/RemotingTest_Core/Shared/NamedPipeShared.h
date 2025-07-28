#ifndef VCZH_REMOTINGTEST_SHARED_NAMEDPIPESHARED
#define VCZH_REMOTINGTEST_SHARED_NAMEDPIPESHARED

#include "ProtocolCallback.h"

constexpr const wchar_t* NamedPipeId = L"\\\\.\\pipe\\GacUIRemoteProtocol";

class NamedPipeSharedCommon : public INetworkProtocol
{
protected:
	INetworkProtocolCallback*						callback = nullptr;
	HANDLE											hPipe = INVALID_HANDLE_VALUE;
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

	NamedPipeSharedReading();
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

	NamedPipeSharedWriting();
	~NamedPipeSharedWriting();
};

class NamedPipeShared
	: public NamedPipeSharedReading
	, public NamedPipeSharedWriting
{
protected:
	NamedPipeShared(HANDLE _hPipe);
	~NamedPipeShared();

public:

	void											Stop();

	void											InstallCallback(INetworkProtocolCallback* _callback) override;
	void											BeginReadingLoopUnsafe() override;
	void											SendStringArray(vint count, List<WString>& strs) override;
	void											SendSingleString(const WString& str) override;
};

class NamedPipeServer : public NamedPipeShared
{
protected:
	static HANDLE									ServerCreatePipe();

public:
	NamedPipeServer();
	~NamedPipeServer();

	void											WaitForClient();
};

class NamedPipeClient : public NamedPipeShared
{
protected:
	static HANDLE									ClientCreatePipe();

public:
	NamedPipeClient();
	~NamedPipeClient();

	void											WaitForServer();
};

#endif