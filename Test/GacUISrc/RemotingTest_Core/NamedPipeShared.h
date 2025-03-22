#include <VlppOS.h>
#include <Windows.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;

constexpr const wchar_t* NamedPipeId = L"\\\\.\\pipe\\GacUIRemoteProtocol";

class NamedPipeShared : public Object
{
protected:
	HANDLE											hPipe = INVALID_HANDLE_VALUE;

private:
	BYTE											bufferReadFile[65536];
	stream::MemoryStream							streamReadFile;
	HANDLE											hWaitHandleReadFile = INVALID_HANDLE_VALUE;
	OVERLAPPED										overlappedReadFile;
	HANDLE											hEventReadFile = INVALID_HANDLE_VALUE;

	stream::MemoryStream							streamWriteFile;
	OVERLAPPED										overlappedWriteFile;
	HANDLE											hEventWriteFile = INVALID_HANDLE_VALUE;

protected:

	virtual void OnReadStringThreadUnsafe(const WString& str) = 0;

private:

	void BeginReadingUnsafe()
	{
		streamReadFile.SeekFromBegin(0);
	}

	void SubmitReadBufferUnsafe(vint bytes)
	{
		streamReadFile.Write(bufferReadFile, bytes);
	}

	void EndReadingUnsafe()
	{
		vint32_t position = (vint32_t)streamReadFile.Position();
		streamReadFile.SeekFromBegin(0);

		vint32_t bytes = 0;
		vint consumed = 0;
		consumed = streamReadFile.Read(&bytes, sizeof(bytes));
		CHECK_ERROR(consumed == sizeof(bytes), L"ReadFile failed on incomplete message.");
		CHECK_ERROR(bytes == position - sizeof(bytes), L"ReadFile failed on incomplete message.");

		Array<wchar_t> strBuffer;
		while (streamReadFile.Position() < position)
		{
			vint32_t count = 0;
			consumed = streamReadFile.Read(&count, sizeof(count));
			CHECK_ERROR(consumed == sizeof(count) && streamReadFile.Position() <= position, L"ReadFile failed on incomplete message.");

			if (count == 0)
			{
				OnReadStringThreadUnsafe(WString::Empty);
			}
			else
			{
				strBuffer.Resize(count);
				consumed = streamReadFile.Read(&strBuffer[0], count * sizeof(wchar_t));
				CHECK_ERROR(consumed == count * sizeof(wchar_t) && streamReadFile.Position() <= position, L"ReadFile failed on incomplete message.");
				OnReadStringThreadUnsafe(WString::CopyFrom(&strBuffer[0], count));
			}
		}
	}

protected:

	void BeginReadingLoopUnsafe()
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
			if (error == ERROR_MORE_DATA)
			{
				SubmitReadBufferUnsafe((vint)read);
				goto RESTART_LOOP;
			}

			CHECK_ERROR(error == ERROR_IO_PENDING, L"ReadFile failed on unexpected GetLastError.");

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
						CHECK_ERROR(error == ERROR_IO_PENDING, L"GetOverlappedResult(ReadFile) failed on unexpected GetLastError.");
						self->SubmitReadBufferUnsafe((vint)read);
					}
					self->BeginReadingLoopUnsafe();
				},
				this,
				INFINITE,
				WT_EXECUTEONLYONCE);
		}
	}

private:

	vint32_t WriteInt32ToStream(vint32_t number)
	{
		return (vint32_t)streamWriteFile.Write(&number, sizeof(number));
	}

	vint32_t WriteStringToStream(const WString& str)
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

	void BeginSendStream()
	{
		vint32_t bytes = 0;
		streamWriteFile.SeekFromBegin(0);
		streamWriteFile.Write(&bytes, sizeof(bytes));
	}

	void EndSendStream(vint32_t bytes)
	{
		streamWriteFile.SeekFromBegin(0);
		WriteInt32ToStream(bytes);

		WaitForSingleObject(hEventWriteFile, INFINITE);
		ResetEvent(hEventWriteFile);
		ZeroMemory(&overlappedWriteFile, sizeof(overlappedWriteFile));
		overlappedWriteFile.hEvent = hEventWriteFile;
		WriteFile(hPipe, streamWriteFile.GetInternalBuffer(), (DWORD)(bytes + sizeof(bytes)), NULL, &overlappedWriteFile);
	}

protected:

	void SendStringArray(vint count, List<WString>& strs)
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

	void SendSingleString(const WString& str)
	{
		vint32_t bytes = 0;
		BeginSendStream();
		bytes += WriteInt32ToStream((vint32_t)1);
		bytes += WriteStringToStream(str);
		EndSendStream(bytes);
	}

public:

	NamedPipeShared(HANDLE _hPipe)
		: hPipe(_hPipe)
	{
		hEventReadFile = CreateEvent(NULL, TRUE, TRUE, NULL);
		CHECK_ERROR(hEventReadFile != NULL, L"NamedPipeCoreChannel initialization failed on CreateEvent(hEventReadFile).");
		hEventWriteFile = CreateEvent(NULL, TRUE, TRUE, NULL);
		CHECK_ERROR(hEventWriteFile != NULL, L"NamedPipeCoreChannel initialization failed on CreateEvent(hEventWriteFile).");
	}

	~NamedPipeShared()
	{
		CloseHandle(hEventReadFile);
		CloseHandle(hEventWriteFile);
	}
};