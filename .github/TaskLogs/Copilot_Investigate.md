# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

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
- Ordinary pointer/nullable boxing loses the declared type when the result is `Value::Null`, so retain the separate active-option tag before boxing the payload. During variant unboxing, validate that tag first and then reuse the existing payload parameter accessor, which already accepts a matching null pointer or empty nullable. Do not change ordinary reflection null-unboxing behavior to enforce Workflow cast rules.
- Both full and metaonly builds use `DynamicVariantBox`; only no-reflection builds use `StaticVariantBox<C>`. `VCZH_DESCRIPTABLEOBJECT_WITH_METADATA` is defined only for full reflection, so it is not the switch for this distinction. Neither wrapper needs its own registered descriptor.
- Parameter-only boxing still needs to work at every typed reflection boundary. Class fields and invocation already use parameter accessors, but struct fields, collection elements and lazy-list adapters currently contain value-boxing paths. Audit `<VlppReflection repo>/Source/Reflection/Metadata/Metadata.h`, `<VlppReflection repo>/Source/Reflection/Wrappers/ContainerWrappers.h`, and `<VlppReflection repo>/Source/Reflection/Boxing/BoxingParameter_Containers.h`; extend the applicable typed paths without making ordinary value boxing accept a variant.
- Reuse existing parameter boxing/unboxing and `Unboxed<T>` ownership conventions for each option. A saved box must not borrow storage from a destroyed or reassigned variant alternative. Native collection options need the existing collection-copy machinery where a C++ copy constructor is unavailable; `TypedBox::Copy()` also requires a valid owned copy. Preserve normal value-copy and pointer-sharing semantics.
- Comparison is a partial ordering, as with `Value` and `IBoxedValue::NotComparable`; do not invent ordering by option index, descriptor name or variant-wrapper address. Preserve the existing underlying pointer-identity semantics where applicable. Not every reflectable option has a native three-way comparison: functions and native collections need deliberate handling using existing comparison/boxing conventions. Do not accidentally compare `Func` values by their implicit boolean conversion or silently restrict the feature to arithmetic/gesture alternatives.
- Audit metadata serialization/deserialization, friendly names, logging, conversion diagnostics and all consumers of `GetTypeDescriptor()` for the new null-descriptor node. Preserve ordered arguments and an explicit absent-descriptor representation; do not invent a descriptor from the first option. The final review comment records the existing assumption that must be corrected.

### VERIFICATION

- Run the prescribed VlppReflection generation/build/test sequence in `<VlppReflection repo>/Project.md` for Win32 and x64, exercising full, metaonly and no-reflection consumers where supported.
- Cover every option in round trips through value, const-reference and reference parameters, returns, class/struct fields, properties, event arguments and typed collections. Include collection/function options, copied boxes, source reassignment/destruction and correct borrowed/owned unboxing lifetimes.
- Check raw/shared pointers, a base-pointer option containing a derived instance, `T` versus `T?`, and distinct null-bearing options. Verify repeated type-info construction and serialized metadata preserve option identity/order, hints and friendly names.
- For null-bearing options, assert both the outer non-null boxed value and the saved exact option, not just the extracted null result. Copies, source reassignment, metadata round trips and native parameter/return round trips must preserve the tag even though different options have indistinguishable inner null values.
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
- Strong variant extraction succeeds on an exact active-option match even when the payload is null. For `variant<GuiWindow*, GuiControl*>`, a null in the `GuiControl*` slot strongly extracts as a null `GuiControl*`; a null in the `GuiWindow*` slot fails that same extraction. Apply the same rule to shared-pointer, nullable and object options. TV/scalar infer boxes an already-typed null without ordinary pointer conversion; VV/reordered subset infer preserves the null payload and active tag when that option is allowed by the target.
- Dispatch variant conversions from the static source/target decorators before ordinary cast lowering. Implement the variant rules in new variant instructions and corresponding C++ helpers; do not lower extraction to `VUnbox` followed by an ordinary pointer cast or a non-null check. Native helpers may inspect the active slot using existing `Variant` accessors, but the truthiness of the payload is never the success test. In particular, do not apply `vl::__vwsn::Ensure` to a successfully extracted null option.
- Keep existing instruction implementations and ordinary conversion behavior unchanged wherever possible. New opcode dispatch, `_TYPEINFO` plumbing and narrowly needed handling of descriptorless boxes in existing error paths are appropriate; they must not introduce implicit payload extraction or new numeric/pointer conversion rules. Expected new variant failures use the existing exception-reporting mechanism without inheriting ordinary cast success conditions.
- There is a pre-existing backend difference in the ordinary-null example from the reply. `<Workflow repo>/Source/Cpp/WfCpp_Expression.cpp` emits `vl::__vwsn::Ensure` for ordinary strong pointer conversions, and `<Workflow repo>/Source/Library/WfLibraryCppHelper.h` rejects null there. In contrast, `OPERATOR_OpConvertToType` in `<Workflow repo>/Source/Runtime/WfRuntimeExecution.cpp` currently skips its raw/shared pointer body for a null input and returns success with a null result; `<Workflow repo>/Source/Emitter/WfEmitter_Assembly.cpp` emits that instruction. Treat this as a source-inspection finding to capture in baseline verification, not proof that both backends already reject ordinary null casts. Do not silently harmonize that existing behavior as part of adding variants; the new exact-tag/null rules must agree across backends.
- Variant null testing and accepting a bare null initializer are different operations. Do not globally make `IsNullAcceptableType` accept Variant, because it also validates null literals. `is null`/`is not null` check that at least one option can contain null and unwrap exactly once. Bare `null` does not choose an option; first type it as the desired pointer/nullable/object option, then explicitly cast/infer it into the variant. The same principle applies to other expressions needing an expected type: do not guess an option for an empty collection or an untyped lambda.
- Comparisons involving a statically known variant compare its active payload; unwrap each statically variant operand once and reuse `Value` comparison semantics. Do not require ordered-equal variant types or an intersecting option set merely to compare them. An operand statically typed as `object` retains ordinary object behavior. The current `CompareValue` instruction implements equality only, while the numeric `CompareLiteral` path does not preserve unordered as a separate result. Reuse existing instructions where their semantics fit; otherwise prefer new general `Value` comparison instruction(s) for the missing operations. Preserve the existing `CompareValue` boolean stack contract and the behavior of `CompareLiteral` and its relational consumers. New comparison instructions need not be variant-specific and must preserve partial/unordered results for all six operators. Avoid common-type inference introducing an otherwise forbidden variant conversion.
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
| Runtime strong VT | A permitted target option is inactive, including a misleading derived runtime object and a null payload tagged as a different pointer/nullable/object option. Null in the matching active slot is a success, not a negative case. |
| Runtime strong VV | The variants intersect statically but the active source option is outside the intersection, tested with both non-null and null payloads. |
| Runtime weak casts | Failed pointer cast and failed nullable-value cast both return null when a different option is active, including a mismatched null-bearing option. Exact `T` and exact `T?` are both successful active options for weak casting to `T?`. |
| Object boundary | Variant escape followed by ordinary strong payload cast fails; unsupported recovery from object is rejected; explicit object-option extraction fails when a different option is active. |
| Native/generic boundary | An erased native container option does not silently satisfy a typed Workflow option; cover compile-time rejection and any reachable runtime mismatch independently. |
| RPC boundary | Every direct and recursively contained variant rejection described in step 4. |

- Add positive controls beside the failures: every strong/weak/infer direction, reordered VV casts, every member of an intersection and subset, both nullable representations, raw/shared typed nulls, and base-pointer tags containing derived objects. Test null checks with both null and non-null payloads and with a nonnullable option active in a variant that also contains a nullable option.
- Add paired tagged-null cases: strong extraction of the active null `GuiControl*` succeeds, while extracting `GuiControl*` from an active null `GuiWindow*` fails even when both options are declared. Use corresponding reflected base/derived test types in Workflow's own suite. Repeat the relevant distinctions for shared pointers, empty `T?` and null `object`; verify matching/inactive tags, VV inclusion/exclusion and reordered subset infer. Run new variant cases in the VM and all generated-C++ reflection modes, including after box copies and assembly reloads.
- Runtime failure fixtures must catch/assert the intended Workflow cast failure and remain able to run the next assertion; an arbitrary compiler crash, internal invariant failure or access violation is not a passing negative test. Weak failures assert null without an exception. Verify exception behavior in both interpreted and generated C++ execution.
- Capture ordinary strong/weak raw/shared pointer and nullable casts before implementation, including typed-null inputs, and confirm their behavior is unchanged afterward. Check the existing VM/C++ discrepancy separately using each backend's appropriate expected-result/error test harness; do not mistake it for a new variant regression or silently rewrite old expectations to force agreement.
- Cover comparisons of equal and differing variant types, variant/scalar in both operand orders, different numeric options, unordered values/NaN, null and pointer payloads, and explicit object escapes. Confirm that nonvariant type equality, numeric conversions, object conversions and existing comparisons have not changed.
- Cover locals, arguments/returns, fields, collection elements, variant caches in bindings with nonvariant results, captured/coroutine values and state-machine storage. Test the documented direct variant-result binding rejection separately. Test first-option defaults for both nullable and nonnullable first options, including struct fields read before explicit assignment. Verify native erased-generic boundaries without inventing metadata that the native declaration does not provide.
- Round-trip parser printing/copying, reflection metadata and serialized assemblies. Load assemblies after discarding the original compiler/type-info objects; check option order/tags and all `_TYPEINFO` operands, including instruction-only references. Compare emitted instruction logs to ensure known TV/infer cases do not perform unnecessary runtime type tests.
- Check instruction logs and generated C++ for the separation explicitly: TV/scalar infer uses boxing with the known option, VT/VV uses active-tag validation, and matching-null extraction does not enter ordinary conversion or `Ensure`. Representative nonvariant samples retain their existing instruction sequences and behavior; incidental numeric opcode renumbering is not a semantic change.
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

# UPDATES

## UPDATE

The change is overall good! I have one thing to add, you need to still add test cases, verify Workflow, VlppParser2, GacUI, fix the website, publish the website without running job:copilotInitAll.

Keep the current keyword list and add the new unary operator "variant in X", the precedence design should be the same as the existing 4 unary operators +/-/not/cast. It does `VUnbox`, which is extract the actual value from inside a variant box, and its result type will be `object`. You can add a new `VariantExtractObject` C++ helper in this case.

So we offer users a way to convert `variant` to `object`, but it needs explicit code. And we will see there is no easy way to convert a `variant` to `object` while keeping the variant box in Workflow, so this should not making the script difficult to use.

Do some research to see if my statement is correct, execute it right away if yes.

# TEST [CONFIRMED]

Reproduce missing variant metadata and parameter boxing in VlppReflection first. Capture ordinary Workflow pointer/nullable casts before implementation. The verification matrices in the problem description are acceptance requirements; each semantic/runtime failure must have an independently reached assertion. Implementation, generated-output verification and publication remain pending.

# PROPOSALS

- No.1 Ordered variant metadata and tagged parameter boxes [CONFIRMED]
- No.2 Explicit unary extraction of a variant payload as object

## No.1 Ordered variant metadata and tagged parameter boxes

The first regression in `<VlppReflection repo>/Test/Source/TestBoxing.cpp` builds in Debug x64 with zero warnings/errors and fails at `TypeInfoRetriver<T>::TypeFlag != TypeFlags::NonGenericType`. The completed execution log confirms the classifier routes variants to ordinary value boxing. No production implementation has changed at this checkpoint.

Add a dedicated ordered Variant type-info node, category selection, parameter-only static/dynamic boxes and structural option matching. Preserve null tags and owned alternative copies, including native collections. Extend typed collection/struct boundaries and descriptorless metadata/error handling. Then propagate releases and extend the Workflow AST visitors, semantic diagnostics, dedicated instructions, serialization and native helpers; retain ordinary cast lowering. Complete parser synchronization, recursive RPC rejection, gesture migration and publication in that dependency order.

### CODE CHANGE

Implemented ordered Variant metadata, structural option identity, static/dynamic tagged boxes, owned native collection payloads, parameter-only boxing, typed container/struct boundaries, descriptorless diagnostics and metadata persistence in `<VlppReflection repo>`. Commits `9b1a7ed` and `cb94986` contain the authored and generated changes. Debug Win32 and x64 pass Metadata_Generate (175 cases), Metadata_Test (181 cases) and UnitTest (61 cases). A temporary no-reflection configuration passes six cases on both architectures. Compile probes reject direct BoxValue/UnboxValue, nested options and unregistered options.

The next refinement forwards the owned unboxed variant as an rvalue at value-parameter and field/container assignment boundaries, preserving lvalue references for reference parameters. This is required for alternatives containing noncopyable native collections. Add invocation and collection-kind coverage before retesting. Workflow, parser synchronization, RPC, gesture migration and website publication remain unimplemented; this proposal is not yet confirmed against the full acceptance matrix.
The collection-boundary refinement is committed in `<VlppReflection repo>` as `a14f7f6`, with generated release `3c918e2`. The final run passes 175 generation cases, 182 metadata-only cases and 62 full-reflection cases on both Win32 and x64; the temporary no-reflection configuration passes eight cases on both. CDB identified that the existing SortedList copy constructor transfers storage, so native collection options use CopyFrom even where a copy constructor exists. ObservableList has no native copy/move constructor; its in-place boxing, copies, unboxing and reference invocation are tested, while native by-value invocation is tested for movable collections. No-reflection/metaonly wrong-option and ordinary-value failures now assert their established Error behavior, while full reflection asserts ArgumentTypeMismtatchException.
Before Workflow implementation, add indexed `<Workflow repo>/Test/Resources/Codegen/OrdinaryCastBaseline.txt`: weak typed-null raw/shared/nullable casts and non-null strong casts run in all backends. Its three separate strong-null functions are asserted by the VM harness and by native TEST_EXCEPTION assertions in the generated-C++ consumers. This records the existing null-preserving VM behavior and C++ Ensure failures without changing either lowering path. Run the baseline before importing the variant release.
For generated-C++ variant comparisons, expose CompareVariantPayloads in `<VlppReflection repo>/Source/Reflection/Wrappers/VariantWrappers.h`, reusing the existing boxed numeric comparison matrix independently of reflection metadata. Preserve ordinary Value comparison routing, including its existing no-reflection/metaonly behavior. Tests assert mixed signed/unsigned/floating payload ordering and NaN, alongside the unchanged ordinary comparison baseline. Workflow will invoke this only after unwrapping statically known variant operands once.
Baseline execution found an additional pre-existing ordinary nullable-cast stack issue. CDB stopped in `<Workflow repo>/Source/Runtime/WfRuntimeExecution.cpp` at OpConcat (line 1019 in the baseline): the null branch emitted by GenerateTypeCastInstructions retains the original operand and pushes another null. An inline null nullable cast inside concatenation therefore supplies a null to string serialization. The baseline fixture now returns the weak nullable-cast/null-test result from a separate function, so normal frame return discards the extra operand. Ordinary lowering is unchanged in accordance with the task; this existing issue is recorded, not treated as a passing failure fixture or a variant regression. The new variant instructions must not inherit this stack imbalance.

CompareVariantPayloads and its mode-preservation assertions pass the full Win32/x64 reflection matrices (175/182/62 plus eight no-reflection cases). Authored commit `3719136` and generated release `552283f` are pushed.
Workflow baseline is committed and pushed as `8402987ce`, `5e2b2b249` and `4a6306a44` (authored, text outputs, other generated outputs). Both architectures pass LibraryTest 21/21, metadata generation 2/2, compiler 715/715, runtime 267/267 and each generated-C++ mode 234/234. The three strong-null functions return true in the VM and throw Error in generated C++; weak null and non-null controls agree. RPC stdio and TypeScript remain part of the final full verification.

Begin Workflow implementation by importing the generated reflection release with Import-Project. Add WfVariantType and keyword/grammar support, then regenerate AST/parser visitors. Implement ordered structural identity, native-alternative collision checks, explicit variant conversions, unsupported-operation diagnostics and first-option defaults. Introduce TYPEINFO instruction operands with graph serialization, dedicated boxing/tag-validation operations, and partial Value comparison without changing the old numeric/equality instruction contracts. Native helpers must receive analyzed option-index mappings so erased container types cannot grant forbidden casts. Preserve the baseline functions and their expectations unchanged.
Workflow implementation now adds generated WfVariantType syntax, ordered structural matching, duplicate native-option checks, explicit cast/infer semantics and unsupported-operation diagnostics. VBox stores the declared option metadata; VUnbox extracts one payload; VCast/VTryCast validate tags without ordinary pointer conversion or null checks. TYPEINFO operands serialize their ordered type graph and contribute reachable descriptors. General CompareValueLT/GT/LE/GE preserve partial ordering while existing equality and numeric instructions stay unchanged. First-option defaults are added to compiler lowering and reflected struct fields. C++ helpers receive analyzed option mappings and throw catchable Exception on a mismatched active option. The first compilation checkpoint precedes adding the full indexed variant fixture matrix; no variant acceptance claim is made yet.

Initial Workflow fixtures: VariantCasts asserts exact scalar/variant casts and infers, mismatched active tags, raw/shared typed-null pairs, nullable T/T? handling, object-option precedence, copies and pointer payload comparisons. VariantComparison asserts all six operators, mixed operand types, nulls, explicit object escapes and NaN. VariantStorage covers fields, defaults (including an option struct), value copies, typed lists/dictionaries and captures. IndexInvalidType adds three malformed forms; independently indexed AnalyzerError fixtures cover arity, option validity, native collisions, implicit conversions, explicit/infer/weak failures and unsupported operations. Struct dependency analysis now descends through variant options, rejecting recursive value layouts before default construction or C++ generation.

The native Workflow fixture type VariantProvider exposes value/const-reference/reference echoes, a variant field/property/event, typed variant enumeration, function/lazy-list options, an empty nullable option and an erased List pointer option. VariantNative verifies these boundaries, including runtime rejection when an erased native option is cast into a structurally typed Workflow list option. VariantBinding produces a scalar while caching a variant; VariantCoroutine and VariantStateMachine retain variant state across suspension/input boundaries. VariantInstructionTypes places PointClass pointer metadata only in a VCast target graph, exercising instruction-driven descriptor collection after assembly reload. VariantStorage also checks an object-first default: an absent value descriptor creates a tagged null payload.

Workflow verification adds native harness coverage for the ordinary cast after a variant-to-object escape, asserting the existing full-reflection mismatch exception or metaonly/no-reflection Error. The VM harness asserts a nonfatal cast exception. A separate fresh-assembly-load test checks independent type-info objects, exact ordered options including an instruction-only PointClass reference, and one VBox plus two VCast operands. Win32 LibraryTest passed 21/21 and metadata generation passed 2/2; reviewed metadata changes add only the Variant decorator and VariantProvider signatures.

The indexed VariantNative fixture exposed a lowering gap: foreach obtains an object from IValueEnumerator::GetCurrent and generates an ordinary cast to the statically known element type. Object-to-variant recovery is intentionally rejected, so this generated cast cannot implement typed variant enumeration. The correction retains the known element type on the compiler-generated call AST (an optional result-type annotation with no authored syntax), preserves it through existing generated copy visitors, and uses parameter unboxing in native output while leaving the already-boxed VM value intact. This is limited to the typed collection boundary; authored casts and the direct variant-result binding rejection keep their existing rules. The same audit covers indexed typed list/dictionary reads, whose VM payload is already boxed.

CompilerTest_LoadAndCompile passed 809/809 cases for each target architecture after fixture corrections and typed enumeration lowering. The first VM run passed VariantBinding, VariantCasts (54), VariantComparison (31), VariantCoroutine, VariantInstructionTypes, and VariantIteration before exposing a native test-provider lifetime defect. CDB stopped at a freed WString copy in SelectEnumerator: VariantProvider::Items returned From(values).Evaluate(), which can retain the borrowed native list. The fixture now uses Evaluate(true) as prescribed by LazyList ownership documentation. Runtime failure assertions now require a nonfatal exception whose message identifies a variant option conversion. Generated nested-struct defaults also use the existing native Variant default constructor, avoiding a reference to an alias before that alias is declared.

### Workflow rejection and runtime-failure checklist

Each semantic row below is a separate file in `<Workflow repo>/Test/Resources/AnalyzerError`, registered in `IndexAnalyzerError.txt`. CompilerTest_LoadAndCompile passed every row for x86 and x64. The diagnostic prefix is asserted by the harness; later statements are not counted as independent negative coverage.

| Rejection branch / indexed fixture | Expected diagnostic | Result |
| --- | --- | --- |
| `J0_VariantNoOptions` | `J0` | Passed x86/x64 |
| `J0_VariantOneOption` | `J0` | Passed x86/x64 |
| `J0_VariantDuplicate` | `J0` | Passed x86/x64 |
| `J0_VariantAlias` | `J0` | Passed x86/x64 |
| `J0_VariantErasedList` | `J0` | Passed x86/x64 |
| `J0_VariantErasedDictionary` | `J0` | Passed x86/x64 |
| `J0_VariantErasedUntypedList` | `J0` | Passed x86/x64 |
| `J0_VariantNested` | `J0` | Passed x86/x64 |
| `B0_VariantVoid` | `B0` | Passed x86/x64 |
| `B1_VariantInterfaceValue` | `B1` | Passed x86/x64 |
| `B7_VariantClassValue` | `B7` | Passed x86/x64 |
| `B2_VariantRawDecoration` | `B2` | Passed x86/x64 |
| `B3_VariantSharedDecoration` | `B3` | Passed x86/x64 |
| `B4_VariantNullableDecoration` | `B4` | Passed x86/x64 |
| `A10_VariantScalarInitializer` | `A10` | Passed x86/x64 |
| `A10_VariantScalarExtraction` | `A10` | Passed x86/x64 |
| `A10_VariantReorderedAssignment` | `A10` | Passed x86/x64 |
| `A10_VariantImplicitSubset` | `A10` | Passed x86/x64 |
| `A10_VariantImplicitObject` | `A10` | Passed x86/x64 |
| `A10_VariantReturn` | `A10` | Passed x86/x64 |
| `A21_VariantArgument` | `A21` | Passed x86/x64 |
| `A14_VariantCommonType` | `A14` | Passed x86/x64 |
| `A10_VariantBareNull` | `A10` | Passed x86/x64 |
| `A9_VariantNullSelection` | `A9` | Passed x86/x64 |
| `A9_VariantEmptySelection` | `A9` | Passed x86/x64 |
| `A9_VariantLambdaSelection` | `A9` | Passed x86/x64 |
| `A11_VariantStrongAbsent` | `A11` | Passed x86/x64 |
| `A11_VariantStrongNumeric` | `A11` | Passed x86/x64 |
| `A11_VariantStrongExtractNumeric` | `A11` | Passed x86/x64 |
| `A11_VariantStrongNullablePromotion` | `A11` | Passed x86/x64 |
| `A11_VariantStrongNullableExtraction` | `A11` | Passed x86/x64 |
| `A11_VariantStrongDisjoint` | `A11` | Passed x86/x64 |
| `A11_VariantStrongErasedDisjoint` | `A11` | Passed x86/x64 |
| `A11_VariantObjectRecovery` | `A11` | Passed x86/x64 |
| `A12_VariantWeakScalar` | `A12` | Passed x86/x64 |
| `A12_VariantWeakTV` | `A12` | Passed x86/x64 |
| `A12_VariantWeakVV` | `A12` | Passed x86/x64 |
| `A11_VariantWeakMissingNullable` | `A11` | Passed x86/x64 |
| `A11_VariantInferMissing` | `A11` | Passed x86/x64 |
| `A11_VariantInferNumeric` | `A11` | Passed x86/x64 |
| `A11_VariantInferNotSubset` | `A11` | Passed x86/x64 |
| `A10_VariantNullTest` | `A10` | Passed x86/x64 |
| `A10_VariantNotNullTest` | `A10` | Passed x86/x64 |
| `J1_VariantArithmetic` | `J1` | Passed x86/x64 |
| `J1_VariantLogical` | `J1` | Passed x86/x64 |
| `A16_VariantUnary` | `A16` | Passed x86/x64 |
| `A17_VariantIndex` | `A17` | Passed x86/x64 |
| `A19_VariantInvoke` | `A19` | Passed x86/x64 |
| `J1_VariantMember` | `J1` | Passed x86/x64 |
| `J1_VariantTypeTestSource` | `J1` | Passed x86/x64 |
| `J1_VariantTypeTestTarget` | `J1` | Passed x86/x64 |
| `J1_VariantNotTypeTest` | `J1` | Passed x86/x64 |
| `J1_VariantTypeQuery` | `J1` | Passed x86/x64 |
| `J1_VariantExpressionTypeQuery` | `J1` | Passed x86/x64 |
| `J1_VariantConstructorSelection` | `J1` | Passed x86/x64 |
| `A18_VariantEnumeration` | `A18` | Passed x86/x64 |
| `J0_VariantIntegerAlias` | `J0` | Passed x86/x64 |
| `D13_VariantRecursiveStruct` | `D13` | Passed x86/x64 |
| `A11_VariantStrongBaseToDerived` | `A11` | Passed x86/x64 |
| `A11_VariantStrongDerivedToBase` | `A11` | Passed x86/x64 |
| `A11_VariantStrongRawToShared` | `A11` | Passed x86/x64 |
| `A11_VariantStrongSharedToRaw` | `A11` | Passed x86/x64 |
| `A11_VariantWeakMissingPointer` | `A11` | Passed x86/x64 |
| `A11_VariantWeakPointerOwnership` | `A11` | Passed x86/x64 |
| `A11_VariantNativeUntypedOption` | `A11` | Passed x86/x64 |
| `A16_VariantUnaryPositive` | `A16` | Passed x86/x64 |
| `A16_VariantUnaryNegative` | `A16` | Passed x86/x64 |
| `A14_VariantUnion` | `A14` | Passed x86/x64 |
| `A14_VariantIntersection` | `A14` | Passed x86/x64 |
| `A21_VariantBindingResult` | `A21` | Passed x86/x64 |
| `F2_VariantUnknownOption` | `F2` | Passed x86/x64 |
| `A23_VariantNewClass` | `A23` | Passed x86/x64 |
| `A23_VariantNewInterface` | `A23` | Passed x86/x64 |
| `J1_VariantObserveMember` | `J1` | Passed x86/x64 |
| `J1_VariantSetMembership` | `J1` | Passed x86/x64 |
| `A21_VariantOverload` | `A21` | Passed x86/x64 |
| `B5_VariantChild` | `B5` | Passed x86/x64 |
| `G6_VariantBaseType` | `G6` | Passed x86/x64 |
| `A32_VariantMixinSource` | `A32` | Passed x86/x64 |
| `A32_VariantMixinTarget` | `A32` | Passed x86/x64 |

Malformed syntax is separately indexed in `<Workflow repo>/Test/Resources/IndexInvalidType.txt`: VariantMissingClose, VariantMissingComma and VariantMissingOption each assert a parser error. All three passed for both target architectures. Semantically invalid zero/one-option forms remain parseable and are rejected by J0 above.

| Runtime branch | Indexed fixture / independent assertion | Expected outcome | Final verification |
| --- | --- | --- | --- |
| Inactive scalar VT | VariantCasts: number to string | Nonfatal variant cast exception | VM and all three native modes passed Win32/x64 |
| Active option excluded by VV intersection | VariantCasts: number to variant<string, bool> | Nonfatal variant cast exception | VM and all three native modes passed Win32/x64 |
| T versus T? exact strong extraction | VariantCasts: plain to int?, missing to int | Two independent nonfatal exceptions | VM and all three native modes passed Win32/x64 |
| Different raw null tag | VariantCasts: derivedSlot to VariantBase* and variant<VariantBase*, bool> | Two independent nonfatal exceptions | VM and all three native modes passed Win32/x64 |
| Base raw tag containing derived object | VariantCasts: taggedBase to VariantDerived* | Nonfatal exception; weak counterpart returns null | VM and all three native modes passed Win32/x64 |
| Different shared null tag | VariantCasts: sharedDerived to VariantBase^ and variant<VariantBase^, bool> | Two independent nonfatal exceptions | VM and all three native modes passed Win32/x64 |
| Base shared tag containing derived object | VariantCasts: taggedShared to VariantDerived^ | Nonfatal exception | VM and all three native modes passed Win32/x64 |
| Null object excluded by VV intersection | VariantCasts: objectSlot to variant<int?, bool> | Nonfatal exception | VM and all three native modes passed Win32/x64 |
| Explicit object option inactive | VariantCasts: nullableSlot to object | Nonfatal exception | VM and all three native modes passed Win32/x64 |
| Object payload does not select another option | VariantCasts: opaque to int | Nonfatal exception | VM and all three native modes passed Win32/x64 |
| Weak pointer mismatch, including null | VariantCasts: derivedSlot/sharedDerived/taggedBase weak casts | Null without exception | VM and all three native modes passed Win32/x64 |
| Weak nullable mismatch | VariantCasts: other as int? | Null without exception; exact T and T? controls succeed | VM and all three native modes passed Win32/x64 |
| Escaped wrapper ordinary strong cast | VariantCasts::EscapedPayload, TestRuntime.cpp and TestVariantCasts.cpp harnesses | VM nonfatal cast failure; native existing mismatch exception/Error by mode | VM and all three native modes passed Win32/x64 |
| Native untyped collection versus typed option | VariantNative: erased to variant<int[], string> | Nonfatal exception | VM and all three native modes passed Win32/x64 |
| Erased native alternative does not grant Workflow identity | VariantNative: typedVariant to variant<string[], string> | Nonfatal exception | VM and all three native modes passed Win32/x64 |

Each strong runtime assertion catches and checks a nonfatal exception with a message identifying a variant option conversion, then continues to the next assertion. Matching null tags, ordered remapping, copied tags and both weak nullable representations are independently asserted in the same indexed fixture. RPC rejection coverage will be recorded with task 4.

CDB confirmed the metadata-only iteration failure was caused by the test executable never loading metadata: both the saved string tag and the candidate integer tag contained null descriptor leaves. The metadata-only consumer must load its generated metadata before parameter boxing uses type identity. Add the same base/dependent binary loading used by CompilerTest_LoadAndCompile to CppTest_Metaonly, unload it after tests, and map the two existing 32-bit metadata files for its WebAssembly build. This preserves DynamicVariantBox in metadata-only mode rather than changing its representation or inferring a tag from the payload.

Extend metadata-only bootstrap to generated sample types as well: compiler tests will serialize each C++-enabled assembly's custom type loader through GenerateMetaonlyTypes, excluding the already loaded native layers, then immediately unregister it. CppTest_Metaonly will load these generated metadata layers before running native samples. Add independently reached typed-list round trips for base/derived raw and shared null tags; this checks that two generated user-defined types cannot collapse through absent descriptor leaves. The existing release script rebuilds each consumer before execution and will include these test updates.
Native VariantExtract stores success separately from its default-initialized result. This follows the repository rule against nested Nullable wrappers and keeps successful null extraction distinct from inactive-option failure.

### VlppParser2 syntax mirror

Mirror the four authored lexer/type grammar/type AST/expression AST files from Workflow into the built-in Workflow parser. Preserve the built-in generator's namespace/configuration. Extend its indexed-fixture runner with a separate InvalidType path that asserts a parser error; semantic-error fixtures must continue to parse and round-trip through AST JSON. Run the ordered bootstrap/compiler/rebuild/consumer sequence and TypeScript checks after Workflow's active release build.
### RPC validation plan

The existing IsSerializableType traversal already rejects an unrecognized direct decorator and recursively visits nullable/typed-collection arguments, but accepts structs without inspecting fields. Add a cycle-aware variant-containment traversal for struct fields while preserving the existing unrelated serializability rules. Separate indexed negative fixtures will cover parameters, returns, properties, events, list/dictionary positions, nullable structs, nested/repeated structs and collections of recursive structs. Positive nonvariant controls must exercise the same paths. The predefined JSON serializer already ends in the unsupported-value failure; verify descriptorless variant boxes reach that failure through the dynamic-object path in the VM and all generated-C++ modes before deciding whether any runtime guard is necessary.
### GacUI variant-field migration plan

Replace the object-valued Gesture getter with the writable reflected gesture field, retaining the native scrolling/manipulation alternative order. Change the two XML consumers to nullable weak extraction inside their existing handled guards. Ordinary metadata/resource tests will assert the removed API, ordered field metadata, extraction-copy behavior, whole-field write-back, mismatched assignment diagnostics and selected write-through. Preserve all point lifecycle and list-removal code. Verification remains pending until the updated dependency releases are imported and the prescribed metadata/resource/unit/UIA sequence completes.
The GacUI authored migration is prepared: removed GetGesture and its now-unused reflection include, registered CLASS_MEMBER_FIELD(gesture), converted both sample handlers to nested nullable weak casts under the existing handled guard, and expanded ordinary reflection/resource assertions. Both edited XML files pass a structural parse with namespace processing disabled, matching GacUI's prefix handling; Workflow compilation and runtime/UI checks are still pending. Updated the canonical PointInput guidance and synchronized its eight existing copies after verifying they matched the previous canonical content (allowing line-ending differences).
Prepared sixteen separate H2_RpcVariant* analyzer fixtures and the three-file StructTraversal RPC control (expected result 28). They are intentionally not indexed or added to project inventories yet: these files belong to the pending RPC validator change and must be excluded from the Workflow core commit. StructTraversal sends nullable structs containing repeated leaf types, nullable fields, dictionaries and recursive child lists through the service, including an empty recursive leaf. All are still unverified.
### Workflow core checkpoint and current verification

Pushed the Workflow core as a12010e76 (authored), 7037267 (generated text) and 36313fb22 (other generated outputs, including CodePack release files). The full release script rebuilt Win32/x64 and passed both LibraryTest and metadata-generator runs, then stopped at an empty trailing IndexCodegen entry. Restored the index's no-trailing-newline convention. The logged Release Win32 compiler rerun passed 810/810 for each target architecture, including copy/print round trips, and produced 52 generated custom-type metadata layers per architecture. The ordinary cast assembly logs remain unchanged.

Release RuntimeTest passed 278/278 on both Win32 and x64 against the regenerated assemblies, including VariantCasts=60 and its six new typed-list null-tag assertions. Release x64 CppTest_Metaonly successfully loaded the generated metadata layers and passed 244/244, but that binary still contains the earlier VariantCasts=54 body. The new six native assertions, final Debug leak checks, and the required final complete Build.ps1 Workflow run remain pending. The RPC preparation files were excluded from all three core commits. Parser2's initial Debug x64 build has started; its ordered generator/test chain is pending.
### VlppParser2 verification completed

The ordered Debug x64 generator/rebuild sequence passed: AstGen 3/3, AstParserGen 46/46, LexerAndParser 4/4, LexerAndParser_Generated 24/24, ParserGen 142/142, ParserGen_Compiler 172/172, ParserGen_Generated 463/463, BuiltIn_Compiler 56/56, Json 131/131, Xml 25/25, Workflow 812/812 and Cpp 888/888. After indexing the RPC fixtures, corrected class-style var declarations in their struct fields and reran BuiltIn_Workflow: 829/829 passed. TypeScript preparation and tsc --noEmit passed. No Debug memory leaks were reported. Pushed 754a3d4b (authored), db4afd68 (generated text) and 4bd9335f (other generated outputs). Required generator output includes the regenerated BuiltIn-Cpp parser/trace files; its authored syntax was unchanged and all 888 C++ cases passed.

### RPC implementation checkpoint

Indexed sixteen H2_RpcVariant fixtures and StructTraversal=28, and added all nineteen source resources to the compiler project inventories. IsSerializableType now explicitly rejects Variant and uses a visited-type/visited-struct traversal to find variants beneath struct fields without changing unrelated nonvariant RPC rules. Added VM and native harness checks for the existing unsupported-value failure when an escaped variant is passed directly, inside object[], or inside object[string]. Generated the new VariantCasts serialization helpers with the Release compiler (810/810 for both target architectures before adding the RPC indexes), so native test sources can compile against them.

Updated the canonical RPC definition/schema guidance and synchronized all eight existing copies after checking for independent edits. Workflow's x64 Debug build passed with zero warnings/errors; LibraryTest passed 21/21 and metadata generation passed 2/2. Win32 build and final compiler/runtime/native/RPC stdio checks are pending.
### RPC reachability corrections and metadata fix

Both Debug architectures built with zero warnings/errors; LibraryTest passed 21/21 and metadata generation passed 2/2 on each. Before adding the final generated-serializer check and the new positive RPC sample, all three x64 native modes passed 245/245, including VariantCasts=60 and the direct/list/dictionary unsupported-value assertions, with no reported leaks.

The expanded compiler run exposed a missing event path: event validation checked reserved transport types but never checked variant containment. Added a targeted containment check for each event argument, preserving unrelated event acceptance, plus H2_RpcVariantEventStructCollection. The legal script rejection fixtures now contain sixteen H2 variant cases. A collection-valued Workflow struct field is rejected earlier by the existing D12 rule, so renamed that original recursive fixture to D12_RpcVariantRecursiveStruct instead of claiming it exercised RPC rejection. Added a separate ordinary analyzer test that builds a cyclic/repeated struct metadata graph with WfStruct/WfStructField, verifies nonvariant acceptance, then adds a variant leaf and verifies rejection. The end-to-end StructTraversal control now uses legal value-only structs and carries them through nullable, list and dictionary service methods.

CDB stopped in VerifyRpcMetadata and showed F1 for RpcStructTraversal::OptionalLeaf: the generated metadata referenced the nullable field's struct but omitted its declaration. CollectSerializableTypesFromTypeInfo did not descend through Nullable. Added that traversal; metadata reconstruction then proceeded to the client checks. Corrected the client and GacUI examples to Workflow's required explicit nullable declaration form, if (var value : T? = expression). The canonical PointInput example and its synchronized copies use the same form. Parser2's updated fixture pass is 830/830 with a successful TypeScript check; its newly generated RPC outputs remain to commit.

Added VM and native assertions through the actual PrimitiveTypes-generated rpcjson_Serialize function, whose enum/struct schema choices precede the predefined fallback. It must reject a variant box directly and inside an object list/dictionary with the same unsupported-value error. These new assertions, final RPC control execution, all final architecture/backend runs, stdio suites, and the complete release script remain pending.

### Final Debug Workflow and RPC verification

Both Win32 and x64 builds passed with zero warnings/errors. On each architecture LibraryTest passed 21/21, GenerateMetadata 2/2, CompilerTest_LoadAndCompile 829/829 for each generated target architecture, RuntimeTest 281/281, and each of CppTest, CppTest_Metaonly and CppTest_Reflection 247/247. These final native runs supersede the pending entries in the runtime table above, including VariantCasts=60 and the independently checked failure branches. No Debug memory leaks were reported.

CDB confirmed the existing unsupported RPC JSON path throws Error with description "Unsupported RPC JSON serialization value."; it is not a Workflow runtime exception. The VM and all native modes independently passed direct, object-list and object-dictionary checks through both the predefined fallback and the actual PrimitiveTypes-generated serializer. The legal StructTraversal control returned 28, including the nullable-only OptionalLeaf metadata round trip. Both architecture stdio suites passed, and TypeScript preparation/build passed.

All sixteen indexed H2 cases reached their intended first diagnostic on both architectures:

| Fixture suffix after H2_RpcVariant | Result |
| --- | --- |
| DictionaryKey | H2, passed Win32/x64 |
| DictionaryValue | H2, passed Win32/x64 |
| Enumerable | H2, passed Win32/x64 |
| Event | H2, passed Win32/x64 |
| EventStructCollection | H2, passed Win32/x64 |
| List | H2, passed Win32/x64 |
| NestedStruct | H2, passed Win32/x64 |
| NullableStruct | H2, passed Win32/x64 |
| ObservableList | H2, passed Win32/x64 |
| Parameter | H2, passed Win32/x64 |
| Property | H2, passed Win32/x64 |
| ReadonlyList | H2, passed Win32/x64 |
| RepeatedStruct | H2, passed Win32/x64 |
| Return | H2, passed Win32/x64 |
| Struct | H2, passed Win32/x64 |
| StructCollection | H2, passed Win32/x64 |

D12_RpcVariantRecursiveStruct retains the existing earlier language restriction; cyclic/repeated RPC metadata is covered by the separate ordinary analyzer acceptance/rejection control. Parser2's final 830/830 Workflow fixture run and TypeScript check passed, with refreshed fixtures pushed as 76129d25. RPC authored and generated changes were pushed as d944d314f, ec0620416 and 7b8413e41, including the refreshed Workflow CodePack release.

The complete final Workflow release script remains required. Updated Reflection and Workflow releases have been imported into GacUI; its initial Debug Win32 build is running. The removed getter may require the freshly built metadata/compiler projects to bootstrap resource regeneration before the whole solution can succeed.

### GacUI bootstrap and resource fixture correction

The initial Win32 build failed only in stale generated FullControlTest calls to GetGesture and their cascading expressions. Metadata_Generate and GacUI_Compiler built successfully; Win32 metadata generation succeeded and its text snapshot records the writable gesture variant in the original alternative order. The x64 bootstrap build is underway.

The first Win32 ordinary test run passed the native variant-field reflection assertions, then reported F3 in the new resource fixture: its if/else-if nullable declarations reused the same local name value. Workflow resolves both branch names as targets there. Give the two fixture branches distinct scrolling and transform names, matching the actual sample handlers; rerun the resource assertions after rebuilding. No runtime-library crash dialog was found by the delegated native-window check.

The initial x64 build likewise failed only on the stale generated getter calls. Both architecture metadata generators and x64 Metadata_Test completed successfully. GacUI_Compiler is now regenerating resources through the normal tool; no generated source has been edited manually.

The hidden CodePack invocation stopped making visible progress before writing release files. CDB found its main thread in the console-output path at GetFileType. Terminated that exact diagnostic process and reran the same CodePack/configuration through the existing Start-Process-And-Wait helper with inherited interactive console handles; it completed with exit code 0 and regenerated the release files. This is an execution-environment workaround, not a CodePack source change.

The canonical gesture/RPC guidance and its seven other repository copies were committed and pushed: Tools 381570c, Vlpp 444424f, VlppOS d78c332, VlppRegex fdaaa21, VlppReflection 3ae5ad1, VlppParser2 22cd3441, Workflow 9054a8001 and Release 9905309a. The GacUI copy remains with its pending authored commit.

The corrected x64 ordinary suite passed 13/13 selected files and 245/245 cases with no reported leaks. Both valid whole-field assignments compiled; scalar, plain-payload and reordered-variant assignments independently reached A10. The native reflection round trips and PointScrolling interaction/snapshot cases passed. Snapshot differences are limited to the two architecture-specific compiled Workflow text files reflecting the new handlers; useful rendering frames and protocol baselines are unchanged. Final post-generation builds, metadata refresh, Win32 rerun and the four application UIA runs remain pending.

Pushed the GacUI authored migration and guidance as 3a44bc7d4. The resource compiler completed its x86 pass and produced native nullable extraction from the writable gesture field. The x64 pass remains active. The required full Workflow release script completed its Win32 rebuild and advanced to the x64 rebuild. Read-only native-window checks found no runtime-error dialog for either process tree.

The x64 FullControlTest resource pass completed with the same expected handler changes as x86: scrolling uses alternative 0, manipulation uses alternative 1, and generated code no longer calls GetGesture. Both architecture resource binaries increased by 2,755 bytes. The compiler continued through the remaining TUI resources.

An early GacUI_Host Touching attempt timed out before its automation endpoint became ready, while the full Release rebuild and resource compiler were active. No Touching assertion ran and no native runtime-error dialog was found. The wrapper restored user arguments and cleaned up its owned process. The x64 executable was present and freshly built; this is an unresolved startup timeout, not evidence of a platform-path mismatch. Retry after the heavy builds finish, and investigate further if it recurs.

### Final build, metadata and ordinary-test verification

GacUI_Compiler completed both architectures and final source merges with exit code 0. No resource error file was found. Subsequent Debug Win32 and x64 solution builds both passed with zero warnings/errors. Metadata_Generate succeeded on both architectures, followed by successful x64 Metadata_Test.

The final ordinary GacUI runs passed all selected tests: Win32 11/11 files and 225/225 cases; x64 13/13 files and 245/245 cases. Existing architecture-specific filters were preserved, and both include the reflection, resource and PointScrolling tests. No memory leaks were reported. Rendering snapshots and protocol baselines remain unchanged; the two compiled Workflow text snapshots contain only the expected handler migration.

The complete required `<Tools repo>/Tools/Build.ps1 Workflow` run succeeded with exit code 0, including both Release architectures, compiler/runtime/native consumers, TypeScript, stdio RPC, CodePack and CppMerge. StructTraversal again produced 28; the existing SharedMemsp case remains intentionally skipped by that suite. It left no additional tracked Workflow changes.

All updated GacUI imports match their owning release files byte for byte. Generated imports, release files, metadata and resources were pushed as 52c15447d; the two unit-test snapshots were pushed separately as 03c1f4c12. All other affected repositories are clean and pushed. The four Debug x64 Touching application scenarios are now running without the earlier heavy build load.

### Four-application interaction verification completed

All required Debug x64 Touching scenarios passed: CppTest 6,826 assertions, CppTest_Metaonly 6,825, CppTest_Reflection 6,826 and GacUI_Host 6,826. Each completed normal Window.Close, owned-process termination and endpoint release. No native runtime-error dialog appeared, and the shared runner restored project user arguments.

Coverage includes the raw point lifecycle and fallback routing, one/two-contact manipulation, ListView scrolling and clamping, TouchingList right/partial/full/reversed swipes, cancellation, extra contacts, final-Up-only motion, exactly-once deferred identity-based removal, recycled rows, and text selection across template replacement. Automation and UIA name/selection/bounds/resize checks ran in the existing shared scenario. The earlier Host startup timeout did not recur once the heavy builds finished; the successful retry used the verified x64 executable.

Implementation, generated-output, metadata, ordinary-test and application verification are complete. Commit and push this checkpoint before starting the requested website documentation and publication phase.

### Website documentation and publication completed

The implementation checkpoint was pushed as ed8f62cca before starting the website phase. Authored documentation in `<WebsiteSource repo>/packages/website-doc2/src/articles/workflow/lang` now includes a dedicated Variants page and updates to types, expressions, binary operators, bindings, keyword indexing and navigation. `<WebsiteSource repo>/packages/website-doc2/src/articles/workflow/rpc.xml` documents recursive variant rejection and the existing dynamic JSON serialization failure. WebsiteSource commit 892e45d is pushed.

The five language code blocks were checked against the statements in the executed VariantCasts, VariantComparison and A10_VariantImplicitObject fixtures. The RPC example exactly matches H2_RpcVariantParameter. These fixtures passed their positive, expected-diagnostic or nonfatal-runtime-failure assertions in the final compiler/VM/generated-C++ test matrix described above; this check reused those executed fixtures rather than claiming a separate standalone example run.

`yarn install --frozen-lockfile`, `yarn build` and `yarn test` succeeded. The main website download, documentation download and Markdown export all completed with exit code 0. Changed XML parsed successfully. All 137 local links in the seven edited Markdown content pages resolve. The Markdown exporter still reports eight occurrences of existing missing registered/reference article URLs outside the changed pages.

Browser-rendered checks passed for Variants, Types, Expressions, Binary Operators, Index of Keywords, Data Binding and RPC, with no console errors. The Variants layout, code blocks and navigation were inspected. The publication copy contained all 364 exported documentation files with matching hashes and no stale files. Generated HTML escaped newlines were normalized to the published convention, removing 286 files' newline-only changes. The final publication contains the seven changed content pages and four necessary Types-child navigation updates.

Published commit 086e19f82 is pushed to `<vczh-libraries.github.io repo>` master. [Pages build and deployment run 39458446261](https://github.com/vczh-libraries/vczh-libraries.github.io/actions/runs/39458446261) completed successfully for that exact commit. Opening the published homepage redirected to the canonical gaclib.net domain. All seven edited live pages rendered the new content, and the RPC-to-Variants link worked with no browser console errors. The [live Variants documentation](https://gaclib.net/doc/current/workflow/lang/variant.html) is available.

Both `copilotInitAll` calls were skipped as requested. All 213 exported manual files were copied and hash-verified in `<Tools repo>/Copilot/KnowledgeBase/manual`, and the final Copy of Online Manual section of `<Tools repo>/Copilot/KnowledgeBase/Index.md` was replaced from the export. There were no stale manual files. Canonical manual commit 50f612e is pushed. The existing Index line-ending convention and generated Markdown terminal blank line were preserved.

This Windows host has no WSL/Linux WebAssembly build environment, so the publishing job's fresh Copy-WASM build was unavailable and was not counted as passed. The existing published demo was used for the local main-site export and left untouched in the publication repository. All 18 published wasm-fct files retained their original SHA-256 hashes. No new WebAssembly binary or browser-demo interaction coverage is claimed.

### CONFIRMED

The requested ordered variant model is implemented across reflection, Workflow compilation and execution, the mirrored parser, RPC validation and GacUI event reflection. Variant metadata deliberately has no element or descriptor; serialization and affected consumers handle that representation explicitly. Parameter boxes preserve declared option tags, including typed nulls, while exact variant conversion remains separate from ordinary conversions.

The positive and negative tests establish exact ordered identity, native alternative collision rejection, explicit strong/weak/infer behavior, first-option defaults, payload-based partial comparison, object-option precedence and escape behavior, ownership/copy semantics, assembly reloads, and recursive RPC rejection. Final Workflow tests passed for Win32/x64 in the VM and all three generated-C++ reflection modes; full Release, stdio RPC and TypeScript verification also passed. Reflection and VlppParser2 verification passed as recorded above. Existing ordinary cast expectations were retained.

GacUI now exposes the writable gesture variant field and removes the old Gesture/GetGesture workaround. Reflection/resource tests establish option order, extraction copies, whole-field write-back and mismatched-assignment rejection. Final Win32/x64 unit runs and all four Touching automation/UIA scenarios passed without reported leaks or runtime-error dialogs. The shared scenario preserves the original gesture lifecycle, transformations, scrolling and deferred identity-based removal behavior.

Authored changes, required generated outputs, metadata, snapshots, synchronized guidance, website publication and the canonical manual have been committed and pushed in their owning repositories. The implementation is retained as the confirmed proposal. Coverage limitations are explicitly recorded: a fresh WebAssembly rebuild was unavailable, the existing published demo was preserved, and automation/UIA execution does not claim physical-desktop pixel inspection.

## No.2 Explicit unary extraction of a variant payload as object

Research confirms that the existing lexer already contains both variant and in. Adding the expression to the recursive _Exp0 grammar gives it the same right-associative precedence as unary +, -, not and cast without adding keywords. The VM's VUnbox already extracts the active payload, and the native VariantPayload helper performs the corresponding parameter boxing.

One premise needs qualification: the existing explicit cast to object preserves the outer variant box when object is not a declared option. When object is an option, that cast instead performs exact option extraction and fails for another active option. Keep this previously requested behavior; the new operator supplies an explicit, unconditional extraction of the active option and has static type object. It requires a statically variant operand and does not recursively unwrap an object payload that itself holds another box.

### CODE CHANGE

Add VariantExtract to WfUnaryOperator, parse variant in _Exp0, print it, validate a variant operand with the existing unary diagnostic, emit VUnbox, and emit the C++ payload helper. Reuse the existing helper implementation under the descriptive VariantExtractObject name for extraction, comparison and null tests. Mirror only the authored AST/grammar files into VlppParser2 and regenerate with the prescribed tools. Extend indexed fixtures and GacUI ordinary resource tests; publish updated syntax, precedence, binding and RPC documentation and the canonical manual after implementation verification, skipping both copilotInitAll calls.

Verify scalar/string/nullable/raw/shared/object/struct/collection/function payloads, temporary and copied variants, one-time operand evaluation, ordinary object casts after extraction, explicit object-option behavior, box-preserving cast compatibility and one-level extraction. Exercise unary chaining and precedence, postfix/index/call operands, lower-precedence expressions, parser print/copy/JSON round trips, nonvariant and object operand rejection, and missing syntax. Run the prescribed Workflow Win32/x64 compiler/VM/all-three-C++-mode/RPC/TypeScript matrix and full release workflow, VlppParser2 ordered regeneration tests, GacUI dependency refresh/build/metadata/unit verification, then website build/export/browser/live/CI checks. Keep generated files and snapshots in their required separate commits.
