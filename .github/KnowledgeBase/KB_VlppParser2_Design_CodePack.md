# CodePack Inputs, Outputs, and Release Layout

`CodePack` turns a repository's ordinary `.h` and `.cpp` source tree into a small set of dependency-ordered C++ code pairs. The monorepo uses the concatenated pairs as release artifacts and copies owning repositories' generated pairs into downstream `Import` directories. The source tree and `CodegenConfig.xml` remain authoritative; generated `Release` files are not edited by hand.

The command-line entry point is [`Tools/CodePack/CodePack/Main.cpp`](../../Tools/CodePack/CodePack/Main.cpp). It accepts exactly one argument:

```text
CodePack.exe <config-xml>
```

There are no command-line modes or switches. On Windows, the monorepo release scripts normally invoke the last-known-good `Tools/Tools/CodePack.backup.exe`; `CodePack.exe` has the same interface. On Linux and macOS, the locally built binary is `Tools/CodePack/Bin/CodePack`. In every case, pass a repository's `Release/CodegenConfig.xml`.

# Path Model and Configuration

The directory containing the configuration file is the working directory for all configured paths. Every `<folder path="...">` and `<output path="...">` is resolved relative to that directory, not relative to the process's current directory. CodePack normalizes filesystem delimiters for I/O, but converts full paths to backslashes before matching patterns, so the checked-in configurations use backslash path fragments on every platform.

A complete configuration has this shape:

```xml
<codegen>
  <folders>
    <folder path="..\Source"/>
    <folder path="..\Import"/>
    <except pattern="\Resource.xml.log\"/>
  </folders>
  <categories>
    <category name="dependency" pattern="\Import\Dependency."/>
    <category name="core" pattern="\Source\">
      <except pattern=".Windows."/>
      <except pattern=".Linux."/>
    </category>
    <category name="platform" pattern=".Windows.;.Linux."/>
  </categories>
  <output path=".">
    <codepair category="dependency" filename="Dependency" generate="false"/>
    <codepair category="core" filename="MyLibrary" generate="true"/>
    <codepair category="platform" filename="MyLibrary.Platform" generate="true"/>
  </output>
</codegen>
```

The root name is conventionally `codegen`. The implementation reads the named child elements directly and expects all shown attributes to exist.

| Configuration item | Meaning |
| --- | --- |
| `folders/folder@path` | A directory searched recursively. Multiple roots are allowed and duplicate discovered paths are removed. Only filenames ending in `.h` or `.cpp`, case-insensitively, are selected; `.hpp`, `.cc`, and other extensions are not inputs. |
| `folders/except@pattern` | A case-insensitive substring exclusion applied to each discovered file's full, backslash-normalized path. It excludes matching files even though directory recursion itself continues. GacUI uses this to keep generated `*.log` trees out of release packaging. |
| `categories/category@name` | The logical code-pair category assigned to matching files. Repeating the same name in multiple `<category>` elements unions several selections into one category. |
| `categories/category@pattern` | One or more case-insensitive full-path substrings. Separate alternatives with `;`; empty alternatives are ignored. These are substring tests, not globs or regular expressions. |
| `categories/category/except@pattern` | A case-insensitive full-path substring that removes a file from this category's positive selection. |
| `output@path` | The output directory, relative to the configuration file. CodePack also creates an `IncludeOnly` child directory here. |
| `output/codepair@category` | Connects one category to its output basename. Every used category must have exactly one mapping. |
| `output/codepair@filename` | The basename used in generated includes and filenames. Do not add `.h` or `.cpp`. Periods are ordinary filename characters and do not create subdirectories. |
| `output/codepair@generate` | Only the exact string `true` enables output. `false` declares an externally supplied dependency category: CodePack analyzes it and rewrites dependencies to its configured basename, but emits no files for it. An enabled category needs at least one categorized `.cpp`; the implementation unconditionally combines its implementation list. Headers are optional. |

All discovered source files are expected to belong to exactly one logical category. Matching two different category names is rejected. A category can be assembled from multiple same-named declarations, but the same file is added only once to that category. Local quoted includes must resolve to headers covered by the configured folder roots and categories; otherwise dependency analysis reports the included file as uncategorized.

Categories with `generate="false"` are how imported release files participate without being repackaged. Their header basenames are indexed so an input such as `#include <Vlpp.h>` can be recognized as the declared `Vlpp` dependency instead of being treated as an unrelated system header. Header basenames in these skipped categories must therefore be unique.

# Include Scanning and Source Instructions

[`Codepack_GetIncludeFiles.cpp`](../../Tools/CodePack/CodePack/Codepack_GetIncludeFiles.cpp) recognizes only standalone include lines in these forms:

```cpp
#include "relative/path.h"
#include <HeaderName.h>
```

A quoted include is resolved relative to the including file and becomes a dependency on that exact source file. An angle-bracket include becomes a repository dependency only when its basename matches a header in a `generate="false"` category; all other angle-bracket includes remain normal C++ system or library includes.

CodePack also recognizes standalone source annotations:

```cpp
/* CodePack:BeginIgnore() */
/* CodePack:ConditionOff(VCZH_DEBUG_NO_REFLECTION, RelativeHeader.h) */
/* CodePack:EndIgnore() */
```

Their behavior is:

- `BeginIgnore()` starts a region whose ordinary contents and includes are omitted from both generated forms.
- `EndIgnore()` ends that region.
- `ConditionOff(MACRO, relative-file)` records a dependency on the categorized relative file. The instruction is commonly placed inside an ignored region that contains the source-tree include. CodePack replaces it in the generated preamble with `#ifndef MACRO`, an include of the target category's `{filename}.h`, and `#endif`.
- `ConditionOn(MACRO, relative-file)` is also recognized and is meant to produce the corresponding `#ifdef` dependency. There are no current monorepo call sites for it, while `ConditionOff` is exercised extensively. The current implementation performs an additional output-map lookup for `ConditionOn`; verify that path with a focused generator test before introducing the first production use.

Instruction names and arity must match exactly. An unrecognized instruction is printed as an error with file and line, but current CodePack does not abort solely for that diagnostic; because instruction-shaped lines are removed during combination, such a warning must still be treated as a failed release review.

# Dependency Ordering and Text Transformation

CodePack scans transitive includes and builds two dependency graphs:

1. A category graph determines which generated headers each code pair must include. A dependency cycle spanning different categories is rejected.
2. A file graph within each generated header or implementation determines concatenation order. A cycle between files in the same code pair is also rejected.

The existing `IncludeOnly/{filename}.h` or `.cpp` file is read before rewriting. Its source `#include` order is used as a stable ordering preference; newly discovered files fall back to normalized full-path order. Dependency ordering still takes precedence. This makes `IncludeOnly` both an output and the stability seed for the next run.

For each generated code pair, CodePack then applies these transformations:

- Quoted source-tree includes are removed from concatenated text because the referenced file is either concatenated into the same artifact or represented by a generated dependency include.
- Generated category dependencies are written at the top as `#include "{dependency filename}.h"`.
- Angle-bracket includes that are not skipped-category dependencies are retained only at their first occurrence. The system-include set collected while writing a category's header is reused while writing its `.cpp`, so a system include already supplied by the generated header is not repeated in the implementation.
- Source files are separated by banner comments containing their path relative to the common source folder.
- CodePack instructions and ignored regions are removed after their dependency effect has been collected.

# Generated File Organization

For the normal `<codepair generate="true">` category containing both headers and implementations, the output is flat except for one fixed mirror directory:

```text
{output path}/
|-- {filename}.h
|-- {filename}.cpp
`-- IncludeOnly/
    |-- {filename}.h
    `-- {filename}.cpp
```

Several enabled categories place several such quartets in the same output directory. If an enabled category contains no categorized headers, CodePack skips both `.h` outputs and writes only `{filename}.cpp` and `IncludeOnly/{filename}.cpp`; the generated implementation receives category dependencies directly instead of including its own header. The implementation calls the `.cpp` combiner unconditionally and that combiner requires at least one path, so a header-only enabled category is not a supported successful layout. A `generate="false"` category produces no files.

| Generated file | Contents | Purpose and consumer |
| --- | --- | --- |
| `{filename}.h` | When the category has headers: dependency includes followed by the dependency-ordered contents of every categorized `.h`, with local includes removed and system includes deduplicated. | This is the distributable public amalgamated header. Repository releases, the aggregate `Release` repository, and downstream `Import` snapshots consume it. Include it from client code in place of the original multi-file header tree. It is absent for an implementation-only category. |
| `{filename}.cpp` | Usually starts with `{filename}.h`, then contains every categorized `.cpp` in dependency order. A category with no headers instead receives its external category dependencies directly. | This is the distributable amalgamated implementation. Compile it once with the matching generated header and the declared dependency pairs. It is copied with the header into downstream `Import` directories. |
| `IncludeOnly/{filename}.h` | When the category has headers: the same generated dependency preamble, followed by relative `#include` directives for the original categorized `.h` files rather than copied header bodies. | This provides the same code-pair boundary while compiling the owning repository's original source files. It is useful for maintenance/source-layout dependency resolution and preserves the source include order for future CodePack runs. It is absent for an implementation-only category and is not copied by the normal cross-repository release/import workflow. |
| `IncludeOnly/{filename}.cpp` | The same generated implementation preamble, followed by relative `#include` directives for the original categorized `.cpp` files. | This is the source-wrapper counterpart to the amalgamated `.cpp`. Compile it only in workflows that intentionally build the original owning source tree through one translation unit. Do not ship or import it as if it were self-contained release source. |

The ordinary dependency synchronization rule copies generated `.h` and `.cpp` files from the output root and explicitly excludes the `IncludeOnly` directory. A repository-specific workflow can consume an `IncludeOnly` pair deliberately, but it must not silently replace the distributable pair.

`CodegenConfig.xml` itself is an input and remains beside these artifacts; CodePack does not generate a manifest, dependency file, binary table, or log directory. Console output is the only run record: it prints every scanned file and reports each successfully written amalgamated path. Automation that needs a durable log must capture stdout and stderr externally.

# Regeneration and Failure Behavior

CodePack rewrites both implementation forms and, when categorized headers exist, both header forms for every enabled category on each successful pass. It writes UTF-8 generated text and creates the output and `IncludeOnly` directories as needed. It does not remove stale files for deleted or newly disabled categories, or obsolete header files after a category becomes implementation-only, so configuration changes that reduce the artifact set require an explicit source-control review and removal of obsolete generated files.

Important failure behavior follows directly from the CLI:

- An invalid argument count prints the usage line and returns success (`0`), so callers must supply exactly one path instead of relying on that exit code as argument validation.
- Malformed configuration XML prints row/column parser errors and returns `1`.
- Missing attributes/elements, unmatched categories, duplicate mappings, missing local includes, or dependency cycles fail through validation checks; some of these checks terminate the process rather than returning a polished diagnostic.
- An unrecognized `CodePack:` source instruction prints an error but does not itself force a nonzero exit, as noted above.
- File I/O has no recovery transaction. Keep generated artifacts under source control and inspect the complete diff after every run.

The production examples under each repository's `Release/CodegenConfig.xml` demonstrate the intended monorepo layering: imported dependency categories use `generate="false"`, owning source categories use `generate="true"`, platform-specific source receives separate code pairs, and all generated pairs share one `Release` output root.
