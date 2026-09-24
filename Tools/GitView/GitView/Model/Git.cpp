#include "Git.h"

namespace gitview
{
	using namespace vl;
	using namespace vl::collections;
	using namespace vl::filesystem;

	WString DecodeCommandOutput(stream::MemoryStream& bytes)
	{
		// String conversion is NUL-terminated; Git's -z output is a sequence of strings.
		auto buffer = static_cast<char8_t*>(bytes.GetInternalBuffer());
		auto size = static_cast<vint>(bytes.Size());
		List<wchar_t> decoded;
		vint start = 0;
		for (vint i = 0; i <= size; i++)
		{
			if (i != size && buffer[i] != 0) continue;
			if (i > start)
			{
				auto part = u8tow(U8String::CopyFrom(buffer + start, i - start));
				for (vint j = 0; j < part.Length(); j++) decoded.Add(part[j]);
			}
			if (i != size) decoded.Add(0);
			start = i + 1;
		}
		return decoded.Count() ? WString::CopyFrom(&decoded[0], decoded.Count()) : WString::Empty;
	}

	List<WString> MakeArguments(std::initializer_list<WString> arguments)
	{
		List<WString> result;
		for (auto&& argument : arguments) result.Add(argument);
		return result;
	}

	void SplitRecords(const WString& text, wchar_t separator, List<WString>& records)
	{
		vint start = 0;
		for (vint i = 0; i < text.Length(); i++)
		{
			if (text[i] != separator) continue;
			records.Add(text.Sub(start, i - start));
			start = i + 1;
		}
		if (start < text.Length()) records.Add(text.Sub(start, text.Length() - start));
	}

	WString TrimLineEnding(const WString& text)
	{
		auto length = text.Length();
		while (length > 0 && (text[length - 1] == L'\r' || text[length - 1] == L'\n')) length--;
		return text.Left(length);
	}

	void ParseStatus(const WString& text, List<GitFile>& files)
	{
		List<WString> records;
		SplitRecords(text, 0, records);
		for (vint i = 0; i < records.Count(); i++)
		{
			auto&& record = records[i];
			if (record.Length() < 4 || record[2] != L' ') throw Exception(WString::Unmanaged(L"Malformed Git status record."));
			GitFile file;
			file.path = record.Sub(3, record.Length() - 3);
			file.status = record.Left(2);
			if (record[0] == L'R' || record[0] == L'C' || record[1] == L'R' || record[1] == L'C')
			{
				if (++i == records.Count()) throw Exception(WString::Unmanaged(L"Missing Git rename source."));
				file.oldPath = records[i];
			}
			if (record[0] == L'?' && record[1] == L'?')
			{
				file.kind = FileKind::Untracked;
				files.Add(file);
			}
			else
			{
				// Conflicted entries belong to the working tree, not to a resolved index diff.
				auto conflict = record[0] == L'U' || record[1] == L'U' || file.status == WString::Unmanaged(L"AA") || file.status == WString::Unmanaged(L"DD");
				if (!conflict && record[0] != L' ')
				{
					file.kind = FileKind::Staged;
					files.Add(file);
				}
				if (conflict || record[1] != L' ')
				{
					file.kind = FileKind::Unstaged;
					files.Add(file);
				}
			}
		}
	}

	struct DiffHunk : Object
	{
		vint				oldStart = 0;
		vint				oldCount = 0;
		vint				newStart = 0;
		vint				newCount = 0;
		List<WString>		removed;
		List<WString>		added;
	};

	void ReadDiffRange(const WString& range, vint& start, vint& count)
	{
		List<WString> parts;
		SplitRecords(range.Sub(1, range.Length() - 1), L',', parts);
		CHECK_ERROR(parts.Count() == 1 || parts.Count() == 2, L"Malformed Git hunk range.");
		start = wtoi(parts[0]);
		count = parts.Count() == 2 ? wtoi(parts[1]) : 1;
		if (count > 0) start--;
	}

	void ParseDiff(const WString& patch, const WString& newText, List<DiffLine>& lines)
	{
		List<WString> records;
		SplitRecords(patch, L'\n', records);
		List<Ptr<DiffHunk>> hunks;
		bool binary = false;
		for (auto&& record : records)
		{
			auto line = TrimLineEnding(record);
			if (hunks.Count())
			{
				auto hunk = hunks[hunks.Count() - 1];
				if (hunk->removed.Count() < hunk->oldCount || hunk->added.Count() < hunk->newCount)
				{
					if (line.Length() && line[0] == L'\\') continue; // No final newline marker.
					CHECK_ERROR(line.Length() && (line[0] == L'-' || line[0] == L'+'), L"Expected zero-context Git diff.");
					(line[0] == L'-' ? hunk->removed : hunk->added).Add(line.Sub(1, line.Length() - 1));
					continue;
				}
			}
			if (line.Length() >= 3 && line.Left(3) == WString::Unmanaged(L"@@ "))
			{
				List<WString> fields;
				SplitRecords(line, L' ', fields);
				CHECK_ERROR(fields.Count() >= 4 && fields[3] == WString::Unmanaged(L"@@"), L"Malformed Git hunk header.");
				auto hunk = Ptr(new DiffHunk);
				ReadDiffRange(fields[1], hunk->oldStart, hunk->oldCount);
				ReadDiffRange(fields[2], hunk->newStart, hunk->newCount);
				hunks.Add(hunk);
			}
			else if (line.Length() >= 13 && line.Left(13) == WString::Unmanaged(L"Binary files ")) binary = true;
		}

		List<WString> content;
		SplitRecords(newText, L'\n', content);
		for (auto [line, i] : indexed(content)) content[i] = TrimLineEnding(line);
		auto append = [&](vint index, const WString& text, vint change)
		{
			auto number = itow(index + 1);
			lines.Add({ WString::Unmanaged(L"    ").Left(number.Length() < 4 ? 4 - number.Length() : 0) + number + WString::Unmanaged(L" ") + text, change });
		};
		vint cursor = 0;
		for (auto [hunk, i] : indexed(hunks))
		{
			CHECK_ERROR(hunk->removed.Count() == hunk->oldCount && hunk->added.Count() == hunk->newCount, L"Incomplete Git hunk.");
			if (hunk->newStart < cursor || hunk->newStart + hunk->newCount > content.Count())
				throw Exception(WString::Unmanaged(L"File changed while reading its diff. REFRESH and select it again."));
			auto start = cursor > hunk->newStart - 3 ? cursor : hunk->newStart - 3;
			if (i > 0 && start > cursor) lines.Add({ .separator = true });
			for (vint j = start; j < hunk->newStart; j++) append(j, content[j], 0);
			for (auto [line, j] : indexed(hunk->removed)) append(hunk->oldStart + j, line, -1);
			for (auto [line, j] : indexed(hunk->added))
			{
				auto index = hunk->newStart + j;
				if (line != content[index]) throw Exception(WString::Unmanaged(L"File changed while reading its diff. REFRESH and select it again."));
				append(index, content[index], 1);
			}
			cursor = hunk->newStart + hunk->newCount;
			auto end = content.Count() < cursor + 3 ? content.Count() : cursor + 3;
			if (i + 1 < hunks.Count() && hunks[i + 1]->newStart < end) end = hunks[i + 1]->newStart;
			while (cursor < end) { append(cursor, content[cursor], 0); cursor++; }
		}
		if (lines.Count() == 0) lines.Add({ binary ? WString::Unmanaged(L"Binary file changed.") : WString::Unmanaged(L"No textual changes."), 0 });
	}

/***********************************************************************
GitRepository
***********************************************************************/

	GitRepository::GitRepository(const FilePath& directory)
		: root(directory)
	{
		root = FilePath(TrimLineEnding(Read({ WString::Unmanaged(L"rev-parse"), WString::Unmanaged(L"--show-toplevel") })));
	}

	FilePath GitRepository::GetRoot() const
	{
		return root;
	}

	CommandResult GitRepository::Run(const List<WString>& arguments) const
	{
		auto full = MakeArguments({ WString::Unmanaged(L"--no-pager"), WString::Unmanaged(L"--no-optional-locks"), WString::Unmanaged(L"--literal-pathspecs"), WString::Unmanaged(L"-c"), WString::Unmanaged(L"color.ui=false"), WString::Unmanaged(L"-c"), WString::Unmanaged(L"core.quotepath=false"), WString::Unmanaged(L"-c"), WString::Unmanaged(L"i18n.logOutputEncoding=utf-8"), WString::Unmanaged(L"-c"), WString::Unmanaged(L"credential.interactive=false"), WString::Unmanaged(L"-C"), root.GetFullPath() });
		for (auto&& argument : arguments) full.Add(argument);
		return RunCliCommand(WString::Unmanaged(L"git"), full);
	}

	CommandResult GitRepository::Run(std::initializer_list<WString> arguments) const
	{
		return Run(MakeArguments(arguments));
	}

	WString GitRepository::Read(std::initializer_list<WString> arguments) const
	{
		auto result = Run(arguments);
		if (result.exitCode != 0) throw Exception(WString::Unmanaged(L"git ") + *arguments.begin() + WString::Unmanaged(L" failed (") + itow(result.exitCode) + WString::Unmanaged(L"): ") + result.error + result.output);
		return result.output;
	}

	WString GitRepository::CurrentBranch() const
	{
		auto result = Run({ WString::Unmanaged(L"symbolic-ref"), WString::Unmanaged(L"--quiet"), WString::Unmanaged(L"HEAD") });
		if (result.exitCode == 1) return {};
		if (result.exitCode != 0) throw Exception(result.error);
		auto reference = TrimLineEnding(result.output);
		return reference.Sub(11, reference.Length() - 11); // refs/heads/
	}

	void GitRepository::Branches(List<WString>& branches) const
	{
		auto current = CurrentBranch();
		if (current.Length()) branches.Add(current);
		else branches.Add(WString::Unmanaged(L"(detached HEAD)"));
		List<WString> records;
		SplitRecords(Read({ WString::Unmanaged(L"for-each-ref"), WString::Unmanaged(L"--format=%(refname:lstrip=2)"), WString::Unmanaged(L"refs/heads/") }), L'\n', records);
		for (auto&& record : records)
		{
			auto branch = TrimLineEnding(record);
			if (branch != current) branches.Add(branch);
		}
	}

	void GitRepository::Changes(List<GitFile>& files) const
	{
		ParseStatus(Read({ WString::Unmanaged(L"status"), WString::Unmanaged(L"--porcelain=v1"), WString::Unmanaged(L"-z"), WString::Unmanaged(L"--untracked-files=all"), WString::Unmanaged(L"--ignore-submodules=none") }), files);
	}

	void GitRepository::History(const WString& branch, List<GitCommit>& commits) const
	{
		auto reference = branch == WString::Unmanaged(L"(detached HEAD)") ? WString::Unmanaged(L"HEAD") : WString::Unmanaged(L"refs/heads/") + branch;
		auto exists = Run({ WString::Unmanaged(L"rev-parse"), WString::Unmanaged(L"--verify"), WString::Unmanaged(L"--quiet"), reference });
		if (exists.exitCode == 1 && branch == CurrentBranch()) return; // Unborn branch.
		if (exists.exitCode != 0) throw Exception(WString::Unmanaged(L"Cannot resolve branch: ") + branch);
		List<WString> records;
		SplitRecords(Read({ WString::Unmanaged(L"log"), WString::Unmanaged(L"-z"), WString::Unmanaged(L"--format=%H%x00%s%x00%an%x00%cI"), TrimLineEnding(exists.output), WString::Unmanaged(L"--") }), 0, records);
		if (records.Count() % 4) throw Exception(WString::Unmanaged(L"Malformed Git history."));
		for (vint i = 0; i < records.Count(); i += 4) commits.Add({ records[i], records[i + 1], records[i + 2], records[i + 3] });
	}

	void GitRepository::CommitFiles(const WString& commit, List<GitFile>& files) const
	{
		List<WString> records;
		SplitRecords(Read({ WString::Unmanaged(L"diff-tree"), WString::Unmanaged(L"--root"), WString::Unmanaged(L"--diff-merges=first-parent"), WString::Unmanaged(L"--no-commit-id"), WString::Unmanaged(L"--name-status"), WString::Unmanaged(L"-r"), WString::Unmanaged(L"-z"), WString::Unmanaged(L"-M"), commit, WString::Unmanaged(L"--") }), 0, records);
		for (vint i = 0; i < records.Count();)
		{
			GitFile file;
			file.kind = FileKind::History;
			file.status = records[i++];
			if (!file.status.Length() || i == records.Count()) throw Exception(WString::Unmanaged(L"Malformed Git file list."));
			file.path = records[i++];
			if (file.status[0] == L'R' || file.status[0] == L'C')
			{
				file.oldPath = file.path;
				if (i == records.Count()) throw Exception(WString::Unmanaged(L"Missing Git rename destination."));
				file.path = records[i++];
			}
			files.Add(file);
		}
	}

	List<DiffLine> GitRepository::Diff(const GitFile& file, const WString& commit) const
	{
		List<WString> arguments;
		if (file.kind == FileKind::History)
		{
			arguments = MakeArguments({ WString::Unmanaged(L"show"), WString::Unmanaged(L"--format="), WString::Unmanaged(L"--root"), WString::Unmanaged(L"--first-parent"), WString::Unmanaged(L"--diff-merges=first-parent") });
		}
		else arguments.Add(WString::Unmanaged(L"diff"));
		CopyFrom(arguments, MakeArguments({ WString::Unmanaged(L"--no-ext-diff"), WString::Unmanaged(L"--no-textconv"), WString::Unmanaged(L"--no-color"), WString::Unmanaged(L"--unified=0"), WString::Unmanaged(L"--inter-hunk-context=0"), WString::Unmanaged(L"--submodule=short"), WString::Unmanaged(L"-M") }), true);
		if (file.kind == FileKind::Staged) arguments.Add(WString::Unmanaged(L"--cached"));
		if (file.kind == FileKind::Unstaged) arguments.Add(WString::Unmanaged(L"--ours"));
		if (file.kind == FileKind::Untracked) arguments.Add(WString::Unmanaged(L"--no-index"));
		if (file.kind == FileKind::History) arguments.Add(commit);
		arguments.Add(WString::Unmanaged(L"--"));
		if (file.kind == FileKind::Untracked)
		{
#ifdef VCZH_MSVC
			arguments.Add(WString::Unmanaged(L"NUL"));
#else
			arguments.Add(WString::Unmanaged(L"/dev/null"));
#endif
		}
		else if (file.oldPath.Length()) arguments.Add(file.oldPath);
		arguments.Add(file.path);
		auto result = Run(arguments);
		if (result.exitCode != 0 && !(file.kind == FileKind::Untracked && result.exitCode == 1)) throw Exception(result.error + result.output);
		List<DiffLine> lines;
		List<WString> records;
		SplitRecords(result.output, L'\n', records);
		bool hasHunk = false;
		for (auto&& record : records)
		{
			if (record.Length() >= 3 && record.Left(3) == WString::Unmanaged(L"@@ ")) { hasHunk = true; break; }
			if (record.Length() >= 7 && record.Right(7) == WString::Unmanaged(L" 160000"))
			{
				lines.Add({ WString::Unmanaged(L"Submodule revision or working tree changed."), 0 });
				return lines;
			}
		}
		WString newText;
		if (hasHunk)
		{
			auto deleted = file.kind == FileKind::History || file.kind == FileKind::Staged ? file.status[0] == L'D' : file.status.Length() == 2 && file.status[1] == L'D';
			if (!deleted)
			{
				if (file.kind == FileKind::History || file.kind == FileKind::Staged)
				{
					auto reference = (file.kind == FileKind::History ? commit : WString::Empty) + WString::Unmanaged(L":") + file.path;
					newText = Read({ WString::Unmanaged(L"cat-file"), WString::Unmanaged(L"blob"), reference });
				}
				else
				{
					stream::BomEncoder::Encoding encoding;
					bool containsBom;
					if (!File(root / file.path).ReadAllTextWithEncodingTesting(newText, encoding, containsBom))
						throw Exception(WString::Unmanaged(L"Cannot read file: ") + file.path);
					// Git includes a UTF-8 BOM in its first source line.
					if (containsBom && encoding == stream::BomEncoder::Utf8) newText = WString::Unmanaged(L"\xFEFF") + newText;
				}
			}
		}
		ParseDiff(result.output, newText, lines);
		return lines;
	}

	CommandResult GitRepository::Pull(const WString& branch, bool rebaseOnConflict) const
	{
		if (!branch.Length()) return { 1, {}, WString::Unmanaged(L"Select a branch before pulling.") };
		auto currentBranch = CurrentBranch();
		if (!currentBranch.Length()) return { 1, {}, WString::Unmanaged(L"HEAD is detached. Check out a branch in another terminal, then REFRESH. No pull was performed.") };
		if (branch != currentBranch)
		{
			return { 1, {}, WString::Unmanaged(L"Selected branch '") + branch + WString::Unmanaged(L"' does not match the checked-out branch '") + currentBranch + WString::Unmanaged(L"'. Select the checked-out branch or REFRESH. No pull was performed.") };
		}
		for (auto name : { WString::Unmanaged(L"MERGE_HEAD"), WString::Unmanaged(L"rebase-merge"), WString::Unmanaged(L"rebase-apply"), WString::Unmanaged(L"CHERRY_PICK_HEAD"), WString::Unmanaged(L"REVERT_HEAD") })
		{
			auto path = root / TrimLineEnding(Read({ WString::Unmanaged(L"rev-parse"), WString::Unmanaged(L"--git-path"), name }));
			if (File(path).Exists() || Folder(path).Exists()) return { 1, {}, WString::Unmanaged(L"Finish the existing Git operation in another terminal before pulling.") };
		}
		if (Read({ WString::Unmanaged(L"status"), WString::Unmanaged(L"--porcelain=v1"), WString::Unmanaged(L"--untracked-files=all") }).Length()) return { 1, {}, WString::Unmanaged(L"Commit or stash local changes in another terminal before pulling.") };
		auto result = Run({ WString::Unmanaged(L"-c"), WString::Unmanaged(L"pull.ff=true"), WString::Unmanaged(L"-c"), WString::Unmanaged(L"merge.autoStash=false"), WString::Unmanaged(L"pull"), WString::Unmanaged(L"--no-rebase"), WString::Unmanaged(L"--no-edit"), WString::Unmanaged(L"--no-autostash"), WString::Unmanaged(L"origin"), branch });
		if (result.exitCode != 0 && rebaseOnConflict && Read({ WString::Unmanaged(L"ls-files"), WString::Unmanaged(L"--unmerged") }).Length())
		{
			auto aborted = Run({ WString::Unmanaged(L"merge"), WString::Unmanaged(L"--abort") });
			if (aborted.exitCode != 0) return aborted;
			result = Run({ WString::Unmanaged(L"-c"), WString::Unmanaged(L"pull.ff=true"), WString::Unmanaged(L"pull"), WString::Unmanaged(L"--rebase"), WString::Unmanaged(L"--no-autostash"), WString::Unmanaged(L"origin"), branch });
		}
		return result;
	}
}
