#include "NamedPipeShared.h"

using namespace vl;
using namespace vl::console;
using namespace vl::collections;

/***********************************************************************
Reading
***********************************************************************/

void NamedPipeShared::BeginReadingUnsafe()
{
	if (firstRead)
	{
		streamReadFile.SeekFromBegin(0);
	}
}

void NamedPipeShared::SubmitReadBufferUnsafe(vint bytes)
{
	streamReadFile.Write(bufferReadFile, bytes);
}

void NamedPipeShared::EndReadingUnsafe()
{
	vint32_t position = (vint32_t)streamReadFile.Position();
	streamReadFile.SeekFromBegin(0);
	CHECK_ERROR(position >= 2 * sizeof(vint32_t), L"ReadFile failed on incomplete message.");

	vint32_t bytes = 0;
	vint consumed = 0;
	consumed = streamReadFile.Read(&bytes, sizeof(bytes));
	CHECK_ERROR(consumed == sizeof(bytes), L"ReadFile failed on incomplete message.");

	if (bytes > position - (vint32_t)sizeof(bytes))
	{
		firstRead = false;
		streamReadFile.SeekFromBegin(position);
		return;
	}
	CHECK_ERROR(bytes == position - sizeof(bytes), L"ReadFile failed on corrupted message.");
	firstRead = true;

	vint32_t count = 0;
	consumed = streamReadFile.Read(&count, sizeof(count));
	CHECK_ERROR(consumed == sizeof(count), L"ReadFile failed on incomplete message.");

	Array<wchar_t> strBuffer;
	auto strs = Ptr(new List<WString>);
	for (vint i = 0; i < count; i++)
	{
		vint32_t length = 0;
		consumed = streamReadFile.Read(&length, sizeof(length));
		CHECK_ERROR(consumed == sizeof(length) && streamReadFile.Position() <= position, L"ReadFile failed on incomplete message.");

		if (length == 0)
		{
			strs->Add(WString::Empty);
		}
		else
		{
			strBuffer.Resize(length);
			consumed = streamReadFile.Read(&strBuffer[0], length * sizeof(wchar_t));
			CHECK_ERROR(consumed == length * sizeof(wchar_t) && streamReadFile.Position() <= position, L"ReadFile failed on incomplete message.");
			strs->Add(WString::CopyFrom(&strBuffer[0], length));
		}
	}
	callback->OnReadStringThreadUnsafe(strs);

	CHECK_ERROR(streamReadFile.Position() == position, L"ReadFile failed on incomplete message.");
}

void NamedPipeShared::BeginReadingLoopUnsafe()
{
RESTART_LOOP:
	{
		BeginReadingUnsafe();
		ResetEvent(hEventReadFile);
		ZeroMemory(&overlappedReadFile, sizeof(overlappedReadFile));
		overlappedReadFile.hEvent = hEventReadFile;
		DWORD read = 0;
		BOOL result = ReadFile(hPipe, &bufferReadFile, sizeof(bufferReadFile), &read, &overlappedReadFile);

		if (result == TRUE)
		{
			SubmitReadBufferUnsafe((vint)read);
			EndReadingUnsafe();
			goto RESTART_LOOP;
		}

		DWORD error = GetLastError();
		if (error == ERROR_BROKEN_PIPE || error == ERROR_INVALID_HANDLE)
		{
			callback->OnReadStoppedThreadUnsafe();
			return;
		}
		CHECK_ERROR(error == ERROR_MORE_DATA || error == ERROR_IO_PENDING, L"ReadFile failed on unexpected GetLastError.");

		RegisterWaitForSingleObject(
			&hWaitHandleReadFile,
			hEventReadFile,
			[](PVOID lpParameter, BOOLEAN TimerOrWaitFired)
			{
				auto self = (NamedPipeShared*)lpParameter;
				UnregisterWait(self->hWaitHandleReadFile);
				self->hWaitHandleReadFile = INVALID_HANDLE_VALUE;

				DWORD read = 0;
				BOOL result = GetOverlappedResult(self->hPipe, &self->overlappedReadFile, &read, FALSE);
				if (result == TRUE)
				{
					self->SubmitReadBufferUnsafe((vint)read);
					self->EndReadingUnsafe();
				}
				else
				{
					DWORD error = GetLastError();
					if (error == ERROR_BROKEN_PIPE || error == ERROR_INVALID_HANDLE)
					{
						self->callback->OnReadStoppedThreadUnsafe();
						return;
					}
					CHECK_ERROR(error == ERROR_MORE_DATA, L"GetOverlappedResult(ReadFile) failed on unexpected GetLastError.");
					self->SubmitReadBufferUnsafe((vint)read);
				}
				self->BeginReadingLoopUnsafe();
			},
			this,
			INFINITE,
			WT_EXECUTEONLYONCE);
	}
}

/***********************************************************************
Writing
***********************************************************************/

vint32_t NamedPipeShared::WriteInt32ToStream(vint32_t number)
{
	return (vint32_t)streamWriteFile.Write(&number, sizeof(number));
}

vint32_t NamedPipeShared::WriteStringToStream(const WString& str)
{
	vint32_t bytes = 0;
	vint32_t count = (vint32_t)str.Length();
	bytes += (vint32_t)streamWriteFile.Write(&count, sizeof(count));
	if (count > 0)
	{
		bytes += (vint32_t)streamWriteFile.Write((void*)str.Buffer(), sizeof(wchar_t) * str.Length());
	}
	return bytes;
}

void NamedPipeShared::BeginSendStream()
{
	vint32_t bytes = 0;
	streamWriteFile.SeekFromBegin(0);
	streamWriteFile.Write(&bytes, sizeof(bytes));
}

void NamedPipeShared::EndSendStream(vint32_t bytes)
{
	streamWriteFile.SeekFromBegin(0);
	WriteInt32ToStream(bytes);

	vint32_t length = bytes + sizeof(bytes);
	for (vint i = 0; i < (length + MaxMessageSize - 1) / MaxMessageSize; i++)
	{
		vint offset = i * MaxMessageSize;
		vint bytesToSend = length >= (i + 1) * MaxMessageSize ? MaxMessageSize : length % MaxMessageSize;
		auto buffer = (LPCVOID)((char*)streamWriteFile.GetInternalBuffer() + offset);

		WaitForSingleObject(hEventWriteFile, INFINITE);
		ResetEvent(hEventWriteFile);
		ZeroMemory(&overlappedWriteFile, sizeof(overlappedWriteFile));
		overlappedWriteFile.hEvent = hEventWriteFile;
		WriteFile(hPipe, buffer, (DWORD)bytesToSend, NULL, &overlappedWriteFile);
	}
}

void NamedPipeShared::SendStringArray(vint count, List<WString>& strs)
{
	vint32_t bytes = 0;
	BeginSendStream();
	bytes += WriteInt32ToStream((vint32_t)count);
	for (vint i = 0; i < count; i++)
	{
		bytes += WriteStringToStream(strs[i]);
	}
	EndSendStream(bytes);
}

void NamedPipeShared::SendSingleString(const WString& str)
{
	vint32_t bytes = 0;
	BeginSendStream();
	bytes += WriteInt32ToStream((vint32_t)1);
	bytes += WriteStringToStream(str);
	EndSendStream(bytes);
}

/***********************************************************************
Helpers
***********************************************************************/

void NamedPipeShared::InstallCallback(INetworkProtocolCallback* _callback)
{
	callback = _callback;
}

NamedPipeShared::NamedPipeShared(HANDLE _hPipe)
	: hPipe(_hPipe)
{
	hEventReadFile = CreateEvent(NULL, TRUE, TRUE, NULL);
	CHECK_ERROR(hEventReadFile != NULL, L"NamedPipeCoreChannel initialization failed on CreateEvent(hEventReadFile).");
	hEventWriteFile = CreateEvent(NULL, TRUE, TRUE, NULL);
	CHECK_ERROR(hEventWriteFile != NULL, L"NamedPipeCoreChannel initialization failed on CreateEvent(hEventWriteFile).");
}

NamedPipeShared::~NamedPipeShared()
{
	CloseHandle(hEventReadFile);
	CloseHandle(hEventWriteFile);
}

/***********************************************************************
Server Helpers
***********************************************************************/

HANDLE NamedPipeShared::ServerCreatePipe()
{
	HANDLE hPipe = CreateNamedPipe(
		NamedPipeId,
		PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_REJECT_REMOTE_CLIENTS,
		1,
		65536,
		65536,
		6000,
		NULL);
	CHECK_ERROR(hPipe != INVALID_HANDLE_VALUE, L"CreateNamedPipe failed.");
	return hPipe;
}

void NamedPipeShared::ServerWaitForClient(HANDLE hPipe)
{
	OVERLAPPED overlapped;
	ZeroMemory(&overlapped, sizeof(overlapped));
	overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	CHECK_ERROR(overlapped.hEvent != NULL, L"ConnectNamedPipe failed on CreateEvent.");

	BOOL result = ConnectNamedPipe(hPipe, &overlapped);
	CHECK_ERROR(result == FALSE, L"ConnectNamedPipe failed.");
	DWORD error = GetLastError();
	switch (error)
	{
	case ERROR_IO_PENDING:
		WaitForSingleObject(overlapped.hEvent, INFINITE);
		break;
	default:
		CHECK_ERROR(error == ERROR_PIPE_CONNECTED, L"ConnectNamedPipe failed on unexpected GetLastError.");
	}

	CloseHandle(overlapped.hEvent);
}

/***********************************************************************
Client Helpers
***********************************************************************/

HANDLE NamedPipeShared::ClientCreatePipe()
{
	HANDLE hPipe = CreateFile(
		NamedPipeId,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		NULL);
	CHECK_ERROR(hPipe != INVALID_HANDLE_VALUE, L"CreateFile failed.");
	CHECK_ERROR(GetLastError() == 0, L"Another renderer already connected.");
	return hPipe;
}

void NamedPipeShared::ClientWaitForServer(HANDLE hPipe)
{
	DWORD dwPipeMode = PIPE_READMODE_MESSAGE;
	BOOL bSucceeded = SetNamedPipeHandleState(
		hPipe,
		&dwPipeMode,
		NULL,
		NULL);
	CHECK_ERROR(bSucceeded, L"SetNamedPipeHandleState failed.");
}