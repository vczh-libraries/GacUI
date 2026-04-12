export type PrimitiveTypes = "Boolean" | "Integer" | "Float" | "Double" | "String" | "Char" | "Key" | "Color" | "Binary";

export type StructType = "Struct" | "Class";

export type Type = PrimitiveType | ReferenceType | OptionalType | ArrayType | ArrayMapType | MapType;

export type Declaration = EnumDecl | UnionDecl | StructDecl | MessageDecl | EventDecl;

export interface Declaration_Common {
    attributes: (Attribute | null)[];
    name: string;
}

export interface PrimitiveType {
    $ast: "PrimitiveType";
    type: PrimitiveTypes;
}

export interface ReferenceType {
    $ast: "ReferenceType";
    name: string;
}

export interface OptionalType {
    $ast: "OptionalType";
    element: Type | null;
}

export interface ArrayType {
    $ast: "ArrayType";
    element: Type | null;
}

export interface ArrayMapType {
    $ast: "ArrayMapType";
    element: string;
    keyField: string;
}

export interface MapType {
    $ast: "MapType";
    element: Type | null;
    keyType: Type | null;
}

export interface Attribute {
    $ast: "Attribute";
    name: string;
    cppType: string;
}

export interface EnumMember {
    $ast: "EnumMember";
    name: string;
}

export interface EnumDecl extends Declaration_Common {
    $ast: "EnumDecl";
    members: (EnumMember | null)[];
}

export interface UnionMember {
    $ast: "UnionMember";
    name: string;
}

export interface UnionDecl extends Declaration_Common {
    $ast: "UnionDecl";
    members: (UnionMember | null)[];
}

export interface StructMember {
    $ast: "StructMember";
    name: string;
    type: Type | null;
}

export interface StructDecl extends Declaration_Common {
    $ast: "StructDecl";
    type: StructType;
    members: (StructMember | null)[];
}

export interface MessageRequest {
    $ast: "MessageRequest";
    type: Type | null;
}

export interface MessageResponse {
    $ast: "MessageResponse";
    type: Type | null;
}

export interface MessageDecl extends Declaration_Common {
    $ast: "MessageDecl";
    request: MessageRequest | null;
    response: MessageResponse | null;
}

export interface EventRequest {
    $ast: "EventRequest";
    type: Type | null;
}

export interface EventDecl extends Declaration_Common {
    $ast: "EventDecl";
    request: EventRequest | null;
}

export interface Schema {
    $ast: "Schema";
    declarations: (Declaration | null)[];
}

