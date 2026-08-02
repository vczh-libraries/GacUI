# GenerateMetaonlyTypes

## Generate and load binary reflection metadata

`GenerateMetaonlyTypes` serializes the registered reflection metadata in a loaded global type manager.
The output can be a self-contained file or a dependent layer whose previously registered types are referenced without repeating their metadata records.

The related APIs are:

```cpp
void CollectRegisteredTypes(collections::List<ITypeDescriptor*>& types);
void GenerateMetaonlyTypes(
    const collections::List<ITypeDescriptor*>& excludedTypes,
    stream::IStream& outputStream
);
Ptr<ITypeLoader> LoadMetaonlyTypes(
    stream::IStream& inputStream,
    const collections::Dictionary<WString, Ptr<ISerializableType>>& serializableTypes
);
```

## Self-contained metadata

Pass an empty exclusion list to create an independently loadable file:

```cpp
collections::List<ITypeDescriptor*> dependencies;
stream::FileStream output(fileName, stream::FileStream::WriteOnly);
GenerateMetaonlyTypes(dependencies, output);
```

The global type manager must already be loaded before generation.
An empty dependency list is serialized at the beginning of the file, and `LoadMetaonlyTypes` does not require pre-existing registered types when reading it.

## Dependent metadata layers

Use a snapshot of all currently registered descriptors when a later group of types should be stored in a separate file:

1. Register and load every type in the base layer.
2. Call `CollectRegisteredTypes` to capture the base descriptors. The function replaces the output list instead of appending to it.
3. Register the types for the dependent layer. Adding a loader to an already loaded type manager applies it immediately; do not call `ITypeManager::Load` again.
4. Pass the captured base descriptors to `GenerateMetaonlyTypes`.

```cpp
auto manager = GetGlobalTypeManager();
manager->Load();

collections::List<ITypeDescriptor*> baseTypes;
CollectRegisteredTypes(baseTypes);

manager->AddTypeLoader(CreateDependentTypeLoader());

stream::FileStream output(layerFileName, stream::FileStream::WriteOnly);
GenerateMetaonlyTypes(baseTypes, output);
```

The exclusion snapshot identifies descriptors supplied by previous layers.
Those descriptors can still be referenced by local base types, signatures, generic arguments, properties, events, attributes, and other metadata, but their own records are not emitted again.

## Loading order

Load and activate every dependency before even calling `LoadMetaonlyTypes` for a dependent file:

```cpp
auto manager = GetGlobalTypeManager();

auto baseLoader = LoadMetaonlyTypes(baseStream, serializableTypes);
manager->AddTypeLoader(baseLoader);
manager->Load();

auto dependentLoader = LoadMetaonlyTypes(dependentStream, serializableTypes);
manager->AddTypeLoader(dependentLoader);
```

Dependency validation happens while `LoadMetaonlyTypes` reads the file.
If any required registered name is missing, it raises an error that includes that name.
When the manager is already loaded, adding the dependent loader registers its local descriptors immediately.

## Dependency identity and ordering

- Dependencies are identified by `ITypeDescriptor::GetTypeName()`, which is the registered reflection name. Do not use the C++ full name.
- `GenerateMetaonlyTypes` sorts dependency names in ascending order. The result does not depend on caller order or `ITypeManager::GetTypeDescriptor(vint)` enumeration order.
- The file begins with exactly one serialized `List<WString>` containing the sorted dependency names.
- Foreign descriptor indices occupy the dependency prefix in serialized-name order. Local descriptors follow in deterministic type-name order.
- Type-descriptor references use the combined foreign-and-local table. Method, property, and event indices remain local to the current layer.

## Preconditions and lifetime

- The global type manager must be loaded before calling `CollectRegisteredTypes` or `GenerateMetaonlyTypes`.
- Every excluded descriptor must be non-null, unique, and the exact descriptor currently registered under its name.
- Captured descriptors must remain registered and unchanged until generation finishes. Adding the new local types is expected, but do not replace captured descriptors, reset the manager, or mutate it concurrently with collection or generation.
- Keep dependency layers loaded while their dependent layers are in use. The dependent loader retains resolved foreign descriptors so its local metadata references remain valid.

Attribute values participate in the same combined descriptor table.
For the attribute-specific representation of serializable values and `ITypeDescriptor*`, see [Attribute Registration](./KB_VlppReflection_AttributeRegistration.md).
