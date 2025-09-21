## Overview

The FeatureInjection system provides a linked-list based dependency injection mechanism that allows runtime replacement and extension of feature implementations while maintaining delegation capabilities. This system enables testing with mock implementations, feature enhancement through layered behaviors, and platform-specific customizations without modifying core application code.

## Core Components Architecture

### IFeatureImpl Base Interface

The `IFeatureImpl` interface serves as the foundation for all injectable implementations:

- `GetPreviousImpl()`: Returns the previous implementation in the injection chain
- `BeginInjection(IFeatureImpl* previousImpl)`: Lifecycle hook called when implementation becomes active in the chain
- `EndInjection()`: Lifecycle hook called when implementation is being removed from the chain

**IMPORTANT**: The `EndInjection` lifecycle method has limited guarantees - it will only be invoked during explicit `Eject` and `EjectAll` operations. During application shutdown and object destruction, `EndInjection` calls cannot be guaranteed due to unpredictable destruction order.

### FeatureImpl<TImpl> Template Base Class

The `FeatureImpl<TImpl>` template provides type-safe delegation and chain management:

- Inherits from `Object` and virtually from `TImpl` interface
- Maintains typed `_previousImpl` pointer for safe delegation access
- Provides `Previous()` method returning correctly typed pointer to previous implementation
- Handles automatic type casting and validation in `BeginInjection`
- Offers virtual `BeginInjection(TImpl* previousImpl)` for derived classes to override

### FeatureInjection<TImpl> Manager Class

The `FeatureInjection<TImpl>` class manages the injection chain:

- Maintains `currentImpl` pointer to the active implementation
- `Inject(TImpl* impl)`: Adds new implementation to top of chain (LIFO structure)
- `Eject(TImpl* impl)`: Removes implementation and all subsequent ones from chain
- `EjectAll()`: Removes all injected implementations, restoring default behavior
- `Get()`: Returns current active implementation for feature usage

## Standard Implementation Pattern

### Interface Definition

Define the feature interface inheriting from `IFeatureImpl`:

```cpp
class IMyFeatureImpl : public virtual IFeatureImpl
{
public:
	virtual void DoThis() = 0;
	virtual void DoThat() = 0;
};
```

### Default Implementation

Create default implementation inheriting from `FeatureImpl<TImpl>`:

```cpp
class DefaultMyFeatureImpl : public FeatureImpl<IMyFeatureImpl>
{
public:
	void DoThis() override
	{
		// Default implementation - empty for demo purposes
	}
	
	void DoThat() override
	{
		// Default implementation - empty for demo purposes
	}
};
```

### Global Management Functions

Implement singleton pattern with static local variables for thread safety and initialization order independence:

```cpp
IMyFeatureImpl* GetDefaultMyFeatureImpl()
{
	static DefaultMyFeatureImpl impl;
	return &impl;
}

FeatureInjection<IMyFeatureImpl>& GetMyFeatureInjection()
{
	static FeatureInjection<IMyFeatureImpl> injection(GetDefaultMyFeatureImpl());
	return injection;
}

void InjectMyFeatureImpl(IMyFeatureImpl* impl)
{
	GetMyFeatureInjection().Inject(impl);
}

void EjectMyFeatureImpl(IMyFeatureImpl* impl)
{
	if (impl == nullptr)
	{
		GetMyFeatureInjection().EjectAll();
	}
	else
	{
		GetMyFeatureInjection().Eject(impl);
	}
}
```

## Delegation Mechanism

### Type-Safe Previous Access

Injected implementations can delegate to previous implementations through the `Previous()` method provided by `FeatureImpl<TImpl>`:

```cpp
class DemoMyFeatureImpl : public FeatureImpl<IMyFeatureImpl>
{
public:
	void DoThis() override
	{
		// Demo override - completely replace behavior
	}
	
	void DoThat() override
	{
		// Demo delegation - calls previous implementation
		Previous()->DoThat();
	}
};
```

### Chain Traversal Behavior

- **LIFO Structure**: Last injected implementation becomes current, previous current becomes its predecessor
- **Cascading Ejection**: Ejecting from middle of chain removes all subsequent implementations
- **Type Safety**: `FeatureImpl<TImpl>` ensures `Previous()` returns correctly typed pointer
- **Lifecycle Management**: `BeginInjection`/`EndInjection` called automatically during inject/eject operations

## Real-World Example: DateTime System

### Interface Implementation

The DateTime system demonstrates the complete pattern through `IDateTimeImpl`:

- Extends `IFeatureImpl` with DateTime-specific methods: `FromDateTime`, `FromOSInternal`, `LocalTime`, `UtcTime`, `LocalToUtcTime`, `UtcToLocalTime`, `Forward`, `Backward`
- Platform-specific implementations: `WindowsDateTimeImpl` and `LinuxDateTimeImpl` inherit from `FeatureImpl<IDateTimeImpl>`
- Global management through `GetDateTimeInjection()`, `InjectDateTimeImpl()`, and `EjectDateTimeImpl()`

### Testing Integration

The DateTime tests demonstrate practical usage:

- `MockDateTimeImpl` overrides only `LocalTime()` and `UtcTime()` to return fixed values for deterministic testing
- Other methods like `FromDateTime()` delegate through `Previous()->FromDateTime()` unchanged
- Tests verify proper lifecycle management through call counts on `BeginInjection`/`EndInjection`

## Static Local Variables Pattern

### Critical Design Choice

The use of static local variables in `GetMyFeatureInjection()` and `GetDefaultMyFeatureImpl()` serves multiple purposes:

- **Thread-Safe Singleton**: C++11+ guarantees thread-safe initialization of static locals
- **Global State Management**: Ensures application-wide consistency for feature behavior
- **Lazy Initialization**: Objects created only when first accessed, avoiding startup overhead
- **Initialization Order Independence**: Prevents static initialization order fiasco
- **Automatic Lifetime**: Objects destroyed at program termination without manual cleanup

### Memory and Performance Benefits

- **No Heap Allocation**: Objects live in static storage, avoiding dynamic allocation overhead
- **Guaranteed Availability**: Injection manager and default implementation always available when needed
- **Cross-Module Sharing**: Static locals in functions provide controlled global access without namespace pollution

## Usage Example

### Complete Demo Program

```cpp
void DemoMyFeature()
{
	DemoMyFeatureImpl demoImpl;
	
	// Inject demo implementation
	InjectMyFeatureImpl(&demoImpl);
	
	// Test both methods - DoThis is overridden, DoThat delegates
	GetMyFeatureInjection().Get()->DoThis();  // Uses demo implementation
	GetMyFeatureInjection().Get()->DoThat();  // Delegates to default through Previous()
	
	// Always cleanup to restore default behavior
	EjectMyFeatureImpl(nullptr);
}
```

### Key Usage Principles

- **Always Clean Up**: Use `EjectMyFeatureImpl(nullptr)` to restore default behavior
- **LIFO Injection**: Last injected becomes current implementation
- **Delegation Choice**: Each method can choose to override completely or delegate to previous
- **Error Handling**: Include null pointer checks and descriptive error messages
- **Testing Support**: Perfect for injecting mock implementations during unit tests
- **IMPORTANT**: Restrict injection and ejection calls to application-level code, frameworks, or unit tests only - never from within library code. This constraint exists because injection/ejection operations must maintain strict ordering to prevent crashes. For instance, calling `Inject(a)`, `Inject(b)`, `Eject(a)`, `Eject(b)` will cause `Eject(a)` to remove both implementations, making the subsequent `Eject(b)` call fail. Applications and frameworks maintain global knowledge of the injection state and can ensure proper cleanup ordering, while libraries cannot guarantee safe usage patterns.