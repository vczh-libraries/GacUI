# Workflow JSON Serialization Schema

There are two kinds of JSON schema for RPC serializable types:
- Schema for known types. They are used when the context gives you enough information therefore you know what type is expected without having to look into actual values, including `object` or `system::JsonNode`.
  - Functions in wrapper Workflow Script:
    - `func rpcjson_Serialize_Enum_Type_Full_Name(value : enum::type::full::name) : (system::JsonNode^)`
    - `func rpcjson_Serialize_Struct_Type_Full_Name(value : struct::type::full::name) : (system::JsonNode^)`
    - `rpcjson_Deserialize_*` in reversed form.
- Schema for unknown types.
  - Typically needed when access elements in collections transferred using `@rpc:Byref`.
  - Functions in wrapper Workflow Script:
    - `func rpcjson_Serialize(value : object) : (system::JsonNode^)`
    - `func rpcjson_Deserialize(node : system::JsonNode^) : (object)`

Pay attention to the `RpcObjectReference` and `RpcException` types, they are treated like structs. Serialization for both types is always needed, even when they do not appear in RPC metadata.
Types listed here should include every type appearing in `WfLexicalScopeManager::rpcMetadata`, but when `RpcObjectReference` or `RpcException` also appears in the list, do not duplicate its processing.

## Schema for Known Types

1) primitive types

Use the second element in `UnknownType_PrimitiveSchema` if it is an array, unless it is `null | true | false | string`.

2) enum types

Use `number`.

3) struct types

`{ field: value, ...}`
- `field` is a string, which is required in JSON anyway.
- `value` is schema for known types.

4) collection types

For list: array of schema for known types.
For dictionary: array of `[key, value]`, both are schema for known types.

## Schema for Unknown Types

1) primitive types

See `UnknownType_PrimitiveSchema` in `## Expected format of generated .d.ts files`

2) enum types

`["enum::type::full::name", number]`.

3) struct types

`{ "$": "struct::type::full::name", strong-type-version-of-this-struct...}`
`system::RpcObjectReference` and `system::RpcException` use this standard unknown-struct schema whenever they cross a JSON RPC boundary as dynamically typed transport values. Unknown RPC method or event ids are local dispatch errors, so they do not use the `system::RpcException` JSON transport schema.

4) collection types

- For list and oblist: `{ "$": "list" | "oblist", values: [elements ...]}`.
- For dictionary: `{ "$": "map", values: [[key1, value1], [key2, value2], ...]}`.

## Implementation of rpcjson_Serialize

Check if the value is null or bool or string, they can be serialized directly.
And then try to weak cast it to any `PrimitiveType?` type.
And then try to weak cast it to any collection interface type.
And then try to weak cast it to any `InterfaceType^` type.

Weak casting failure results in `null` instead of an exception.
Throw an exception if all type testings fail.

## Implementation of rpcjson_Deserialize

First check if it is `JsonArray` or `JsonObject` or `JsonLiteral` or `JsonString`,
and then check the first element of array or "$" field of object,
and we know the exact type.
Throw an exception if all type testings fail.

## JSON Return Values for Byval Collections

When an RPC method returns an `@rpc:Byval` collection, JSON serialization still uses the same known-type or unknown-type schema for the collection value. The generated object ops put the resulting `JsonNode` in `system::RpcByvalReturnValue.value` and use `system::RpcByvalReturnValue.slot` to keep the recursive copied collection alive until the caller calls `EndInvokeMethod(slot)`. Non-byval JSON returns still return the `JsonNode` directly.

## TypeScript Schema for Dispatcher Messages

`Release/Rpc.d.ts` describes the JSON message envelopes for the generic RPC ops boundary represented by `IRpcDispatcher`. It does not know the generated RPC metadata for a sample, so every field that contains a serialized `JsonNode` uses a generic type parameter `T`. A concrete test should read `T` as `KnownTypeSchema | UnknownTypeSchema` from the generated `Serialization_*.d.ts` file for that sample.

The declarations map directly to the C++ ops interfaces in `Source/Library/Rpc/WfLibraryRpc.h`:

- `IObjectOps_*` maps to `IRpcObjectOps`.
- `IObjectEventOps_*` maps to `IRpcObjectEventOps`.
- `IRpcDispatcher_DeclareRemoteService` maps to dispatcher-level service declaration and is handled by `IRpcLifecycle::DeclareRemoteService(ref)`.

Request envelopes model how `IRpcDispatcher` chooses an ops object. Calls made through `SendToClient_ObjectOps` include both `sourceClientId` and `targetClientId`. Calls made through `BroadcastFromClient_ObjectEventOps` include only `sourceClientId` because the dispatcher expands the broadcast target list. Response envelopes are always one-to-one from the receiving client back to the requesting client, so they always contain both client ids. `IRpcListOps` and `IRpcListEventOps` are local adapters only: list methods are transported as `IObjectOps_InvokeMethod` with predefined negative method ids, and observable-list `ItemChanged` is transported as `IObjectEventOps_InvokeEvent` with the predefined negative event id.

The stable internal transport structs are declared in `Rpc.d.ts` itself: `system_RpcObjectReference`, `system_RpcException`, and `system_RpcByvalReturnValue<T>`. The predefined JSON serializer owns `system::RpcObjectReference` and `system::RpcException` serialization; per-RPC generated serializers should not emit dedicated struct functions for them. Void-returning ops still have response envelopes, but no `response` field. Value-returning ops put the serialized value in `response`. `IRpcObjectEventOps::InvokeEvent`, including predefined observable-list `ItemChanged` events, returns the JSON form of `null | [number, system_RpcException][]`, matching `system::RpcException[int]` after deserialization. Request routing, response consolidation, and service declaration replay rules are specified in [Workflow JSON Request Routing](./KB_Workflow_JsonRequestRouting.md).

## Other Strict Rules

DO NOT generate any helper function that is not mentioned here, especially which just builds any `system::JsonNode^`.
You are going to repeat JSON AST building code in each function.
For JSON related functions, stick to the list in the beginning of this document.


## Expected format of generated .d.ts files

```TypeScript
export type UnknownType_PrimitiveSchema =
  | ["UInt8", number]
  | ["UInt16", number]
  | ["UInt32", number]
  | ["UInt64", number]
  | ["Int8", number]
  | ["Int16", number]
  | ["Int32", number]
  | ["Int64", number]
  | ["Single", number]
  | ["Double", number]
  | ["Char", string]
  | ["DateTime", string]
  | ["Locale", string]
  | null
  | true
  | false
  | string
  ;

export type TypeList_Enum =
  | "enum::type::full::name"
  ...
  ;

export type UnknownType_EnumSchema = [TypeList_Enum, number];

export interface UnknownType_List
{
  "$": "list" | "oblist";
  values: UnknownTypeSchema[];
}

export interface UnknownType_Map
{
  "$": "map";
  values: [UnknownTypeSchema, UnknownTypeSchema][];
}

export interface UnknownType_struct_type_full_name extends struct_type_full_name
{
  "$": "struct::type::full::name";
}

export type UnknownTypeSchema =
  | UnknownType_PrimitiveSchema
  | UnknownType_EnumSchema
  | UnknownType_List
  | UnknownType_Map
  | UnknownType_struct_type_full_name
  | ...
  ...

// UnknownType_* interfaces are generated for all structs, including
// system::RpcObjectReference and system::RpcException.

// below are all known types

export enum enum_type_full_name
{
  item = number-literal,
  ...
}

export interface struct_type_full_name
{
  field: value_type;
}

// system::RpcObjectReference and system::RpcException are always generated here.

// All enum_type_full_name is omitted because in known type enums are just numbers
export type KnownTypeSchema =
  | number
  | true
  | false
  | string
  | KnownTypeSchema[]
  | [KnownTypeSchema, KnownTypeSchema][]
  | struct_type_full_name
  | ...
  ;
```

`JsonValue_*.ts` files generated for TypeScript validation contain JSON values captured at the generic RPC ops boundary, so each element is typed as `KnownTypeSchema | UnknownTypeSchema`.
