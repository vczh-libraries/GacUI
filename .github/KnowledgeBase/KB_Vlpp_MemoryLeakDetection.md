# Memory Leak Detection

## Overview

Vlpp provides memory leak detection capabilities for debugging and testing C++ applications on Windows. The system addresses the problem of global variables being destructed after the `main` function returns, which can cause false negatives in memory leak detection tools. The framework offers both basic detection mechanisms and a global storage system for managing application-wide resources that need explicit lifecycle control.

## Basic Memory Leak Detection

### Standard Detection Pattern

On Windows (guarded by the `VCZH_MSVC` macro), applications typically use this pattern in `main`, `wmain` or `WinMain` to detect memory leaks:

```cpp
int main(int argc, char** argv)
{
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	vl::FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
```

**Key characteristics:**
- **Platform-specific**: Only available on Windows with MSVC compiler
- **Conditional compilation**: Protected by `VCZH_CHECK_MEMORY_LEAKS` macro
- **Requires finalization**: Must call `FinalizeGlobalStorage()` before leak detection
- **End-of-program checking**: Performs leak detection just before application exit

### The Global Variable Problem

Global variables are destructed after `main` returns, which causes `_CrtDumpMemoryLeaks()` to report false negatives. This happens because:

- Global destructors run after `main` completes
- Memory allocated by global variables appears as leaks to the detection tool
- The recommended solution is explicit initialization and finalization in `main`
- When explicit management isn't feasible, the global storage system provides an alternative

## Global Storage System

### Definition and Purpose

Global storage provides a managed approach to handling application-wide resources that need explicit lifecycle control. It's designed for situations where direct global variable management in `main` isn't practical.

**Usage guidelines:**
- **Not recommended**: Global storage should be avoided unless specifically instructed
- **Explicit preference**: Direct initialization and finalization in `main` is preferred
- **Fallback option**: Use only when direct management is not feasible
- **Resource management**: Typically used for pointer-based global variables

### Global Storage Implementation

A global storage is defined using a specific macro pattern:

```cpp
BEGIN_GLOBAL_STORAGE_CLASS(MyGlobalStorage)
	Ptr<vint>	resource;

	INITIALIZE_GLOBAL_STORAGE_CLASS
		resource = Ptr(new vint(100));

	FINALIZE_GLOBAL_STORAGE_CLASS
		resource = nullptr;

END_GLOBAL_STORAGE_CLASS(MyGlobalStorage)
```

**Implementation details:**
- **Class definition**: `BEGIN_GLOBAL_STORAGE_CLASS` defines a class to contain resources
- **Resource fields**: Declare global resources as class members
- **Initialization block**: `INITIALIZE_GLOBAL_STORAGE_CLASS` contains setup code
- **Finalization block**: `FINALIZE_GLOBAL_STORAGE_CLASS` contains cleanup code
- **Access function**: Automatically generates `GetMyGlobalStorage()` function

### Accessing Global Storage

Use the generated access function to interact with global storage:

```cpp
// Check if global storage is available
if (GetMyGlobalStorage().IsInitialized())
{
    // Access the resource
    auto value = GetMyGlobalStorage().resource;
}
```

**Access patterns:**
- **Availability checking**: Use `IsInitialized()` to verify storage state
- **Resource access**: Direct member access through the storage object
- **Post-finalization**: Returns `false` after `FinalizeGlobalStorage()` is called
- **Thread safety**: No automatic synchronization - add locks if needed

## Integration with Applications

### Unit Testing Integration

Global storage integrates with the unit testing framework:

```cpp
int main(int argc, char** argv)
{
	int result = unittest::UnitTest::RunAndDisposeTests(argc, argv);
	vl::FinalizeGlobalStorage();
#ifdef VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
```

**Testing considerations:**
- **Test isolation**: Global storage is shared across all tests
- **Cleanup responsibility**: Tests should not assume global storage state
- **Resource lifecycle**: Storage is finalized after all tests complete
- **Memory validation**: Leak detection occurs after global storage cleanup

### Application Lifecycle

The typical application lifecycle with global storage:

1. **Application start**: Global storage is uninitialized
2. **First access**: Storage is automatically initialized on first use
3. **Normal operation**: Resources are accessible through storage object
4. **Pre-shutdown**: Call `FinalizeGlobalStorage()` explicitly
5. **Leak detection**: Run memory leak detection tools
6. **Application exit**: Normal termination

## Best Practices

### When to Use Global Storage

**Appropriate scenarios:**
- Global resources that cannot be managed directly in `main`
- Application-wide caches or registries
- Cross-module shared resources
- Legacy code integration where direct management is impractical

**Inappropriate scenarios:**
- Simple global variables that can be managed directly
- Resources with clear ownership by specific modules
- Temporary or short-lived data
- Thread-local resources

### Resource Management

**Initialization practices:**
- Initialize resources to valid states in `INITIALIZE_GLOBAL_STORAGE_CLASS`
- Use smart pointers (`Ptr<T>`) for automatic reference counting
- Avoid throwing exceptions during initialization
- Keep initialization code simple and deterministic

**Finalization practices:**
- Explicitly reset smart pointers to `nullptr` in `FINALIZE_GLOBAL_STORAGE_CLASS`
- Release external resources (files, network connections, etc.)
- Avoid complex operations that might fail during shutdown
- Ensure proper cleanup order for interdependent resources

### Memory Management Integration

**With Ptr<T> smart pointers:**
```cpp
BEGIN_GLOBAL_STORAGE_CLASS(ResourceStorage)
	Ptr<SomeObject>	sharedResource;

	INITIALIZE_GLOBAL_STORAGE_CLASS
		sharedResource = Ptr(new SomeObject());

	FINALIZE_GLOBAL_STORAGE_CLASS
		sharedResource = nullptr;  // Automatic cleanup via reference counting

END_GLOBAL_STORAGE_CLASS(ResourceStorage)
```

**With raw pointers (avoid when possible):**
```cpp
BEGIN_GLOBAL_STORAGE_CLASS(LegacyStorage)
	SomeObject*	legacyResource;

	INITIALIZE_GLOBAL_STORAGE_CLASS
		legacyResource = new SomeObject();

	FINALIZE_GLOBAL_STORAGE_CLASS
		delete legacyResource;
		legacyResource = nullptr;

END_GLOBAL_STORAGE_CLASS(LegacyStorage)
```

## Extra Content

### Platform Considerations

**Windows-specific features:**
- `_CrtDumpMemoryLeaks()` is a Microsoft Visual C++ runtime function
- Memory leak detection is primarily designed for debug builds
- Release builds may not include leak detection overhead
- The `VCZH_MSVC` macro guards Windows-specific code paths

**Cross-platform compatibility:**
- Global storage system works on all platforms
- Memory leak detection is Windows-only
- Other platforms may use different debugging tools
- Application structure remains consistent across platforms

### Performance Implications

**Runtime overhead:**
- Global storage initialization occurs on first access (lazy initialization)
- `IsInitialized()` checks have minimal performance impact
- Access through storage objects is essentially direct member access
- Finalization is a one-time operation during shutdown

**Memory overhead:**
- Storage objects themselves use minimal memory
- Primary memory usage comes from stored resources
- No additional indirection beyond the storage object access
- Smart pointer overhead is minimal for reference counting

### Advanced Usage Patterns

**Multiple storage objects:**
```cpp
BEGIN_GLOBAL_STORAGE_CLASS(UIStorage)
	Ptr<FontManager>	fontManager;
	Ptr<ImageCache>		imageCache;
	// ... UI-related resources
END_GLOBAL_STORAGE_CLASS(UIStorage)

BEGIN_GLOBAL_STORAGE_CLASS(NetworkStorage)
	Ptr<ConnectionPool>	connectionPool;
	Ptr<CertificateStore>	certificates;
	// ... network-related resources
END_GLOBAL_STORAGE_CLASS(NetworkStorage)
```

**Dependency management:**
```cpp
BEGIN_GLOBAL_STORAGE_CLASS(DependentStorage)
	Ptr<Database>		database;
	Ptr<CacheManager>	cache;

	INITIALIZE_GLOBAL_STORAGE_CLASS
		database = Ptr(new Database());
		cache = Ptr(new CacheManager(database));  // Cache depends on database

	FINALIZE_GLOBAL_STORAGE_CLASS
		cache = nullptr;     // Clean up dependent resource first
		database = nullptr;  // Clean up dependency second

END_GLOBAL_STORAGE_CLASS(DependentStorage)
```

### Integration with Other Systems

**With VlppOS threading:**
```cpp
BEGIN_GLOBAL_STORAGE_CLASS(ThreadSafeStorage)
	CriticalSection		lock;
	Ptr<SharedResource>	resource;

	INITIALIZE_GLOBAL_STORAGE_CLASS
		resource = Ptr(new SharedResource());

	FINALIZE_GLOBAL_STORAGE_CLASS
		CS_LOCK(lock)
		{
			resource = nullptr;
		}

END_GLOBAL_STORAGE_CLASS(ThreadSafeStorage)
```

**Error handling during initialization:**
```cpp
BEGIN_GLOBAL_STORAGE_CLASS(SafeStorage)
	Ptr<Resource>	resource;
	bool			initializationFailed;

	INITIALIZE_GLOBAL_STORAGE_CLASS
		initializationFailed = false;
		try
		{
			resource = Ptr(new Resource());
		}
		catch (...)
		{
			initializationFailed = true;
			resource = nullptr;
		}

	FINALIZE_GLOBAL_STORAGE_CLASS
		resource = nullptr;

END_GLOBAL_STORAGE_CLASS(SafeStorage)
```

### Debugging and Troubleshooting

**Common issues:**
- Forgetting to call `FinalizeGlobalStorage()` before leak detection
- Accessing global storage after finalization
- Circular dependencies between global storage objects
- Exception handling during initialization or finalization

**Debugging techniques:**
- Use `IsInitialized()` to verify storage state at runtime
- Add logging to initialization and finalization blocks
- Use debugger breakpoints in storage lifecycle methods
- Monitor resource creation and destruction patterns

**Memory leak investigation:**
- Ensure all `Ptr<T>` references are properly reset to `nullptr`
- Check for circular references that prevent automatic cleanup
- Verify that external resources (files, handles) are properly released
- Use memory profiling tools to identify actual leak sources