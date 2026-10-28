#include "GuiRemoteController.h"

namespace vl::presentation
{
	using namespace collections;
	using namespace filesystem;
	using namespace remoteprotocol;

/***********************************************************************
Portable values
***********************************************************************/

	InjectionDateTime EncodeInjectionDateTime(DateTime value)
	{
		return { value.year, value.month, value.day, value.hour, value.minute, value.second, value.milliseconds };
	}

	DateTime DecodeInjectionDateTime(const InjectionDateTime& value)
	{
		return DateTime::FromDateTime(value.year, value.month, value.day, value.hour, value.minute, value.second, value.milliseconds);
	}

	InjectionFileInfo EncodeInjectionFileInfo(const FileInfo& value)
	{
		InjectionFileInfo result{};
		result.size = u64tow(value.size);
		result.hardLinkCount = u64tow(value.hardLinkCount);
		if (value.creationTime) result.creationTime = EncodeInjectionDateTime(value.creationTime.Value());
		if (value.lastAccessTime) result.lastAccessTime = EncodeInjectionDateTime(value.lastAccessTime.Value());
		if (value.lastModifiedTime) result.lastModifiedTime = EncodeInjectionDateTime(value.lastModifiedTime.Value());
		if (value.lastChangeTime) result.lastChangeTime = EncodeInjectionDateTime(value.lastChangeTime.Value());
		result.isDirectory = value.isDirectory;
		result.isSymbolicLink = value.isSymbolicLink;
		result.isReparsePoint = value.isReparsePoint;
		result.canRead = value.canRead;
		result.canWrite = value.canWrite;
		result.canExecute = value.canExecute;
		result.isReadOnly = value.isReadOnly;
		result.isHidden = value.isHidden;
		result.isSystem = value.isSystem;
		result.isArchive = value.isArchive;
		result.isCompressed = value.isCompressed;
		result.isEncrypted = value.isEncrypted;
		result.isSparse = value.isSparse;
		result.isTemporary = value.isTemporary;
		result.isOffline = value.isOffline;
		result.isNotContentIndexed = value.isNotContentIndexed;
		result.isImmutable = value.isImmutable;
		result.isAppendOnly = value.isAppendOnly;
		return result;
	}

	FileInfo DecodeInjectionFileInfo(const InjectionFileInfo& value)
	{
		FileInfo result;
		bool valid = false;
		result.size = wtou64_test(value.size, valid);
		CHECK_ERROR(valid, L"vl::presentation::DecodeInjectionFileInfo()#Invalid file size.");
		result.hardLinkCount = wtou64_test(value.hardLinkCount, valid);
		CHECK_ERROR(valid, L"vl::presentation::DecodeInjectionFileInfo()#Invalid link count.");
		if (value.creationTime) result.creationTime = DecodeInjectionDateTime(value.creationTime.Value());
		if (value.lastAccessTime) result.lastAccessTime = DecodeInjectionDateTime(value.lastAccessTime.Value());
		if (value.lastModifiedTime) result.lastModifiedTime = DecodeInjectionDateTime(value.lastModifiedTime.Value());
		if (value.lastChangeTime) result.lastChangeTime = DecodeInjectionDateTime(value.lastChangeTime.Value());
		result.isDirectory = value.isDirectory;
		result.isSymbolicLink = value.isSymbolicLink;
		result.isReparsePoint = value.isReparsePoint;
		result.canRead = value.canRead;
		result.canWrite = value.canWrite;
		result.canExecute = value.canExecute;
		result.isReadOnly = value.isReadOnly;
		result.isHidden = value.isHidden;
		result.isSystem = value.isSystem;
		result.isArchive = value.isArchive;
		result.isCompressed = value.isCompressed;
		result.isEncrypted = value.isEncrypted;
		result.isSparse = value.isSparse;
		result.isTemporary = value.isTemporary;
		result.isOffline = value.isOffline;
		result.isNotContentIndexed = value.isNotContentIndexed;
		result.isImmutable = value.isImmutable;
		result.isAppendOnly = value.isAppendOnly;
		return result;
	}

/***********************************************************************
GuiRemoteControllerInjectionState
***********************************************************************/

	GuiRemoteControllerInjectionState::Invocation::Invocation()
	{
		CHECK_ERROR(completed.CreateManualUnsignal(false), L"vl::presentation::GuiRemoteControllerInjectionState::Invocation()#Failed to create completion event.");
	}

	GuiRemoteControllerInjectionState::GuiRemoteControllerInjectionState(GuiRemoteController* _remote)
		: remote(_remote)
		, mainThreadId(Thread::GetCurrentThreadId())
	{
	}

	bool GuiRemoteControllerInjectionState::Execute(const Func<bool(GuiRemoteController*)>& proc)
	{
		if (Thread::GetCurrentThreadId() == mainThreadId)
		{
			return remote && proc(remote);
		}

		auto invocation = Ptr(new Invocation);
		invocation->proc = proc;
		SPIN_LOCK(lockPending)
		{
			if (!remote || !acceptingWorkers) return false;
			pending.Add(invocation);
			auto self = remote->injectionState;
			remote->asyncService.InvokeInMainThread(nullptr, [self, invocation]()
			{
				bool execute = false;
				SPIN_LOCK(self->lockPending)
				{
					execute = self->pending.Remove(invocation.Obj());
				}
				if (!execute) return;
				invocation->result = invocation->proc(self->remote);
				invocation->proc = {};
				invocation->completed.Signal();
			});
		}
		invocation->completed.Wait();
		return invocation->result;
	}

	void GuiRemoteControllerInjectionState::Stop()
	{
		List<Ptr<Invocation>> canceled;
		SPIN_LOCK(lockPending)
		{
			acceptingWorkers = false;
			remote = nullptr;
			canceled = std::move(pending);
		}
		for (auto invocation : canceled)
		{
			invocation->proc = {};
			invocation->completed.Signal();
		}
	}

/***********************************************************************
GuiRemoteDateTimeInjection
***********************************************************************/

	GuiRemoteDateTimeInjection::GuiRemoteDateTimeInjection(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	vuint64_t GuiRemoteDateTimeInjection::Convert(vuint64_t value, bool toLocal, bool localTime)
	{
		auto input = EncodeInjectionDateTime(Previous()->FromOSInternal(value));
		InjectionDateTime output{};
		auto succeeded = remote->injectionState->Execute([&](GuiRemoteController* controller)
		{
			if (!controller->controllerConnected || !controller->remoteGlobalConfig.injectDateTime) return false;
			auto connection = controller->connectionGeneration;
			auto& messages = controller->remoteMessages;
			auto id = toLocal ? messages.RequestInjectionUtcToLocal(input) : messages.RequestInjectionLocalToUtc(input);
			bool disconnected = false;
			messages.Submit(disconnected);
			if (disconnected || connection != controller->connectionGeneration) return false;
			output = toLocal ? messages.RetrieveInjectionUtcToLocal(id) : messages.RetrieveInjectionLocalToUtc(id);
			return true;
		});
		if (!succeeded)
		{
			if (localTime) return Previous()->LocalTime();
			return toLocal ? Previous()->UtcToLocalTime(value) : Previous()->LocalToUtcTime(value);
		}
		return Previous()->FromDateTime(output.year, output.month, output.day, output.hour, output.minute, output.second, output.milliseconds).osInternal;
	}

	DateTime GuiRemoteDateTimeInjection::FromDateTime(vint year, vint month, vint day, vint hour, vint minute, vint second, vint milliseconds)
	{
		return Previous()->FromDateTime(year, month, day, hour, minute, second, milliseconds);
	}

	DateTime GuiRemoteDateTimeInjection::FromOSInternal(vuint64_t value)
	{
		return Previous()->FromOSInternal(value);
	}

	vuint64_t GuiRemoteDateTimeInjection::LocalTime()
	{
		return Convert(Previous()->UtcTime(), true, true);
	}

	vuint64_t GuiRemoteDateTimeInjection::UtcTime()
	{
		return Previous()->UtcTime();
	}

	vuint64_t GuiRemoteDateTimeInjection::LocalToUtcTime(vuint64_t value)
	{
		return Convert(value, false);
	}

	vuint64_t GuiRemoteDateTimeInjection::UtcToLocalTime(vuint64_t value)
	{
		return Convert(value, true);
	}

	vuint64_t GuiRemoteDateTimeInjection::Forward(vuint64_t value, vuint64_t milliseconds)
	{
		return Previous()->Forward(value, milliseconds);
	}

	vuint64_t GuiRemoteDateTimeInjection::Backward(vuint64_t value, vuint64_t milliseconds)
	{
		return Previous()->Backward(value, milliseconds);
	}

/***********************************************************************
GuiRemoteLocaleInjection
***********************************************************************/

	GuiRemoteLocaleInjection::GuiRemoteLocaleInjection(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	ILocaleImpl* GuiRemoteLocaleInjection::Predecessor() const
	{
		return const_cast<GuiRemoteLocaleInjection*>(this)->Previous();
	}

	Ptr<List<WString>> GuiRemoteLocaleInjection::Query(InjectionLocaleOperation operation, const WString& localeName, const WString& text, vint index) const
	{
		Ptr<List<WString>> result;
		remote->injectionState->Execute([&](GuiRemoteController* controller)
		{
			if (!controller->controllerConnected || !controller->remoteGlobalConfig.injectLocale) return false;
			auto connection = controller->connectionGeneration;
			auto& messages = controller->remoteMessages;
			auto id = messages.RequestInjectionLocale({ operation, localeName, text, index });
			bool disconnected = false;
			messages.Submit(disconnected);
			if (disconnected || connection != controller->connectionGeneration) return false;
			result = messages.RetrieveInjectionLocale(id);
			CHECK_ERROR(result, L"vl::presentation::GuiRemoteLocaleInjection::Query()#Missing locale response.");
			return true;
		});
		return result;
	}

	Locale GuiRemoteLocaleInjection::Invariant() const
	{
		auto result = Query(InjectionLocaleOperation::Invariant);
		return result ? Locale(result->Get(0)) : Predecessor()->Invariant();
	}

	Locale GuiRemoteLocaleInjection::SystemDefault() const
	{
		auto result = Query(InjectionLocaleOperation::SystemDefault);
		return result ? Locale(result->Get(0)) : Predecessor()->SystemDefault();
	}

	Locale GuiRemoteLocaleInjection::UserDefault() const
	{
		auto result = Query(InjectionLocaleOperation::UserDefault);
		return result ? Locale(result->Get(0)) : Predecessor()->UserDefault();
	}

	void GuiRemoteLocaleInjection::Enumerate(List<Locale>& locales) const
	{
		if (auto result = Query(InjectionLocaleOperation::Enumerate))
		{
			for (auto name : *result.Obj()) locales.Add(Locale(name));
		}
		else
		{
			Predecessor()->Enumerate(locales);
		}
	}

	void GuiRemoteLocaleInjection::GetShortDateFormats(const WString& localeName, List<WString>& formats) const
	{
		if (auto result = Query(InjectionLocaleOperation::ShortDateFormats, localeName))
		{
			CopyFrom(formats, *result.Obj(), true);
		}
		else
		{
			Predecessor()->GetShortDateFormats(localeName, formats);
		}
	}

	void GuiRemoteLocaleInjection::GetLongDateFormats(const WString& localeName, List<WString>& formats) const
	{
		if (auto result = Query(InjectionLocaleOperation::LongDateFormats, localeName))
		{
			CopyFrom(formats, *result.Obj(), true);
		}
		else
		{
			Predecessor()->GetLongDateFormats(localeName, formats);
		}
	}

	void GuiRemoteLocaleInjection::GetYearMonthDateFormats(const WString& localeName, List<WString>& formats) const
	{
		if (auto result = Query(InjectionLocaleOperation::YearMonthDateFormats, localeName))
		{
			CopyFrom(formats, *result.Obj(), true);
		}
		else
		{
			Predecessor()->GetYearMonthDateFormats(localeName, formats);
		}
	}

	void GuiRemoteLocaleInjection::GetLongTimeFormats(const WString& localeName, List<WString>& formats) const
	{
		if (auto result = Query(InjectionLocaleOperation::LongTimeFormats, localeName))
		{
			CopyFrom(formats, *result.Obj(), true);
		}
		else
		{
			Predecessor()->GetLongTimeFormats(localeName, formats);
		}
	}

	void GuiRemoteLocaleInjection::GetShortTimeFormats(const WString& localeName, List<WString>& formats) const
	{
		if (auto result = Query(InjectionLocaleOperation::ShortTimeFormats, localeName))
		{
			CopyFrom(formats, *result.Obj(), true);
		}
		else
		{
			Predecessor()->GetShortTimeFormats(localeName, formats);
		}
	}

	WString GuiRemoteLocaleInjection::GetShortDayOfWeekName(const WString& localeName, vint index) const
	{
		auto result = Query(InjectionLocaleOperation::ShortDayOfWeekName, localeName, L"", index);
		return result ? result->Get(0) : Predecessor()->GetShortDayOfWeekName(localeName, index);
	}

	WString GuiRemoteLocaleInjection::GetLongDayOfWeekName(const WString& localeName, vint index) const
	{
		auto result = Query(InjectionLocaleOperation::LongDayOfWeekName, localeName, L"", index);
		return result ? result->Get(0) : Predecessor()->GetLongDayOfWeekName(localeName, index);
	}

	WString GuiRemoteLocaleInjection::GetShortMonthName(const WString& localeName, vint index) const
	{
		auto result = Query(InjectionLocaleOperation::ShortMonthName, localeName, L"", index);
		return result ? result->Get(0) : Predecessor()->GetShortMonthName(localeName, index);
	}

	WString GuiRemoteLocaleInjection::GetLongMonthName(const WString& localeName, vint index) const
	{
		auto result = Query(InjectionLocaleOperation::LongMonthName, localeName, L"", index);
		return result ? result->Get(0) : Predecessor()->GetLongMonthName(localeName, index);
	}

	WString GuiRemoteLocaleInjection::ToLower(const WString& localeName, const WString& str) const
	{
		auto result = Query(InjectionLocaleOperation::ToLower, localeName, str);
		return result ? result->Get(0) : Predecessor()->ToLower(localeName, str);
	}

	WString GuiRemoteLocaleInjection::ToUpper(const WString& localeName, const WString& str) const
	{
		auto result = Query(InjectionLocaleOperation::ToUpper, localeName, str);
		return result ? result->Get(0) : Predecessor()->ToUpper(localeName, str);
	}

	WString GuiRemoteLocaleInjection::ToLinguisticLower(const WString& localeName, const WString& str) const
	{
		auto result = Query(InjectionLocaleOperation::ToLinguisticLower, localeName, str);
		return result ? result->Get(0) : Predecessor()->ToLinguisticLower(localeName, str);
	}

	WString GuiRemoteLocaleInjection::ToLinguisticUpper(const WString& localeName, const WString& str) const
	{
		auto result = Query(InjectionLocaleOperation::ToLinguisticUpper, localeName, str);
		return result ? result->Get(0) : Predecessor()->ToLinguisticUpper(localeName, str);
	}

	WString GuiRemoteLocaleInjection::FormatDate(const WString& localeName, const WString& format, DateTime date) const
	{
		return Predecessor()->FormatDate(localeName, format, date);
	}

	WString GuiRemoteLocaleInjection::FormatTime(const WString& localeName, const WString& format, DateTime time) const
	{
		return Predecessor()->FormatTime(localeName, format, time);
	}

	WString GuiRemoteLocaleInjection::FormatNumber(const WString& localeName, const WString& number) const
	{
		return Predecessor()->FormatNumber(localeName, number);
	}

	WString GuiRemoteLocaleInjection::FormatCurrency(const WString& localeName, const WString& currency) const
	{
		return Predecessor()->FormatCurrency(localeName, currency);
	}

	vint GuiRemoteLocaleInjection::Compare(const WString& localeName, const WString& s1, const WString& s2, Locale::Normalization normalization) const
	{
		return Predecessor()->Compare(localeName, s1, s2, normalization);
	}

	vint GuiRemoteLocaleInjection::CompareOrdinal(const WString& s1, const WString& s2) const
	{
		return Predecessor()->CompareOrdinal(s1, s2);
	}

	vint GuiRemoteLocaleInjection::CompareOrdinalIgnoreCase(const WString& s1, const WString& s2) const
	{
		return Predecessor()->CompareOrdinalIgnoreCase(s1, s2);
	}

	collections::Pair<vint, vint> GuiRemoteLocaleInjection::FindFirst(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const
	{
		return Predecessor()->FindFirst(localeName, text, find, normalization);
	}

	collections::Pair<vint, vint> GuiRemoteLocaleInjection::FindLast(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const
	{
		return Predecessor()->FindLast(localeName, text, find, normalization);
	}

	bool GuiRemoteLocaleInjection::StartsWith(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const
	{
		return Predecessor()->StartsWith(localeName, text, find, normalization);
	}

	bool GuiRemoteLocaleInjection::EndsWith(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const
	{
		return Predecessor()->EndsWith(localeName, text, find, normalization);
	}

/***********************************************************************
Empty filesystem
***********************************************************************/

	WString NormalizeEmptyInjectionPath(const WString& path)
	{
		List<WString> components;
		vint start = 0;
		for (vint i = 0; i <= path.Length(); i++)
		{
			if (i != path.Length() && path[i] != L'/' && path[i] != L'\\') continue;
			auto part = path.Sub(start, i - start);
			start = i + 1;
			if (part.Length() == 0 || part == L".") continue;
			if (part == L"..")
			{
				if (components.Count() > 0) components.RemoveAt(components.Count() - 1);
			}
			else
			{
				components.Add(part);
			}
		}
		WString result;
		for (auto part : components) result += L"/" + part;
		return result.Length() == 0 ? WString::Unmanaged(L"/") : result;
	}

	InjectionFileResponse EmptyInjectionFileResponse(InjectionFileOperation operation, const WString& path, const WString& target)
	{
		InjectionFileResponse result{};
		switch (operation)
		{
		case InjectionFileOperation::PathDelimiter:
			result.success = true;
			result.text = L"/";
			break;
		case InjectionFileOperation::CompatibleDelimiters:
			result.success = true;
			result.text = L"\\";
			break;
		case InjectionFileOperation::Initialize:
			result.success = true;
			result.text = NormalizeEmptyInjectionPath(path);
			break;
		case InjectionFileOperation::Concat:
			result.success = true;
			result.text = NormalizeEmptyInjectionPath(target.Length() > 0 && (target[0] == L'/' || target[0] == L'\\') ? target : path + L"/" + target);
			break;
		case InjectionFileOperation::Relative:
			{
				auto from = NormalizeEmptyInjectionPath(path);
				auto to = NormalizeEmptyInjectionPath(target);
				vint common = 0;
				for (vint i = 0; i < from.Length() && i < to.Length() && from[i] == to[i]; i++)
				{
					if (from[i] == L'/') common = i + 1;
					if (i + 1 == from.Length() && (i + 1 == to.Length() || to[i + 1] == L'/')) common = i + 1;
				}
				result.success = true;
				if (from == to) { result.text = L"."; break; }
				auto remaining = from.Sub(common, from.Length() - common);
				for (vint i = 0; i < remaining.Length(); i++)
				{
					if (i == 0 ? remaining[i] != L'/' : remaining[i - 1] == L'/') result.text += L"../";
				}
				if (common < to.Length() && to[common] == L'/') common++;
				result.text += to.Sub(common, to.Length() - common);
			}
			break;
		case InjectionFileOperation::IsRoot:
		case InjectionFileOperation::IsFolder:
			result.success = path == L"/" || path.Length() == 0;
			break;
		case InjectionFileOperation::Folders:
		case InjectionFileOperation::Files:
			result.success = path == L"/" || path.Length() == 0;
			result.paths = Ptr(new List<WString>);
			break;
		default:
			break;
		}
		return result;
	}

/***********************************************************************
GuiRemoteFileSystemInjection
***********************************************************************/

	GuiRemoteFileSystemInjection::GuiRemoteFileSystemInjection(GuiRemoteController* _remote)
		: remote(_remote)
	{
	}

	InjectionFileResponse GuiRemoteFileSystemInjection::Query(InjectionFileOperation operation, const WString& path, const WString& target) const
	{
		InjectionFileResponse result{};
		auto succeeded = remote->injectionState->Execute([&](GuiRemoteController* controller)
		{
			if (!controller->controllerConnected || !controller->remoteGlobalConfig.injectFileSystem) return false;
			auto connection = controller->connectionGeneration;
			auto& messages = controller->remoteMessages;
			auto id = messages.RequestInjectionFileSystem({ operation, path, target, {} });
			bool disconnected = false;
			messages.Submit(disconnected);
			if (disconnected || connection != controller->connectionGeneration) return false;
			result = messages.RetrieveInjectionFileSystem(id);
			return true;
		});
		return succeeded ? result : EmptyInjectionFileResponse(operation, path, target);
	}

	wchar_t GuiRemoteFileSystemInjection::GetPathDelimiter() const
	{
		auto result = Query(InjectionFileOperation::PathDelimiter);
		CHECK_ERROR(result.success && result.text.Length() == 1, L"vl::presentation::GuiRemoteFileSystemInjection::GetPathDelimiter()#Invalid delimiter.");
		return result.text[0];
	}

	const wchar_t* GuiRemoteFileSystemInjection::GetCompatibleDelimiters() const
	{
		auto result = Query(InjectionFileOperation::CompatibleDelimiters);
		const wchar_t* value = nullptr;
		SPIN_LOCK(lockDelimiters)
		{
			auto index = delimiters.IndexOf(result.text);
			if (index == -1) index = delimiters.Add(result.text);
			value = delimiters[index].Buffer();
		}
		return value;
	}

	WString GuiRemoteFileSystemInjection::ConcatPath(const WString& fullPath, const WString& relativePath) const
	{
		return Query(InjectionFileOperation::Concat, fullPath, relativePath).text;
	}

	void GuiRemoteFileSystemInjection::Initialize(WString& fullPath) const
	{
		fullPath = Query(InjectionFileOperation::Initialize, fullPath).text;
	}

	WString GuiRemoteFileSystemInjection::GetRelativePathFor(const WString& fromPath, const WString& toPath) const
	{
		return Query(InjectionFileOperation::Relative, fromPath, toPath).text;
	}

	FileInfo GuiRemoteFileSystemInjection::GetFileInfo(const FilePath& path) const
	{
		auto result = Query(InjectionFileOperation::Info, path.GetFullPath());
		CHECK_ERROR(result.success && result.info, L"vl::presentation::GuiRemoteFileSystemInjection::GetFileInfo()#Entry is unavailable.");
		return DecodeInjectionFileInfo(result.info.Value());
	}

	bool GuiRemoteFileSystemInjection::IsFile(const WString& fullPath) const
	{
		return Query(InjectionFileOperation::IsFile, fullPath).success;
	}

	bool GuiRemoteFileSystemInjection::IsFolder(const WString& fullPath) const
	{
		return Query(InjectionFileOperation::IsFolder, fullPath).success;
	}

	bool GuiRemoteFileSystemInjection::IsRoot(const WString& fullPath) const
	{
		return Query(InjectionFileOperation::IsRoot, fullPath).success;
	}

	bool GuiRemoteFileSystemInjection::FileDelete(const FilePath& path) const
	{
		return Query(InjectionFileOperation::DeleteFile, path.GetFullPath()).success;
	}

	bool GuiRemoteFileSystemInjection::CreateFolder(const FilePath& path) const
	{
		return Query(InjectionFileOperation::CreateFolder, path.GetFullPath()).success;
	}

	bool GuiRemoteFileSystemInjection::DeleteFolder(const FilePath& path) const
	{
		return Query(InjectionFileOperation::DeleteFolder, path.GetFullPath()).success;
	}

	bool GuiRemoteFileSystemInjection::FileRename(const FilePath& path, const WString& name) const
	{
		return Query(InjectionFileOperation::RenameFile, path.GetFullPath(), name).success;
	}

	bool GuiRemoteFileSystemInjection::FolderRename(const FilePath& path, const WString& name) const
	{
		return Query(InjectionFileOperation::RenameFolder, path.GetFullPath(), name).success;
	}

	bool GuiRemoteFileSystemInjection::FileCopy(const FilePath& source, const FilePath& destination) const
	{
		return Query(InjectionFileOperation::Copy, source.GetFullPath(), destination.GetFullPath()).success;
	}

	bool GuiRemoteFileSystemInjection::GetFolders(const FilePath& path, List<Folder>& items) const
	{
		auto result = Query(InjectionFileOperation::Folders, path.GetFullPath());
		if (!result.success) return false;
		for (auto name : *result.paths.Obj()) items.Add(Folder(FilePath(name)));
		return true;
	}

	bool GuiRemoteFileSystemInjection::GetFiles(const FilePath& path, List<File>& items) const
	{
		auto result = Query(InjectionFileOperation::Files, path.GetFullPath());
		if (!result.success) return false;
		for (auto name : *result.paths.Obj()) items.Add(File(FilePath(name)));
		return true;
	}

	Ptr<stream::IFileStreamImpl> GuiRemoteFileSystemInjection::GetFileStreamImpl(const WString& fileName, stream::FileStream::AccessRight accessRight) const
	{
		return Ptr(new GuiRemoteFileStreamInjection(remote->injectionState, fileName, accessRight));
	}

/***********************************************************************
GuiRemoteFileStreamInjection
***********************************************************************/

	GuiRemoteFileStreamInjection::GuiRemoteFileStreamInjection(Ptr<GuiRemoteControllerInjectionState> _state, const WString& _fileName, stream::FileStream::AccessRight _access)
		: state(_state)
		, fileName(_fileName)
		, access(_access)
	{
	}

	GuiRemoteFileStreamInjection::~GuiRemoteFileStreamInjection()
	{
		Close();
	}

	bool GuiRemoteFileStreamInjection::Open()
	{
		CHECK_ERROR(!opened, L"vl::presentation::GuiRemoteFileStreamInjection::Open()#Stream is already open.");
		opened = state->Execute([&](GuiRemoteController* controller)
		{
			if (!controller->controllerConnected || !controller->remoteGlobalConfig.injectFileSystem) return false;
			connection = controller->connectionGeneration;
			auto operation = access == stream::FileStream::ReadOnly ? InjectionFileOperation::OpenRead
				: access == stream::FileStream::WriteOnly ? InjectionFileOperation::OpenWrite : InjectionFileOperation::OpenReadWrite;
			auto& messages = controller->remoteMessages;
			auto id = messages.RequestInjectionFileSystem({ operation, fileName, {}, {} });
			bool disconnected = false;
			messages.Submit(disconnected);
			if (disconnected || connection != controller->connectionGeneration) return false;
			auto result = messages.RetrieveInjectionFileSystem(id);
			if (!result.success) return false;
			CHECK_ERROR(result.content, L"vl::presentation::GuiRemoteFileStreamInjection::Open()#Missing file content.");
			buffer = result.content;
			buffer->SeekFromBegin(0);
			return true;
		});
		return opened;
	}

	void GuiRemoteFileStreamInjection::Close()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiRemoteFileStreamInjection::Close()#"
		if (!opened) return;
		opened = false;
		auto succeeded = state->Execute([&](GuiRemoteController* controller)
		{
			if (!controller->controllerConnected || !controller->remoteGlobalConfig.injectFileSystem || connection != controller->connectionGeneration) return false;
			if (access == stream::FileStream::ReadOnly) return true;
			auto& messages = controller->remoteMessages;
			auto id = messages.RequestInjectionFileSystem({ InjectionFileOperation::Commit, fileName, {}, buffer });
			bool disconnected = false;
			messages.Submit(disconnected);
			if (disconnected || connection != controller->connectionGeneration) return false;
			return messages.RetrieveInjectionFileSystem(id).success;
		});
		CHECK_ERROR(succeeded, ERROR_MESSAGE_PREFIX L"Original renderer is unavailable or file writeback failed.");
		buffer->Close();
#undef ERROR_MESSAGE_PREFIX
	}

	pos_t GuiRemoteFileStreamInjection::Position() const
	{
		return opened ? buffer->Position() : -1;
	}

	pos_t GuiRemoteFileStreamInjection::Size() const
	{
		return opened ? buffer->Size() : -1;
	}

	void GuiRemoteFileStreamInjection::Seek(pos_t size)
	{
		CHECK_ERROR(opened, L"vl::presentation::GuiRemoteFileStreamInjection::Seek()#Stream is closed.");
		buffer->Seek(size);
	}

	void GuiRemoteFileStreamInjection::SeekFromBegin(pos_t size)
	{
		CHECK_ERROR(opened, L"vl::presentation::GuiRemoteFileStreamInjection::SeekFromBegin()#Stream is closed.");
		buffer->SeekFromBegin(size);
	}

	void GuiRemoteFileStreamInjection::SeekFromEnd(pos_t size)
	{
		CHECK_ERROR(opened, L"vl::presentation::GuiRemoteFileStreamInjection::SeekFromEnd()#Stream is closed.");
		buffer->SeekFromEnd(size);
	}

	vint GuiRemoteFileStreamInjection::Read(void* data, vint size)
	{
		CHECK_ERROR(opened && access != stream::FileStream::WriteOnly, L"vl::presentation::GuiRemoteFileStreamInjection::Read()#Stream is closed or access is denied.");
		return buffer->Read(data, size);
	}

	vint GuiRemoteFileStreamInjection::Write(void* data, vint size)
	{
		CHECK_ERROR(opened && access != stream::FileStream::ReadOnly, L"vl::presentation::GuiRemoteFileStreamInjection::Write()#Stream is closed or access is denied.");
		return buffer->Write(data, size);
	}

	vint GuiRemoteFileStreamInjection::Peek(void* data, vint size)
	{
		CHECK_ERROR(opened && access != stream::FileStream::WriteOnly, L"vl::presentation::GuiRemoteFileStreamInjection::Peek()#Stream is closed or access is denied.");
		return buffer->Peek(data, size);
	}
}

