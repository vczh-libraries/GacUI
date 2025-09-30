# !!!SCRUM!!!

# DESIGN REQUEST
# Problem
There are 6 test files naming after TestItemProviders_*.cpp for:
- TextItemProvider
- TextItemBindableProvider
- ListViewItemProvider
- ListViewItemBindableProvider
- TreeViewItemRootProvider
- TreeViewItemBindableRootProvider

Different list controls has different requirements about what a set of IXXXView interfaces need to be implemented, they are predefined implementations.

A XProvider is a container of a dedicated item type. When anything is changed, the XProvider shoot callbacks

A XBindableProvider works like a XProvider except that the item type is user defined. So it can only track item changing in the container, it doesn't track properties changing in an item. But it still offers methods so that users can do the property changing callback by themselves.

Each test file already has a simple test case, and you can notice how items are updated in each provider and what callbacks are expected to happen.

You will need to make 6 tasks, each for a test plan for each provider, in the order how I listed.

# TASKS
- [ ] TASK No.1: Expand TextItemProvider callback coverage
- [ ] TASK No.2: Expand TextItemBindableProvider observable source coverage
- [ ] TASK No.3: Harden ListViewItemProvider item and column callbacks
- [ ] TASK No.4: Validate ListViewItemBindableProvider binding semantics
- [ ] TASK No.5: Validate TreeViewItemRootProvider node lifecycle callbacks
- [ ] TASK No.6: Validate TreeViewItemBindableRootProvider binding and expansion flows

## TASK No.1: Expand TextItemProvider callback coverage
Build out a comprehensive regression suite for `TextItemProvider` so every mutation path on `TextItem` activates the correct mix of `IItemProviderCallback` and `ITextItemProviderCallback` notifications. This includes range additions/removals via the base `ListProvider`, item edits performed through the item API (text and check state), and the `ITextItemView` interface when accessed through `RequestView`. The goal is to guarantee the non-bindable provider keeps logging sequences consistent no matter which entry point the control uses.

### what to be done
- Augment `TestItemProviders_TextItemProvider.cpp` with new test cases covering batched `AddRange`, `Insert`, `RemoveAt`, and `Clear` calls, verifying start/count/newCount semantics.
- Add tests that mutate existing items via `SetText`, `SetChecked`, and verify the corresponding callback sequences (including `OnItemCheckedChanged`).
- Exercise the `ITextItemView` fetched through `RequestView` to toggle check state and assert notifications still flow through `MockTextItemProviderCallback` and `MockItemProviderCallback`.
- Verify callback detachment by removing the `MockItemProviderCallback` and ensuring subsequent operations no longer append log entries.

### how to test it
- Extend the existing unit test file with multiple `TEST_CASE` blocks that reuse `MockItemProviderCallback` and `MockTextItemProviderCallback`, asserting callback logs via `AssertCallbacks`.
- Ensure edge scenarios (e.g., toggling the same state twice) assert the provider skips redundant notifications.

### rationale
- `TextItemProvider` is the foundational provider consumed by simple list controls; covering every mutation route prevents regressions in higher-level components.
- The provider forwards notifications both through the generic item callback and the text-specific callback, so we must validate both to maintain parity with UI expectations.
- Using `RequestView` mirrors how the control accesses the provider, providing stronger confidence than testing only direct item mutations.
- Detachment coverage is needed to ensure test isolation for later tasks that rely on shared mocks.

## TASK No.2: Expand TextItemBindableProvider observable source coverage
Create a test matrix that stresses `TextItemBindableProvider` with observable item sources, manual update notifications, and writable properties so users’ custom data objects can rely on accurate callback propagation. The plan should validate list delta tracking, explicit property refresh requests, and the opt-in writable check property pathway.

### what to be done
- In `TestItemProviders_TextItemBindableProvider.cpp`, add helper lambdas to build observable lists of `BindableItem` plus a `WritableItemProperty<bool>` that writes back to the `checked` field.
- Add tests covering `SetItemSource` onto observable and read-only enumerables, validating initial `OnItemModified` arguments and subsequent add/remove/replace operations on the backing list.
- Exercise `NotifyUpdate` for valid and invalid ranges, asserting the boolean return value and the presence or absence of callbacks.
- Verify `SetChecked` invoked through the `ITextItemView` updates the data object and enqueues the expected `OnItemModified` entry with `itemReferenceUpdated=false`.

### how to test it
- Introduce multiple `TEST_CASE` entries that perform list mutations, leverage `AssertCallbacks`, and inspect `BindableItem` fields to confirm two-way binding for the writable property.
- For negative range tests, assert the callback log remains unchanged and the provider returns `false`.

### rationale
- Bindable providers rely on user-defined models; ensuring the provider correctly mirrors observable list mutations is critical to prevent silent data discrepancies.
- Exercising `NotifyUpdate` and writable properties documents the intended escape hatch for item-level property changes, aligning with the explanation in the design request.
- Validating both observable and read-only sources reflects real consumption patterns and prevents regressions when switching between container types.

## TASK No.3: Harden ListViewItemProvider item and column callbacks
Design exhaustive tests for `ListViewItemProvider` that confirm it synchronizes item mutations and column metadata updates across both `IItemProviderCallback` and `IColumnItemViewCallback`. Coverage should span sub-item edits, image/tag updates, data column indexing, and column layout changes that trigger rebuild versus refresh flows.

### what to be done
- Extend `TestItemProviders_ListViewItemProvider.cpp` with scenarios for editing sub-items, changing images, and updating tags, verifying each mutation triggers a single `OnItemModified` entry with `itemReferenceUpdated=false`.
- Add tests for removing and inserting multiple items to ensure `start`, `count`, and `newCount` align with list operations.
- Introduce column-focused tests that add, remove, and modify `ListViewColumn` instances (text, size, dropdown, sorting), asserting the expected `OnColumnRebuilt`/`OnColumnChanged` sequences and subsequent `OnItemModified` refreshes.
- Validate `ListViewDataColumns` updates propagate through `NotifyColumnRebuilt` by appending/removing indices and observing both callbacks and item refreshes.

### how to test it
- Use combined callback logs from `MockItemProviderCallback` and `MockColumnItemViewCallback` to assert interleaved notifications for item and column operations.
- Add targeted tests for column size and text changes to ensure `needToRefreshItems` toggles correctly in the column callback log.

### rationale
- `GuiVirtualListView` depends on accurate coordination between item provider and column arranger; wrong callback ordering breaks UI redraws.
- Column metadata modifications are easy to regress because they funnel through observable containers—explicit tests catch mistakes in `NotifyColumnRebuilt` vs `NotifyColumnChanged`.
- Ensuring sub-item and image edits trigger refresh-only notifications protects performance-sensitive code paths that assume `itemReferenceUpdated=false` for in-place updates.

## TASK No.4: Validate ListViewItemBindableProvider binding semantics
Build a test plan that mirrors Task 3 but for the bindable counterpart, confirming observable lists, column data bindings, and manual refresh APIs behave as expected. The suite should also check `RebuildAllItems`, `RefreshAllItems`, and custom data column mappings.

### what to be done
- In `TestItemProviders_ListViewItemBindableProvider.cpp`, create utility fixtures to configure columns, data column indices, and optional image properties on `BindableItem` data.
- Add tests for observable list mutations (add/remove/replace) ensuring the provider delivers correct `OnItemModified` arguments with `itemReferenceUpdated=true` when backing collection changes.
- Cover `UpdateBindingProperties`, `RebuildAllItems`, and `RefreshAllItems`, verifying when each produces `itemReferenceUpdated=true` vs `false` and how column callbacks are affected.
- Validate bound text and sub-item values respond to data changes by mutating `BindableItem` fields and invoking `NotifyUpdate`, then asserting the column callback log stays untouched while the item callback reports a refresh.

### how to test it
- Combine callback logs for item and column observers, using dedicated `TEST_CASE`s to isolate collection deltas from property refresh scenarios.
- Inspect the underlying `BindableItem` list to confirm property changes flowed through the writable image/text properties when configured.

### rationale
- Bindable list views are widely used in MVVM scenarios; tests must demonstrate the provider’s guarantees around observable collections and manual refresh hooks.
- Distinguishing between rebuild and refresh events protects virtual list arrangers from unnecessary layout invalidation.
- Verifying property updates without column notifications ensures developers can update data bindings without retriggering structural changes.

## TASK No.5: Validate TreeViewItemRootProvider node lifecycle callbacks
Construct tests around `TreeViewItemRootProvider` to confirm it propagates node additions, removals, data updates, and explicit refresh requests through `INodeProviderCallback`. The suite should leverage `MemoryNodeProvider` children to simulate nested trees, ensuring before/after notifications fire with accurate parameters.

### what to be done
- Expand `TestItemProviders_TreeViewItemRootProvider.cpp` with fixtures that build small hierarchies of `TreeViewItem` wrapped in `MemoryNodeProvider` instances attached to the root provider.
- Add tests for inserting, removing, and reparenting child nodes, verifying both `OnBeforeItemModified` and `OnAfterItemModified` log expected start/count/newCount tuples.
- Cover `SetTreeViewData` and `UpdateTreeViewData` to ensure edits on an existing node trigger a refresh (`itemReferenceUpdated=false`) without structural changes.
- Validate `GetTreeViewData`/`SetTreeViewData` round-trip by modifying item text/image and asserting the provider reflects those changes when queried.

### how to test it
- Use `MockNodeProviderCallback` to capture notification sequences for each mutation scenario, asserting logs via `AssertCallbacks` or custom expectations when counts grow large.
- Include nested node operations to confirm callbacks reference the correct parent node context.

### rationale
- Tree-based controls rely on balanced before/after callbacks for virtualization; missing notifications cause incorrect row counts and crashes.
- Exercising both structural and data update pathways ensures the provider behaves consistently when consumers mutate `TreeViewItem` directly.
- Verifying helper methods (`GetTreeViewData`, `SetTreeViewData`) protects public API guarantees that many tests and utility code depend on.

## TASK No.6: Validate TreeViewItemBindableRootProvider binding and expansion flows
Develop an extensive test strategy for `TreeViewItemBindableRootProvider` covering observable child collections, reverse mapping support, node expansion state transitions, and image/text bindings. The focus is to prove the provider’s bindable data handling mirrors the non-bindable root provider while supporting user-defined models.

### what to be done
- In `TestItemProviders_TreeViewItemBindableRootProvider.cpp`, add helper routines to create bindable trees of `BindableItem`, including optional image properties and a `WritableItemProperty<Value>` for reverse mapping verification.
- Add tests for adding, removing, and replacing children within the observable `children` list, verifying before/after callbacks and ensuring `reverseMappingProperty` stores the `TreeViewItemBindableNode` pointer in each item.
- Cover `UpdateBindingProperties` with both `updateChildrenProperty=true` and `false`, observing the differences in callback arguments and child cache rebuilding.
- Exercise `SetExpanding` on child nodes to trigger `OnItemExpanded`/`OnItemCollapsed`, and validate `GetNodeImage`/`GetTextValue` return the bound properties.

### how to test it
- Chain multiple `OnNextIdleFrame`-free unit tests that manipulate the bindable tree while asserting callback logs captured by `MockNodeProviderCallback`.
- Inspect bound `BindableItem` instances after operations to confirm reverse mapping assignments and image properties remain in sync.

### rationale
- Bindable tree views are complex; without a rigorous test matrix, regressions in reverse mapping or expansion handling can slip in unnoticed.
- Validating both manual updates and observable notifications ensures users can choose either strategy without losing callback fidelity.
- Confirming visual property bindings (text/image) aligns with how `GuiBindableTreeView` consumes the provider, bolstering confidence in higher-level controls built atop it.

# Impact to the Knowledge Base
## GacUI
No updates required; existing guidance on item providers and binding already covers the documented behaviors these tests validate.

# !!!FINISHED!!!
