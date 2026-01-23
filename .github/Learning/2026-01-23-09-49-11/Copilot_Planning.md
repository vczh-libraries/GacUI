# !!!PLANNING!!!

# UPDATES

## UPDATE

# Update
you can remove the 'if (pieces.Count() == 0)' block as you will notice it is covers by the following loop.

and you can make regexSeparators a field as it will be repeatly used.

## UPDATE

# Update
this is redundant (folderPath.GetFullPath() == L"" ? L"/" + name : folderPath.GetFullPath() + L"/" + name) you can just say folderPath.GetFullPath() + L"/" + name  The relative path function is to resolve if fromPath = /a/b/c toPath = /a/d it returns ../../d  you have to also consider ".." as a path component in Initialize.

# EXECUTION PLAN

## STEP 1: Add `FileItemMock`

Add a minimal in-memory tree node type in `Test/GacUISrc/UnitTest/TestControls.h` (namespace `gacui_unittest_template`) for representing a read-only filesystem:

	class FileItemMock : public Object
	{
	public:
		bool isFile = false;
		collections::Dictionary<WString, Ptr<FileItemMock>> children;
	};

Why:
- `FileDialogViewModel` queries `vl::filesystem::Folder` / `vl::filesystem::File` APIs; a deterministic tree is the smallest representation to drive enumeration and existence checks in unit tests.

## STEP 2: Add `FileSystemMock` and inject it (test-only)

Following the `TooltipTimer` pattern, add a helper that injects an `vl::filesystem::IFileSystemImpl` implementation for the duration of a test.

	class FileSystemMock : protected vl::feature_injection::FeatureImpl<vl::filesystem::IFileSystemImpl>
	{
	private:
		Ptr<FileItemMock> root;
		vl::regex::Regex regexSeparators;

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
		// IFileSystemImpl overrides (implemented in later steps)
	};

Why:
- This matches the existing unit-test helper style and ensures all `vl::filesystem::*` calls inside the view model route to the in-memory implementation without changing production code.

## STEP 3: Normalize paths in `Initialize` (single source of truth)

Implement `Initialize(WString& fullPath) const` to:
- Treat `/` and `\\` equivalently.
- Collapse duplicate separators.
- Remove trailing separators.
- Resolve `.` and `..` components.
- Produce root as `""`, and non-root paths as `/A/B/C` (leading `/`, no trailing `/`).

Key rule:
- After `FilePath::Initialize()` calls into this function, all other `IFileSystemImpl` APIs receive already-normalized `fullPath`, so they should not re-normalize.

Implementation sketch:

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

Why:
- `vl::filesystem::FilePath` delegates to `IFileSystemImpl::Initialize` and then forwards the same normalized string to all other methods, so concentrating normalization here avoids inconsistencies and duplicated logic.

## STEP 4: Resolve a path to a `FileItemMock` node

Add an internal helper that assumes `fullPath` is already normalized and traverses the tree by path segments.

Implementation sketch:

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

Use it to implement the query APIs:

	bool IsRoot(const WString& fullPath) const override
	{
		return fullPath == L"";
	}

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

Why:
- All other operations (enumeration, existence, relative formatting) reduce to "resolve then inspect", ensuring consistent behavior across the mock.

## STEP 5: Enumerate folders and files in a folder

Implement `GetFolders` / `GetFiles` to:
- Fail (return `false`) if the folder path does not exist or resolves to a file.
- Enumerate `children` and return `vl::filesystem::Folder` / `vl::filesystem::File` values with correct full paths.

Implementation sketch:

	bool GetFolders(const vl::filesystem::FilePath& folderPath, collections::List<vl::filesystem::Folder>& folders) const override
	{
		auto item = ResolvePath(folderPath.GetFullPath());
		if (!item || item->isFile) return false;

		for (auto [name, child] : item->children)
		{
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
		auto item = ResolvePath(folderPath.GetFullPath());
		if (!item || item->isFile) return false;

		for (auto [name, child] : item->children)
		{
			if (child->isFile)
			{
				auto childPath = folderPath.GetFullPath() + L"/" + name;
				files.Add(vl::filesystem::File(vl::filesystem::FilePath(childPath)));
			}
		}
		return true;
	}

Why:
- `FileDialogViewModel` does background refresh using `filesystem::Folder(path).GetFolders(...)` and `.GetFiles(...)`; stable results are required for deterministic tests.

## STEP 6: Implement `GetRelativePathFor` for test diagnostics

Implement `..`-based relative paths so that (for example) `fromPath = /a/b/c` and `toPath = /a/d` returns `../../d`.

Behavior:
- Treat `fromPath` as a folder when it resolves to a folder.
- If `fromPath` resolves to a file, use its parent folder as the base for relative calculation (same intent as platform implementations).
- Return a relative path using `/` as separators, with `..` components when needed, and no leading `/`.

Implementation sketch:

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

Why:
- `TryConfirm(...)` formats paths using `GetRelativePathFor` in failure reporting; tests need predictable strings including `..` behavior to match real filesystem semantics.

## STEP 7: Fail-fast for unsupported (mutation / stream) APIs

Because the mock is explicitly read-only, immediately stop if any write/mutation is invoked unexpectedly.

	bool FileDelete(const vl::filesystem::FilePath&) const override					{ CHECK_FAIL(L"Not Implemented!"); }
	bool FileRename(const vl::filesystem::FilePath&, const WString&) const override	{ CHECK_FAIL(L"Not Implemented!"); }
	bool CreateFolder(const vl::filesystem::FilePath&) const override					{ CHECK_FAIL(L"Not Implemented!"); }
	bool DeleteFolder(const vl::filesystem::FilePath&) const override					{ CHECK_FAIL(L"Not Implemented!"); }
	bool FolderRename(const vl::filesystem::FilePath&, const WString&) const override	{ CHECK_FAIL(L"Not Implemented!"); }
	Ptr<vl::stream::IFileStreamImpl> GetFileStreamImpl(const WString&, vl::stream::FileStream::AccessRight) const override
	{
		CHECK_FAIL(L"Not Implemented!");
	}

Why:
- This keeps the mock minimal and surfaces any unexpected dependencies in the dialog/view-model logic during unit tests.

## STEP 8: Test plan (new unit tests exercising the mock)

Add a focused unit test file (e.g. `Test/GacUISrc/UnitTest/TestFileSystemMock.cpp`) to validate the mock itself before using it to test view models:

- Path normalization:
  - `FilePath(L\"\")`, `FilePath(L\"/\")`, `FilePath(L\"//\")`, `FilePath(L\"\\\\\")` normalize to root `\"\"`.
  - `FilePath(L\"/A/B/\")`, `FilePath(L\"\\\\A\\\\B\\\\\")` normalize to `\"/A/B\"`.
  - `FilePath(L\"/A//B\")` normalizes to `\"/A/B\"`.
  - `FilePath(L\"/A/B/../C\")` normalizes to `\"/A/C\"`.
  - `FilePath(L\"/A/..\")` normalizes to root `\"\"`.
  - `FilePath(L\"/../A\")` normalizes to `\"/A\"` (clamp above-root `..`).
- Existence/type queries:
  - `IsRoot(\"\")` true; `IsRoot(\"/A\")` false.
  - `IsFolder(\"/F\")` true for folders; `IsFile(\"/F\")` false; missing paths are false for both.
  - File nodes return `IsFile == true` and are not treated as folders.
- Enumeration:
  - `GetFolders(root)` returns only folder children.
  - `GetFiles(root)` returns only file children.
  - Enumerating a file path returns `false`.
- Relative path:
  - `GetRelativePathFor(\"\", \"/A/B\") == \"A/B\"`.
  - `GetRelativePathFor(\"/A\", \"/A/B\") == \"B\"`.
  - `GetRelativePathFor(\"/a/b/c\", \"/a/d\") == \"../../d\"`.

Why:
- It is cheaper to debug problems in the mock directly than through view-model tests, and it locks down the normalization and enumeration contracts expected by later `GuiFileDialog` / `FileDialogViewModel` unit tests.

# !!!FINISHED!!!
