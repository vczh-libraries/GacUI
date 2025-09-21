# Reflection Compilation Levels

## Three different compilation modes for reflection support with varying runtime capabilities.

The reflection system supports three compilation levels:
- Full reflection: Complete metadata and runtime support for type registration and function calls
- Metadata-only (`VCZH_DESCRIPTABLEOBJECT_WITH_METADATA`): Type metadata without runtime support
- No reflection (`VCZH_DEBUG_NO_REFLECTION`): Reflection disabled entirely
Always prefer code compatible with `VCZH_DEBUG_NO_REFLECTION` when possible.

## Compilation Levels

### Full Reflection (Default)
When no special macros are defined, VlppReflection compiles with full reflection support:
- Complete metadata and runtime support for type registration and function calls
- You can register your own classes, get metadata from registered types and objects, and call reflectable functions at runtime
- Executing a Workflow script runs in this level
- Use `vl::reflection::description::GetTypeDescriptor<T>` to get the metadata of a type

### Metadata-Only Mode
Defined by `VCZH_DESCRIPTABLEOBJECT_WITH_METADATA`:
- Metadata of types are loaded from external sources
- You can get metadata from types, but lose all runtime support
- Running a Workflow or GacUI XML compiler operates in this level
- Type information is available but dynamic invocation is not supported

### No Reflection Mode
Defined by `VCZH_DEBUG_NO_REFLECTION`:
- Reflection is completely disabled
- Workflow or GacUI XML Compiler generated code should be able to run in this level
- Minimal runtime overhead
- No metadata or dynamic capabilities available

## Best Practices

Always prefer code that is compatible with `VCZH_DEBUG_NO_REFLECTION` when possible. This ensures:
- Maximum performance with minimal overhead
- Broader compatibility across different compilation scenarios
- Cleaner separation between reflection-dependent and reflection-independent code

When reflection is enabled (when `VCZH_DEBUG_NO_REFLECTION` is not defined), you can access type metadata and perform dynamic operations.

## Extra Content

### Conditional Compilation Patterns

Use conditional compilation to handle different reflection levels:

```cpp
#ifndef VCZH_DEBUG_NO_REFLECTION
    // Code that requires reflection
    auto typeDescriptor = vl::reflection::description::GetTypeDescriptor<MyClass>();
#endif
```

### Performance Considerations

- Full reflection: Highest capability, highest overhead
- Metadata-only: Medium capability, medium overhead  
- No reflection: No reflection capability, minimal overhead

Choose the appropriate level based on your application's requirements for dynamic behavior versus performance.