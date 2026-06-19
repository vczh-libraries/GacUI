# Workflow Knowledge Base

Project introduction remains in [Index.md](./Index.md#workflow).

### Choosing APIs

#### Interface-Based RPC Definitions

Workflow interface-based RPC uses `@rpc:*` attributes to decide which Workflow interfaces can cross an RPC boundary, which members are serializable, and how collection values are transported.

- Use `@rpc:Interface` on non-generic interfaces that are intended to be represented as RPC objects.
- Use `@rpc:Ctor` on service interfaces that can be registered as local singleton services and requested remotely.
- Use `@rpc:Byval` and `@rpc:Byref` on collection-valued properties, methods, and parameters when the default collection transport rule is not the desired rule.
- Use `@rpc:Cached` and `@rpc:Dynamic` on properties to control wrapper-side property caching behavior.
- Use RPC serializable primitive, struct, enum, nullable, strong collection, and `@rpc:Interface` pointer types in RPC signatures; do not expose internal transport structs in user-authored RPC APIs.

[API Explanation](./KB_Workflow_InterfaceBasedRpcDefinition.md)

#### JSON Request Routing

Workflow JSON RPC uses `RpcJsonDispatcher`, `RpcJsonLifecycle`, generated JSON ops, and channel-backed dispatcher helpers to route direct method calls, event broadcasts, service declarations, and byval-return cleanup through JSON envelopes.

- Use `RpcJsonDispatcher` and `RpcJsonLifecycle` once per endpoint, configured with generated ids, serializers, JSON object ops, JSON object-event ops, event attachers, and wrapper factories before initialization.
- Use `RpcJsonDispatcherClientForTaskQueue` for endpoint-side channel IO when incoming requests should be processed on a shared `TaskQueue`.
- Use `RpcJsonDispatcherServerForTaskQueue` as the coordinator that tracks connected clients, forwards broadcasts, caches service declarations, and consolidates broadcast responses.
- Use `WaitForServer` or `ConnectLocalServer` with required service type names when client startup must wait for remote service declarations.
- Use `FinalizeRpc()` before shutting down the transport or unloading generated Workflow context.

[API Explanation](./KB_Workflow_JsonRequestRouting.md)

#### Compiler Driver and C++ Generation

Workflow compiler driver APIs let callers parse and analyze Workflow source modules, detect when RPC metadata is generated, create RPC wrapper Workflow modules, generate C++ for the final module set, and merge x86/x64 generated C++ output.

- Use `vl::workflow::emitter::Compile` for ordinary Workflow source strings when a runtime assembly is the only output.
- Use `vl::workflow::analyzer::WfLexicalScopeManager` when compiler state, errors, RPC metadata, generated wrapper modules, or C++ generation are needed.
- Use `vl::workflow::analyzer::GenerateModuleRpc` and `vl::workflow::analyzer::GenerateModuleRpcJson` after a successful RPC-validating rebuild leaves `manager.rpcMetadata->metadataModule` available.
- Use `vl::workflow::WfPrint` to capture generated RPC metadata and wrapper modules as Workflow text.
- Use `vl::workflow::cppcodegen::GenerateCppFiles` to generate the full C++ file set from the final analyzed module set.
- Use `vl::workflow::cppcodegen::MergeCppMultiPlatform` and `vl::workflow::cppcodegen::MergeCppFileContent` to combine x86/x64 generated output and preserve user implementation regions.

[API Explanation](./KB_Workflow_CompilerDriverAndCppGeneration.md)

#### AST Building Helper Functions

Workflow AST building helper functions let code generators construct or reuse `WfModule` AST and child nodes instead of assembling Workflow source text, while still using compiler-owned helpers for reflected types, default-value expressions, complete RPC wrapper modules, and printed output.

- Use direct `Wf*` node construction when generated code is already structured and no exported helper matches the needed node shape.
- Use `ParseType`, `ParseExpression`, `ParseStatement`, `ParseCoProviderStatement`, `ParseDeclaration`, and `ParseModule` only when the input is already Workflow text.
- Use `GetExpressionFromTypeDescriptor`, `GetTypeFromTypeDescriptor`, and `GetTypeFromTypeInfo` when generated AST should follow reflection metadata.
- Use `CopyType`, `CopyExpression`, `CopyStatement`, `CopyDeclaration`, and `CopyModule` when reusing existing AST without sharing nodes.
- Use `CreateDefaultValue` when a generated initializer should match Workflow compiler conventions for a reflected type.
- Use `CopyAndClearRpcMetadata`, `GenerateModuleRpc`, and `GenerateModuleRpcJson` only for complete generated RPC support modules.
- Use `WfPrint` to inspect or serialize generated AST after construction.

[API Explanation](./KB_Workflow_AstBuildingHelperFunctions.md)

### Design Explanation

#### Compiler Rebuild And Desugaring

Workflow compiler analysis is organized around `WfLexicalScopeManager::Rebuild`, a barriered frontend pipeline that validates modules, builds names and scopes, resolves semantics, and rewrites high-level Workflow syntax into ordinary AST before bytecode or C++ generation.

- `Rebuild` separates context-free rewrites, structure checks, global-name construction, scope construction, scope completion, semantic validation, and post-semantic metadata population.
- Context-sensitive virtual AST nodes such as `bind`, `$coroutine`, co-provider statements, and state machines are expanded only after their original nodes have enough resolved type and scope information.
- `bind` lowers to an anonymous `IValueSubscription` implementation; co-provider syntax first lowers to a generated raw `WfNewCoroutineExpression`, and that raw coroutine then lowers to a generated `ICoroutine` implementation; state machines lower to generated class members plus a generated coroutine.
- Backends consume `expanded*` ordinary Workflow AST and treat raw high-level nodes as frontend-only constructs.

[Design Explanation](./KB_Workflow_Design_CompilerRebuildAndDesugaring.md)

#### Attribute System

Workflow script attributes (`@category:name`) are translated to reflected struct types following the naming convention `system::workflow_attributes::att_category_name`. The compiler resolves, evaluates, and populates attributes onto type descriptors during assembly generation, and the binary serialization format preserves attribute metadata across assembly load/save cycles. Predefined `@cpp:*` attributes control C++ code generation behavior.

- The naming convention is implemented by `WfLexicalScopeManager::GetWorkflowAttributeTypeName` in `Source/Analyzer/WfAnalyzer.cpp`.
- Predefined attributes include `@cpp:File`, `@cpp:UserImpl`, `@cpp:Private`, `@cpp:Protected`, and `@cpp:Friend`.
- Assembly population is performed by `PopulateAttributesForDeclarations` in `Source/Emitter/WfEmitter_Assembly.cpp`.
- Binary serialization is handled by `IOAttributeBag` in `Source/Runtime/WfRuntimeAssembly.cpp`.
- C++ code generation emits `ATTRIBUTE_TYPE` / `ATTRIBUTE_MEMBER` macros via `WriteAttributeMacro` in `Source/Cpp/WfCpp_WriteReflection.cpp`.

[Design Explanation](./KB_Workflow_Design_AttributeSystem.md)

#### Generated RPC Wrappers

Workflow RPC wrapper generation turns RPC metadata into generated ids, serializers, object ops, object-event ops, caller-side ops, listener attachers, and wrapper factories that connect application objects to an `IRpcLifecycle`.

- Generated `rpc_GetIds()` provides the shared name-to-id map used by lifecycle setup, service registration, and service lookup.
- Generated object ops receive remote method calls, holds, unholds, and event replays, while generated caller-side ops are used by wrappers to send calls and broadcasts through the dispatcher.
- Generated wrapper factories create remote-object wrappers from `RpcObjectReference` values; wrapper construction and destruction send hold and unhold messages to the owner lifecycle.
- Generated JSON ops reuse the same wrapper classes with JSON caller-side ops, adding JSON serialization around arguments, returns, events, exceptions, and byval collection returns.

[Design Explanation](./KB_Workflow_Design_GeneratedRpcWrappers.md)

#### JSON Serialization Schema

Workflow RPC JSON serialization defines the generated TypeScript schema and Workflow serializer behavior for known static types, unknown reflected values, internal transport structs, dispatcher envelopes, and captured test JSON values.

- Known-type schemas serialize primitives, enums, structs, lists, observable lists, and dictionaries according to the static RPC metadata.
- Unknown-type schemas carry enough type information to reconstruct reflected values for dynamic values and byref collection elements.
- `system::RpcObjectReference`, `system::RpcException`, and `system::RpcByvalReturnValue<T>` are stable internal transport shapes used by generated JSON RPC infrastructure.
- Generated `.d.ts` files expose `KnownTypeSchema`, `UnknownTypeSchema`, concrete enum and struct declarations, and captured `JsonValue_*.ts` validation data.

[Design Explanation](./KB_Workflow_Design_JsonSerializationSchema.md)
