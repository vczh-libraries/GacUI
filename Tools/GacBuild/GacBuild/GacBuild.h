#ifndef VCZH_GACBUILD
#define VCZH_GACBUILD

#include "VlppWorkflowCompiler.h"

namespace gacbuild
{
	using namespace vl;
	using namespace vl::collections;
	using namespace vl::filesystem;

	struct FileInfo
	{
		vuint64_t						size;
		vuint64_t						modified;
	};

	extern void							Require(bool condition, const WString& message);
	extern bool							IsAbsolutePath(const WString& path);
	extern void							ValidateExecutable(const WString& path);
	extern Nullable<FileInfo>			GetFileInfo(const FilePath& path);
	extern void							CopyFileNative(const FilePath& source, const FilePath& destination);
	extern void							RunProcess(const FilePath& executable, const List<WString>& arguments);
	extern void							Run(const List<WString>& arguments);
}

#endif
