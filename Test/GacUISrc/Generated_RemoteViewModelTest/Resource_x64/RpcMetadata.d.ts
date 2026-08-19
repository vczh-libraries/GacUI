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

export type TypeList_Enum = never;

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

export type UnknownTypeSchema =
  | UnknownType_PrimitiveSchema
  | UnknownType_EnumSchema
  | UnknownType_List
  | UnknownType_Map
  | UnknownType_system_RpcObjectReference
  | UnknownType_system_RpcException
  ;

export interface UnknownType_system_RpcObjectReference extends system_RpcObjectReference
{
  "$": "system::RpcObjectReference";
}

export interface UnknownType_system_RpcException extends system_RpcException
{
  "$": "system::RpcException";
}

// below are all known types

export interface system_RpcObjectReference
{
  clientId: number;
  objectId: number;
  typeId: number;
}

export interface system_RpcException
{
  message: string;
}

// All enum_type_full_name is omitted because in known type enums are just numbers
export type KnownTypeSchema =
  | number
  | true
  | false
  | string
  | KnownTypeSchema[]
  | [KnownTypeSchema, KnownTypeSchema][]
  | system_RpcObjectReference
  | system_RpcException
  ;
