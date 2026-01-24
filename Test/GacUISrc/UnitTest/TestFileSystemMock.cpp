#include "TestControls.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace gacui_unittest_template;

namespace
{
	Ptr<FileItemMock> AddFolder(Ptr<FileItemMock> parent, const WString& name)
	{
		auto item = Ptr(new FileItemMock);
		item->isFile = false;
		parent->children.Add(name, item);
		return item;
	}

	Ptr<FileItemMock> AddFile(Ptr<FileItemMock> parent, const WString& name)
	{
		auto item = Ptr(new FileItemMock);
		item->isFile = true;
		parent->children.Add(name, item);
		return item;
	}
}

TEST_FILE
{
	TEST_CASE(L"FileSystemMock: PathNormalization")
	{
		auto root = Ptr(new FileItemMock);
		FileSystemMock mock(root);

		auto TestNormalize = [](const WString& input, const WString& expected)
		{
			FilePath path(input);
			TEST_ASSERT(path.GetFullPath() == expected);
		};

		TestNormalize(L"", L"");
		TestNormalize(L"/", L"");
		TestNormalize(L"//", L"");
		TestNormalize(L"\\\\", L"");

#if defined VCZH_MSVC
		TestNormalize(L"/A/B/", L"\\A\\B");
		TestNormalize(L"\\A\\B\\", L"\\A\\B");
		TestNormalize(L"/A//B", L"\\A\\B");
		TestNormalize(L"/A/B/../C", L"\\A\\C");
		TestNormalize(L"/A/..", L"");
		TestNormalize(L"/../A", L"\\A");
#else
		TestNormalize(L"/A/B/", L"/A/B");
		TestNormalize(L"\\A\\B\\", L"/A/B");
		TestNormalize(L"/A//B", L"/A/B");
		TestNormalize(L"/A/B/../C", L"/A/C");
		TestNormalize(L"/A/..", L"");
		TestNormalize(L"/../A", L"/A");
#endif
	});

	TEST_CASE(L"FileSystemMock: ExistenceAndType")
	{
		auto root = Ptr(new FileItemMock);
		auto folderA = AddFolder(root, L"A");
		AddFile(folderA, L"a.txt");
		AddFolder(folderA, L"B");
		AddFile(root, L"root.txt");

		FileSystemMock mock(root);

		TEST_ASSERT(FilePath(L"").IsRoot());
		TEST_ASSERT(!FilePath(L"/A").IsRoot());

		TEST_ASSERT(FilePath(L"/A").IsFolder());
		TEST_ASSERT(!FilePath(L"/A").IsFile());

		TEST_ASSERT(FilePath(L"/A/a.txt").IsFile());
		TEST_ASSERT(!FilePath(L"/A/a.txt").IsFolder());

		TEST_ASSERT(!FilePath(L"/NotExist").IsFile());
		TEST_ASSERT(!FilePath(L"/NotExist").IsFolder());
	});

	TEST_CASE(L"FileSystemMock: Enumeration")
	{
		auto root = Ptr(new FileItemMock);
		AddFile(root, L"a.txt");
		AddFile(root, L"b.txt");
		AddFolder(root, L"A");
		AddFolder(root, L"B");

		FileSystemMock mock(root);

		List<Folder> folders;
		List<File> files;

		TEST_ASSERT(Folder(FilePath(L"")).GetFolders(folders));
		TEST_ASSERT(Folder(FilePath(L"")).GetFiles(files));

		TEST_ASSERT(folders.Count() == 2);
		TEST_ASSERT(files.Count() == 2);

		TEST_ASSERT(Folder(FilePath(L"/a.txt")).GetFolders(folders) == false);
		TEST_ASSERT(Folder(FilePath(L"/a.txt")).GetFiles(files) == false);
	});

	TEST_CASE(L"FileSystemMock: RelativePaths")
	{
		auto root = Ptr(new FileItemMock);
		FileSystemMock mock(root);

#if defined VCZH_MSVC
		TEST_ASSERT(FilePath(L"").GetRelativePathFor(FilePath(L"/A/B")) == L"A\\B");
		TEST_ASSERT(FilePath(L"/A").GetRelativePathFor(FilePath(L"/A/B")) == L"B");
		TEST_ASSERT(FilePath(L"/a/b/c").GetRelativePathFor(FilePath(L"/a/d")) == L"..\\..\\d");
#else
		TEST_ASSERT(FilePath(L"").GetRelativePathFor(FilePath(L"/A/B")) == L"A/B");
		TEST_ASSERT(FilePath(L"/A").GetRelativePathFor(FilePath(L"/A/B")) == L"B");
		TEST_ASSERT(FilePath(L"/a/b/c").GetRelativePathFor(FilePath(L"/a/d")) == L"../../d");
#endif
	});
}
