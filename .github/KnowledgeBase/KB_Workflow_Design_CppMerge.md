# CppMerge Tool

`CppMerge` combines one x86 Workflow-generated C++ file and the corresponding x64 file into one native-width C++ file. It then uses an existing copy of that output, when present, as the source of user-written implementation regions. The executable is a thin file-system wrapper around `vl::workflow::cppcodegen::MergeCppMultiPlatform` and `vl::workflow::cppcodegen::MergeCppFileContent` from [Workflow-Repo/Source/Cpp/WfMergeCpp.cpp](https://github.com/vczh-libraries/Workflow/blob/master/Source/Cpp/WfMergeCpp.cpp).

`CppMerge` operates on one file pair per invocation. Directory enumeration, filename-set validation, stale-file cleanup, and deciding where the stable output tree lives are responsibilities of its caller.

## Command Line

The command has no modes, switches, configuration file, or environment-variable input:

```text
CppMerge <x32-file> <x64-file> <output-file>
```

On Windows the executable is normally named `CppMerge.exe`; the release build on Linux and macOS is named `CppMerge`. Quote every path that can contain spaces.

- `<x32-file>` is a C++ source or header generated for `vl::workflow::analyzer::WfCpuArchitecture::x86`.
- `<x64-file>` is the corresponding file generated from the same logical input and generator settings for `vl::workflow::analyzer::WfCpuArchitecture::x64`.
- `<output-file>` is the stable architecture-neutral file to create or update. Its parent directory must already exist. If the file already exists, its recognized user regions participate in the merge.

Relative paths are resolved by the process working directory. The executable does not inspect filename extensions, require the three basenames to match, or compare sibling files in the input directories. Those are caller conventions, not part of the CLI.

The two inputs must have the same generated filename role and must be textually identical except for the architecture differences listed below. In particular, use the same Workflow modules, C++ generation options, includes, file splitting, and corresponding x86/x64 reflection metadata. Do not put user edits in either staging input; edit only the stable output inside recognized user regions.

`CppMerge` reads each input and an existing output with `filesystem::File::ReadAllTextByBom`:

- UTF-8, UTF-16 little-endian, and UTF-16 big-endian are selected by their BOM.
- A file without a BOM is decoded using the platform MBCS decoder.
- The written output is always UTF-8 with a BOM.

## Processing Order

The executable performs two distinct merges in a fixed order:

1. `MergeCppMultiPlatform(x32Code, x64Code)` compares the two newly generated staging inputs and normalizes only known CPU-width differences.
2. Only after that succeeds, and only when `<output-file>` already exists, `MergeCppFileContent(existingOutput, multiPlatformCode)` moves destination-owned user content into matching regions of the new code.
3. The executable compares the final text with the existing output and writes only when it changed.

User-content markers do not make arbitrary differences between the x86 and x64 staging inputs acceptable. Both staging files must contain the same generated marker layout and generated default bodies. Conversely, the existing destination is not an input to the multi-platform comparison; its generated text outside user regions is replaced during the second phase.

## Required File Organization

The tool itself is file-oriented, so it accepts any organization that supplies one corresponding pair at a time. A caller normally uses parallel staging directories and a separate stable output directory:

```text
generated/
  x32/
    Foo.h
    Foo.cpp
  x64/
    Foo.h
    Foo.cpp
source/
  Foo.h
  Foo.cpp
```

Invoke the tool once for `Foo.h` and once for `Foo.cpp`. Repeat for every generated basename. Before invoking it in a directory loop, verify that the x86 and x64 directory listings are identical. `CppMerge` does not recurse, enumerate, delete an output whose pair disappeared, or report an output that the caller forgot to process.

For example, one Windows invocation in a `GacGen.ps1`-style layout is:

```text
CppMerge.exe "Resource.xml.log\x32\Source\Foo.cpp" "Resource.xml.log\x64\Source\Foo.cpp" "Source\Foo.cpp"
```

The repository contains two representative organizations:

- [Tools-Repo/Tools/GacGen.ps1](https://github.com/vczh-libraries/Tools/blob/master/Tools/GacGen.ps1) enumerates direct children of `<resource-file>.log\x32\Source`, finds the x64 peer at `<resource-file>.log\x64\Source\<same-name>`, reads the stable destination directory from `<resource-file>.log\x32\CppOutput.txt`, and invokes `CppMerge` for that basename. The staging `.log` tree belongs to `GacGen`; it is input to `CppMerge`, not output from `CppMerge`.
- Workflow compiler tests generate matching sets under `Test/Generated/Cpp32` and `Test/Generated/Cpp64` (and the corresponding `CppRpc32` / `CppRpc64` trees), validate the filename sets, and merge into `Test/SourceCppGen` or `Test/SourceCppGenRpc`. These tests call the underlying merge functions directly.

## Architecture Merge

`MergeCppMultiPlatform` compares the complete input strings from left to right. Common text is copied unchanged. Only the following generated differences are accepted:

| x86 text | x64 text | merged text | Purpose |
| --- | --- | --- | --- |
| `vint32_t` | `vint64_t` | `vint` | Restore the platform-sized signed integer type. |
| `vuint32_t` | `vuint64_t` | `vuint` | Restore the platform-sized unsigned integer type. |
| a decimal digit sequence without a suffix | the same digits followed by `L` | the unsuffixed digits | Remove the x64-only signed literal suffix. |
| a decimal digit sequence without a suffix | the same digits followed by `UL` | the unsuffixed digits | Remove the x64-only unsigned literal suffix. |
| `static_cast<::vl::vint32_t>(N)` | `NL` | `static_cast<::vl::vint>(N)` | Normalize the generator's x86 cast versus x64 suffixed-literal form. |
| `N` | `static_cast<::vl::vint64_t>(NL)` | `static_cast<::vl::vint>(N)` | Normalize the inverse generated cast-versus-literal form. |

The cast normalization is deliberately narrow: `N` is a sequence of decimal digits and the accepted text must have the exact generated spelling used by `WfMergeCpp.cpp`. The merger is not a C++ parser and does not normalize whitespace, line endings, include order, arbitrary typedefs, hexadecimal literals, or general preprocessor differences.

The result is therefore either:

- the common generated text, with only the recognized CPU-width spellings normalized; or
- a `MergeCppMultiPlatformException` at the first unsupported difference.

Treat an exception as evidence that the two files do not represent the same logical generation, or that the generator introduced a new architecture-dependent form. Do not manually patch a staging file merely to make the merge pass.

## Preserving User Implementations

After the architecture merge succeeds, the executable checks whether `<output-file>` already exists. If it does, `MergeCppFileContent(existingOutput, newlyMergedCode)` extracts recognized user regions from the existing output and inserts them into matching regions in the new generated text.

All existing text outside recognized user regions is discarded. Consequently, edits outside those regions are not persistent.

### Current `USER_CONTENT` Regions

Current Workflow C++ generation marks editable regions with:

```C++
/* USER_CONTENT_BEGIN(region identity) */
// user-owned lines
/* USER_CONTENT_END() */
```

The generated layouts include:

- `/* USER_CONTENT_BEGIN(custom global declarations) */` in a generated `.cpp` file for declarations or helper definitions needed by user implementations;
- `/* USER_CONTENT_BEGIN(custom members of ::qualified::Class) */` in a generated `.h` file for additional class members;
- `{/* USER_CONTENT_BEGIN(::qualified::Class) */` through `}/* USER_CONTENT_END() */` around a method body generated for `@cpp:UserImpl`.

For global and member regions, the text inside `USER_CONTENT_BEGIN(...)` is the identity. For a generated method-body region whose begin marker immediately follows `{`, the merger also includes the preceding left-trimmed declaration line in the identity. This lets several methods in one class use the same qualified class name without exchanging bodies.

Do not edit, reformat, duplicate, or add trailing text to marker lines. In particular, the end marker must remain at the end of its line. Changing the method declaration can also change a method region's identity; if that happens, the old body is retained as unused content instead of being inserted into a potentially different method.

The current delimiter format does not parse braces inside the body. It preserves complete interior lines until the end marker.

### Legacy `USERIMPL` Regions

`MergeCppFileContent` also recognizes the older line-oriented form beginning with a left-trimmed `USERIMPL(...)` line. The identity consists of that line plus the immediately following header/declaration line. The implementation starts after the following opening-brace line and ends at the matching closing-brace line.

Legacy brace matching only counts lines whose left-trimmed text begins with `{` or `}`; it does not parse C++ strings, comments, or arbitrary inline braces. Keep braces in the generated legacy layout, with structural braces at the beginning of their own lines. New generators should emit `USER_CONTENT_BEGIN` / `USER_CONTENT_END` regions instead.

### Removed and Restored Regions

When an old user region has no identity match in the newly generated file, the merger does not silently delete it. It appends the complete old region, commented out, beneath this in-file marker:

```C++
// UNUSED_USER_CONTENT:
```

This section is part of `<output-file>`; it is not a separate report or sidecar. On a later run, `CppMerge` decodes the commented regions after this marker and considers them again. If a matching region reappears in newly generated code, its user content is restored to that region and removed from the unused section. Regions that still have no match remain commented at the end of the file for manual review.

The exact line `// UNUSED_USER_CONTENT:` is reserved: encountering it makes the region scanner treat the remainder of the existing file as the recovery section. Do not place that line inside active user code. Do not delete the unused section until its preserved implementations are intentionally retired or moved, and do not hand-edit its comment prefixes if automatic restoration is still desired.

## Generated Output and Its Consumer

One successful invocation produces or updates exactly one file: `<output-file>`.

The file contains architecture-neutral generated C++ and is intended to be checked into or compiled from the application's stable source tree. Headers remain headers and translation units remain translation units; `CppMerge` does not create an aggregate header, project file, manifest, dependency file, resource file, or additional source pair.

If the existing output already equals the fully merged result, the executable returns without rewriting it, preserving its timestamp. Otherwise it writes the complete result in UTF-8 with a BOM. It does not use a temporary or transactional file.

`CppMerge` generates no `.log` directory and no log, error, warning, mapping, or deployment file. Any `*.log` tree seen in a `GacGen.ps1` workflow is produced by `GacGen` and merely consumed as the x86/x64 staging input organization.

## Diagnostics and Automation Caveats

With any argument count other than exactly three positional arguments, the executable prints:

```text
Usage: CppMerge <x32-file> <x64-file> <output-file>
```

and returns exit code `0`. A wrapper must therefore validate the argument list itself instead of treating this usage path as a failed process.

At the first unsupported x86/x64 difference, `MergeCppMultiPlatformException` records zero-based `row32`, `column32`, `row64`, and `column64` fields. The CLI prints an error with one-based row and column values, then rethrows the exception, so the process terminates abnormally and does not update the output. The message lists the common integer-type and literal-suffix cases; the implementation also supports the narrow `static_cast<::vl::vint>(N)` normalization described above.

The CLI does not explicitly validate file-system operations:

- it ignores the Boolean result of `ReadAllTextByBom`, so an unreadable or missing input is observed as empty text;
- it ignores the Boolean result of `WriteAllText`, so a missing output directory or unwritable destination is not reliably reflected by the process exit code;
- it does not verify that the x86 and x64 filenames, extensions, or directory file sets match.

Automation should preflight both input files and the output parent directory, validate paired filename sets before iteration, stop on an abnormal merge process, and verify that every expected output file exists afterward. There is no success report to parse: a normal successful invocation is silent.

## Executable Versus Library Use

Use the executable when an external build script already has two staging paths and one stable output path. Use the library APIs when the generator already holds both strings in memory, needs its own diagnostics, encoding, directory transaction, or filename-set policy:

1. Call `vl::workflow::cppcodegen::MergeCppMultiPlatform(code32, code64)`.
2. If a stable output exists, call `vl::workflow::cppcodegen::MergeCppFileContent(existingCode, mergedCode)`.
3. Compare with the existing file before writing if stable timestamps matter.

See [Workflow Compiler Driver and C++ Generation](./KB_Workflow_CompilerDriverAndCppGeneration.md#cross-architecture-c-output) for the surrounding compiler API flow and [C++ Code Generation](./manual/workflow/codegen.md#c-code-generation) for the `@cpp:UserImpl` source-language contract.
