/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote feature injection
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLERINJECTION
#define VCZH_PRESENTATION_GUIREMOTECONTROLLERINJECTION

#include "GuiRemoteProtocol.h"

namespace vl::presentation
{
	class GuiRemoteController;

	/// <summary>Opt-in renderer implementations of process-wide features.</summary>
	struct GuiRemoteControllerConfig
	{
		bool									injectDateTime = false;
		bool									injectLocale = false;
		bool									injectFileSystem = false;
	};

	// Shared by the controller and its buffered streams. Requests execute on the
	// core thread; pending workers are released when its event loop stops.
	// Streams retain this state, never a dangling controller pointer.
	class GuiRemoteControllerInjectionState : public Object
	{
		friend class GuiRemoteController;
		struct Invocation : Object
		{
			Func<bool(GuiRemoteController*)>		proc;
			EventObject							completed;
			bool								result = false;
			Invocation();
		};

		GuiRemoteController*					remote;
		vint									mainThreadId;
		// covers remote, acceptingWorkers and pending
		SpinLock								lockPending;
		bool									acceptingWorkers = false;
		collections::List<Ptr<Invocation>>		pending;

		void									Stop();
	public:
		GuiRemoteControllerInjectionState(GuiRemoteController* _remote);
		bool									Execute(const Func<bool(GuiRemoteController*)>& proc);
	};

	// Only timezone conversions are remote. LocalTime uses the predecessor UTC
	// clock and the selected conversion. Construction, arithmetic and fallback
	// always use Previous(), preserving application injections.
	class GuiRemoteDateTimeInjection : public feature_injection::FeatureImpl<IDateTimeImpl>
	{
		GuiRemoteController*					remote;
		vuint64_t								Convert(vuint64_t value, bool toLocal, bool localTime = false);
	public:
		GuiRemoteDateTimeInjection(GuiRemoteController* _remote);
		DateTime								FromDateTime(vint year, vint month, vint day, vint hour, vint minute, vint second, vint milliseconds) override;
		DateTime								FromOSInternal(vuint64_t value) override;
		vuint64_t								LocalTime() override;
		vuint64_t								UtcTime() override;
		vuint64_t								LocalToUtcTime(vuint64_t value) override;
		vuint64_t								UtcToLocalTime(vuint64_t value) override;
		vuint64_t								Forward(vuint64_t value, vuint64_t milliseconds) override;
		vuint64_t								Backward(vuint64_t value, vuint64_t milliseconds) override;
	};

	// Defaults, enumeration, format lists, names and case mapping are remote.
	// Formatting, comparisons and searches stay local through Previous().
	// An unavailable renderer also delegates all operations to Previous().
	class GuiRemoteLocaleInjection : public feature_injection::FeatureImpl<ILocaleImpl>
	{
		GuiRemoteController*					remote;
		ILocaleImpl*							Predecessor() const;
		Ptr<collections::List<WString>>			Query(remoteprotocol::InjectionLocaleOperation operation, const WString& localeName = L"", const WString& text = L"", vint index = 0) const;
	public:
		GuiRemoteLocaleInjection(GuiRemoteController* _remote);
		Locale							Invariant() const override;
		Locale							SystemDefault() const override;
		Locale							UserDefault() const override;
		void							Enumerate(collections::List<Locale>& locales) const override;
		void							GetShortDateFormats(const WString& localeName, collections::List<WString>& formats) const override;
		void							GetLongDateFormats(const WString& localeName, collections::List<WString>& formats) const override;
		void							GetYearMonthDateFormats(const WString& localeName, collections::List<WString>& formats) const override;
		void							GetLongTimeFormats(const WString& localeName, collections::List<WString>& formats) const override;
		void							GetShortTimeFormats(const WString& localeName, collections::List<WString>& formats) const override;
		WString							FormatDate(const WString& localeName, const WString& format, DateTime date) const override;
		WString							FormatTime(const WString& localeName, const WString& format, DateTime time) const override;
		WString							FormatNumber(const WString& localeName, const WString& number) const override;
		WString							FormatCurrency(const WString& localeName, const WString& currency) const override;
		WString							GetShortDayOfWeekName(const WString& localeName, vint dayOfWeek) const override;
		WString							GetLongDayOfWeekName(const WString& localeName, vint dayOfWeek) const override;
		WString							GetShortMonthName(const WString& localeName, vint month) const override;
		WString							GetLongMonthName(const WString& localeName, vint month) const override;
		WString							ToLower(const WString& localeName, const WString& str) const override;
		WString							ToUpper(const WString& localeName, const WString& str) const override;
		WString							ToLinguisticLower(const WString& localeName, const WString& str) const override;
		WString							ToLinguisticUpper(const WString& localeName, const WString& str) const override;
		vint							Compare(const WString& localeName, const WString& s1, const WString& s2, Locale::Normalization normalization) const override;
		vint							CompareOrdinal(const WString& s1, const WString& s2) const override;
		vint							CompareOrdinalIgnoreCase(const WString& s1, const WString& s2) const override;
		collections::Pair<vint, vint>	FindFirst(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const override;
		collections::Pair<vint, vint>	FindLast(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const override;
		bool							StartsWith(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const override;
		bool							EndsWith(const WString& localeName, const WString& text, const WString& find, Locale::Normalization normalization) const override;
	};

	// All filesystem operations use renderer path rules and storage. Absence
	// supplies an empty '/' root, lexical paths and normal unavailable results;
	// it never delegates to the predecessor or accesses core storage.
	class GuiRemoteFileSystemInjection : public feature_injection::FeatureImpl<filesystem::IFileSystemImpl>
	{
		GuiRemoteController*					remote;
		mutable SpinLock						lockDelimiters;
		mutable collections::SortedList<WString>	delimiters;
		remoteprotocol::InjectionFileResponse	Query(remoteprotocol::InjectionFileOperation operation, const WString& path = L"", const WString& target = L"") const;
	public:
		GuiRemoteFileSystemInjection(GuiRemoteController* _remote);
		wchar_t												GetPathDelimiter() const override;
		const wchar_t*										GetCompatibleDelimiters() const override;
		WString												ConcatPath(const WString& fullPath, const WString& relativePath) const override;
		void												Initialize(WString& fullPath) const override;
		bool												IsFile(const WString& fullPath) const override;
		bool												IsFolder(const WString& fullPath) const override;
		bool												IsRoot(const WString& fullPath) const override;
		WString												GetRelativePathFor(const WString& fromPath, const WString& toPath) const override;
		filesystem::FileInfo								GetFileInfo(const filesystem::FilePath& path) const override;
		bool												FileCopy(const filesystem::FilePath& source, const filesystem::FilePath& destination) const override;
		bool												FileDelete(const filesystem::FilePath& filePath) const override;
		bool												FileRename(const filesystem::FilePath& filePath, const WString& newName) const override;
		bool												GetFolders(const filesystem::FilePath& folderPath, collections::List<filesystem::Folder>& folders) const override;
		bool												GetFiles(const filesystem::FilePath& folderPath, collections::List<filesystem::File>& files) const override;
		bool												CreateFolder(const filesystem::FilePath& folderPath) const override;
		bool												DeleteFolder(const filesystem::FilePath& folderPath) const override;
		bool												FolderRename(const filesystem::FilePath& folderPath, const WString& newName) const override;
		Ptr<stream::IFileStreamImpl>						GetFileStreamImpl(const WString& fileName, stream::FileStream::AccessRight accessRight) const override;
	};

	// ReadOnly snapshots the complete file; native WriteOnly and ReadWrite
	// validate an actual native open (including native creation/truncation) and
	// start empty. New buffered bytes become visible after successful Close;
	// existing readers retain their snapshot. Close uploads once, including
	// zero bytes. Every open stream, even ReadOnly, must close on its original
	// live connection; disconnect/replacement/destruction of Core is an error.
	class GuiRemoteFileStreamInjection : public Object, public stream::IFileStreamImpl
	{
		Ptr<GuiRemoteControllerInjectionState>	state;
		WString									fileName;
		stream::FileStream::AccessRight			access;
		Ptr<stream::MemoryStream>				buffer;
		vuint64_t								connection = 0;
		bool									opened = false;
	public:
		GuiRemoteFileStreamInjection(Ptr<GuiRemoteControllerInjectionState> _state, const WString& _fileName, stream::FileStream::AccessRight _access);
		~GuiRemoteFileStreamInjection();
		bool									Open() override;
		void									Close() override;
		pos_t							Position() const override;
		pos_t							Size() const override;
		void									Seek(pos_t size) override;
		void									SeekFromBegin(pos_t size) override;
		void									SeekFromEnd(pos_t size) override;
		vint									Read(void* data, vint size) override;
		vint									Write(void* data, vint size) override;
		vint									Peek(void* data, vint size) override;
	};

	extern remoteprotocol::InjectionDateTime		EncodeInjectionDateTime(DateTime value);
	extern DateTime								DecodeInjectionDateTime(const remoteprotocol::InjectionDateTime& value);
	extern remoteprotocol::InjectionFileInfo		EncodeInjectionFileInfo(const filesystem::FileInfo& value);
	extern filesystem::FileInfo					DecodeInjectionFileInfo(const remoteprotocol::InjectionFileInfo& value);
}

#endif

