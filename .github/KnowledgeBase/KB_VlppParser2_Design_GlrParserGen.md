# GlrParserGen Inputs, Outputs, and Generated Parser Layout

`GlrParserGen` compiles typed AST definitions, one lexer definition, and GLR syntax definitions into ordinary C++ source plus embedded compressed lexer and parser tables. Generated code depends on the VlppParser2 runtime but not on the grammar compiler or the original definition files at application runtime.

The production driver is [VlppParser2-Repo/Tools/GlrParserGen/GlrParserGen/Main.cpp](https://github.com/vczh-libraries/VlppParser2/blob/master/Tools/GlrParserGen/GlrParserGen/Main.cpp). It accepts exactly one argument and has no other modes or switches:

```text
GlrParserGen.exe <config-xml>
```

Monorepo scripts pass an absolute `Parser.xml` path. On Linux and macOS, the locally built equivalent is `Tools/GlrParserGen/Bin/GlrParserGen`. All path-bearing XML values are resolved relative to the directory containing `Parser.xml`, independent of the caller's current directory.

# `Parser.xml` Input

The production XML driver expects one parser, one lexer, one syntax-manager block, and one or more AST groups:

```xml
<Parser name="Workflow">
  <Includes>&lt;VlppGlrParser.h&gt;</Includes>
  <CppNamespace>vl::workflow</CppNamespace>
  <HeaderGuard>VCZH_WORKFLOW_PARSER</HeaderGuard>
  <OutputDir>../Generated</OutputDir>

  <Asts>
    <Ast name="Ast">
      <CppNamespace>vl::workflow</CppNamespace>
      <ReflectionNamespace>system::workflow</ReflectionNamespace>
      <ClassPrefix>Wf</ClassPrefix>
      <File file="Ast/Ast.txt"/>
      <File file="Ast/Expressions.txt"/>
      <BlockedUtilities>
        <Copy/>
      </BlockedUtilities>
    </Ast>
  </Asts>

  <Lexer file="Lexer.txt"/>
  <Syntax name="Parser">
    <File file="Syntax/Expressions.txt"/>
    <File file="Syntax/Module.txt"/>
  </Syntax>
</Parser>
```

Every displayed top-level element is required. At least one `<Ast>` is required, every AST group needs at least one `<File>`, and `<Syntax>` needs at least one `<File>`.

| Configuration item | Meaning and constraints |
| --- | --- |
| `Parser@name` | The global generated name. It prefixes all parser-wide files and generated lexer/class/field identifiers. |
| `Includes` | A nonempty semicolon-separated list placed in generated AST and lexer headers. An item beginning with `<` is emitted as an angle-bracket include; every other item is quoted. Angle brackets must be XML-escaped. Empty items and trailing semicolons do not match the driver's format. |
| `CppNamespace` | The `::`-separated namespace for parser-wide lexer, assembler, and parser declarations. Empty namespace segments are not accepted. |
| `HeaderGuard` | The base for generated header guards. An empty element selects `#pragma once`; a nonempty value is normally used. |
| `OutputDir` | The single output directory. It is resolved relative to `Parser.xml` and created recursively when absent. All generated files are direct children; the generator creates no artifact subdirectories. |
| `Asts/Ast@name` | The AST-group name. It becomes part of that group's filenames. Multiple groups are supported and share one parser-wide assembler. |
| `Ast/CppNamespace` | The namespace containing the group's generated enum and class types. |
| `Ast/ReflectionNamespace` | The namespace used for generated reflection type names. |
| `Ast/ClassPrefix` | Prepended to every generated AST enum/class type in the group; it does not alter the group's filename formula. |
| `Ast/File@file` | An AST definition file path relative to `Parser.xml`. Files are compiled together in listed order into one group. Forward type declarations are supported; a symbol used from another AST file must be `@public`. |
| `Ast/BlockedUtilities` | Optional per-group suppression. Its only accepted children are `Builder`, `Empty`, `Copy`, `Traverse`, and `Json`. `Json` suppresses the C++ pair and its `.d.ts`. Core AST and assembler files cannot be blocked. |
| `Lexer@file` | Exactly one line-oriented lexer definition, relative to `Parser.xml`. Token declaration order fixes token priority and numeric IDs. |
| `Syntax@name` | The generated parser class and the suffix of the syntax output pair. The XML CLI exposes one syntax manager even though the underlying library can host several. |
| `Syntax/File@file` | One or more syntax definition files, relative to `Parser.xml`, compiled into that syntax manager. Rules can be forward-referenced; a cross-file rule must be `@public`. Only rules marked `@parser` receive typed C++ entry points. |

The driver reads the first matching named child for scalar blocks. Do not repeat `<Lexer>` or `<Syntax>` and expect multiple independent outputs.

# Definition-File Inputs

The XML coordinates three separate text languages. Their quick references are [AST definitions](./manual/vlppparser2/ast.md), [lexer definitions](./manual/vlppparser2/lexer.md), and [syntax definitions](./manual/vlppparser2/syntax.md); the current compiler behavior and visibility rules are detailed in [VlppParser2-Repo/doc/GrammarCompilation.md](https://github.com/vczh-libraries/VlppParser2/blob/master/doc/GrammarCompilation.md).

## AST definitions

An AST file declares enums and classes. Class fields can be:

- `token`, generated as `vl::glr::ParsingToken`;
- an enum type;
- a class type, generated as `vl::Ptr<T>`;
- a class array, generated as `vl::collections::List<vl::Ptr<T>>`.

Classes support single inheritance and generated visitor dispatch. `@public` exposes a declaration to other files in the same group. `@ambiguous` requests generated `ToResolve` support so local parse alternatives can be represented in the typed AST. All AST files are parsed first, declarations are created in one pass, and bases/fields are resolved in a second pass, so forward declarations within the group do not depend on file order.

## Lexer definitions

The lexer file is read one line at a time:

```text
TOKEN_NAME:REGEX
discard TOKEN_NAME:REGEX
$FRAGMENT_NAME:REGEX
```

`discard` tokens are recognized but removed before parsing. A previously declared fragment is inserted textually with `{$FRAGMENT_NAME}`; fragments are neither recursive nor forward-referenced and do not add parentheses automatically. Everything after the colon is regex text, so trailing comments are not supported. A full-line `//` comment must begin in column zero in the current reader. Blank or whitespace-only lines are accepted.

Token order is significant: it determines both the generated token enum values and the expression order supplied to the combined regex lexer. Fixed-string regexes also become display text, allowing syntax files to use a matching double-quoted literal. Regexes must belong to the pure regular-language subset accepted by the combined lexer.

## Syntax definitions

Syntax files declare EBNF-like rules over token names, fixed literals, and other rules. Clauses can create an AST class with `as Type`, reuse an object through `!Rule`, assign matched values to fields with `Name:field`, assign enum fields at a clause end, and use alternatives, sequences, optionals, and loops. Partial rules are compile-time grammar fragments. Switch declarations specialize grammar branches before automaton construction.

`@public` controls cross-file visibility. `@parser` asks the generator to expose typed `ParseRule` overloads; an unmarked rule still participates in the automaton but has no public generated wrapper. Clause result types and field assignments are checked against the compiled AST model before any output is written.

# Generation Pipeline

The driver performs these stages in a fixed order:

1. Parse `Parser.xml` with the built-in XML parser and create the shared output manifest.
2. Parse every AST file with the generated `vl::glr::parsergen::TypeParser`, compile every AST group, assign filenames, and generate AST/assembler text in memory.
3. Compile the handwritten lexer-definition format, build the combined regex lexer, and generate lexer text in memory.
4. Parse every syntax file with `vl::glr::parsergen::RuleParser`, resolve and validate it against the AST and tokens, then build the epsilon NFA.
5. Compact and cross-reference the NFA and pack it into `vl::glr::automaton::Executable` plus diagnostic metadata.
6. Generate the typed parser and embedded executable data in memory.
7. Remove blocked utility entries, create `OutputDir`, compare each remaining output with the existing file, and write only changed UTF-8 files.

AST/assembler emission must precede syntax compilation. `vl::glr::parsergen::CppParserGenOutput` in [VlppParser2-Repo/Source/ParserGen_Global/ParserCppGen.h](https://github.com/vczh-libraries/VlppParser2/blob/master/Source/ParserGen_Global/ParserCppGen.h) receives class and field IDs while the assembler header is generated; syntax lowering embeds those exact IDs in AST instructions. The complete contract is described in [VlppParser2-Repo/doc/CodeGeneration.md](https://github.com/vczh-libraries/VlppParser2/blob/master/doc/CodeGeneration.md).

Because filesystem writes happen only in the final stage, a configuration, AST, lexer, syntax, or automaton error does not leave a partially regenerated output set from earlier compilation stages.

# Generated File Organization

Let:

- `{P}` be `Parser@name`;
- `{A}` be one `Ast@name`;
- `{S}` be `Syntax@name`.

With all utilities enabled for one AST group, `OutputDir` contains 19 files:

```text
{OutputDir}/
|-- {P}{A}.h
|-- {P}{A}.cpp
|-- {P}{A}_Builder.h
|-- {P}{A}_Builder.cpp
|-- {P}{A}_Empty.h
|-- {P}{A}_Empty.cpp
|-- {P}{A}_Copy.h
|-- {P}{A}_Copy.cpp
|-- {P}{A}_Traverse.h
|-- {P}{A}_Traverse.cpp
|-- {P}{A}_Json.h
|-- {P}{A}_Json.cpp
|-- {P}{A}_Json.d.ts
|-- {P}_Assembler.h
|-- {P}_Assembler.cpp
|-- {P}_Lexer.h
|-- {P}_Lexer.cpp
|-- {P}{S}.h
`-- {P}{S}.cpp
```

Each additional AST group adds its own 13-file AST family. The assembler, lexer, and syntax pair remain parser-wide. `BlockedUtilities` subtracts only the named group's optional files.

## AST group artifacts

| Generated file | Purpose and usage |
| --- | --- |
| `{P}{A}.h` | Declares the group's enums, typed AST classes and fields, inheritance visitors, optional ambiguity types, reflection declarations, and type-loading surface. Include it wherever code consumes or constructs the typed AST. |
| `{P}{A}.cpp` | Implements visitor dispatch and reflection/type information for the core AST declarations. Compile and link it with every consumer of that AST family. |
| `{P}{A}_Builder.h/.cpp` | Provides fluent `builder::MakeType` wrappers and inherited field setters for programmatic AST construction. Include the header and compile the implementation when builders are used; block `Builder` when this surface is unnecessary. |
| `{P}{A}_Empty.h/.cpp` | Provides a no-op implementation of all generated visitor interfaces, including dispatch hooks for abstract branches. Derive from it when a visitor handles only selected node types; block `Empty` when unused. |
| `{P}{A}_Copy.h/.cpp` | Provides deep-copy visitors for tokens, enums, object fields, arrays, inheritance, and source ranges. Use it before independently rewriting an AST; block `Copy` when no copy operation is needed. |
| `{P}{A}_Traverse.h/.cpp` | Provides recursive inspection with pre-order `Traverse` hooks and post-order `Finishing` hooks over nodes and tokens. Use it for analysis or normalization; block `Traverse` when unused. |
| `{P}{A}_Json.h/.cpp` | Provides a `vl::glr::JsonVisitorBase`-based serializer for concrete AST type tags, optional ranges, tokens, enums, nested nodes, arrays, and nulls. Compile it when C++ code exports AST JSON; block `Json` to omit the entire JSON family. |
| `{P}{A}_Json.d.ts` | Describes the compact JSON representation in TypeScript: enum string unions, concrete `$ast` discriminants, abstract unions, nullable child objects, and arrays. Consume it in TypeScript packages that read JSON emitted by the matching C++ JSON visitor. It is not a C++ input. |

## Shared parser artifacts

| Generated file | Purpose and usage |
| --- | --- |
| `{P}_Assembler.h` | Includes every core AST-group header and declares stable class/field enums, diagnostic name lookups, and the generated `AstInsReceiverBase` subclass. Parser code uses this receiver to turn generic AST instructions into strongly typed object creation and field assignment without reflection. |
| `{P}_Assembler.cpp` | Implements class creation, object/token/enum field setters, ambiguity wrapping, and class/field diagnostic names. Compile and link it with the parser. |
| `{P}_Lexer.h` | Declares the token enum in definition order, token count, discarded-token predicate, token name/display/regex lookup functions, and the serialized-lexer loader. Tools can use the metadata APIs; the generated parser uses the deleter and loader. |
| `{P}_Lexer.cpp` | Implements token metadata and embeds the serialized combined `vl::regex::RegexLexer` as compressed byte-string rows. Compile it with the generated parser; runtime startup deserializes this table instead of recompiling regexes. |
| `{P}{S}.h` | Declares rule start-state values, rule/state diagnostic lookups, the parser-data loader, and class `{S}` derived from `vl::glr::ParserBase`. It exposes string- and token-list-based `ParseRule` overloads only for rules marked `@parser`. This is the normal client entry header. |
| `{P}{S}.cpp` | Embeds the compressed serialized `vl::glr::automaton::Executable`, state labels and rule names, common-base lookup for ambiguity, parser construction callbacks, and each typed parse wrapper. Compile and link it with the assembler and lexer implementations. |

The embedded lexer and executable data are C++ string literals split into 256-byte rows. No separate `.bin`, table, metadata, state-map, or runtime grammar file is produced. Rule names and state labels needed for diagnostics are compiled into `{P}{S}.cpp`.

# Optional Utilities and Stale Outputs

`BlockedUtilities` filters generated strings before the final write:

| Element | Files omitted |
| --- | --- |
| `<Builder/>` | `_Builder.h`, `_Builder.cpp` |
| `<Empty/>` | `_Empty.h`, `_Empty.cpp` |
| `<Copy/>` | `_Copy.h`, `_Copy.cpp` |
| `<Traverse/>` | `_Traverse.h`, `_Traverse.cpp` |
| `<Json/>` | `_Json.h`, `_Json.cpp`, `_Json.d.ts` |

Filtering does not delete an already existing file. Renaming `{P}`, `{A}`, or `{S}`, changing `OutputDir`, removing an AST group, or newly blocking a utility can therefore leave stale generated files on disk. Remove obsolete files explicitly after reviewing the before/after filename manifest; do not assume a successful run performed cleanup.

# Console Output, Errors, and Logs

For each input, the tool prints `Processing <relative path> ...`. At the end it prints every output's absolute path; an interactive console colors unchanged files yellow and newly written files green. Exact content comparison prevents timestamp-only rewrites.

The generator creates no `.log` directory or durable log/manifest file. Its only outputs are the files listed above. If another tool needs generation diagnostics, capture stdout/stderr externally or consume the generated metadata APIs; there is no sidecar protocol comparable to GacGen's resource logs.

Failure reporting is stage-specific:

- Invalid argument count prints the usage line and returns success (`0`), so automation must always pass exactly one path.
- Malformed `Parser.xml` and AST/syntax definition parse failures return `1` with one-based row and column diagnostics.
- Missing required XML nodes/attributes, an absent AST definition file, or an unknown `BlockedUtilities` child returns `1` with a targeted message.
- Semantic errors are tagged with `AstGroup`, `Ast`, `Lexer`, or `Syntax`, source range, error kind, and named arguments, then return `1`.
- Automaton-compaction and packing errors use the same shared semantic error list and stop before file output.
- Lexer and syntax files do not receive the same explicit existence check as AST files in the CLI. Keep all referenced paths valid; an unreadable file will surface through parsing/compilation or underlying I/O rather than the dedicated AST missing-file message.

The tool does not provide rollback around final file I/O and does not clean stale outputs. Keep generated directories under source control and review the entire generated diff.

# Monorepo Production Uses

The current production XML configurations demonstrate three useful utility profiles:

- [Workflow-Repo/Source/Parser/Syntax/Parser.xml](https://github.com/vczh-libraries/Workflow/blob/master/Source/Parser/Syntax/Parser.xml) uses multiple AST and syntax input files and emits every artifact into `Workflow/Source/Parser/Generated`.
- [GacUI-Repo/Source/Compiler/InstanceQuery/Syntax/Parser.xml](https://github.com/vczh-libraries/GacUI/blob/master/Source/Compiler/InstanceQuery/Syntax/Parser.xml) blocks all five optional utility families and emits only the core AST, assembler, lexer, and parser pairs.
- [GacUI-Repo/Source/Compiler/RemoteProtocol/Syntax/Parser.xml](https://github.com/vczh-libraries/GacUI/blob/master/Source/Compiler/RemoteProtocol/Syntax/Parser.xml) keeps JSON/TypeScript output but blocks builder, empty, copy, and traversal visitors.

Those generated directories are compiled by their owning projects and later included in CodePack release amalgamations. Change the definition files or generator, rerun only the affected `Parser.xml`, rebuild the owning project, and commit the input and generated output changes together.
