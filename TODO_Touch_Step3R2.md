You are going to make `vl::Variant` reflectable and integrate it into Workflow script, so that to fix the gesture workaround in `<GacUI repo>/TODO_Touch_Step3R1.md`.

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

**IMPORTANT**:
- You must follow the existing pattern to add a new type, coding like the original author is extreme important in this task, your own preference should yield to this.
- You must use any existing classes or functions if a certain feature is already implement, many reflection features have nuance definitions that are difficult to explain, coding in this way prevent you from mis-align with any existing design.

1) Fix VlppReflection

There are a lot of template constructions around building `ITypeInfo`, you should find all necessary places to insert the new `vl::Variant` stuff.

Firstly we need to represent the `vl::Variant` type, we can add `ITypeInfo::Decorator::Variant`, and remember the order of generic arguments are important. `ITypeInfo` will represent `vl::Variant` in this way:
- Decorator: Variant
- TypeInfoHint: Normal
- ElementType: nullptr
- TypeDescriptor: nullptr
- TypeFriendlyName: Variant<...>

A reflectable variant requires all type options are already reflectable. And you should also reject variant as variant type option during reflection at compile time. No need to check in runtime.

Secondly we need boxing value. There are 2 ways to do it.
We only support parameter boxing on variant, value boxing does not support variant, as we should know the actual variant type before unboxing.
This requires you to add `VariantWrappers.h` and `BoxingParameter_Variant.h`.

When reflection is disabled, aka `VCZH_DEBUG_NO_REFLECTION` is used:
For example, if a `Variant<A, B, C>` stores a value of `C`, we should retrieve the value first and then stores into a new wrapper struct `IValueType::TypedBox<vl::reflection::description::StaticVariantBox<C>>`, and the value will be copied into that struct.
And then call `Value::From()` to store that value. The second parameter will be `nullptr` as we don't have access to any `ITypeDescriptor*` with `VCZH_DEBUG_NO_REFLECTION`.

When reflection is enabled, no matter if it is meta only or full reflection:
We will use `IValueType::TypedBox<vl::reflection::description::DynamicVariantBox>` instead, we box the value directly and stores into `DynamicVariantBox`, as well as a `Ptr<ITypeInfo>` of `C`.

To be clear, when a `Variant<A, B, C>` stores a `C` value, the actual value might be better described with `A` or `B`.
For example, casting a `GuiWindow*` to `GuiControl*` and putting it in `Variant<GuiWindow*, GuiControl*>`, you get a `GuiControl*` value storing `GuiWindow*`.
But during reflection, the actual value is not important, the type option it uses is important, so you will either get `StaticVariantBox<GuiControl*>` or `DynamicVariantBox` with `TypeInfoRetriver<GuiControl*>::CreateTypeInfo`.
By doing this, during unboxing, we could still know that we are going to store the value as `GuiControl*` into the parameter value in type `Variant<GuiWindow*, GuiControl*>`.
Such design keeps boxing and unboxing have no unexpected side effect if we use the same variant type in and out.

`StaticVariantBox` and `DynamicVariantBox` will be put in `VariantWrappers.h`.
You are going to implement standard comparison operators for them.
For `StaticVariantBox` we compare the value directly as we know the actual type.
For `DynamicVariantBox` we delegate the comparison to two `Value`.

Technically `StaticVariantBox` and `DynamicVariantBox` should not need reflection for themselves.

You need to involve `Variant` a few types in the unit test so that your test covers everything including boxing, unboxing, type reading, type serialization, type deserialization, etc.

### DETAILS

- Implement in the owning source repositories and propagate generated releases through the existing tools. Do not hand-edit downstream imports or generated files. The new files belong in the existing reflection wrapper/parameter-boxing inventories and CodePack configuration.
- Extend the existing type-category selection, `TypeInfoRetriver`, const/reference handling, parameter accessors, and reflection registration of `ITypeInfo::Decorator`. Use a dedicated Variant type-info node with ordered arguments; `GenericTypeInfo` currently requires an element and cannot simply be constructed with `nullptr`. Keep ordinary `BoxValue<Variant<...>>` and `UnboxValue<Variant<...>>` unsupported.
- Match the saved active option structurally using the existing type-identity conventions: decorators, descriptor identity and ordered generic arguments. Neither `Ptr<ITypeInfo>` identity nor the dynamic descriptor of the payload determines the active option. Preserve existing hint behavior. Separately created or deserialized type-info objects must match.
- Keep the active option even when its payload is null. An outer variant box remains a boxed value when it contains a null pointer or empty nullable value; ordinary `Value::IsNull` must not silently unwrap it. A base-pointer option holding a derived object remains the base-pointer option.
- Both full and metaonly builds use `DynamicVariantBox`; only no-reflection builds use `StaticVariantBox<C>`. `VCZH_DESCRIPTABLEOBJECT_WITH_METADATA` is defined only for full reflection, so it is not the switch for this distinction. Neither wrapper needs its own registered descriptor.
- Parameter-only boxing still needs to work at every typed reflection boundary. Class fields and invocation already use parameter accessors, but struct fields, collection elements and lazy-list adapters currently contain value-boxing paths. Audit `<VlppReflection repo>/Source/Reflection/Metadata/Metadata.h`, `<VlppReflection repo>/Source/Reflection/Wrappers/ContainerWrappers.h`, and `<VlppReflection repo>/Source/Reflection/Boxing/BoxingParameter_Containers.h`; extend the applicable typed paths without making ordinary value boxing accept a variant.
- Reuse existing parameter boxing/unboxing and `Unboxed<T>` ownership conventions for each option. A saved box must not borrow storage from a destroyed or reassigned variant alternative. Native collection options need the existing collection-copy machinery where a C++ copy constructor is unavailable; `TypedBox::Copy()` also requires a valid owned copy. Preserve normal value-copy and pointer-sharing semantics.
- Comparison is a partial ordering, as with `Value` and `IBoxedValue::NotComparable`; do not invent ordering by option index, descriptor name or variant-wrapper address. Preserve the existing underlying pointer-identity semantics where applicable. Not every reflectable option has a native three-way comparison: functions and native collections need deliberate handling using existing comparison/boxing conventions. Do not accidentally compare `Func` values by their implicit boolean conversion or silently restrict the feature to arithmetic/gesture alternatives.
- Audit metadata serialization/deserialization, friendly names, logging, conversion diagnostics and all consumers of `GetTypeDescriptor()` for the new null-descriptor node. Preserve ordered arguments and an explicit absent-descriptor representation; do not invent a descriptor from the first option. The final review comment records the existing assumption that must be corrected.

### VERIFICATION

- Run the prescribed VlppReflection generation/build/test sequence in `<VlppReflection repo>/Project.md` for Win32 and x64, exercising full, metaonly and no-reflection consumers where supported.
- Cover every option in round trips through value, const-reference and reference parameters, returns, class/struct fields, properties, event arguments and typed collections. Include collection/function options, copied boxes, source reassignment/destruction and correct borrowed/owned unboxing lifetimes.
- Check raw/shared pointers, a base-pointer option containing a derived instance, `T` versus `T?`, and distinct null-bearing options. Verify repeated type-info construction and serialized metadata preserve option identity/order, hints and friendly names.
- Verify unsupported direct value boxing and direct nested variants fail at compile time in the applicable modes. Check unreflectable-option rejection during metadata-enabled registration/type retrieval; no-reflection builds do not provide those metadata declarations. Verify ordinary-value/variant and wrong-option unboxing failures use the existing mismatch behavior without crashing while formatting a missing descriptor. Exercise equal, unequal and unordered comparisons, including pointer identity and NaN.

2) Fix Workflow Script

Note that Workflow compiler can only be used when we are in metaonly or full reflection, Workflow script can only be used when we are in full reflection, environment protection are already done, no need to add useless defensive code. During script execution, boxed variant value will always be `DynamicVariantBox`.

Add a new keyword `variant` and use `variant<A, B, C...>` to represent the variant type.
During syntax checking we should deny:
- 0 or 1 parameter
- Duplicate parameter, just by reading the resolved type AST, I think workflow do not have customized type alias right now so it should be fin, but:
  - You should check `string` against `system::String`, etc.
  - You should treat `int[]` equals to `string[]` as container generic types since its actual type is `Ptr<IValueList>`.

Here are all supported operators:
- Comparison: as any two `Value` is comparible, we take the runtime value, no special rule for Workflow.
- Assignment expression: a variant can only be assigned with values in exactly the same variant type (including the parameter order). Type mismatch could be resolved using casting expression.
  - This also mean no implicit type casting is available on variant types.
- Null testing operator: compiler should check if there is at least one type is nullable, execution could grab the value from `DynamicVariantBox` and perform the null testing.
  - Since we don't allow variant inside variant, so we only need to do such casting once instead of a loop. No useless defensive code here, as well as others.
  - The principle through out Workflow project is that, if compiler already checks something, execution don't need to verify.
- Strong casting expression:
  - Casting from variant to trivial type: compiler should make sure such trivial type is exactly inside variant type options, no implicit conversion is allowed here, execution will need to do type testing.
  - Casting from trivial type to variant: compiler should make sure such trivial type is exactly inside variant type options, no implicit conversion is allowed here, execution just box the value.
  - Casting from variant to variant: compiler should make sure two variant has non-zero intersection between two list of type options, no implicit conversion is allowed here, execution will need to do type testing.
- Weak casting expression:
  - Casting from variant to trivial type:
    - When the trivial type is a pointer, compiler should make sure such trivial type is exactly inside variant type options, no implicit conversion is allowed here, execution will need to do type testing.
    - When the trivial type is a nullable type `T?`, compiler should make sure either `T` or `T?` is exactly inside variant type options, no implicit conversion is allowed here, execution will need to do type testing.
  - Execution will leave a null value for failed casting.
- Infer expression:
  - Infer from trivial type to variant: compiler should make sure such trivial type is exactly inside variant type options, no implicit conversion is allowed here, execution just box the value.
  - Infer from variant to variant: compiler should make sure the source variant type options is exactly or a subset of target variant type options, no implicit conversion is allowed here, execution nothing needs to do.

Other operators are not supported, to be clear, this includes type testing expression.

Instruction generation:

If anything could be figured out in compile time, do not generated dynamic operations, e.g. "execution just box the value" might be or not able to generate variant boxing instruction directly.
The current instruction design cannot specify `Ptr<ITypeInfo>`, but we have no choice now and we have to do it, so you will need `_TYPEINFO` to the `APPLY*` macro list.
The variant syntax design only care about the compiler time type, so that when supported operators know one or both side are variant types, it should generate variant specific instructions.
If compiler time sees nothing, for example, casting variant to `object` and then do casting or comparison later, behavior difference is acceptable, for example:
- casting `variant<int, string>` to `int` succeeds if it is actuall an `int`.
- casting `variant<int, string>` to `object` to `int` will always fail in execution because `DynamicVariantBox` cannot be casted.
  - This is easy to understand, because `variant<int>` is not `int`, and `variant<int>` do not have implicit type conversion.

Here are the list of instructions I can imagine, you can do any change if all features cannot be covered:
- `VBox`: turn a value into `DynamicVariantBox`, but the `Ptr<ITypeInfo>` field is already written into the instruction.
- `VUnbox`: Read the `Value` from `DyanmicVariantBox`.
- `VCast(VV|VT|TV)`: 3 type of strong casting.
- `VTryCastVT`: weak casting.

Comparison should not need variant specific instruction.
C++ helper files for generated C++ from workflow script might need to add variant supporting functions.
Visitor pattern is heavily used through out the Workflow project, you should definitely follow this pattern when processing any AST data, otherwise I won't accept the work.

### DETAILS

- Add the type AST and lexer/grammar changes through the normal parser generator. Update type visitors, printing/copying, structure and semantic validation, reflection-to-AST conversion, scope rebuilding, both backends and diagnostics. A check needing resolved aliases/descriptors belongs after resolution, not in a text-only syntax check. Keep semantically invalid but syntactically well-formed cases parseable for the analyzer tests.
- Distinguish duplicate-option validation from exact Workflow type identity. Reject options that collapse to the same generated native alternative, including `int[]` and `string[]`; do not globally change `IsSameType` to consider these ordinary Workflow types equal. For assignment, membership, intersection and subset checks, retain resolved structural type identity and ordered variant identity. Resolve aliases such as `int` according to the target architecture.
- Follow the existing native type mapping when detecting collisions. `<Workflow repo>/Source/Cpp/WfCpp.cpp` preserves function signatures in `Func` and enumerable element types in `LazyList`, while many other generic containers become interface pointers. Do not indiscriminately erase all generic arguments. Native metadata is authoritative at native boundaries: an untyped `Ptr<IValueList>` option does not acquire `int[]` metadata by inspecting its contents. Test this limitation explicitly rather than silently accepting an exact typed-option match.
- Generated native variants can have the same C++ type even when their Workflow container options differ. Generate cast/infer option mappings from the analyzed Workflow types; do not let C++ type equality or a converting constructor grant a conversion rejected by the analyzer. Reordered/superset conversions may require remapping a native alternative index even when the VM can retain the existing tagged box.
- Apply the no-implicit-conversion rule consistently to initializers, assignment, arguments, returns, overload selection and common-type inference. In `<Workflow repo>/Source/Analyzer/WfAnalyzer_TypeInfo.cpp`, the general conversion-to-`object` shortcut currently precedes decorator dispatch: handle variants before that shortcut, preserving existing nonvariant behavior. An explicit infer expression has the stated variant-specific rules without making the same conversion implicit everywhere.
- Resolve the `object` edge case using exact-option precedence. If `object` is an explicit option, converting an object-typed value to the variant selects that option without inspecting its contents, and casting the variant to `object` extracts only an active `object` option. If `object` is not an option, the explicitly described variant-to-`object` cast is the escape that preserves the outer wrapper. An object-to-variant cast with no exact `object` option is rejected by the stated trivial-to-variant rule; this task does not add a dynamic recovery operation. An escaped wrapper does not become its payload through a later ordinary cast. Document this precedence and the explicit-only escape.
- This has an intentional binding limitation: `<Workflow repo>/Source/Analyzer/WfAnalyzer_ExpandBindExpression.cpp` sends a bind result to `ValueChanged(object)`. A direct variant result cannot use that implicit conversion. Require the authored bind expression to extract a payload or explicitly convert its result to an allowed nonvariant type; otherwise issue a semantic diagnostic. Do not secretly insert an object cast, which would extract the `object` option under the preceding rule, or introduce an internal alternate cast language. Observing/caching a variant while producing a nonvariant bind result remains supported. A complete variant cannot round-trip through the ordinary object-based binding callback under these cast rules.
- Validate casts against the declared active tag, not the payload's dynamic class or non-null value. Strong VT/VV mismatches use the existing catchable cast-failure convention. Weak casts return null on mismatch. For weak `T?`, an active exact `T` is wrapped as nullable and an active exact `T?` is preserved; if both are options, either is accepted without changing the original tag. A typed null can be a successful cast, so null result alone cannot distinguish success from weak failure.
- Variant null testing and accepting a bare null initializer are different operations. Do not globally make `IsNullAcceptableType` accept Variant, because it also validates null literals. `is null`/`is not null` check that at least one option can contain null and unwrap exactly once. Bare `null` does not choose an option; first type it as the desired pointer/nullable/object option, then explicitly cast/infer it into the variant. The same principle applies to other expressions needing an expected type: do not guess an option for an empty collection or an untyped lambda.
- Comparisons involving a statically known variant compare its active payload; unwrap each statically variant operand once and reuse `Value` comparison semantics. Do not require ordered-equal variant types or an intersecting option set merely to compare them. An operand statically typed as `object` retains ordinary object behavior. The current `CompareValue` instruction implements equality only, while the numeric `CompareLiteral` path does not preserve unordered as a separate result. Extend the general comparison machinery/instructions as needed for all six comparison operators, preserving partial/unordered results without adding a variant-specific comparison instruction or changing ordinary nonvariant semantics. Avoid common-type inference introducing an otherwise forbidden variant conversion.
- Other operators must produce compiler diagnostics rather than reach code assuming a non-null descriptor. This includes arithmetic/logical operations, direct indexing/calling/member access, `is T`/`is not T` involving a variant, and variant `typeof`/`type` operations. Reading a variant-valued field, passing it, or extracting an option before using that option's operators remains valid. Reject void options, direct variant options and unsupported pointer/nullable decoration of the variant itself; a collection containing variant elements is a separate, supported typed-container boundary.
- Default values must preserve an active option. Follow native `vl::Variant`: default the first declared option using the existing default-value helper, then box/tag it as that option. Cover both compiler-generated defaults and reflected Workflow struct-field defaults/comparison. `<Workflow repo>/Source/Analyzer/WfAnalyzer_ExpandBindExpression.cpp` and `<Workflow repo>/Source/Runtime/WfRuntimeTypeDescriptor.cpp` currently assume a descriptor when creating defaults; bind, coroutine and state-machine lowering also use these paths.
- Extend every instruction-macro consumer for `_TYPEINFO`, including construction, metadata collection, logging, assembly writing/reading and execution. Collect descriptors reachable through option arguments, including types referenced only by an instruction. Serialize the type graph rather than process pointers, and preserve it after a fresh load. Use `VBox` with compile-time metadata for TV and scalar infer, retaining runtime tests only where the active option is unknown.
- Generated C++ must preserve the same direct variant operator contract in all three reflection modes. Use the existing visitor/helper patterns and parameter adapters; neither native `Variant` index ordering nor comparing outer `StaticVariantBox` objects implements payload comparison. Test mixed numeric and unordered comparisons across backends, since existing low-level `Value` comparison has mode-dependent metadata paths.

### VERIFICATION

The following is an execution requirement, including the additional review instruction: **cover every distinct kind of failure at least once, including both semantic and runtime failures**. Maintain a checklist mapping each rejection/runtime-failure branch introduced or changed by this feature to an indexed fixture and an asserted expected outcome. One test that stops at its first error does not cover later failures in the same sample. Split fixtures or make independently reached assertions as appropriate; extend this list for any additional failure discovered during implementation.

| Failure category | Required negative coverage |
| --- | --- |
| Syntax and arity | Malformed variant syntax; zero options; one option. Keep valid-syntax semantic errors in the analyzer suite. |
| Option validity | Unresolved/unusable option, void, direct nested variant, unsupported variant pointer/nullable decoration. |
| Duplicate identity | Repeated exact option; alias spelling such as `string`/`system::String`; architecture-dependent native integer alias; two generic options mapping to the same native alternative. |
| Implicit conversions | Scalar-to-variant, variant-to-scalar, different/reordered variant assignment, implicit subset widening, and implicit variant-to-`object`. Exercise initializer, assignment, argument, return, overload, common-type and direct variant-result binding contexts so no alternate path bypasses the rule. |
| Strong TV/VT validation | Absent exact option; a numerically convertible option; base/derived pointer conversion; raw/shared conversion; nullable/nonnullable mismatch. Each distinct rejected conversion needs a case. |
| Strong VV validation | Disjoint option sets, including container options that share an erased native type but differ structurally. |
| Weak-cast validation | Nonpointer/nonnullable target; TV and VV forms; absent exact pointer option; `T?` with neither `T` nor `T?` present. |
| Infer validation | Missing scalar option; variant source not a subset; attempted implicit conversion to make an option fit. |
| Missing option selection | Bare null and other target-dependent expressions without an explicitly typed option. |
| Null testing | `is null` and `is not null` on a variant with no null-bearing option. |
| Unsupported operations | Each newly handled rejection path for arithmetic, logical operations, indexing, invocation, member access, type testing and type queries, including a variant appearing only as a target type. |
| Runtime strong VT | A permitted target option is inactive, including a misleading derived runtime object and an inactive option whose payload is null. |
| Runtime strong VV | The variants intersect statically but the active source option is outside the intersection. |
| Runtime weak casts | Failed pointer cast and failed nullable-value cast both return null when a different option is active, including a mismatched null-bearing option. Exact `T` and exact `T?` are both successful active options for weak casting to `T?`. |
| Object boundary | Variant escape followed by ordinary strong payload cast fails; unsupported recovery from object is rejected; explicit object-option extraction fails when a different option is active. |
| Native/generic boundary | An erased native container option does not silently satisfy a typed Workflow option; cover compile-time rejection and any reachable runtime mismatch independently. |
| RPC boundary | Every direct and recursively contained variant rejection described in step 4. |

- Add positive controls beside the failures: every strong/weak/infer direction, reordered VV casts, every member of an intersection and subset, both nullable representations, raw/shared typed nulls, and base-pointer tags containing derived objects. Test null checks with both null and non-null payloads and with a nonnullable option active in a variant that also contains a nullable option.
- Runtime failure fixtures must catch/assert the intended Workflow cast failure and remain able to run the next assertion; an arbitrary compiler crash, internal invariant failure or access violation is not a passing negative test. Weak failures assert null without an exception. Verify exception behavior in both interpreted and generated C++ execution.
- Cover comparisons of equal and differing variant types, variant/scalar in both operand orders, different numeric options, unordered values/NaN, null and pointer payloads, and explicit object escapes. Confirm that nonvariant type equality, numeric conversions, object conversions and existing comparisons have not changed.
- Cover locals, arguments/returns, fields, collection elements, variant caches in bindings with nonvariant results, captured/coroutine values and state-machine storage. Test the documented direct variant-result binding rejection separately. Test first-option defaults for both nullable and nonnullable first options, including struct fields read before explicit assignment. Verify native erased-generic boundaries without inventing metadata that the native declaration does not provide.
- Round-trip parser printing/copying, reflection metadata and serialized assemblies. Load assemblies after discarding the original compiler/type-info objects; check option order/tags and all `_TYPEINFO` operands, including instruction-only references. Compare emitted instruction logs to ensure known TV/infer cases do not perform unnecessary runtime type tests.
- Follow `<Workflow repo>/Project.md`: generate Win32/x64 metadata, compile indexed samples, rebuild generated code, then run RuntimeTest and all three CppTest modes for both architectures. Run the prescribed RPC stdio suites and TypeScript preparation/build. Update expected baselines only after checking that each difference follows the intended semantics, and commit all test-generated tracked outputs.

3) Fix VlppParser2

Sync new syntax back to VlppParser2 so that the `BuiltIn_Workflow` could work.
Syntax files are not shared, just copy them.

### DETAILS

- Mirror the changed lexer, type AST and grammar files from `<Workflow repo>/Source/Parser/Syntax` into `<VlppParser2 repo>/Test/Source/BuiltIn-Workflow/Syntax`. Preserve each repository's generator configuration and namespace instead of copying generated parser files or replacing `Parser.xml`.
- The built-in Workflow parser consumes the indexed Workflow fixtures, including semantic-error cases. Its syntax acceptance and AST shape must remain compatible with the compiler's parser; semantic rejection belongs to Workflow analysis.

### VERIFICATION

- Regenerate Workflow with the prescribed parser tool and run VlppParser2's ordered compiler-generation, rebuild and built-in test sequence from `<VlppParser2 repo>/Project.md`.
- Exercise variant types inside declarations and expressions, parsed semantic-error fixtures, AST JSON serialization/deserialization and printed round trips. Review and commit the generated parser, AST, fixture and TypeScript outputs required by these suites.

4) Fix Workflow RPC

DO NOT support variant for now, treat any variant type as unserializable.

### DETAILS

- Reject variants before generating RPC metadata or serializers, including method parameters/returns, properties and event arguments. Check recursively through collection type arguments, nullable wrappers and struct fields, preserving existing unrelated RPC type rules. `<Workflow repo>/Source/Analyzer/Rpc/WfAnalyzer_ValidateRPC.cpp` currently accepts structs without inspecting their members, so adding one switch case for Variant is insufficient. Use the existing traversal conventions and account for repeated/cyclic metadata where necessary.
- Do not add a dynamic JSON representation for an object containing a variant box. The existing unsupported-value failure must remain a deliberate RPC failure, without dereferencing a missing descriptor or silently serializing the inner payload.

### VERIFICATION

- Add separate reachable rejection cases for direct parameters, returns, properties and event arguments, plus list/dictionary contents, struct fields, nested structs and collections of such structs. Include a positive nonvariant control for each changed recursive validation path.
- Verify any permitted dynamic-object entry path rejects a variant payload at runtime with the existing serialization-failure behavior. Run the normal RPC runtime/generated-C++ suites, stdio tests and TypeScript checks; no variant schema should be generated.

5) Fix GacUI

Remove `GuiPointGestureEventArgs::GetGesture` and add `gesture` to reflection directly because we already support it.
Fix `Touching` and `TouchingList` tab pages to adopt the new variant feature.
This would require fixing knowledge base pages to remove stale facts.

### DETAILS

- The opening capture paragraph is historical context: point capture/release and routing already exist. Preserve them and the stated unsupported synchronous deletion behavior; this task replaces the reflection workaround, not the point-sequence lifecycle.
- This intentionally changes the reflected API from read-only object `Gesture` to the directly registered, writable variant field `gesture`. Remove `GetGesture` and its old registration, preserve the native alternative order, and document the spelling/write-access change. Reading/extracting a value remains a copy; assigning a complete valid variant to `gesture` writes back to the native event argument.
- Replace the old `arguments.Gesture is Payload` tests with nullable weak casts such as `arguments.gesture as GuiPointGesture_Scrolling?`, then the existing `if (var ...)` idiom. Keep handled/selected guards, transformation math, scrolling, raw Action/End delivery, marker data and deferred identity-based list removal unchanged.
- Update `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` and `<GacUI repo>/Test/GacUISrc/UnitTest/TestResource.cpp`, which currently test the old object getter and reject assignment. Amend the canonical `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_PointInput.md` and synchronized copies to remove the workaround and explain the variant API.

### VERIFICATION

- Check reflected field type/order, removal of `Gesture`/`GetGesture`, extraction-copy semantics, valid whole-field write-back and rejection of mismatched assignment. Exercise both gesture alternatives and mutation of `selected` on the original event arguments.
- Follow `<GacUI repo>/Project.md` for Win32/x64 metadata generation, GacUI_Compiler resource generation and subsequent metadata refresh, then unit tests. Run the `Touching` UIA scenarios for CppTest and CppTest_Metaonly and the shared scenario with CppTest_Reflection and GacUI_Host, covering generated C++ in all reflection modes and interpreted Workflow.
- Retain the interaction regressions for manipulation, vertical scrolling, right/partial/full swipes, reversal, final-Up-only motion, cancellation, extra contacts and starts outside rows. Verify deferred deletion occurs exactly once only after successful completion, with item identity/count and useful visual frames. Use ordinary tests for metadata/compiler assertions; do not create empty snapshot cases. Remote/browser consumer checks do not require a point-protocol schema change.

6) Additional works

All files updated by unit test should commit.
When everything is ready, commit and push all local changes first, and then:
- You need to add variant to Workflow's syntax in the website, publish it but ignore `job:copilotInitAll`.

### DETAILS

- Complete the implementation and generated-output commits/pushes before the website phase, following each owning repository's commit separation rules for authored files, generated files and snapshots. The present review only changes this task document.
- Update the authored Workflow type, expression and keyword documentation under `<WebsiteSource repo>/packages/website-doc2/src/articles/workflow/lang`, plus RPC restrictions. Explain exact ordered identity, casts/infer, partial comparison, null/default behavior, object-option precedence, the binding limitation and the exception to the current statement that all values implicitly convert to `object`.
- During the later publication, skip both instruction-initialization calls requested above while retaining the publishing workflow's build/render checks, Markdown export, canonical manual update, publishing and live verification. Preserve the existing published WebAssembly demo unless separately instructed to replace it.

### VERIFICATION

- Check every new language example against the implemented compiler, include successful and failing examples with their reason, and check navigation/links in the generated website.
- Verify publication/CI and the live changed pages; commit and push the documentation and generated publication outputs as required. Record unavailable coverage accurately rather than treating static inspection as an execution pass.

6) Things to verify, if my assumption is incorrect, they should appear in review comments:

- Make sure `ITypeInfo` doesn't need non-null ElementType and TypeDescriptor currently.

## REVIEW COMMENTS

### Existing consumers require a non-null type descriptor

**review comment**: The final assumption is not true of the current implementation. Descriptor leaves already have a null `ElementType`, but existing generic nodes dereference their element, and multiple consumers assume a non-null `TypeDescriptor`. In `<VlppReflection repo>/Source/Reflection/DescriptableInterfaces_Metaonly.cpp`, metadata writing indexes the descriptor and reading retrieves it from the descriptor table; generic friendly-name formatting also expects an element. Constructor logging in `<VlppReflection repo>/Source/Reflection/DescriptableInterfaces_Log.cpp` and boxed-value diagnostics in `<VlppReflection repo>/Source/Reflection/DescriptableValue.cpp` dereference descriptors. Workflow has additional descriptor assumptions in default generation, type conversion, code generation and metadata collection. Simply adding the decorator and template specializations would leave valid variant signatures or failed casts able to crash.

**suggested solution**: Keep the requested Variant representation with null element and descriptor. Add a dedicated node, explicit serialization of the absent descriptor and ordered alternatives, and variant-aware handling at the affected consumers. Use compiler diagnostics for unsupported operations and the existing cast-error behavior for expected runtime failures. Do not supply a fake first-alternative descriptor or silently unwrap ordinary object values. The details and verification above include the necessary audit and round trips. This comment is retained because the task explicitly asks for an incorrect assumption to appear in review comments; the proposed implementation direction is otherwise clear.

Good idea, since we already add a `Variant` decorator, it is clear that the current type is a variant, special code is reasonable.

By the way, here are additional information for workflow casting.
Currently when we cast a `GuiWindow*` value to `GuiControl*`, if it is null, the strong casting won't succeed.
But in variant is different, `variant<GuiWindow*, GuiControl*>` casting to `GuiControl*` could results in null if the variant stores null to the `GuiCOntrol*` slot.
That's how I say type conversion involving or not involving variant are totally different, and adding instructions should be easier instead of messing up existing instructions. And I would expect no or only very limited changes applied to existing instructions' implementation.
