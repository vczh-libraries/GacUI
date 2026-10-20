You are going to make `vl::Variant` reflectable and integrate it into Workflow script, so that to fix the [gesture workaround](./TODO_Touch_StepR1.md). 

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

3) Fix VlppParser2

Sync new syntax back to VlppParser2 so that the `BuiltIn_Workflow` could work.
Syntax files are not shared, just copy them.

4) Fix Workflow RPC

DO NOT support variant for now, treat any variant type as unserializable.

5) Fix GacUI

Remove `GuiPointGestureEventArgs::GetGesture` and add `gesture` to reflection directly because we already support it.
Fix `Touching` and `TouchingList` tab pages to adopt the new variant feature.
This would require fixing knowledge base pages to remove stale facts.

6) Additional works

All files updated by unit test should commit.
When everything is ready, commit and push all local changes first, and then:
- You need to add variant to Workflow's syntax in the website, publish it but ignore `job:copilotInitAll`.

6) Things to verify, if my assumption is incorrect, they should appear in review comments:

- Make sure `ITypeInfo` doesn't need non-null ElementType and TypeDescriptor currently.
