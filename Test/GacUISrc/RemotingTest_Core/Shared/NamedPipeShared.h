#include <VlppOS.h>
#include <Windows.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;

constexpr const wchar_t* NamedPipeId = L"\\\\.\\pipe\\GacUIRemoteProtocol";

class NamedPipeShared : public Object
{
	// NamedPipe doesn't support a single message that is larger than 64K
	static constexpr vint32_t						MaxMessageSize = 65536;

protected:
	HANDLE											hPipe = INVALID_HANDLE_VALUE;

private:
	bool											firstRead = true;
	BYTE											bufferReadFile[MaxMessageSize];
	stream::MemoryStream							streamReadFile;
	HANDLE											hWaitHandleReadFile = INVALID_HANDLE_VALUE;
	OVERLAPPED										overlappedReadFile;
	HANDLE											hEventReadFile = INVALID_HANDLE_VALUE;

	stream::MemoryStream							streamWriteFile;
	OVERLAPPED										overlappedWriteFile;
	HANDLE											hEventWriteFile = INVALID_HANDLE_VALUE;

protected:

	virtual void									OnReadStringThreadUnsafe(Ptr<List<WString>> strs) = 0;
	virtual void									OnReadStoppedThreadUnsafe() = 0;

private:

	void											BeginReadingUnsafe();
	void											SubmitReadBufferUnsafe(vint bytes);
	void											EndReadingUnsafe();

protected:

	void											BeginReadingLoopUnsafe();

private:

	vint32_t										WriteInt32ToStream(vint32_t number);
	vint32_t										WriteStringToStream(const WString& str);
	void											BeginSendStream();
	void											EndSendStream(vint32_t bytes);

protected:

	void											SendStringArray(vint count, List<WString>& strs);
	void											SendSingleString(const WString& str);

public:

	NamedPipeShared(HANDLE _hPipe);
	~NamedPipeShared();

	static HANDLE									ServerCreatePipe();
	static void										ServerWaitForClient(HANDLE hPipe);
	static HANDLE									ClientCreatePipe();
	static void										ClientWaitForServer(HANDLE hPipe);
};