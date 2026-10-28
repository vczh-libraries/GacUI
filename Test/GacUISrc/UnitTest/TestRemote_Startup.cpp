#include "TestRemote.h"
#include "TestControls.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../Generated_FullControlTest/FullControlTestEventHandlers.h"

namespace remote_startup_tests
{
	class NotImplementedProtocolBase : public UnitTestRemoteProtocolBase
	{
	public:
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Request ## NAME()									override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id)							override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Request ## NAME(const REQUEST& arguments)			override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id, const REQUEST& arguments)	override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

		NotImplementedProtocolBase(UnitTestScreenConfig _globalConfig)
			: UnitTestRemoteProtocolBase(_globalConfig)
		{
		}
	};

	class StartUpProtocol : public NotImplementedProtocolBase
	{
	public:
		StartUpProtocol()
			: NotImplementedProtocolBase({})
		{
		}

		void Submit(bool& disconnected) override
		{
		}

		IGuiRemoteEventProcessor* GetRemoteEventProcessor() override
		{
			return nullptr;
		}

		WString GetExecutablePath() override
		{
			return L"/StartUp/Protocol.exe";
		}
	
		void RequestControllerGetFontConfig(vint id) override
		{
			FontConfig response;
			response.defaultFont.fontFamily = L"StartUpDefault";
			response.defaultFont.bold = true;
			response.supportedFonts = Ptr(new List<WString>());
			response.supportedFonts->Add(L"StartUpDefault");
			response.supportedFonts->Add(L"Another");
			response.supportedFonts->Add(L"YetAnother");
			events->RespondControllerGetFontConfig(id, response);
		}
	
		void RequestControllerGetScreenConfig(vint id) override
		{
			ScreenConfig response;
			response.bounds = { 0,0,30,20 };
			response.clientBounds = { 1,1,29,19 };
			response.scalingX = 1;
			response.scalingY = 1;
			events->RespondControllerGetScreenConfig(id, response);
		}
	
		void RequestControllerConnectionEstablished() override
		{
		}
	
		void RequestControllerConnectionStopped() override
		{
		}

		void RequestWindowNotifySetBounds(const NativeRect& arguments) override
		{
		}
	
		void RequestWindowGetBounds(vint id) override
		{
			WindowSizingConfig response;
			response.bounds = { 0,0,50,40 };
			response.clientBounds = { 1,1,49,39 };
			response.customFramePadding = { 8,8,8,8 };
			response.sizeState = INativeWindow::Restored;
			events->RespondWindowGetBounds(id, response);
		}

		void RequestIOUpdateGlobalShortcutKey(const Ptr<List<GlobalShortcutKey>>& arguments) override
		{
		}

		void RequestImageCreated(vint id, const ImageCreation& arguments) override
		{
			ImageMetadata metadata{};
			metadata.id = arguments.id;
			metadata.format = INativeImage::Jpeg;
			metadata.frames = Ptr(new List<ImageFrameMetadata>);
			metadata.frames->Add({ { 2, 3 } });
			events->RespondImageCreated(id, metadata);
		}

		void RequestImageSave(vint id, const ImageSaveRequest& arguments) override
		{
			TEST_ASSERT(arguments.format == INativeImage::Png);
			auto content = Ptr(new stream::MemoryStream);
			vuint8_t encoded[] = { 137, 80, 78, 71, 0, 255 };
			content->Write(encoded, sizeof(encoded));
			events->RespondImageSave(id, content);
		}

		void RequestImageDestroyed(const vint& arguments) override
		{
		}
	};

	class InjectionClock : public feature_injection::FeatureImpl<IDateTimeImpl>
	{
	public:
		DateTime FromDateTime(vint y, vint m, vint d, vint h, vint n, vint s, vint ms) override { return Previous()->FromDateTime(y, m, d, h, n, s, ms); }
		DateTime FromOSInternal(vuint64_t value) override { return Previous()->FromOSInternal(value); }
		vuint64_t UtcTime() override { return FromDateTime(2024, 12, 31, 23, 30, 42, 123).osInternal; }
		vuint64_t LocalTime() override { return Backward(UtcTime(), 3600000); }
		vuint64_t LocalToUtcTime(vuint64_t value) override { return Forward(value, 3600000); }
		vuint64_t UtcToLocalTime(vuint64_t value) override { return Backward(value, 3600000); }
		vuint64_t Forward(vuint64_t value, vuint64_t ms) override { return Previous()->Forward(value, ms); }
		vuint64_t Backward(vuint64_t value, vuint64_t ms) override { return Previous()->Backward(value, ms); }
	};

	class InjectionLocale : public EnUsLocaleImpl
	{
	public:
		Locale UserDefault() const override { return Locale(L"core-locale"); }
		WString FormatNumber(const WString&, const WString&) const override { return L"core-number"; }
	};

	class InjectionProtocol : public StartUpProtocol
	{
	public:
		vint dateRequests = 0;
		vint localeRequests = 0;
		vint fileRequests = 0;
		vint commits = 0;
		vint imageCreates = 0;
		vint imageSaves = 0;
		bool cancelNext = false;
		bool cancelPending = false;
		Dictionary<WString, Ptr<stream::MemoryStream>> files;
		List<WString> folders;

		InjectionProtocol()
		{
			folders.Add(L"/renderer");
			files.Add(L"/renderer/entry", Ptr(new stream::MemoryStream));
		}

		WString Parent(const WString& path)
		{
			for (vint i = path.Length() - 1; i >= 0; i--) if (path[i] == L'/') return path.Left(i);
			return L"";
		}

		bool Cancel()
		{
			if (!cancelNext) return false;
			cancelNext = false;
			cancelPending = true;
			return true;
		}

		void Submit(bool& disconnected) override
		{
			if (cancelPending)
			{
				cancelPending = false;
				disconnected = true;
				events->OnControllerDisconnect();
			}
		}

		void Convert(vint id, const InjectionDateTime& arguments, bool toLocal)
		{
			dateRequests++;
			if (Cancel()) return;
			// Deliberately different winter/summer offsets, never a renderer clock.
			auto value = DecodeInjectionDateTime(arguments);
			auto offset = arguments.month >= 4 && arguments.month <= 10 ? 3 * 3600000 : 2 * 3600000;
			auto result = EncodeInjectionDateTime(toLocal ? value.Forward(offset) : value.Backward(offset));
			if (toLocal) events->RespondInjectionUtcToLocal(id, result);
			else events->RespondInjectionLocalToUtc(id, result);
		}

		void RequestInjectionUtcToLocal(vint id, const InjectionDateTime& arguments) override { Convert(id, arguments, true); }
		void RequestInjectionLocalToUtc(vint id, const InjectionDateTime& arguments) override { Convert(id, arguments, false); }

		void RequestImageCreated(vint id, const ImageCreation& arguments) override
		{
			imageCreates++;
			if (!Cancel()) StartUpProtocol::RequestImageCreated(id, arguments);
		}

		void RequestImageSave(vint id, const ImageSaveRequest& arguments) override
		{
			imageSaves++;
			if (!Cancel()) StartUpProtocol::RequestImageSave(id, arguments);
		}

		void RequestInjectionLocale(vint id, const InjectionLocaleRequest& arguments) override
		{
			localeRequests++;
			if (Cancel()) return;
			auto result = Ptr(new List<WString>);
			result->Add(L"renderer-" + itow((vint)arguments.operation));
			if (arguments.operation == InjectionLocaleOperation::Enumerate) result->Add(L"renderer-second");
			events->RespondInjectionLocale(id, result);
		}

		void RequestInjectionFileSystem(vint id, const InjectionFileRequest& arguments) override
		{
			fileRequests++;
			if (Cancel()) return;
			InjectionFileResponse result{};
			result.success = true;
			switch (arguments.operation)
			{
			case InjectionFileOperation::PathDelimiter: result.text = L"/"; break;
			case InjectionFileOperation::CompatibleDelimiters: result.text = L""; break;
			case InjectionFileOperation::Initialize:
				result.text = arguments.path.Length() == 0 ? L"/renderer" : arguments.path;
				break;
			case InjectionFileOperation::Concat: result.text = arguments.path + L"/" + arguments.target; break;
			case InjectionFileOperation::Relative: result.text = L"renderer-relative"; break;
			case InjectionFileOperation::IsRoot: result.success = arguments.path == L"/renderer"; break;
			case InjectionFileOperation::IsFolder: result.success = folders.Contains(arguments.path); break;
			case InjectionFileOperation::IsFile: result.success = files.Keys().Contains(arguments.path); break;
			case InjectionFileOperation::Folders:
			case InjectionFileOperation::Files:
				result.paths = Ptr(new List<WString>);
				result.success = folders.Contains(arguments.path);
				if (arguments.operation == InjectionFileOperation::Folders)
				{
					for (auto path : folders) if (Parent(path) == arguments.path) result.paths->Add(path);
				}
				else
				{
					for (auto path : files.Keys()) if (Parent(path) == arguments.path) result.paths->Add(path);
				}
				break;
			case InjectionFileOperation::CreateFolder:
				result.success = folders.Contains(Parent(arguments.path)) && !folders.Contains(arguments.path);
				if (result.success) folders.Add(arguments.path);
				break;
			case InjectionFileOperation::DeleteFolder:
				result.success = folders.Remove(arguments.path);
				break;
			case InjectionFileOperation::RenameFolder:
				result.success = folders.Remove(arguments.path);
				if (result.success) folders.Add(Parent(arguments.path) + L"/" + arguments.target);
				break;
			case InjectionFileOperation::DeleteFile:
				result.success = files.Remove(arguments.path);
				break;
			case InjectionFileOperation::RenameFile:
			case InjectionFileOperation::Copy:
				result.success = files.Keys().Contains(arguments.path);
				if (result.success)
				{
					auto copy = Ptr(new stream::MemoryStream);
					files[arguments.path]->SeekFromBegin(0);
					CopyStream(*files[arguments.path].Obj(), *copy.Obj());
					files.Set(arguments.operation == InjectionFileOperation::Copy ? arguments.target : Parent(arguments.path) + L"/" + arguments.target, copy);
					if (arguments.operation == InjectionFileOperation::RenameFile) files.Remove(arguments.path);
				}
				break;
			case InjectionFileOperation::Info:
				{
					filesystem::FileInfo info;
					info.size = 0x100000000ULL + 7;
					info.hardLinkCount = 0x200000000ULL + 9;
					info.lastModifiedTime = DateTime::FromDateTime(2024, 2, 29, 12, 34, 56, 789);
					info.canRead = info.isSparse = info.isHidden = true;
					result.info = EncodeInjectionFileInfo(info);
				}
				break;
			case InjectionFileOperation::OpenRead:
				if (!files.Keys().Contains(arguments.path)) { result.success = false; break; }
				result.content = Ptr(new stream::MemoryStream);
				files[arguments.path]->SeekFromBegin(0);
				CopyStream(*files[arguments.path].Obj(), *result.content.Obj());
				break;
			case InjectionFileOperation::OpenWrite:
			case InjectionFileOperation::OpenReadWrite:
				if (arguments.path == L"/denied" || (Parent(arguments.path).Length() > 0 && !folders.Contains(Parent(arguments.path)))) { result.success = false; break; }
				result.content = Ptr(new stream::MemoryStream);
				break;
			case InjectionFileOperation::Commit:
				commits++;
				files.Set(arguments.path, arguments.content);
				break;
			default: CHECK_FAIL(L"Unexpected filesystem test operation.");
			}
			events->RespondInjectionFileSystem(id, result);
		}
	};

	class PersistenceProtocol : public InjectionProtocol
	{
	public:
		List<INativeImage::FormatType> savedFormats;

		void RequestImageSave(vint id, const ImageSaveRequest& arguments) override
		{
			savedFormats.Add(arguments.format);
			// Transport tests use deterministic bytes; the native fixture verifies codecs.
			auto content = Ptr(new stream::MemoryStream);
			vuint8_t bytes[] = { 0, 255, 137, 42, 0, 1 };
			content->Write(bytes, sizeof(bytes));
			events->RespondImageSave(id, content);
		}
	};

	class PersistenceEditor : public controls::GuiCustomControl
	{
	public:
		Event<void(WString)> LoadAsPrivateFormat, SaveAsPrivateFormat, SaveAsRTF, SaveAsHTML;
		controls::GuiDocumentViewer* viewer;

		PersistenceEditor()
			: GuiCustomControl(theme::ThemeName::CustomControl)
		{
			viewer = new controls::GuiDocumentViewer(theme::ThemeName::DocumentViewer);
			GetContainerComposition()->AddChild(viewer->GetBoundsComposition());
			SetNamedObject(L"document", reflection::description::Value::From(viewer));
		}
	};

	class PersistenceWindow : public controls::GuiCustomControl
	{
	public:
		Event<void(vint)> PaletteSelected;
		PersistenceEditor* ribbon;
		PersistenceEditor* toolstrip;

		PersistenceWindow()
			: GuiCustomControl(theme::ThemeName::CustomControl)
		{
			ribbon = new PersistenceEditor;
			toolstrip = new PersistenceEditor;
			GetContainerComposition()->AddChild(ribbon->GetBoundsComposition());
			GetContainerComposition()->AddChild(toolstrip->GetBoundsComposition());
			SetNamedObject(L"editorRibbon", reflection::description::Value::From(ribbon));
			SetNamedObject(L"editorToolstrip", reflection::description::Value::From(toolstrip));
		}
	};

	Ptr<DocumentModel> PersistenceDocument(const WString& text, bool withImage)
	{
		auto model = Ptr(new DocumentModel);
		auto paragraph = Ptr(new DocumentParagraphRun);
		paragraph->alignment = Alignment::Right;
		auto style = Ptr(new DocumentStylePropertiesRun);
		style->style = Ptr(new DocumentStyleProperties);
		style->style->face = L"Consolas";
		style->style->size = DocumentFontSize(24, false);
		style->style->color = Color(21, 80, 134);
		style->style->bold = true;
		auto run = Ptr(new DocumentTextRun);
		run->text = text;
		style->runs.Add(run);
		paragraph->runs.Add(style);
		model->paragraphs.Add(paragraph);
		if (withImage)
		{
			auto image = Ptr(new DocumentImageRun);
			vuint8_t bytes[] = { 1, 2, 3 };
			image->image = GetCurrentController()->ImageService()->CreateImageFromMemory(bytes, sizeof(bytes));
			image->source = L"original-image.jpg";
			auto imageParagraph = Ptr(new DocumentParagraphRun);
			imageParagraph->runs.Add(image);
			model->paragraphs.Add(imageParagraph);
		}
		return model;
	}

	// This fixture runs the real controller pump without a graphical window.
	// Only the protocol handshake/window notifications are irrelevant here.
	class InjectionWorkerProtocol : public InjectionProtocol
	{
	public:
		vint mainThread = Thread::GetCurrentThreadId();
		void RequestInjectionFileSystem(vint id, const InjectionFileRequest& arguments) override
		{
			TEST_ASSERT(Thread::GetCurrentThreadId() == mainThread);
			InjectionProtocol::RequestInjectionFileSystem(id, arguments);
		}
		void RequestWindowNotifySetTitle(const WString&) override {}
		void RequestWindowNotifySetEnabled(const bool&) override {}
		void RequestWindowNotifySetTopMost(const bool&) override {}
		void RequestWindowNotifySetShowInTaskBar(const bool&) override {}
		void RequestWindowNotifySetCustomFrameMode(const bool&) override {}
		void RequestWindowNotifySetMaximizedBox(const bool&) override {}
		void RequestWindowNotifySetMinimizedBox(const bool&) override {}
		void RequestWindowNotifySetBorder(const bool&) override {}
		void RequestWindowNotifySetSizeBox(const bool&) override {}
		void RequestWindowNotifySetIconVisible(const bool&) override {}
		void RequestWindowNotifySetTitleBar(const bool&) override {}
		void RequestWindowNotifyShow(const WindowShowing&) override {}
		void RequestIOReleaseCapture() override {}
	};

	class InjectionWorkerController : public GuiRemoteController
	{
	public:
		InjectionWorkerController(IGuiRemoteProtocol* protocol)
			: GuiRemoteController(protocol, { false, false, true })
		{
			Initialize();
			controllerConnected = true;
			remoteGlobalConfig.injectFileSystem = true;
		}
		void RunFixture() { Run(&remoteWindow); }
		void StopFixture() { connectionStopped = true; }
	};

	class ShortTransferFileStream : public Object, public stream::IFileStreamImpl
	{
	public:
		stream::MemoryStream buffer;
		bool shortRead;
		bool shortWrite;
		bool closed = false;

		ShortTransferFileStream(stream::FileStream::AccessRight access, bool _shortRead, bool _shortWrite)
			: shortRead(_shortRead), shortWrite(_shortWrite)
		{
			if (access == stream::FileStream::ReadOnly)
			{
				vuint8_t bytes[] = { 1, 0, 2, 255 };
				buffer.Write(bytes, sizeof(bytes));
				buffer.SeekFromBegin(0);
			}
		}
		bool Open() override { return true; }
		void Close() override { closed = true; }
		pos_t Position() const override { return buffer.Position(); }
		pos_t Size() const override { return buffer.Size(); }
		void Seek(pos_t offset) override { buffer.Seek(offset); }
		void SeekFromBegin(pos_t offset) override { buffer.SeekFromBegin(offset); }
		void SeekFromEnd(pos_t offset) override { buffer.SeekFromEnd(offset); }
		vint Read(void* data, vint size) override
		{
			if (shortRead)
			{
				if (buffer.Position() >= 2) return 0;
				if (size > 2 - buffer.Position()) size = (vint)(2 - buffer.Position());
			}
			return buffer.Read(data, size);
		}
		vint Write(void* data, vint size) override { return buffer.Write(data, shortWrite && size > 0 ? size - 1 : size); }
		vint Peek(void* data, vint size) override { return buffer.Peek(data, size); }
	};

	class ShortTransferFileSystem : public feature_injection::FeatureImpl<filesystem::IFileSystemImpl>
	{
		filesystem::IFileSystemImpl* Predecessor() const { return const_cast<ShortTransferFileSystem*>(this)->Previous(); }
	public:
		bool shortRead = false;
		bool shortWrite = false;
		mutable Ptr<ShortTransferFileStream> last;
		wchar_t GetPathDelimiter() const override { return Predecessor()->GetPathDelimiter(); }
		const wchar_t* GetCompatibleDelimiters() const override { return Predecessor()->GetCompatibleDelimiters(); }
		WString ConcatPath(const WString& path, const WString& relative) const override { return Predecessor()->ConcatPath(path, relative); }
		void Initialize(WString& path) const override { Predecessor()->Initialize(path); }
		bool IsFile(const WString& path) const override { return Predecessor()->IsFile(path); }
		bool IsFolder(const WString& path) const override { return Predecessor()->IsFolder(path); }
		bool IsRoot(const WString& path) const override { return Predecessor()->IsRoot(path); }
		WString GetRelativePathFor(const WString& from, const WString& to) const override { return Predecessor()->GetRelativePathFor(from, to); }
		filesystem::FileInfo GetFileInfo(const filesystem::FilePath& path) const override { return Predecessor()->GetFileInfo(path); }
		bool FileCopy(const filesystem::FilePath& from, const filesystem::FilePath& to) const override { return Predecessor()->FileCopy(from, to); }
		bool FileDelete(const filesystem::FilePath& path) const override { return Predecessor()->FileDelete(path); }
		bool FileRename(const filesystem::FilePath& path, const WString& name) const override { return Predecessor()->FileRename(path, name); }
		bool GetFolders(const filesystem::FilePath& path, List<filesystem::Folder>& items) const override { return Predecessor()->GetFolders(path, items); }
		bool GetFiles(const filesystem::FilePath& path, List<filesystem::File>& items) const override { return Predecessor()->GetFiles(path, items); }
		bool CreateFolder(const filesystem::FilePath& path) const override { return Predecessor()->CreateFolder(path); }
		bool DeleteFolder(const filesystem::FilePath& path) const override { return Predecessor()->DeleteFolder(path); }
		bool FolderRename(const filesystem::FilePath& path, const WString& name) const override { return Predecessor()->FolderRename(path, name); }
		Ptr<stream::IFileStreamImpl> GetFileStreamImpl(const WString&, stream::FileStream::AccessRight access) const override
		{
			last = Ptr(new ShortTransferFileStream(access, shortRead, shortWrite));
			return last;
		}
	};

	class InjectionFileEvents : public GuiRemoteEventCombinator_PassingThrough
	{
	public:
		InjectionFileResponse result;
		vint responses = 0;
		void RespondInjectionFileSystem(vint id, const InjectionFileResponse& arguments) override
		{
			TEST_ASSERT(id == 1);
			result = arguments;
			responses++;
		}
	};
}
using namespace remote_startup_tests;

TEST_FILE
{
	TEST_CASE(L"Native renderer rejects incomplete file transfers")
	{
		ShortTransferFileSystem files;
		filesystem::InjectFileSystemImpl(&files);
		{
			remote_renderer::GuiRemoteRendererSingle renderer(false);
			InjectionFileEvents events;
			renderer.Initialize(&events);
			for (bool incomplete : { false, true })
			{
				files.shortRead = incomplete;
				renderer.RequestInjectionFileSystem(1, { InjectionFileOperation::OpenRead, L"/memory", {}, {} });
				TEST_ASSERT(events.result.success == !incomplete);
				TEST_ASSERT(events.result.content->Size() == (incomplete ? 2 : 4));
				TEST_ASSERT(files.last->closed);
				files.shortWrite = incomplete;
				auto content = Ptr(new stream::MemoryStream);
				vuint8_t bytes[] = { 0, 1, 255, 2 };
				content->Write(bytes, sizeof(bytes));
				renderer.RequestInjectionFileSystem(1, { InjectionFileOperation::Commit, L"/memory", {}, content });
				TEST_ASSERT(events.result.success == !incomplete);
				TEST_ASSERT(files.last->buffer.Size() == (incomplete ? 3 : 4));
				TEST_ASSERT(files.last->closed);
				content = Ptr(new stream::MemoryStream);
				renderer.RequestInjectionFileSystem(1, { InjectionFileOperation::Commit, L"/empty", {}, content });
				TEST_ASSERT(events.result.success && files.last->closed && files.last->buffer.Size() == 0);
			}
			TEST_ASSERT(events.responses == 6);
		}
		filesystem::EjectFileSystemImpl(&files);
	});

	TEST_CASE(L"Older connect messages have no injection capabilities")
	{
		glr::json::Parser parser;
		ControllerGlobalConfig config{};
		ConvertJsonToCustomType(glr::json::JsonParse(L"{}", parser), config);
		TEST_ASSERT(!config.injectDateTime && !config.injectLocale && !config.injectFileSystem);
		GuiRemoteControllerConfig core;
		TEST_ASSERT(!core.injectDateTime && !core.injectLocale && !core.injectFileSystem);
	});

	TEST_CASE(L"Worker filesystem calls marshal through the real UI pump")
	{
		InjectionWorkerProtocol protocol;
		InjectionWorkerController controller(protocol.GetProtocol());
		EventObject completed;
		TEST_ASSERT(completed.CreateManualUnsignal(false));
		bool available = true;
		controller.AsyncService()->InvokeAsync([&]()
		{
			List<filesystem::File> files;
			available = filesystem::Folder(filesystem::FilePath()).GetFiles(files);
			TEST_ASSERT(available && files.Count() == 0); // startup virtual root, no dispatched requests
			completed.Signal();
		});
		TEST_ASSERT(completed.WaitForTime(5000));
		TEST_ASSERT(protocol.fileRequests == 0);
		completed.Unsignal();
		bool uiWork = false;
		controller.AsyncService()->InvokeInMainThread(nullptr, [&]()
		{
			uiWork = true;
			controller.AsyncService()->InvokeAsync([&]()
			{
				List<filesystem::File> files;
				available = filesystem::Folder(filesystem::FilePath()).GetFiles(files);
				TEST_ASSERT(available && files.Count() == 1);
				controller.AsyncService()->InvokeInMainThread(nullptr, [&]() { controller.StopFixture(); });
				completed.Signal();
			});
		});
		controller.RunFixture();
		TEST_ASSERT(completed.WaitForTime(5000) && uiWork && protocol.fileRequests >= 3);
		completed.Unsignal();
		controller.AsyncService()->InvokeAsync([&]()
		{
			stream::FileStream file(L"/after-stop", stream::FileStream::WriteOnly);
			available = file.IsAvailable();
			completed.Signal();
		});
		TEST_ASSERT(completed.WaitForTime(5000) && !available);
		controller.Finalize();
	});

	TEST_CASE(L"A surviving stream retains safe state after controller destruction")
	{
		Ptr<stream::FileStream> surviving;
		InjectionWorkerProtocol protocol;
		{
			InjectionWorkerController controller(protocol.GetProtocol());
			surviving = Ptr(new stream::FileStream(L"/surviving", stream::FileStream::ReadWrite));
			TEST_ASSERT(surviving->IsAvailable());
		}
		vuint8_t value = 42;
		TEST_ASSERT(surviving->Write(&value, 1) == 1);
		TEST_ERROR(surviving->Close());
		surviving = nullptr;
		TEST_ASSERT(protocol.commits == 0);
	});

	TEST_CASE(L"Stopping the UI loop releases a waiting filesystem worker")
	{
		InjectionWorkerProtocol protocol;
		InjectionWorkerController controller(protocol.GetProtocol());
		EventObject started;
		EventObject completed;
		TEST_ASSERT(started.CreateManualUnsignal(false));
		TEST_ASSERT(completed.CreateManualUnsignal(false));
		bool emptyRoot = false;
		controller.AsyncService()->InvokeInMainThread(nullptr, [&]()
		{
			controller.AsyncService()->InvokeAsync([&]()
			{
				started.Signal();
				List<filesystem::File> files;
				emptyRoot = filesystem::Folder(filesystem::FilePath()).GetFiles(files) && files.Count() == 0;
				completed.Signal();
			});
			TEST_ASSERT(started.WaitForTime(5000));
			TEST_ASSERT(!completed.WaitForTime(100));
			controller.StopFixture();
		});
		controller.RunFixture();
		TEST_ASSERT(completed.WaitForTime(5000));
		TEST_ASSERT(emptyRoot && protocol.fileRequests == 0);
		controller.Finalize();
	});

	TEST_CATEGORY(L"Remote injection routing and portable values")
	{
		InjectionClock clock;
		InjectionLocale locale;
		InjectDateTimeImpl(&clock);
		InjectLocaleImpl(&locale);
		auto localRoot = filesystem::FilePath().GetFullPath();
		for (vint coreMask = 0; coreMask < 8; coreMask++)
		{
			TEST_CASE(L"Core configuration " + itow(coreMask))
			{
				InjectionProtocol protocol;
				JsonProtocol json(protocol.GetProtocol());
				SetGuiMainProxy([&]()
				{
					TEST_ASSERT(DateTime::LocalTime().hour == 22);
					TEST_ASSERT(Locale::UserDefault().GetName() == L"core-locale");
					TEST_ASSERT(filesystem::FilePath().GetFullPath() == ((coreMask & 4) ? L"/" : localRoot));
					for (vint rendererMask = 0; rendererMask < 8; rendererMask++)
					{
						auto config = MakeGlobalConfig();
						config.injectDateTime = (rendererMask & 1) != 0;
						config.injectLocale = (rendererMask & 2) != 0;
						config.injectFileSystem = (rendererMask & 4) != 0;
						protocol.GetEvents()->OnControllerConnect(config);
						protocol.dateRequests = protocol.localeRequests = protocol.fileRequests = 0;
						auto enabled = coreMask & rendererMask;
						auto now = DateTime::LocalTime();
						TEST_ASSERT(now.hour == ((enabled & 1) ? 1 : 22));
						TEST_ASSERT(now.year == ((enabled & 1) ? 2025 : 2024));
						TEST_ASSERT(now.milliseconds == 123);
						TEST_ASSERT(DateTime::UtcTime().hour == 23);
						auto winter = DateTime::FromDateTime(2024, 12, 31, 23, 30, 42, 123);
						TEST_ASSERT(winter.ToLocalTime().hour == ((enabled & 1) ? 1 : 22));
						TEST_ASSERT(winter.ToUtcTime().hour == ((enabled & 1) ? 21 : 0));
						auto summer = DateTime::FromDateTime(2024, 6, 1, 23, 30, 42, 456).ToLocalTime();
						TEST_ASSERT(summer.hour == ((enabled & 1) ? 2 : 22));
						TEST_ASSERT(summer.milliseconds == 456);
						TEST_ASSERT(protocol.dateRequests == ((enabled & 1) ? 4 : 0));
						TEST_ASSERT(Locale::UserDefault().GetName() == ((enabled & 2) ? L"renderer-2" : L"core-locale"));
						TEST_ASSERT(protocol.localeRequests == ((enabled & 2) ? 1 : 0));
						TEST_ASSERT(Locale(L"en-US").FormatNumber(L"42") == L"core-number");
						TEST_ASSERT(filesystem::FilePath().GetFullPath() == ((enabled & 4) ? L"/renderer" : (coreMask & 4) ? L"/" : localRoot));
						TEST_ASSERT(protocol.fileRequests == ((enabled & 4) ? 1 : 0));
						// Replacement while connected must also reevaluate capabilities.
					}
					if (coreMask & 2)
					{
						Locale remoteLocale(L"en-US");
						List<Locale> locales;
						Locale::Enumerate(locales);
						TEST_ASSERT(locales.Count() == 2 && locales[1].GetName() == L"renderer-second");
						List<WString> formats;
						remoteLocale.GetShortDateFormats(formats);
						remoteLocale.GetLongDateFormats(formats);
						remoteLocale.GetYearMonthDateFormats(formats);
						remoteLocale.GetLongTimeFormats(formats);
						remoteLocale.GetShortTimeFormats(formats);
						TEST_ASSERT(formats.Count() == 5 && formats[0] == L"renderer-4" && formats[4] == L"renderer-8");
						TEST_ASSERT(remoteLocale.GetShortDayOfWeekName(1) == L"renderer-9");
						TEST_ASSERT(remoteLocale.GetLongDayOfWeekName(1) == L"renderer-10");
						TEST_ASSERT(remoteLocale.GetShortMonthName(1) == L"renderer-11");
						TEST_ASSERT(remoteLocale.GetLongMonthName(1) == L"renderer-12");
						TEST_ASSERT(remoteLocale.ToLower(L"ABC") == L"renderer-13");
						TEST_ASSERT(remoteLocale.ToUpper(L"abc") == L"renderer-14");
						TEST_ASSERT(remoteLocale.ToLinguisticLower(L"ABC") == L"renderer-15");
						TEST_ASSERT(remoteLocale.ToLinguisticUpper(L"abc") == L"renderer-16");
						auto count = protocol.localeRequests;
						TEST_ASSERT(remoteLocale.FormatNumber(L"42") == L"core-number");
						TEST_ASSERT(remoteLocale.CompareOrdinal(L"a", L"b") < 0);
						TEST_ASSERT(remoteLocale.CompareOrdinalIgnoreCase(L"a", L"A") == 0);
						TEST_ASSERT(remoteLocale.StartsWith(L"abc", L"a", Locale::None));
						TEST_ASSERT(remoteLocale.EndsWith(L"abc", L"c", Locale::None));
						TEST_ASSERT(remoteLocale.FindFirst(L"abc", L"b", Locale::None).key == 1);
						TEST_ASSERT(remoteLocale.FindLast(L"abc", L"b", Locale::None).key == 1);
						TEST_ASSERT(protocol.localeRequests == count);
						protocol.cancelNext = true;
						TEST_ASSERT(Locale::UserDefault().GetName() == L"core-locale");
					}
					if (coreMask & 1)
					{
						auto config = MakeGlobalConfig();
						config.injectDateTime = true;
						protocol.GetEvents()->OnControllerConnect(config);
						protocol.cancelNext = true;
						TEST_ASSERT(DateTime::LocalTime().hour == 22);
					}
					protocol.GetEvents()->OnControllerDisconnect();
					TEST_ASSERT(DateTime::LocalTime().hour == 22);
					TEST_ASSERT(Locale::UserDefault().GetName() == L"core-locale");
					if (coreMask & 4)
					{
						filesystem::Folder root{ filesystem::FilePath() };
						List<filesystem::File> files;
						List<filesystem::Folder> folders;
						TEST_ASSERT(root.GetFiles(files) && files.Count() == 0);
						TEST_ASSERT(root.GetFolders(folders) && folders.Count() == 0);
						TEST_ASSERT(!filesystem::FilePath(localRoot).IsFile());
						stream::FileStream unavailable(localRoot, stream::FileStream::ReadOnly);
						TEST_ASSERT(!unavailable.IsAvailable());
					}
					protocol.GetEvents()->OnControllerForceExit();
				});
				SetupRemoteNativeController(&json, { (coreMask & 1) != 0, (coreMask & 2) != 0, (coreMask & 4) != 0 });
				SetGuiMainProxy({});
				TEST_ASSERT(DateTime::LocalTime().hour == 22);
				TEST_ASSERT(Locale::UserDefault().GetName() == L"core-locale");
				TEST_ASSERT(filesystem::FilePath().GetFullPath() == localRoot);
			});
		}
		EjectLocaleImpl(&locale);
		EjectDateTimeImpl(&clock);
	});

	TEST_CASE(L"Filesystem paths, nested directories, mutations and copy route to the renderer")
	{
		InjectionProtocol protocol;
		JsonProtocol json(protocol.GetProtocol());
		SetGuiMainProxy([&]()
		{
			using namespace filesystem;
			TEST_ASSERT(FilePath(L"a/../b\\c").GetFullPath() == L"/b/c");
			TEST_ASSERT(FilePath(L"/a").GetRelativePathFor(FilePath(L"/b/c")) == L"../b/c");
			TEST_ASSERT(!Folder(FilePath(L"/unavailable")).Create(false));
			TEST_ERROR(File(FilePath(L"/unavailable")).GetFileInfo());
			auto config = MakeGlobalConfig();
			config.injectFileSystem = true;
			protocol.GetEvents()->OnControllerConnect(config);
			TEST_ASSERT(FilePath::GetPathDelimiter() == L'/' && FilePath::GetCompatibleDelimiters() == L"");
			FilePath root;
			auto child = root / L"nested";
			auto leaf = child / L"leaf";
			TEST_ASSERT(leaf.GetFullPath() == L"/renderer/nested/leaf");
			TEST_ASSERT(root.GetRelativePathFor(leaf) == L"renderer-relative");
			TEST_ASSERT(!Folder(leaf).Create(false));
			TEST_ASSERT(Folder(leaf).Create(true));
			TEST_ASSERT(Folder(child).Exists() && Folder(leaf).Exists());
			List<Folder> folders;
			TEST_ASSERT(Folder(child).GetFolders(folders) && folders.Count() == 1 && folders[0].GetFilePath() == leaf);
			auto source = leaf / L"source";
			{
				stream::FileStream file(source.GetFullPath(), stream::FileStream::WriteOnly);
				vuint8_t bytes[] = { 0, 255, 10, 128 };
				TEST_ASSERT(file.IsAvailable() && file.Write(bytes, sizeof(bytes)) == sizeof(bytes));
			}
			auto destination = child / L"copy";
			TEST_ASSERT(File(source).CopyToFile(destination, false));
			TEST_ASSERT(protocol.files[destination.GetFullPath()]->Size() == 4);
			TEST_ASSERT(File(destination).GetFileInfo().lastModifiedTime.Value().milliseconds == 789);
			TEST_ASSERT(File(destination).Rename(L"renamed"));
			TEST_ASSERT(!File(destination).Exists() && File(child / L"renamed").Exists());
			TEST_ASSERT(File(child / L"renamed").Delete());
			TEST_ASSERT(!File(child / L"renamed").Delete());
			TEST_ASSERT(File(source).Delete());
			TEST_ASSERT(Folder(leaf).Rename(L"renamed-leaf"));
			TEST_ASSERT(!Folder(leaf).Exists() && Folder(child / L"renamed-leaf").Exists());
			TEST_ASSERT(Folder(child / L"renamed-leaf").Delete(false));
			TEST_ASSERT(Folder(child).Delete(false));
			stream::FileStream missingParent(L"/renderer/missing/child", stream::FileStream::WriteOnly);
			TEST_ASSERT(!missingParent.IsAvailable());
			protocol.GetEvents()->OnControllerDisconnect();
			TEST_ASSERT(!File(source).Exists() && !File(source).Delete());
			TEST_ASSERT(!Folder(child).Create(false));
			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(&json, { false, false, true });
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Remote buffered stream semantics")
	{
		InjectionProtocol protocol;
		JsonProtocol json(protocol.GetProtocol());
		SetGuiMainProxy([&]()
		{
			auto config = MakeGlobalConfig();
			config.injectFileSystem = true;
			protocol.GetEvents()->OnControllerConnect(config);
			TEST_CASE(L"Binary snapshot, access rights, seek and close exactly once")
			{
				Array<vuint8_t> data(131079);
				for (vint i = 0; i < data.Count(); i++) data[i] = (vuint8_t)(i % 256);
				{
					stream::FileStream write(L"/data", stream::FileStream::WriteOnly);
					TEST_ASSERT(write.IsAvailable() && write.Size() == 0);
					TEST_ASSERT(write.Write(&data[0], data.Count()) == data.Count());
					TEST_ASSERT(write.Position() == data.Count());
					TEST_ERROR(write.Read(&data[0], 1));
					write.Close();
					write.Close();
				}
				TEST_ASSERT(protocol.commits == 1);
				stream::FileStream read(L"/data", stream::FileStream::ReadOnly);
				TEST_ASSERT(read.IsAvailable() && read.Size() == data.Count());
				vuint8_t bytes[3]{};
				read.SeekFromBegin(255);
				TEST_ASSERT(read.Peek(bytes, 3) == 3 && read.Position() == 255);
				TEST_ASSERT(bytes[0] == 255 && bytes[1] == 0 && bytes[2] == 1);
				TEST_ASSERT(read.Read(bytes, 3) == 3 && read.Position() == 258);
				read.Seek(-1);
				TEST_ASSERT(read.Position() == 257);
				read.SeekFromEnd(1);
				TEST_ASSERT(read.Position() == read.Size() - 1);
				TEST_ERROR(read.Write(bytes, 1));
				{
					stream::FileStream empty(L"/data", stream::FileStream::ReadWrite);
					TEST_ASSERT(empty.Size() == 0);
				}
				TEST_ASSERT(protocol.commits == 2);
				TEST_ASSERT(read.Size() == data.Count());
				read.Close();
				stream::FileStream empty(L"/data", stream::FileStream::ReadOnly);
				TEST_ASSERT(empty.IsAvailable() && empty.Size() == 0);
				empty.Close();
				TEST_ASSERT(protocol.commits == 2);
				stream::FileStream denied(L"/denied", stream::FileStream::WriteOnly);
				stream::FileStream missing(L"/missing", stream::FileStream::ReadOnly);
				TEST_ASSERT(!denied.IsAvailable() && !missing.IsAvailable());
			});
			TEST_CASE(L"Portable metadata retains 64-bit values and optional timestamps")
			{
				auto info = filesystem::File(filesystem::FilePath(L"/data")).GetFileInfo();
				TEST_ASSERT(info.size == 0x100000007ULL && info.hardLinkCount == 0x200000009ULL);
				TEST_ASSERT(!info.creationTime && info.lastModifiedTime);
				TEST_ASSERT(info.lastModifiedTime.Value().day == 29 && info.lastModifiedTime.Value().milliseconds == 789);
				TEST_ASSERT(info.canRead && info.isSparse && info.isHidden);
			});
			TEST_CASE(L"Disconnect and replacement reject old stream closes")
			{
				stream::FileStream read(L"/data", stream::FileStream::ReadOnly);
				stream::FileStream write(L"/data", stream::FileStream::WriteOnly);
				protocol.GetEvents()->OnControllerDisconnect();
				vuint8_t byte = 42;
				TEST_ASSERT(write.Write(&byte, 1) == 1);
				TEST_ERROR(read.Close());
				protocol.GetEvents()->OnControllerConnect(config);
				TEST_ERROR(write.Close());
				TEST_ASSERT(protocol.commits == 2);
			});
			TEST_CASE(L"Canceled requests return unavailable without retrieving a response")
			{
				protocol.cancelNext = true;
				stream::FileStream canceled(L"/data", stream::FileStream::ReadOnly);
				TEST_ASSERT(!canceled.IsAvailable());
				protocol.GetEvents()->OnControllerConnect(config);
				stream::FileStream write(L"/data", stream::FileStream::WriteOnly);
				protocol.cancelNext = true;
				TEST_ERROR(write.Close());
				TEST_ASSERT(protocol.commits == 2);
			});
			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(&json, { false, false, true });
		SetGuiMainProxy({});
	});

	TEST_CASE(L"Document handlers retain independent viewers, styles and embedded images")
	{
		PersistenceProtocol protocol;
		JsonProtocol json(protocol.GetProtocol());
		SetGuiMainProxy([&]()
		{
			theme::RegisterTheme(Ptr(new darkskin::Theme));
			auto config = MakeGlobalConfig();
			config.injectFileSystem = true;
			protocol.GetEvents()->OnControllerConnect(config);
			{
				PersistenceWindow window;
				demo::AttachFctEvents<PersistenceEditor>(&window);
				auto original = PersistenceDocument(L"Ribbon \x4e2d\x6587", true);
				window.ribbon->viewer->SetDocument(original);
				window.toolstrip->viewer->SetDocument(PersistenceDocument(L"Toolstrip \x03c0", false));
				window.ribbon->SaveAsPrivateFormat(L"/ribbon.bin");
				window.toolstrip->SaveAsPrivateFormat(L"/toolstrip.bin");
				TEST_ASSERT(protocol.commits == 2 && protocol.savedFormats.Count() == 1);
				TEST_ASSERT(protocol.savedFormats[0] == INativeImage::Jpeg);
				TEST_ASSERT(original->paragraphs[1]->runs[0].Cast<DocumentImageRun>()->source == L"original-image.jpg");
				window.toolstrip->LoadAsPrivateFormat(L"/ribbon.bin");
				window.ribbon->LoadAsPrivateFormat(L"/toolstrip.bin");
				auto loaded = window.toolstrip->viewer->GetDocument();
				TEST_ASSERT(loaded->paragraphs.Count() == 2 && loaded->paragraphs[0]->GetTextForReading() == L"Ribbon \x4e2d\x6587");
				TEST_ASSERT(window.ribbon->viewer->GetDocument()->paragraphs[0]->GetTextForReading() == L"Toolstrip \x03c0");
				auto style = loaded->SummarizeStyle(TextPos(0, 0), TextPos(0, 2));
				TEST_ASSERT(style->face.Value() == L"Consolas" && style->size.Value() == DocumentFontSize(24, false));
				TEST_ASSERT(style->bold.Value() && style->color.Value() == Color(21, 80, 134));
				TEST_ASSERT(loaded->paragraphs[0]->alignment.Value() == Alignment::Right);
				auto image = loaded->paragraphs[1]->runs[0].Cast<DocumentImageRun>();
				TEST_ASSERT(image && image->image && INVLOC.StartsWith(image->source, L"res://Image_", Locale::None));
				window.toolstrip->SaveAsRTF(L"/ribbon.rtf");
				window.toolstrip->SaveAsHTML(L"/ribbon.html");
				TEST_ASSERT(protocol.commits == 4 && protocol.savedFormats.Count() == 3);
				TEST_ASSERT(protocol.savedFormats[1] == INativeImage::Png);
				TEST_ASSERT(protocol.files[L"/ribbon.rtf"]->Size() > 100 && protocol.files[L"/ribbon.html"]->Size() > 100);
				TEST_ERROR(window.ribbon->LoadAsPrivateFormat(L"/missing"));
				TEST_ASSERT(window.ribbon->viewer->GetDocument()->paragraphs[0]->GetTextForReading() == L"Toolstrip \x03c0");
			}
			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(&json, { false, false, true });
		SetGuiMainProxy({});
	});

	TEST_CASE(L"Image export creates before rendering, recreates after replacement and rejects cancellation")
	{
		InjectionProtocol protocol;
		JsonProtocol json(protocol.GetProtocol());
		SetGuiMainProxy([&]()
		{
			auto config = MakeGlobalConfig();
			protocol.GetEvents()->OnControllerConnect(config);
			const vuint8_t input[] = { 1, 0, 2, 255 };
			auto image = GetCurrentController()->ImageService()->CreateImageFromMemory((void*)input, sizeof(input));
			stream::MemoryStream output;
			image->SaveToStream(output, INativeImage::Png);
			TEST_ASSERT(output.Size() == 6 && protocol.imageCreates == 1 && protocol.imageSaves == 1);
			protocol.GetEvents()->OnControllerConnect(config);
			image->SaveToStream(output, INativeImage::Png);
			TEST_ASSERT(output.Size() == 12 && protocol.imageCreates == 2 && protocol.imageSaves == 2);
			protocol.cancelNext = true;
			TEST_ERROR(image->SaveToStream(output, INativeImage::Png));
			TEST_ASSERT(output.Size() == 12 && protocol.imageSaves == 3);
			TEST_ERROR(image->SaveToStream(output, INativeImage::Png));
			TEST_ASSERT(output.Size() == 12 && protocol.imageSaves == 3);
			protocol.GetEvents()->OnControllerConnect(config);
			protocol.cancelNext = true;
			TEST_ERROR(image->SaveToStream(output, INativeImage::Png));
			TEST_ASSERT(output.Size() == 12 && protocol.imageCreates == 3 && protocol.imageSaves == 3);
			protocol.GetEvents()->OnControllerConnect(config);
			image->SaveToStream(output, INativeImage::Png);
			TEST_ASSERT(output.Size() == 18 && protocol.imageCreates == 4 && protocol.imageSaves == 4);
			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(&json);
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Start and Stop")
	{
		StartUpProtocol protocol;
		SetGuiMainProxy([&]()
		{
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());

			auto rs = GetCurrentController()->ResourceService();
			auto ss = GetCurrentController()->ScreenService();

			TEST_CASE(L"Assert font config")
			{
				TEST_ASSERT(GetCurrentController()->GetExecutablePath() == L"/StartUp/Protocol.exe");
				TEST_ASSERT(rs->GetDefaultFont().fontFamily == L"StartUpDefault");
				TEST_ASSERT(rs->GetDefaultFont().bold == true);
				TEST_ASSERT(rs->GetDefaultFont().italic == false);

				List<WString> fonts;
				rs->EnumerateFonts(fonts);
				TEST_ASSERT(fonts.Count() == 3);
				TEST_ASSERT(fonts[0] == L"StartUpDefault");
				TEST_ASSERT(fonts[1] == L"Another");
				TEST_ASSERT(fonts[2] == L"YetAnother");
			});

			TEST_CASE(L"Remote image export supports document persistence")
			{
				const vuint8_t input[] = { 1, 0, 2, 255 };
				auto image = GetCurrentController()->ImageService()->CreateImageFromMemory((void*)input, sizeof(input));
				stream::MemoryStream output;
				image->SaveToStream(output, INativeImage::Png);
				TEST_ASSERT(output.Size() == 6);
				auto bytes = (vuint8_t*)output.GetInternalBuffer();
				TEST_ASSERT(bytes[0] == 137 && bytes[4] == 0 && bytes[5] == 255);
			});

			TEST_CASE(L"Assert screen config")
			{
				TEST_ASSERT(ss->GetScreenCount() == 1);
				auto screen = ss->GetScreen((vint)0);
				TEST_ASSERT(screen->IsPrimary() == true);
				TEST_ASSERT(screen->GetBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetClientBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetScalingX() == 1);
				TEST_ASSERT(screen->GetScalingY() == 1);
			});

			TEST_CASE(L"Assert updated screen config")
			{
				{
					ScreenConfig response;
					response.bounds = { 1,1,19, 29 };
					response.clientBounds = { 2,2,18,28 };
					response.scalingX = 1.2;
					response.scalingY = 1.5;
					protocol.GetEvents()->OnControllerScreenUpdated(response);
				}
				TEST_ASSERT(ss->GetScreenCount() == 1);
				auto screen = ss->GetScreen((vint)0);
				TEST_ASSERT(screen->IsPrimary() == true);
				TEST_ASSERT(screen->GetBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetClientBounds() == NativeRect(0, 0, 48, 38));
				TEST_ASSERT(screen->GetScalingX() == 1.2);
				TEST_ASSERT(screen->GetScalingY() == 1.5);
			});

			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(protocol.GetProtocol());
		SetGuiMainProxy({});
	});

	TEST_CATEGORY(L"Create windows without calling INativeWindowService::Run")
	{
		StartUpProtocol protocol;
		SetGuiMainProxy([&]()
		{
			// by not calling INativeWindowService::Run
			// non of them will be connected to the native window
			// so no interaction with the native window will happen
			protocol.GetEvents()->OnControllerConnect(MakeGlobalConfig());

			TEST_CASE(L"Create and destroy a window")
			{
				auto window = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
				GetCurrentController()->WindowService()->DestroyNativeWindow(window);
			});

			TEST_CASE(L"Create and destroy two windows")
			{
				auto window1 = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
				auto window2 = GetCurrentController()->WindowService()->CreateNativeWindow(INativeWindow::Normal);
				GetCurrentController()->WindowService()->DestroyNativeWindow(window1);
				GetCurrentController()->WindowService()->DestroyNativeWindow(window2);
			});

			protocol.GetEvents()->OnControllerForceExit();
		});
		SetupRemoteNativeController(protocol.GetProtocol());
		SetGuiMainProxy({});
	});
}
