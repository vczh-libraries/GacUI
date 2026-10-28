#include "GuiRemoteRendererSingle.h"
#include "../Remote/GuiRemoteControllerInjection.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace filesystem;
	using namespace remoteprotocol;

/***********************************************************************
Renderer feature implementations use public APIs, including application
injections on the renderer. They never reach into an injection chain.
***********************************************************************/

	void GuiRemoteRendererSingle::RequestInjectionLocalToUtc(vint id, const InjectionDateTime& arguments)
	{
		CHECK_ERROR(globalConfig.injectDateTime, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestInjectionLocalToUtc()#Capability is disabled.");
		auto result = EncodeInjectionDateTime(DecodeInjectionDateTime(arguments).ToUtcTime());
		if (CanSendEvents()) events->RespondInjectionLocalToUtc(id, result);
	}

	void GuiRemoteRendererSingle::RequestInjectionUtcToLocal(vint id, const InjectionDateTime& arguments)
	{
		CHECK_ERROR(globalConfig.injectDateTime, L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestInjectionUtcToLocal()#Capability is disabled.");
		auto result = EncodeInjectionDateTime(DecodeInjectionDateTime(arguments).ToLocalTime());
		if (CanSendEvents()) events->RespondInjectionUtcToLocal(id, result);
	}

	void GuiRemoteRendererSingle::RequestInjectionLocale(vint id, const InjectionLocaleRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestInjectionLocale()#"
		CHECK_ERROR(globalConfig.injectLocale, ERROR_MESSAGE_PREFIX L"Capability is disabled.");
		auto result = Ptr(new List<WString>);
		Locale locale(arguments.localeName);
		switch (arguments.operation)
		{
		case InjectionLocaleOperation::Invariant:
			result->Add(Locale::Invariant().GetName());
			break;
		case InjectionLocaleOperation::SystemDefault:
			result->Add(Locale::SystemDefault().GetName());
			break;
		case InjectionLocaleOperation::UserDefault:
			result->Add(Locale::UserDefault().GetName());
			break;
		case InjectionLocaleOperation::Enumerate:
			{
				List<Locale> locales;
				Locale::Enumerate(locales);
				for (auto locale : locales) result->Add(locale.GetName());
			}
			break;
		case InjectionLocaleOperation::ShortDateFormats:
			locale.GetShortDateFormats(*result.Obj());
			break;
		case InjectionLocaleOperation::LongDateFormats:
			locale.GetLongDateFormats(*result.Obj());
			break;
		case InjectionLocaleOperation::YearMonthDateFormats:
			locale.GetYearMonthDateFormats(*result.Obj());
			break;
		case InjectionLocaleOperation::LongTimeFormats:
			locale.GetLongTimeFormats(*result.Obj());
			break;
		case InjectionLocaleOperation::ShortTimeFormats:
			locale.GetShortTimeFormats(*result.Obj());
			break;
		case InjectionLocaleOperation::ShortDayOfWeekName:
			result->Add(locale.GetShortDayOfWeekName(arguments.index));
			break;
		case InjectionLocaleOperation::LongDayOfWeekName:
			result->Add(locale.GetLongDayOfWeekName(arguments.index));
			break;
		case InjectionLocaleOperation::ShortMonthName:
			result->Add(locale.GetShortMonthName(arguments.index));
			break;
		case InjectionLocaleOperation::LongMonthName:
			result->Add(locale.GetLongMonthName(arguments.index));
			break;
		case InjectionLocaleOperation::ToLower:
			result->Add(locale.ToLower(arguments.text));
			break;
		case InjectionLocaleOperation::ToUpper:
			result->Add(locale.ToUpper(arguments.text));
			break;
		case InjectionLocaleOperation::ToLinguisticLower:
			result->Add(locale.ToLinguisticLower(arguments.text));
			break;
		case InjectionLocaleOperation::ToLinguisticUpper:
			result->Add(locale.ToLinguisticUpper(arguments.text));
			break;
		default:
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unknown operation.");
		}
		if (CanSendEvents()) events->RespondInjectionLocale(id, result);
#undef ERROR_MESSAGE_PREFIX
	}

	void GuiRemoteRendererSingle::RequestInjectionFileSystem(vint id, const InjectionFileRequest& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remote_renderer::GuiRemoteRendererSingle::RequestInjectionFileSystem()#"
		CHECK_ERROR(globalConfig.injectFileSystem, ERROR_MESSAGE_PREFIX L"Capability is disabled.");
		InjectionFileResponse result{};
		switch (arguments.operation)
		{
		case InjectionFileOperation::PathDelimiter:
			result.success = true;
			result.text = WString::FromChar(FilePath::GetPathDelimiter());
			break;
		case InjectionFileOperation::CompatibleDelimiters:
			result.success = true;
			result.text = FilePath::GetCompatibleDelimiters();
			break;
		case InjectionFileOperation::Initialize:
			result.success = true;
			result.text = FilePath(arguments.path).GetFullPath();
			break;
		case InjectionFileOperation::Concat:
			result.success = true;
			result.text = (FilePath(arguments.path) / arguments.target).GetFullPath();
			break;
		case InjectionFileOperation::Relative:
			result.success = true;
			result.text = FilePath(arguments.path).GetRelativePathFor(FilePath(arguments.target));
			break;
		case InjectionFileOperation::IsRoot:
			result.success = FilePath(arguments.path).IsRoot();
			break;
		case InjectionFileOperation::IsFile:
			result.success = FilePath(arguments.path).IsFile();
			break;
		case InjectionFileOperation::IsFolder:
			result.success = FilePath(arguments.path).IsFolder();
			break;
		case InjectionFileOperation::Info:
			{
				FilePath path(arguments.path);
				if (path.IsFile())
				{
					result.info = EncodeInjectionFileInfo(File(path).GetFileInfo());
					result.success = true;
				}
				else if (path.IsFolder())
				{
					result.info = EncodeInjectionFileInfo(Folder(path).GetFileInfo());
					result.success = true;
				}
			}
			break;
		case InjectionFileOperation::Copy:
			result.success = File(FilePath(arguments.path)).CopyToFile(FilePath(arguments.target), false);
			break;
		case InjectionFileOperation::DeleteFile:
			result.success = File(FilePath(arguments.path)).Delete();
			break;
		case InjectionFileOperation::RenameFile:
			result.success = File(FilePath(arguments.path)).Rename(arguments.target);
			break;
		case InjectionFileOperation::CreateFolder:
			result.success = Folder(FilePath(arguments.path)).Create(false);
			break;
		case InjectionFileOperation::DeleteFolder:
			result.success = Folder(FilePath(arguments.path)).Delete(false);
			break;
		case InjectionFileOperation::RenameFolder:
			result.success = Folder(FilePath(arguments.path)).Rename(arguments.target);
			break;
		case InjectionFileOperation::Folders:
			{
				List<Folder> entries;
				result.success = Folder(FilePath(arguments.path)).GetFolders(entries);
				result.paths = Ptr(new List<WString>);
				for (auto&& entry : entries) result.paths->Add(entry.GetFilePath().GetFullPath());
			}
			break;
		case InjectionFileOperation::Files:
			{
				List<File> entries;
				result.success = Folder(FilePath(arguments.path)).GetFiles(entries);
				result.paths = Ptr(new List<WString>);
				for (auto&& entry : entries) result.paths->Add(entry.GetFilePath().GetFullPath());
			}
			break;
		case InjectionFileOperation::OpenRead:
		case InjectionFileOperation::OpenWrite:
		case InjectionFileOperation::OpenReadWrite:
			{
				auto access = arguments.operation == InjectionFileOperation::OpenRead ? stream::FileStream::ReadOnly
					: arguments.operation == InjectionFileOperation::OpenWrite ? stream::FileStream::WriteOnly : stream::FileStream::ReadWrite;
				stream::FileStream file(arguments.path, access);
				if (!file.IsAvailable()) break;
				result.content = Ptr(new stream::MemoryStream);
				if (access == stream::FileStream::ReadOnly)
				{
					CopyStream(file, *result.content.Obj());
					result.success = result.content->Size() == file.Size();
				}
				else result.success = true;
				file.Close();
			}
			break;
		case InjectionFileOperation::Commit:
			{
				CHECK_ERROR(arguments.content, ERROR_MESSAGE_PREFIX L"Missing file content.");
				stream::FileStream file(arguments.path, stream::FileStream::WriteOnly);
				if (!file.IsAvailable()) break;
				arguments.content->SeekFromBegin(0);
				result.success = CopyStream(*arguments.content.Obj(), file) == arguments.content->Size();
				file.Close();
			}
			break;
		default:
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unknown operation.");
		}
		if (CanSendEvents()) events->RespondInjectionFileSystem(id, result);
#undef ERROR_MESSAGE_PREFIX
	}
}

