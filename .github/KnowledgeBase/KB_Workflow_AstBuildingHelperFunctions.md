# AST Building Helper Functions

Workflow generators should prefer building `WfModule` AST and its child nodes directly instead of composing Workflow text and parsing it back. Direct AST generation keeps structured information in C++, avoids source-formatting concerns, and makes it easier to reuse compiler helpers for reflected types, default values, and safe AST cloning.

The exported helpers do not cover every possible `Wf*` node shape. When no helper exists for the exact node being generated, create the specific AST node directly and fill its fields. Use the helpers below for the common cases where Workflow already provides a stable API.

## Build AST From Existing Workflow Text

Use the parsing helpers only when the input is already Workflow source text:

- Use `ParseType` to parse a textual type into a `WfType`.
- Use `ParseExpression` to parse a textual expression into a `WfExpression`.
- Use `ParseStatement` to parse a textual statement into a `WfStatement`.
- Use `ParseCoProviderStatement` to parse a textual co-provider statement into a `WfCoProviderStatement`.
- Use `ParseDeclaration` to parse a textual declaration into a `WfDeclaration`.
- Use `ParseModule` to parse a whole textual module into a `WfModule`.

These functions tokenize the input, invoke the corresponding generated parser entry, and unescape string-related AST content. They are useful at API boundaries where Workflow source text is the input format. They should not be the first choice for generated code when the generator already knows the intended structure.

## Convert Reflected Types To AST

Use reflection-to-AST helpers when generated Workflow code needs to refer to C++ reflected types:

- Use `GetExpressionFromTypeDescriptor` when a generated expression needs a globally qualified type-name expression for an `ITypeDescriptor`.
- Use `GetTypeFromTypeDescriptor` when a generated type annotation should name an `ITypeDescriptor` directly.
- Use `GetTypeFromTypeInfo` when the source of truth is an `ITypeInfo`, including decorated types.

`GetTypeFromTypeInfo` is the safest choice for generated function arguments, local variables, fields, and return types backed by reflection metadata. It handles raw pointers, shared pointers, nullable values, descriptors, and known generic shapes such as enumerable, map, observable-list, and function types. This avoids manually spelling namespace fragments or accidentally choosing the wrong Workflow type wrapper.

## Copy Existing AST

Use copy helpers when a generator needs to reuse an existing AST subtree without aliasing the original node:

- Use `CopyType` to clone a `WfType`.
- Use `CopyExpression` to clone a `WfExpression`.
- Use `CopyStatement` to clone a `WfStatement`.
- Use `CopyDeclaration` to clone a `WfDeclaration`.
- Use `CopyModule` to clone a `WfModule`.

Each copy helper has overloads accepting either `Ptr<T>` or raw `T*`, so callers can use whichever form they already have.

For `CopyExpression`, `CopyStatement`, `CopyDeclaration`, and `CopyModule`, the `expandVirtualExprStat` argument controls whether virtual frontend constructs are copied as written or copied through their expanded ordinary AST:

- Pass `false` when the generated output should preserve the original AST shape.
- Pass `true` when backend-ready AST is needed and expanded expressions, statements, or declarations should replace virtual nodes when available.

`CopyType` does not take `expandVirtualExprStat` because Workflow types do not use the same virtual expansion mechanism.

## Create Default-Value Expressions

Use `CreateDefaultValue` when generated code needs a default expression for an `ITypeInfo`.

`CreateDefaultValue` produces the Workflow expression shape expected by compiler-generated code:

- enum defaults are built as integer `0`, cast through `U8`, then cast to the target enum type;
- strings are built as empty string expressions;
- non-serializable structs are built as `{}` and cast to the struct type;
- booleans are built as `false`;
- numeric primitives are built as `0` inferred as the target type;
- other reference-like values are built as `null` inferred as the target type.

This helper is the right defaulting API for generated fields, cached values, and temporary variables whose types come from reflection metadata.

## Generate Complete RPC Wrapper Modules

Use the RPC module-generation helpers when the goal is a complete generated module rather than individual node construction:

- Use `CopyAndClearRpcMetadata` to clone RPC metadata input and remove declarations that already exist as reflected types.
- Use `GenerateModuleRpc` after RPC validation has populated metadata and the generator needs the normal RPC wrapper module.
- Use `GenerateModuleRpcJson` after RPC validation has populated metadata and the generator needs the JSON-enabled RPC wrapper and serializer module.

These functions return `WfModule` instances. They are high-level module generators, not general-purpose factories for arbitrary AST nodes. Choose them when generating the standard Workflow RPC support modules; choose the lower-level helpers or direct AST construction for other generated code.

## Print Generated AST

Use `WfPrint` after AST generation when generated Workflow needs to be inspected, logged, compared with baselines, or emitted as text. `WfPrint` has overloads for `WfAttribute`, `WfType`, `WfExpression`, `WfStatement`, `WfDeclaration`, and `WfModule`, and supports both parsing writers and text writers.

`WfPrint` is not an AST-creation API. It is the matching output and verification tool after code has already been represented as AST.

## Choosing The Right Helper

- Use direct `Wf*` node construction when generated code is already structured and no exported helper matches the needed node shape.
- Use `ParseType`, `ParseExpression`, `ParseStatement`, `ParseCoProviderStatement`, `ParseDeclaration`, or `ParseModule` only when the input is Workflow text.
- Use `GetExpressionFromTypeDescriptor`, `GetTypeFromTypeDescriptor`, or `GetTypeFromTypeInfo` when the generated AST should follow reflection metadata.
- Use `CopyType`, `CopyExpression`, `CopyStatement`, `CopyDeclaration`, or `CopyModule` when reusing existing AST without sharing nodes.
- Use `CreateDefaultValue` when a generated initializer should match Workflow compiler conventions for a reflected type.
- Use `CopyAndClearRpcMetadata`, `GenerateModuleRpc`, or `GenerateModuleRpcJson` only for complete generated RPC support modules.
- Use `WfPrint` to inspect or serialize generated AST after construction.
