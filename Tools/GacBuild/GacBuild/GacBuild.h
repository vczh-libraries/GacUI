#ifndef VCZH_GACBUILD
#define VCZH_GACBUILD

#include "VlppWorkflowCompiler.h"

namespace gacbuild
{
	using namespace vl;
	using namespace vl::collections;
	using namespace vl::filesystem;

	extern void							Require(bool condition, const WString& message);
	extern void							ValidateExecutable(const WString& path);
	extern void							RunProcess(const FilePath& executable, const List<WString>& arguments);
	extern void							Run(const List<WString>& arguments);
}

#endif
