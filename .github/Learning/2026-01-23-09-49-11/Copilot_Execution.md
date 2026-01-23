# !!!EXECUTION!!!

# UPDATES

## User Update Spotted

- In `Test/GacUISrc/UnitTest/TestControls.h`, `FileSystemMock` contains an extra helper `SplitComponents` and `ResolvePath` uses it (skipping empty components) instead of iterating raw regex split results.
- In `Test/GacUISrc/UnitTest/TestControls.h`, `GetRelativePathFor` now calls `Initialize` on both `fromPath` and `toPath` before computing relative components (the plan computed against raw inputs).
- In `Test/GacUISrc/UnitTest/TestControls.h`, the separator regex pattern is `L"[\\/\\\\]+"` (fixing escaping to avoid regex parse errors during tests), instead of the earlier planned `L"[///\\]+"`.

# EXECUTION PLAN

## STEP 1: Add `FileItemMock` [DONE]

- File: `Test/GacUISrc/UnitTest/TestControls.h`
- Location: inside `namespace gacui_unittest_template`, after `TooltipTimer`.
- Insert:

	class FileItemMock : public Object
	{
	public:
		bool isFile = false;
		collections::Dictionary<WString, Ptr<FileItemMock>> children;
	};

## STEP 2: Add `FileSystemMock` and inject it (test-only) [DONE]

- File: `Test/GacUISrc/UnitTest/TestControls.h`
- Location: inside `namespace gacui_unittest_template`, after `FileItemMock`.
- Insert:

	class FileSystemMock : protected vl::feature_injection::FeatureImpl<vl::filesystem::IFileSystemImpl>
	{
	private:
		Ptr<FileItemMock>			root;
		vl::regex::Regex			regexSeparators;

	public:
		FileSystemMock(Ptr<FileItemMock> _root)
			: root(_root)
			, regexSeparators(L"[///\\]+")
		{
			vl::filesystem::InjectFileSystemImpl(this);
		}

		~FileSystemMock()
		{
			try
			{
				vl::filesystem::EjectFileSystemImpl(nullptr);
			}
			catch (...)
			{
			}
		}

	protected:
		Ptr<FileItemMock> ResolvePath(const WString& fullPath) const
		{
			if (fullPath == L"") return root;
			if (!fullPath.StartsWith(L"/")) return nullptr;

			vl::regex::RegexMatch::List pieces;
			regexSeparators.Split(fullPath, false, pieces);

			auto current = root;
			for (auto&& piece : pieces)
			{
				auto name = piece->Result().Value();
				vint index = current->children.Keys().IndexOf(name);
				if (index == -1) return nullptr;
				current = current->children.Values()[index];
			}
			return current;
		}

## STEP 3: Normalize paths in `Initialize` (single source of truth) [DONE]

- File: `Test/GacUISrc/UnitTest/TestControls.h`
- Location: inside `class FileSystemMock`.
- Add override:

		void Initialize(WString& fullPath) const override
		{
			vl::regex::RegexMatch::List pieces;
			regexSeparators.Split(fullPath, false, pieces);

			collections::List<WString> components;
			for (auto&& piece : pieces)
			{
				auto value = piece->Result().Value();
				if (value == L".") continue;
				if (value == L"..")
				{
					if (components.Count() > 0) components.RemoveAt(components.Count() - 1);
					continue;
				}
				components.Add(value);
			}

			WString normalized;
			for (auto&& component : components)
			{
				normalized += L"/";
				normalized += component;
			}
			fullPath = normalized;
		}

## STEP 4: Implement root/file/folder queries [DONE]

- File: `Test/GacUISrc/UnitTest/TestControls.h`
- Location: inside `class FileSystemMock`.
- Add overrides:

		bool IsFile(const WString& fullPath) const override
		{
			auto item = ResolvePath(fullPath);
			return item && item->isFile;
		}

		bool IsFolder(const WString& fullPath) const override
		{
			auto item = ResolvePath(fullPath);
			return item && !item->isFile;
		}

		bool IsRoot(const WString& fullPath) const override
		{
			return fullPath == L"";
		}

## STEP 5: Enumerate folders and files in a folder [DONE]

- File: `Test/GacUISrc/UnitTest/TestControls.h`
- Location: inside `class FileSystemMock`.
- Add overrides:

		bool GetFolders(const vl::filesystem::FilePath& folderPath, collections::List<vl::filesystem::Folder>& folders) const override
		{
			folders.Clear();
			auto item = ResolvePath(folderPath.GetFullPath());
			if (!item || item->isFile) return false;

			for (vint i = 0; i < item->children.Count(); i++)
			{
				auto name = item->children.Keys()[i];
				auto child = item->children.Values()[i];
				if (!child->isFile)
				{
					auto childPath = folderPath.GetFullPath() + L"/" + name;
					folders.Add(vl::filesystem::Folder(vl::filesystem::FilePath(childPath)));
				}
			}
			return true;
		}

		bool GetFiles(const vl::filesystem::FilePath& folderPath, collections::List<vl::filesystem::File>& files) const override
		{
			files.Clear();
			auto item = ResolvePath(folderPath.GetFullPath());
			if (!item || item->isFile) return false;

			for (vint i = 0; i < item->children.Count(); i++)
			{
				auto name = item->children.Keys()[i];
				auto child = item->children.Values()[i];
				if (child->isFile)
				{
					auto childPath = folderPath.GetFullPath() + L"/" + name;
					files.Add(vl::filesystem::File(vl::filesystem::FilePath(childPath)));
				}
			}
			return true;
		}

## STEP 6: Implement `GetRelativePathFor` (supports `..`) [DONE]

- File: `Test/GacUISrc/UnitTest/TestControls.h`
- Location: inside `class FileSystemMock`.
- Add override:

		WString GetRelativePathFor(const WString& fromPath, const WString& toPath) const override
		{
			auto baseFromPath = fromPath;
			if (IsFile(baseFromPath))
			{
				auto index = INVLOC.FindLast(baseFromPath, L"/", Locale::None);
				baseFromPath = index.key == -1 ? L"" : baseFromPath.Left(index.key);
			}

			vl::regex::RegexMatch::List fromPieces, toPieces;
			regexSeparators.Split(baseFromPath, false, fromPieces);
			regexSeparators.Split(toPath, false, toPieces);

			collections::List<WString> fromComponents, toComponents;
			for (auto&& piece : fromPieces) fromComponents.Add(piece->Result().Value());
			for (auto&& piece : toPieces) toComponents.Add(piece->Result().Value());

			vint common = 0;
			while (common < fromComponents.Count()
				&& common < toComponents.Count()
				&& fromComponents[common] == toComponents[common])
			{
				common++;
			}

			collections::List<WString> resultComponents;
			for (vint i = common; i < fromComponents.Count(); i++)
			{
				resultComponents.Add(L"..");
			}
			for (vint i = common; i < toComponents.Count(); i++)
			{
				resultComponents.Add(toComponents[i]);
			}

			WString relative;
			for (vint i = 0; i < resultComponents.Count(); i++)
			{
				if (i > 0) relative += L"/";
				relative += resultComponents[i];
			}
			return relative;
		}

## STEP 7: Fail-fast for unsupported APIs (read-only mock) [DONE]

- File: `Test/GacUISrc/UnitTest/TestControls.h`
- Location: inside `class FileSystemMock`.
- Add overrides:

		bool FileDelete(const vl::filesystem::FilePath&) const override					{ CHECK_FAIL(L"Not Implemented!"); return false; }
		bool FileRename(const vl::filesystem::FilePath&, const WString&) const override	{ CHECK_FAIL(L"Not Implemented!"); return false; }
		bool CreateFolder(const vl::filesystem::FilePath&) const override					{ CHECK_FAIL(L"Not Implemented!"); return false; }
		bool DeleteFolder(const vl::filesystem::FilePath&) const override					{ CHECK_FAIL(L"Not Implemented!"); return false; }
		bool FolderRename(const vl::filesystem::FilePath&, const WString&) const override	{ CHECK_FAIL(L"Not Implemented!"); return false; }
		Ptr<vl::stream::IFileStreamImpl> GetFileStreamImpl(const WString&, vl::stream::FileStream::AccessRight) const override
		{
			CHECK_FAIL(L"Not Implemented!");
			return nullptr;
		}

## STEP 8: Unit tests for the mock (recommended) [DONE]

- Add a new test file `Test/GacUISrc/UnitTest/TestFileSystemMock.cpp` (same project / filters as existing unit tests).
- Test cases:
	- Path normalization via `vl::filesystem::FilePath`:
		- `""`, `"/"`, `"//"`, `"\\\\"` normalize to root `""`.
		- `"/A/B/"`, `"\\\\A\\\\B\\\\"` normalize to `"/A/B"`.
		- `"/A//B"` normalizes to `"/A/B"`.
		- `"/A/B/../C"` normalizes to `"/A/C"`.
		- `"/A/.."` normalizes to root `""`.
		- `"/../A"` normalizes to `"/A"`.
	- Existence/type queries:
		- `IsRoot("")` true; `IsRoot("/A")` false.
		- Folder nodes return `IsFolder == true`; file nodes return `IsFile == true`; missing paths return false for both.
	- Enumeration:
		- `GetFolders(root)` returns only folder children.
		- `GetFiles(root)` returns only file children.
		- Enumerating a file path returns `false`.
	- Relative paths:
		- `GetRelativePathFor("", "/A/B") == "A/B"`.
		- `GetRelativePathFor("/A", "/A/B") == "B"`.
		- `GetRelativePathFor("/a/b/c", "/a/d") == "../../d"`.

# FIXING ATTEMPTS

## Fixing attempt No.1

- Why the original change did not work: `vl::WString` (aka `vl::ObjectString<wchar_t>`) doesn't provide a `StartsWith` member function, so `ResolvePath` in `TestControls.h` failed to compile.
- What I changed: Replaced `fullPath.StartsWith(L"/")` with a first-character check `fullPath[0] != L'/'` (the function already returns early for empty strings).
- Why it should fix: `vl::ObjectString<T>` supports indexing via `operator[]`, so this keeps the intended logic (require an absolute path starting with `/`) without relying on a non-existent API.

## Fixing attempt No.2

- Why the original change did not work: The unit tests crashed in `TestFileSystemMock.cpp` with `Regular expression syntax error: Illegal character set definition.` because the separator regex pattern `L"[///\\\\]+"` was written using a PCRE-like escaping assumption, but `vl::regex::Regex` treats both `\` and `/` as escaping characters and requires `\/` and `\\` inside `[]` to represent literal `/` and `\`.
- What I changed: Updated the pattern to `L"[\\/\\\\]+"` in `Test/GacUISrc/UnitTest/TestControls.h`.
- Why it should fix: This matches one-or-more path separators using the correct escaping rules documented in `Import/VlppRegex.h`, so `Split` no longer throws and the test suite can continue executing.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

- In `Test/GacUISrc/UnitTest/TestControls.h`, `FileSystemMock::regexSeparators` was changed from `L"[\\/\\\\]+"` (the verified pattern that matches both `/` and `\\` under `vl::regex` escaping rules) to `L"[///\\]+"`.
