# !!!LEARNING!!!

# Orders

- Account for eager child preparation in item-provider tests [2]
- Isolate callbacks per test case (fresh log + callback) [2]
- Prepare nodes (via `GetChildCount`) before asserting nested callbacks [1]
- Empty item sources and empty `childrenProperty` only trigger `OnAttached` [1]
- Use `TEST_ERROR` for invalid bindings and out-of-range access [1]
- `DetachCallback()` should fire `OnAttached(provider=nullptr)` [1]

# Refinements

## Account for eager child preparation in item-provider tests

In `TreeViewItemBindableRootProvider` binding scenarios, children are prepared eagerly. Callback parameters like `count`/`newCount` reflect actual prepared child counts, not zero, and expansion is mostly a visual state toggle rather than a "prepare children" trigger. Write expectations accordingly.

## Isolate callbacks per test case (fresh log + callback)

Each test case should create its own callback log and `MockNodeProviderCallback` instance to avoid cross-test state pollution. After setup steps (especially `SetItemSource`), clear the callback log unless the test is explicitly validating setup behavior.

## Prepare nodes (via `GetChildCount`) before asserting nested callbacks

When testing nested `ObservableList` changes (grandchildren and deeper), prepare nodes to the needed depth first (typically by calling `GetChildCount()` on the relevant nodes) so the provider subscribes to list events and callbacks fire on the expected node.

## Empty item sources and empty `childrenProperty` only trigger `OnAttached`

In `TreeViewItemBindableRootProvider` tests, setting an empty item source / empty `childrenProperty` collection should only cause `OnAttached` to fire (with `newCount=0`) and should not produce `OnItemModified` noise. For null/empty children scenarios, assert the provider is safe and that callback counts reflect "nothing to prepare".

## Use `TEST_ERROR` for invalid bindings and out-of-range access

When a bindable tree is misconfigured (e.g. missing/invalid `childrenProperty`, inconsistent children types), or when tests intentionally access nodes with invalid indices (including expand/collapse paths), validate the failure with `TEST_ERROR(...)` instead of letting the test crash or accidentally pass.

## `DetachCallback()` should fire `OnAttached(provider=nullptr)`

Detaching a callback is observable: `DetachCallback()` should invoke `OnAttached(provider=nullptr)` to signal cleanup. When the test is validating callback logs around detachment, clear logs after the detachment step (not before), and ensure subsequent operations do not reach the detached callback.
