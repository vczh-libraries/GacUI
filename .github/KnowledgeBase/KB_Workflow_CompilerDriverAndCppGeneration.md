# Workflow Compiler Driver and C++ Generation

Workflow source code can be compiled into a runtime `WfAssembly`, converted into generated C++ files, or extended with generated RPC wrapper Workflow modules before either of those outputs is produced. The ordinary compiler path and the RPC compiler path share the same `WfLexicalScopeManager` analyzer, but the RPC path has an additional definition-only pass that creates wrapper modules and metadata before the final link.

This document focuses on using the compiler APIs. Logging, file layout, and build integration are intentionally left to the caller.

## Main APIs

- Use `vl::workflow::emitter::Compile` when all inputs are ordinary Workflow source strings and the caller only needs a `vl::workflow::runtime::WfAssembly`.
- Use `vl::workflow::analyzer::WfLexicalScopeManager` directly when the caller needs intermediate compiler state, generated RPC wrapper modules, generated C++ output, or custom progress reporting through `IWfCompilerCallback`.
- Use `vl::workflow::emitter::GenerateAssembly` after `WfLexicalScopeManager::Rebuild` succeeds.
- Use `vl::workflow::analyzer::GenerateModuleRpc` and `vl::workflow::analyzer::GenerateModuleRpcJson` after RPC validation succeeds and `manager.rpcMetadata->metadataModule` exists.
- Use `vl::workflow::WfPrint` to turn generated `WfModule` values, such as RPC metadata and wrapper modules, back into Workflow text.
- Use `vl::workflow::cppcodegen::GenerateCppFiles` after the final set of modules has been added to a successfully rebuilt `WfLexicalScopeManager`.
- Use `vl::workflow::cppcodegen::MergeCppMultiPlatform` and `vl::workflow::cppcodegen::MergeCppFileContent` when x86 and x64 generated C++ need to become one architecture-neutral output while preserving user implementation regions.

## Compiler State and Errors

`WfLexicalScopeManager` owns the parser error handler, the parsed modules, all analyzer state, optional RPC metadata, and the `manager.errors` list. Construct it with a `workflow::Parser` and a `WfCpuArchitecture`.

`WfCpuArchitecture` controls Workflow's CPU-sized integer mapping:

- `WfCpuArchitecture::x86` maps CPU-sized signed and unsigned integer types to `vint32_t` and `vuint32_t`.
- `WfCpuArchitecture::x64` maps them to `vint64_t` and `vuint64_t`.
- `WfCpuArchitecture::AsExecutable` maps them to `vint` and `vuint`.

Compiler errors are `glr::ParsingError` values. `error.codeRange.start.row` and `error.codeRange.start.column` are zero-based positions. A user-facing diagnostic normally adds 1 to both values and prints `error.message`.

The following code is a reference example for reading compiler errors. It shows the fields that compiler callers usually need; callers can send the text to any log, UI, exception, or diagnostic system.

```C++
void WriteWorkflowErrors(stream::TextWriter& writer, const collections::List<glr::ParsingError>& errors)
{
	for (auto&& error : errors)
	{
		writer.WriteLine(
			L"Line: " + itow(error.codeRange.start.row + 1) +
			L", Column: " + itow(error.codeRange.start.column + 1) +
			L", Message: " + error.message);
	}
}
```

Check `manager.errors` after every phase that can add diagnostics:

- after `AddModule(const WString&)` or explicit parsing with `ParseModule`;
- after `WfLexicalScopeManager::Rebuild`;
- after `GenerateModuleRpc`;
- after `GenerateModuleRpcJson`;
- after `GenerateCppFiles`.

The convenience `Compile` API copies `manager.errors` to the caller-provided error list and returns `nullptr` when parsing or analysis fails.

## Ordinary Workflow Compilation

For ordinary Workflow source modules, the shortest path is `Compile`. Use the explicit manager path when the caller also needs C++ generation from the same analyzed modules.

The following reference example shows the explicit manager flow for compiling ordinary modules into a `WfAssembly`.

```C++
Ptr<runtime::WfAssembly> CompileWorkflowModules(
	workflow::Parser& parser,
	analyzer::WfCpuArchitecture architecture,
	const collections::List<WString>& moduleCodes,
	collections::List<glr::ParsingError>& errors)
{
	analyzer::WfLexicalScopeManager manager(parser, architecture);
	for (auto&& moduleCode : moduleCodes)
	{
		manager.AddModule(moduleCode);
	}

	if (manager.errors.Count() > 0)
	{
		CopyFrom(errors, manager.errors);
		return nullptr;
	}

	manager.Rebuild(true);
	if (manager.errors.Count() > 0)
	{
		CopyFrom(errors, manager.errors);
		return nullptr;
	}

	return emitter::GenerateAssembly(&manager);
}
```

`WfLexicalScopeManager::Rebuild(true)` keeps reflected type descriptor names in the manager while clearing compiler state for the current module set. During rebuild, the compiler desugars modules, validates structure, builds scopes, validates semantics, populates attributes, and validates RPC declarations by default.

## C++ Generation from Workflow Modules

`GenerateCppFiles` consumes an already analyzed `WfLexicalScopeManager`. It does not parse or rebuild modules by itself. The input object `WfCppInput` controls assembly naming, generated filename stems, file splitting, reflection-file generation, comments, header guard prefix, and includes.

The result is not a single source file. `WfCppOutput::cppFiles` is the complete generated file set, and the caller decides how to store, log, or pass each entry to the C++ build. `WfCppOutput::entryFileName` names the header stem that downstream code should include, but it is not the only generated file.

Generated file groups include:

- the main Workflow translation files: `<defaultFileName>.h` and `<defaultFileName>.cpp`, generated for every successful C++ generation call;
- internal split headers: `<defaultFileName>N.h`, generated when the compiler separates generated classes into additional non-custom headers;
- custom split files: `<CustomFile>.h` and `<CustomFile>.cpp`, generated when Workflow declarations are assigned to explicit C++ file groups;
- the aggregate include header: `<includeFileName>.h`, generated when multi-file output is enabled, and used as `entryFileName` so consumers can include one generated header;
- reflection support files: `<reflectionFileName>.h` and `<reflectionFileName>.cpp`, generated when reflection output is enabled; with `WfCppFileSwitch::OnDemand`, this happens only when the analyzed modules define Workflow types needing reflection registration;
- conditional reflection includes in generated `.cpp` files, so reflection support and caller-provided `reflectionIncludes` can be excluded when `VCZH_DEBUG_NO_REFLECTION` is defined.

`input->normalIncludes` are added to the main generated header. `input->reflectionIncludes` are added to the generated reflection header when reflection files are emitted, or as conditional includes in generated `.cpp` files when the caller supplies reflection dependencies without generating the reflection source pair.

`WfCppOutput::containsReflectionInfo` reports whether the analyzed Workflow modules contain reflection metadata. `WfCppOutput::reflection` reports whether reflection files were actually emitted. These can differ when `input->reflection` is disabled: Workflow types may exist, but no reflection source pair is generated.

The following reference example shows the shape of a C++ generation call after the manager has already been rebuilt successfully.

```C++
Ptr<cppcodegen::WfCppOutput> GenerateWorkflowCpp(
	analyzer::WfLexicalScopeManager& manager,
	const WString& assemblyName,
	const collections::List<WString>& normalIncludes,
	collections::List<glr::ParsingError>& errors)
{
	auto input = Ptr(new cppcodegen::WfCppInput(assemblyName));
	input->multiFile = cppcodegen::WfCppFileSwitch::OnDemand;
	input->reflection = cppcodegen::WfCppFileSwitch::OnDemand;
	for (auto&& include : normalIncludes)
	{
		input->normalIncludes.Add(include);
	}

	auto output = cppcodegen::GenerateCppFiles(input, &manager);
	if (manager.errors.Count() > 0)
	{
		CopyFrom(errors, manager.errors);
		return nullptr;
	}
	return output;
}
```

`WfCppFileSwitch::OnDemand` lets the generator decide whether to emit multi-file output or reflection files:

- multi-file output is enabled when generated/custom file grouping requires it;
- reflection files are emitted when the analyzed modules define Workflow types that need reflection registration.

The caller should consume every `cppFiles` entry, not only the entry header. The following reference example shows the shape of that handoff without assuming where files are written or how generated text is logged.

```C++
void VisitGeneratedCppFiles(cppcodegen::WfCppOutput& output)
{
	for (auto [fileName, index] : indexed(output.cppFiles.Keys()))
	{
		const auto& code = output.cppFiles.Values()[index];
		ConsumeGeneratedCppFile(fileName, code);
	}
}
```

## RPC Wrapper Generation

RPC compilation has two stages:

1. Compile the RPC definition module alone with RPC validation enabled, then generate metadata and wrapper Workflow modules.
2. Add the original definition module, the executable/user modules, and the generated wrapper modules into a fresh final manager, rebuild with RPC validation disabled, and then generate an assembly or C++.

The definition-only stage is necessary because `GenerateModuleRpc` and `GenerateModuleRpcJson` need `manager.rpcMetadata`, which is produced by `ValidateModuleRPC` during `WfLexicalScopeManager::Rebuild`.

The compiler signal for "RPC is involved" is `manager.rpcMetadata`, not a caller-side scan for `@rpc:Interface` text. `WfLexicalScopeManager::Rebuild` first clears `rpcMetadata` to `nullptr`, then runs `ValidateModuleRPC` for each module when `validateRpc` is true. `ValidateModuleRPC` creates `manager.rpcMetadata` only when phase 1 collected at least one valid Workflow RPC interface and no errors have been reported. Metadata generation also fills `manager.rpcMetadata->metadataModule`, and both `GenerateModuleRpc` and `GenerateModuleRpcJson` return `nullptr` if either `rpcMetadata` or `metadataModule` is missing.

After a successful definition-only rebuild with RPC validation enabled, interpret the state as:

- `manager.errors` is non-empty: report diagnostics first; do not decide the compile flow from missing metadata.
- `manager.rpcMetadata && manager.rpcMetadata->metadataModule`: RPC interfaces were found and metadata was generated, so wrapper generation can run.
- `manager.rpcMetadata == nullptr`: no valid Workflow RPC interface was found during this rebuild, so there are no RPC wrapper modules to generate.

This signal belongs to the RPC-validating pass. The final RPC link intentionally calls `Rebuild(..., validateRpc = false)`, so it should not be expected to populate `rpcMetadata`.

`manager.rpcMetadata` contains:

- `metadataModule`, a generated `WfModule` containing serializable enum, struct, and RPC interface declarations;
- `dts`, generated TypeScript schema text for RPC JSON serialization;
- `typeNames`, `methodNames`, and `eventNames`, mapping RPC full names to generated declarations;
- `typeFullNames`, `methodFullNames`, `eventFullNames`, and deterministic `orderedIds`.

The following reference example shows the definition-only stage. It captures generated metadata and wrapper Workflow text with `WfPrint`, but the caller decides what to do with the strings.

```C++
struct RpcGeneratedWorkflow
{
	Ptr<WfModule>	wrapperModule;
	Ptr<WfModule>	wrapperJsonModule;
	WString			metadataWorkflow;
	WString			wrapperWorkflow;
	WString			wrapperJsonWorkflow;
	WString			dts;
};

bool GenerateRpcWorkflowModules(
	workflow::Parser& parser,
	analyzer::WfCpuArchitecture architecture,
	const WString& definitionCode,
	const WString& cppAssemblyName,
	RpcGeneratedWorkflow& generated,
	collections::List<glr::ParsingError>& errors)
{
	analyzer::WfLexicalScopeManager manager(parser, architecture);

	auto definitionModule = ParseModule(definitionCode, parser);
	if (!definitionModule || manager.errors.Count() > 0)
	{
		CopyFrom(errors, manager.errors);
		return false;
	}

	manager.AddModule(definitionModule);
	manager.Rebuild(true);
	if (manager.errors.Count() > 0 || !manager.rpcMetadata || !manager.rpcMetadata->metadataModule)
	{
		CopyFrom(errors, manager.errors);
		return false;
	}

	generated.wrapperModule = analyzer::GenerateModuleRpc(&manager, cppAssemblyName);
	generated.wrapperJsonModule = analyzer::GenerateModuleRpcJson(&manager, cppAssemblyName);
	if (manager.errors.Count() > 0 || !generated.wrapperModule || !generated.wrapperJsonModule)
	{
		CopyFrom(errors, manager.errors);
		return false;
	}

	generated.metadataWorkflow = GenerateToStream([&](stream::StreamWriter& writer)
	{
		WfPrint(manager.rpcMetadata->metadataModule, L"", writer);
	});
	generated.wrapperWorkflow = GenerateToStream([&](stream::StreamWriter& writer)
	{
		WfPrint(generated.wrapperModule, L"", writer);
	});
	generated.wrapperJsonWorkflow = GenerateToStream([&](stream::StreamWriter& writer)
	{
		WfPrint(generated.wrapperJsonModule, L"", writer);
	});
	generated.dts = manager.rpcMetadata->dts;
	return true;
}
```

`GenerateModuleRpc` creates flat RPC Workflow infrastructure, including id constants, `rpc_GetIds`, type-id helpers, object ops, object-event ops, caller-side ops, event listeners, wrapper interfaces, wrapper factories, and wrapper dispatch.

`GenerateModuleRpcJson` creates JSON RPC Workflow infrastructure, including JSON serializers, JSON object ops, JSON object-event ops, and JSON caller-side ops. Flat and JSON generation use the same RPC metadata model.

## Final RPC Link

After wrapper modules are generated, the final compilation should parse the definition code and user/executable code again, then add both generated wrapper modules to the final manager.

The final rebuild should pass `validateRpc = false`. The input definition has already been validated, and the generated wrapper modules are compiler output. Revalidating RPC metadata during the final link is not the intended flow.

The following reference example produces a final `WfAssembly` from a definition module, an executable module, and generated wrapper modules.

```C++
Ptr<runtime::WfAssembly> LinkRpcWorkflowAssembly(
	workflow::Parser& parser,
	analyzer::WfCpuArchitecture architecture,
	const WString& definitionCode,
	const WString& executableCode,
	Ptr<WfModule> wrapperModule,
	Ptr<WfModule> wrapperJsonModule,
	collections::List<glr::ParsingError>& errors)
{
	analyzer::WfLexicalScopeManager manager(parser, architecture);

	auto definitionModule = ParseModule(definitionCode, parser);
	auto executableModule = ParseModule(executableCode, parser);
	if (!definitionModule || !executableModule || manager.errors.Count() > 0)
	{
		CopyFrom(errors, manager.errors);
		return nullptr;
	}

	manager.AddModule(definitionModule);
	manager.AddModule(executableModule);
	manager.AddModule(wrapperModule);
	manager.AddModule(wrapperJsonModule);
	manager.Rebuild(true, nullptr, false);
	if (manager.errors.Count() > 0)
	{
		CopyFrom(errors, manager.errors);
		return nullptr;
	}

	return emitter::GenerateAssembly(&manager);
}
```

The same final manager can be passed to `GenerateCppFiles` to generate C++ for the original Workflow input and both generated RPC wrapper modules.

## Cross-Architecture C++ Output

When generated C++ should be architecture-neutral, run the same logical generation twice:

1. once with `WfCpuArchitecture::x86` and matching x86 reflection metadata;
2. once with `WfCpuArchitecture::x64` and matching x64 reflection metadata.

The caller should require both runs to produce the same generated filename set. For each matching file, call `MergeCppMultiPlatform(code32, code64)`. If there is already an output file containing user implementation regions, call `MergeCppFileContent(existingCode, mergedCode)` before writing the result.

The following reference example shows only the text-level merge. File enumeration and storage are caller policy.

```C++
WString MergeGeneratedCppFile(
	const WString& code32,
	const WString& code64,
	const WString* existingCode)
{
	auto mergedCode = cppcodegen::MergeCppMultiPlatform(code32, code64);
	if (existingCode)
	{
		mergedCode = cppcodegen::MergeCppFileContent(*existingCode, mergedCode);
	}
	return mergedCode;
}
```

`MergeCppMultiPlatform` accepts only expected architecture differences:

- `vint32_t` versus `vint64_t`;
- `vuint32_t` versus `vuint64_t`;
- integer literal suffix differences such as 64-bit `L` or `UL`;
- selected generated `static_cast<::vl::vint32_t>` or `static_cast<::vl::vint64_t>` numeric forms.

Any other difference throws `MergeCppMultiPlatformException` with row and column positions for both inputs. Treat that exception as a generator difference that needs investigation, not as a normal merge conflict.

`MergeCppFileContent` preserves existing user implementation content in generated `USER_CONTENT_BEGIN` / `USER_CONTENT_END` and `USERIMPL` regions. User content that no longer matches a generated region is kept under `UNUSED_USER_CONTENT`, so it can be inspected instead of silently dropped.

## Choosing the Flow

Use the ordinary compile flow when:

- after a successful rebuild with RPC validation enabled, `manager.rpcMetadata` is `nullptr`;
- no generated RPC wrapper modules are needed for the final manager;
- the caller only needs a `WfAssembly`, or C++ generated directly from those modules.

Use the RPC flow when:

- an initial rebuild with RPC validation enabled succeeds and leaves both `manager.rpcMetadata` and `manager.rpcMetadata->metadataModule` non-null;
- generated wrappers, JSON serializers, or `rpc_GetIds` are needed;
- the final assembly or generated C++ must include generated flat or JSON RPC Workflow modules.

Use the x86/x64 merge flow when:

- generated C++ should compile as architecture-neutral code using `vint` and `vuint`;
- the Workflow code or reflected metadata contains CPU-sized integer mappings;
- existing generated C++ files can contain preserved user implementation regions.
