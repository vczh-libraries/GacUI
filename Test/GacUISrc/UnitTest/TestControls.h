#ifndef GACUI_UNITTEST_TESTCONTROLS
#define GACUI_UNITTEST_TESTCONTROLS

#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"
#ifdef VCZH_64
#include "../Generated_DarkSkin/Source_x64/DarkSkin.h"
#else
#include "../Generated_DarkSkin/Source_x86/DarkSkin.h"
#endif
#include "../../../Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::controls::list;
using namespace vl::presentation::controls::tree;
using namespace vl::presentation::unittest;

namespace gacui_unittest_template
{
	class TooltipTimer : protected vl::feature_injection::FeatureImpl<vl::IDateTimeImpl>
	{
	private:
		vl::DateTime			currentTime;

	public:
		TooltipTimer()
		{
			currentTime = vl::DateTime::FromDateTime(2000, 1, 1, 0, 0, 0, 0);
			vl::InjectDateTimeImpl(this);
		}

		~TooltipTimer()
		{
			try
			{
				vl::EjectDateTimeImpl(nullptr);
			}
			catch (...)
			{
			}
		}

		void Tooltip()
		{
			currentTime = currentTime.Forward(600);
		}

		void WaitForClosing()
		{
			currentTime = currentTime.Forward(5100);
		}

	protected:
		vl::DateTime FromDateTime(vint _year, vint _month, vint _day, vint _hour, vint _minute, vint _second, vint _milliseconds) override
		{
			return Previous()->FromDateTime(_year, _month, _day, _hour, _minute, _second, _milliseconds);
		}

		vl::DateTime FromOSInternal(vuint64_t osInternal) override
		{
			return Previous()->FromOSInternal(osInternal);
		}

		vuint64_t LocalTime() override
		{
			return currentTime.osMilliseconds;
		}

		vuint64_t UtcTime() override
		{
			return Previous()->LocalToUtcTime(currentTime.osInternal);
		}

		vuint64_t LocalToUtcTime(vuint64_t osInternal) override
		{
			return Previous()->LocalToUtcTime(osInternal);
		}

		vuint64_t UtcToLocalTime(vuint64_t osInternal) override
		{
			return Previous()->UtcToLocalTime(osInternal);
		}

		vuint64_t Forward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Forward(osInternal, milliseconds);
		}

		vuint64_t Backward(vuint64_t osInternal, vuint64_t milliseconds) override
		{
			return Previous()->Backward(osInternal, milliseconds);
		}
	};

	class FileItemMock : public Object
	{
	public:
		bool											isFile = false;
		collections::Dictionary<WString, Ptr<FileItemMock>>	children;
	};

	class FileSystemMock : protected vl::feature_injection::FeatureImpl<vl::filesystem::IFileSystemImpl>
	{
	private:
		Ptr<FileItemMock>	root;
		vl::regex::Regex	regexSeparators;

	private:
		void SplitComponents(const WString& fullPath, collections::List<WString>& components) const
		{
			vl::regex::RegexMatch::List pieces;
			regexSeparators.Split(fullPath, false, pieces);

			components.Clear();
			for (auto&& piece : pieces)
			{
				auto value = piece->Result().Value();
				if (value == L"") continue;
				components.Add(value);
			}
		}

	public:
		FileSystemMock(Ptr<FileItemMock> _root)
			: root(_root)
			, regexSeparators(L"[///\\\\]+")
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

			collections::List<WString> components;
			SplitComponents(fullPath, components);

			auto current = root;
			for (auto&& name : components)
			{
				vint index = current->children.Keys().IndexOf(name);
				if (index == -1) return nullptr;
				current = current->children.Values()[index];
			}
			return current;
		}

		void Initialize(WString& fullPath) const override
		{
			if (fullPath == L"") return;

			vl::regex::RegexMatch::List pieces;
			regexSeparators.Split(fullPath, false, pieces);

			collections::List<WString> components;
			for (auto&& piece : pieces)
			{
				auto value = piece->Result().Value();
				if (value == L"") continue;
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

		WString GetRelativePathFor(const WString& fromPath, const WString& toPath) const override
		{
			auto baseFromPath = fromPath;
			Initialize(baseFromPath);

			auto normalizedToPath = toPath;
			Initialize(normalizedToPath);

			if (IsFile(baseFromPath))
			{
				auto index = INVLOC.FindLast(baseFromPath, L"/", Locale::None);
				baseFromPath = index.key == -1 ? L"" : baseFromPath.Left(index.key);
			}

			collections::List<WString> fromComponents, toComponents;
			SplitComponents(baseFromPath, fromComponents);
			SplitComponents(normalizedToPath, toComponents);

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
	};
}

#endif