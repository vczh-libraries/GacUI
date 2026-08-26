# VlppParser2 Knowledge Base

Project introduction remains in [Index.md](./Index.md#vlppparser2).

### Choosing APIs

### Design Explanation

#### CodePack Release Amalgamation

CodePack converts categorized source and imported dependency trees into dependency-ordered release code pairs.

- Configure recursive roots, substring-based categories, skipped dependency categories, and flat output basenames in `Release/CodegenConfig.xml`.
- Consume the root `.h/.cpp` pair as distributable source; use the parallel `IncludeOnly` wrappers only for workflows that intentionally compile the owning source layout.
- Preserve source-level conditional dependency annotations, category boundaries, and generated-file ordering when regenerating releases.

[Design Explanation](./KB_VlppParser2_Design_CodePack.md)

#### GlrParserGen Parser Artifact Pipeline

GlrParserGen compiles AST, lexer, and syntax definitions into typed C++ APIs plus embedded lexer and GLR automaton data.

- Define paths, namespaces, AST groups, optional utilities, and the output directory in `Parser.xml`; all paths resolve from that file.
- Compile the core AST, assembler, lexer, and parser pairs together, and add only the generated builder/visitor/JSON families that consumers need.
- Regenerate the complete flat artifact set whenever an input definition or generator contract changes.

[Design Explanation](./KB_VlppParser2_Design_GlrParserGen.md)
