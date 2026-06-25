# Workflow Attributes

Workflow attributes attach metadata to declarations. They are used by analyzers and runtime frameworks, including RPC, without changing the normal type, member, or expression syntax.

An attribute has a category and a name. It can be written without a value, or with one constant value:
```
@category:Name
@category:Name(value)
```

## Where Attributes Can Appear

Attributes can be placed before declarations that participate in Workflow reflection metadata. This includes type declarations, enum items, struct members, class and interface members, and function parameters.

For example, RPC uses attributes on interfaces, methods, properties, and parameters:
```
@rpc:Interface
@rpc:Ctor
interface IExampleService
{
	func GetNames(@rpc:Byref names : List<string>) : void;

	@rpc:Dynamic
	prop CurrentName : string {not observe}
}
```

## Name Resolution

Workflow resolves an attribute name by combining its category and name with the att_ prefix. The category and name must be non-empty identifiers containing only ASCII letters, digits, or underscores.

For example, @ns:Type expands to the C++ attribute type name vl::__vwsn::att_ns_Type and the reflected Workflow type name system::workflow_attributes::att_ns_Type.

The compiler looks up the reflected type. If system::workflow_attributes::att_ns_Type is not visible in reflection metadata, @ns:Type is rejected as an unknown attribute.

## Attribute Value Rules

The reflected attribute type must be a struct. It can expose either zero or one reflected property:
- Zero properties define a marker attribute, used as @category:Name.
- One property defines a valued attribute, used as @category:Name(value).

A valued attribute requires a constant expression compatible with the single reflected property type. Common constant forms include booleans, strings, numbers, enum values, and type literals such as typeof(T).

After validation, the attribute instance is stored in the reflection attribute bag for the corresponding type, member, parameter, or struct field. Runtime code can inspect that metadata through the normal reflection APIs.

## Built-In Categories

Workflow source repositories currently use these real attribute categories:
- @rpc:* describes RPC interfaces, transfer rules, property caching, and service construction. See [RPC](.././workflow/rpc.md).
- @cpp:* is reserved by Workflow tooling for native interop metadata. This page only documents the common attribute mechanism.

