# !!!LEARNING!!!

# Orders

- Account for eager child preparation in item-provider tests [1]
- Isolate callbacks per test case (fresh log + callback) [1]
- Prepare nodes (via `GetChildCount`) before asserting nested callbacks [1]

# Refinements

## Account for eager child preparation in item-provider tests

In `TreeViewItemBindableRootProvider` binding scenarios, children are prepared eagerly. Callback parameters like `count`/`newCount` reflect actual prepared child counts, not zero, and expansion is mostly a visual state toggle rather than a "prepare children" trigger. Write expectations accordingly.

## Isolate callbacks per test case (fresh log + callback)

Each test case should create its own callback log and `MockNodeProviderCallback` instance to avoid cross-test state pollution. After setup steps (especially `SetItemSource`), clear the callback log unless the test is explicitly validating setup behavior.

## Prepare nodes (via `GetChildCount`) before asserting nested callbacks

When testing nested `ObservableList` changes (grandchildren and deeper), prepare nodes to the needed depth first (typically by calling `GetChildCount()` on the relevant nodes) so the provider subscribes to list events and callbacks fire on the expected node.
