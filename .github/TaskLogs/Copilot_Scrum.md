# !!!SCRUM!!!

# DESIGN REQUEST

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

## UPDATE

#file:win-0-scrum.prompt.md 
It is good that you already figured out what to test, but I want you to add the following things:

- In TextItemBindableProvider, it looks like the `Prop_checked` function is creating a `ItemProperty` instead of `WritableItemProperty`, you can just update it.
- In each provider, you mentioned what to test, but you didn't mention how to ensure code coverage and how to test corner cases. Please also pay attention to the quality of test case design. I would like you to think more about test case design.

## UPDATE

#file:win-0-scrum.prompt.md 
You have finished the first task, details is logged in #file:Copilot_Task.md  and #file:Copilot_Planning.md . In those files you can see how you original worked out a plan and how I made adjustments.

Summarize the findings about my philosophy in making test cases for TextItemProvider from all the "UPDATE", and adjust unfinished tasks

## UPDATE

#file:win-0-scrum.prompt.md 
Actually I want to use TEST_ERROR instead of TEST_EXCEPTION for all index out of bound testings. And the implementation should use CHECK_ERROR (or calls into any function that already use CHECK_ERROR, like `Get` for container types).

## UPDATE

#file:win-0-scrum.prompt.md 
For all bindable version of providers, it is not possible to detect if a `Value` has been added before. But at least for ListViewItemBindableProvider, ListViewColumn still need to assume unique.

Please check unfinished task and update them

# TASKS

- [x] TASK No.1: Create comprehensive unit test plan for TextItemProvider
- [x] TASK No.2: Create comprehensive unit test plan for TextItemBindableProvider  
- [x] TASK No.3: Create comprehensive unit test plan for ListViewItemProvider
- [ ] TASK No.4: Create comprehensive unit test plan for ListViewItemBindableProvider
- [ ] TASK No.5: Create comprehensive unit test plan for TreeViewItemRootProvider
- [ ] TASK No.6: Create comprehensive unit test plan for TreeViewItemBindableRootProvider

## Testing Philosophy Summary

Based on the completed TextItemProvider, TextItemBindableProvider, and ListViewItemProvider tasks and your feedback updates, the testing philosophy emphasizes:

**Focus on Behavioral Testing, Not Internal State:**
- Avoid testing provider content directly (Count(), Get()) as these are ensured by other tests
- Focus on callback behavior and interface contracts rather than internal storage verification
- Skip testing private/inaccessible fields like `owner` - focus on observable behavior

**Minimal Test Infrastructure:**
- Use existing mock callbacks without creating complex assertion frameworks
- Avoid unnecessary helper classes like `AssertCallbackSequence` - test names provide sufficient context  
- Simple helper functions (like `CreateTextItem`, `CreateListViewItem`, `CreateListViewColumn`) are sufficient for reducing duplication
- Callback verification through direct log comparison using `AssertCallbacks` is adequate

**Quality Through Practical Edge Cases:**
- Test realistic scenarios like duplicate item handling with CHECK_ERROR (detected via TEST_ERROR)
- Focus on callback coordination and detachment scenarios that reflect real usage
- Avoid performance stress tests - focus on correctness testing
- Remove unrealistic tests like provider destruction without actual verification
- Test actual user operations like `RemoveRange` in bulk operations
- Always merge RequestView and dynamic_cast in the same line for clarity

**Callback-Centric Testing:**
- Verify callback sequences and coordination between multiple callback interfaces
- Test callback behavior before/after provider attachment
- Ensure proper callback detachment behavior
- Focus on callback parameter correctness and ordering
- Use `TEST_ERROR` for out-of-bounds operations (implementations should use CHECK_ERROR or call functions like `Get` that use CHECK_ERROR)
- Verify specific callback types trigger for operations (e.g., OnColumnRebuilt for column additions)

**Interface Validation:**
- Test both direct method calls and interface method equivalence
- Verify RequestView functionality and interface polymorphism
- Test error handling through interface methods (invalid indices, etc.)
- Skip interface methods that are impractical to test (GetSmallImage, GetLargeImage, GetDropdownPopup)

**MockCallback Usage Guidelines:**
- Only use MockTextItemProviderCallback when the provider actually supports ITextItemProviderCallback
- TextItemBindableProvider only supports IItemProviderCallback - MockTextItemProviderCallback should not be used
- SetChecked operations in bindable providers trigger OnItemModified, not text-specific callbacks
- For dual-callback providers (like ListViewItemProvider), use both IItemProviderCallback and IColumnItemViewCallback mocks

**Empty ItemSource Handling:**
- EmptyItemSourceScenarios should expect only OnAttached callback, not OnItemModified
- Setting ItemSource to empty ObservableList should not trigger OnItemModified callback

**Pragmatic Test Scope (from Task No.3):**
- Skip testing methods that require complex object creation (images, dropdown popups)
- Test duplicate detection for concrete objects (TextItem, ListViewItem, ListViewColumn, TreeViewItem nodes) - adding same instance twice should fail with CHECK_ERROR
- Skip duplicate detection for bindable providers' Value objects - they cannot be reliably detected as duplicates
- Exception: ListViewItemBindableProvider columns are still ListViewColumn objects and should be tested for duplicates
- Use BoxValue<T>() syntax instead of Value::From() for boxing values

**Duplicate Detection Rules:**
- Concrete providers (TextItemProvider, ListViewItemProvider, TreeViewItemRootProvider): Test duplicate item detection
- Bindable providers (TextItemBindableProvider, TreeViewItemBindableRootProvider): Skip duplicate item detection (Value objects)
- ListViewItemBindableProvider: Skip duplicate item detection, but still test duplicate column detection (columns are concrete ListViewColumn objects)

This philosophy prioritizes practical, maintainable tests that verify the provider's contract and behavior rather than implementation details.

## TASK No.1: Create comprehensive unit test plan for TextItemProvider

The TextItemProvider is a container for TextItem objects that implements both IItemProvider and ITextItemView interfaces. It manages TextItems with text and check states, providing callbacks when items are modified or checked state changes.

### what to be done

- Expand the existing `TestItemProviders_TextItemProvider.cpp` file with comprehensive test cases
- Cover all lifecycle scenarios: creation, addition, removal, modification of TextItems
- Test callback mechanisms for both `IItemProviderCallback` (OnItemModified, OnAttached) and `ITextItemProviderCallback` (OnItemCheckedChanged)
- Test TextItem property changes (text and checked state) and verify they trigger proper callbacks
- Test edge cases like multiple additions, batch operations, invalid operations
- Test interaction between TextItem and TextItemProvider through the owner relationship
- Verify proper cleanup when items are removed from the provider

### how to test it

**Core Functionality Tests:**
- Test item addition with various TextItem configurations (empty, with text, with checked state)
- Test item removal and verify callbacks and cleanup
- Test TextItem.SetText() and TextItem.SetChecked() and verify OnItemModified callbacks
- Test TextItem.SetChecked() specifically triggers OnItemCheckedChanged callback
- Test bulk operations like adding multiple items and verify callback sequences
- Test provider destruction and verify proper cleanup
- Test ITextItemView interface methods (GetChecked, SetChecked) through the provider
- Test RequestView method returns correct interface

**Code Coverage and Corner Cases:**
- Test TextItem creation with null/empty text and verify safe handling
- Test adding the same TextItem instance multiple times and verify error handling or deduplication
- Test removing non-existent items and verify graceful error handling
- Test TextItem property changes before and after being added to provider
- Test callback detachment scenarios and verify no callbacks fire after detachment
- Test concurrent modifications if applicable (rapid add/remove sequences)
- Test provider with maximum capacity scenarios and boundary conditions
- Test TextItem owner field management: verify proper setting/clearing during add/remove
- Test callback parameter validation: ensure correct item indices and change types
- Test ITextItemProviderCallback vs IItemProviderCallback coordination and ordering
- Test provider reset scenarios (clear all items) and verify comprehensive cleanup
- Test TextItem lifecycle: creation, addition, modification, removal, destruction



### rationale

TextItemProvider is foundational for text-based list controls like GuiVirtualTextList. The existing test only covers basic item addition. A comprehensive test plan is needed because:
- TextItemProvider has dual callback interfaces that need coordination testing
- The TextItem-TextItemProvider relationship through the owner field is critical for proper callbacks
- Property changes in TextItem need to propagate correctly to the provider's callbacks
- This is the simplest provider pattern, so understanding it thoroughly will help with more complex providers

## TASK No.2: Create comprehensive unit test plan for TextItemBindableProvider

The TextItemBindableProvider is a bindable version that works with user-defined item types through property bindings. It implements ITextItemView and manages an ObservableList, with property mappings for text and checked states. This task also includes fixing the `Prop_checked` function to create a `WritableItemProperty` instead of `ItemProperty`.

### what to be done

- Fix `Prop_checked` function in TextItemBindableProvider to create `WritableItemProperty` instead of `ItemProperty`
- Expand the existing `TestItemProviders_TextItemBindableProvider.cpp` file with comprehensive test cases
- Test property binding mechanism for text and checked state mappings
- Test ObservableList integration and callback propagation from list changes to provider callbacks
- Test user-defined item type scenarios with various property configurations
- Test interface methods through ITextItemView for bindable scenarios
- Test callback coordination between ObservableList callbacks and provider callbacks

### how to test it

**Core Functionality Tests:**
- Test item addition through ObservableList with property binding verification
- Test item removal through ObservableList and verify callback sequences
- Test property changes in user-defined items through property bindings
- Test ITextItemView interface methods (GetChecked/SetChecked) through property bindings
- Test RequestView functionality for bindable provider
- Test WritableItemProperty vs ItemProperty behavior after the fix

**Bindable-Specific Scenarios:**
- Test user-defined item types with different property combinations
- Test property binding setup and teardown scenarios
- Test ObservableList direct manipulation vs provider method differences
- Test property change notifications propagating correctly through bindings
- Test callback coordination between ObservableList and ITextItemProviderCallback
- Test property binding with null/invalid user item scenarios

**Edge Cases and Error Handling:**
- Test invalid property bindings and graceful error handling
- Test user item property changes without proper binding setup
- Test ObservableList operations with invalid indices
- Test property binding with items that don't support required properties
- Test callback detachment scenarios in bindable context
- Test interface method calls on empty or invalid bindable providers

**Callback and Interface Testing:**
- Verify ObservableList change notifications trigger proper provider callbacks
- Test dual callback system coordination (IItemProviderCallback + ITextItemProviderCallback)
- Test property binding updates trigger OnItemCheckedChanged appropriately
- Test interface method equivalence between direct property access and ITextItemView methods
- Test callback parameter accuracy for bindable item operations

### rationale

TextItemBindableProvider represents the bridge between user-defined data models and GacUI's provider system. The existing bug in `Prop_checked` creating `ItemProperty` instead of `WritableItemProperty` prevents proper checked state modification. A comprehensive test plan is needed because:
- The property binding mechanism is fundamental to data-driven UI scenarios
- ObservableList integration needs verification to ensure callback propagation works correctly  
- The WritableItemProperty fix needs validation to ensure bidirectional property binding
- This pattern will be repeated in ListView and TreeView bindable providers
- Following the established testing philosophy: focus on callback behavior, avoid internal state testing, use minimal infrastructure, and emphasize practical edge cases that reflect real usage scenarios

## TASK No.3: Create comprehensive unit test plan for ListViewItemProvider

The ListViewItemProvider manages ListViewItem objects and implements IListViewItemView and IColumnItemView interfaces. It supports images, text, subitems, and column management for ListView controls.

### what to be done

- Expand the existing `TestItemProviders_ListViewItemProvider.cpp` file with comprehensive test cases
- Test ListViewItem lifecycle operations (add, remove, modify) and callback behavior
- Test column management through IColumnItemView interface operations and callbacks
- Test dual callback system coordination (IItemProviderCallback + IColumnItemViewCallback)
- Test interface methods for item data access (IListViewItemView) and column operations
- Test subitem access patterns and edge cases

### how to test it

**Core Functionality Tests:**
- Test ListViewItem addition/removal with callback verification
- Test ListViewItem property changes (text, images, subitems) and callback sequences
- Test column operations (add, remove, resize) with IColumnItemViewCallback notifications
- Test IListViewItemView interface methods (GetSmallImage, GetLargeImage, GetText, GetSubItem)
- Test IColumnItemView interface methods and their callback behavior
- Test RequestView functionality for both interfaces (merge RequestView and dynamic_cast)

**Column and Item Integration:**
- Test column-item relationship scenarios (adding items with various subitem counts)
- Test column removal impact on existing item subitems
- Test dual callback coordination when both items and columns change
- Test callback detachment scenarios for both callback types
- Test interface method equivalence between direct access and interface calls

**Edge Cases:**
- Use `TEST_ERROR` for subitem access with invalid column indices
- Use `TEST_ERROR` for column operations with invalid parameters (negative sizes, invalid indices)
- Test item operations with null/empty text and image references
- Use `TEST_ERROR` for GetSubItem beyond available subitems instead of expecting safe fallback
- Test callback ordering when simultaneous item and column operations occur
- Test RemoveRange operations in addition to individual removals

### rationale

ListViewItemProvider extends TextItemProvider concepts with multi-column data and dual callback interfaces. Following the established testing philosophy:
- Focus on callback behavior rather than internal state (avoid testing Count(), column storage details)
- Use minimal test infrastructure leveraging existing mock callbacks
- Test practical edge cases like subitem access beyond column bounds
- Verify interface method contracts and callback coordination
- This provider pattern establishes the foundation for ListView bindable testing

## TASK No.4: Create comprehensive unit test plan for ListViewItemBindableProvider

The ListViewItemBindableProvider is a bindable version for ListView that works with user-defined item types through property bindings. It implements multiple interfaces including IListViewItemView and IColumnItemView, managing both data binding and column structures.

### what to be done

- Expand the existing `TestItemProviders_ListViewItemBindableProvider.cpp` file with comprehensive test cases
- Test property binding mechanism for text properties (skip image properties as they're hard to test)
- Test ObservableList integration with ListView-specific data and callback propagation
- Test column management coordination with property bindings
- Test dual callback system for both item changes and column changes
- Test interface methods through property bindings for ListView scenarios
- Test duplicate item detection if the provider supports it

### how to test it

**Core Functionality Tests:**
- Test property binding setup for text properties with various configurations (skip image testing per Task No.3 learnings)
- Test SetItemSource with ObservableList and verify callback propagation (only OnItemModified, not text-specific callbacks)
- Test item property changes through bindings trigger OnItemModified callbacks
- Test column operations coordination with bindable data - verify OnColumnRebuilt triggers for column additions
- Test IListViewItemView interface methods through property bindings (skip GetSmallImage/GetLargeImage)
- Test IColumnItemView interface methods in bindable context (skip GetDropdownPopup)
- Test RequestView functionality for bindable ListView provider (merge RequestView and dynamic_cast)

**Bindable ListView Integration:**
- Test subitem access through property bindings and column mappings
- Test ObservableList changes coordinated with column structure changes
- Test manual property change notifications for ListView-specific scenarios
- Test callback coordination using only IItemProviderCallback and IColumnItemViewCallback
- Test property binding with items that have varying subitem properties
- Test RemoveRange operations on ObservableList and verify callback behavior
- Skip testing duplicate bindable items (Value objects cannot be reliably detected as duplicates)

**Edge Cases:**
- Use `TEST_ERROR` for property bindings for subitems beyond available properties
- Test column operations when bindable items lack expected properties
- Test SetItemSource transitions (null to non-null, complex to simple) - empty ItemSource should only trigger OnAttached
- Test callback detachment scenarios in bindable ListView context
- Use `TEST_ERROR` for out-of-bounds operations instead of graceful handling
- Test adding duplicate columns (should trigger CHECK_ERROR detected by TEST_ERROR per Task No.3 learnings)

### rationale

ListViewItemBindableProvider combines ListView multi-column functionality with data binding complexity. Following the established testing philosophy including Task No.3 learnings:
- Focus on callback behavior and property binding mechanisms rather than internal storage
- Skip image property testing as it's impractical to create image objects in unit tests (lesson from Task No.3)
- Test duplicate detection for columns only (lesson from Task No.3) - bindable items cannot be tested for duplicates as Value objects cannot be reliably detected
- Verify callback sequences, especially OnColumnRebuilt for column operations (lesson from Task No.3)
- Use minimal test infrastructure with existing mock callbacks
- Emphasize edge cases that reflect real ListView data binding usage
- Verify interface method contracts work correctly through property bindings
- This builds upon TextItemBindableProvider patterns with ListView-specific multi-column complexity

## TASK No.5: Create comprehensive unit test plan for TreeViewItemRootProvider

The TreeViewItemRootProvider manages hierarchical TreeViewItem data through MemoryNodeProvider nodes. It implements INodeRootProvider and uses the node-based tree structure for representing hierarchical data.

### what to be done

- Expand the existing `TestItemProviders_TreeViewItemRootProvider.cpp` file with comprehensive test cases
- Test hierarchical node operations and their callback behavior
- Test tree expansion/collapse operations and callback sequences
- Test INodeProviderCallback notifications for tree structure changes
- Test node lifecycle management (creation, addition, removal)
- Test interface methods for tree navigation and data access (skip image-related methods)
- Test duplicate node detection if applicable

### how to test it

**Core Functionality Tests:**
- Test MemoryNodeProvider node addition to root and verify callback sequences
- Test multi-level tree construction with parent-child relationships
- Test node expansion/collapse operations and verify callback pairs (Before/After)
- Test OnItemExpanded and OnItemCollapsed callback behavior
- Test node removal and callback notification
- Test GetMemoryNode and tree navigation interface methods (skip image retrieval methods per Task No.3 learnings)
- Test RequestView functionality for INodeRootProvider (merge RequestView and dynamic_cast)

**Tree Structure Operations:**
- Test adding child nodes to existing nodes and verify callback sequences
- Test removing nodes with children and verify proper cleanup
- Test expansion state changes and their callback behavior
- Test Before/After callback pairs coordination and ordering
- Test callback detachment scenarios during tree operations
- Test tree structure modifications during expansion/collapse operations
- Test RemoveRange operations on tree nodes if applicable
- Test adding duplicate TreeViewItem nodes (should trigger CHECK_ERROR detected by TEST_ERROR per Task No.3 learnings)

**Edge Cases:**
- Test expanding nodes that have no children vs nodes with children
- Test removing expanded nodes and verify proper state cleanup
- Test expansion/collapse of already expanded/collapsed nodes
- Use `TEST_ERROR` for node operations with invalid node references
- Test rapid expansion/collapse sequences and callback ordering
- Use `TEST_ERROR` for out-of-bounds tree operations instead of graceful handling

### rationale

TreeViewItemRootProvider introduces hierarchical data management with expansion state tracking. Following the established testing philosophy including Task No.3 learnings:
- Focus on callback behavior for tree operations rather than internal tree structure storage
- Skip image-related method testing as it's impractical to create image objects in unit tests (lesson from Task No.3)
- Test duplicate TreeViewItem node detection to ensure CHECK_ERROR is triggered (lesson from Task No.3)
- Test practical tree scenarios like expansion state management and node lifecycle
- Use existing mock callback infrastructure to verify callback sequences
- Emphasize edge cases around expansion state and node removal scenarios
- Verify interface contracts for tree navigation methods
- This establishes patterns for testing hierarchical data providers that will be used in bindable tree testing


### rationale

TreeViewItemRootProvider introduces hierarchical data management with expansion state tracking. Following the established testing philosophy:
- Focus on callback behavior for tree operations rather than internal tree structure storage
- Test practical tree scenarios like expansion state management and node lifecycle
- Use existing mock callback infrastructure to verify callback sequences
- Emphasize edge cases around expansion state and node removal scenarios
- Verify interface contracts for tree navigation methods
- This establishes patterns for testing hierarchical data providers that will be used in bindable tree testing

## TASK No.6: Create comprehensive unit test plan for TreeViewItemBindableRootProvider

The TreeViewItemBindableRootProvider is the most complex provider, combining hierarchical tree structure with data binding. It uses TreeViewItemBindableNode for bindable hierarchical data and supports property bindings for text and children relationships.

### what to be done

- Expand the existing `TestItemProviders_TreeViewItemBindableRootProvider.cpp` file with comprehensive test cases
- Test hierarchical property binding with childrenProperty for tree structure
- Test ObservableList integration at multiple tree hierarchy levels
- Test expansion state management coordination with bindable data changes
- Test callback behavior for tree structure changes through data binding
- Test interface methods for hierarchical navigation in bindable context (skip image-related methods)
- Test duplicate item detection if applicable

### how to test it

**Core Functionality Tests:**
- Test property binding setup for textProperty and childrenProperty (skip imageProperty per Task No.3 learnings)
- Test SetItemSource with hierarchical bindable data and verify callback sequences (only OnItemModified for structure changes)
- Test childrenProperty binding with nested ObservableList structures
- Test node expansion/collapse operations with bindable data coordination
- Test property changes in bindable items trigger OnItemModified callbacks (not tree-specific callbacks if they don't exist)
- Test TreeViewItemBindableNode creation and parent-child relationship management
- Test UpdateBindingProperties coordination with tree structure
- Test interface methods for tree navigation through property bindings (merge RequestView and dynamic_cast, skip image-related methods)

**Hierarchical Binding Scenarios:**
- Test multi-level ObservableList changes and their callback propagation
- Test childrenProperty modifications and their impact on tree structure
- Test expansion state coordination when underlying data structure changes
- Test parent item property changes affecting child node visibility/structure
- Test callback coordination using only existing callback interfaces
- Test manual notification scenarios for hierarchical property changes
- Test RemoveRange operations on hierarchical ObservableList structures
- Skip testing duplicate bindable items in the tree structure (Value objects cannot be reliably detected as duplicates)

**Edge Cases:**
- Test childrenProperty with null or empty child collections - empty should only trigger OnAttached
- Use `TEST_ERROR` for property binding with missing or invalid childrenProperty
- Test SetItemSource transitions between different hierarchical structures
- Test expansion state when childrenProperty changes dynamically
- Test callback detachment scenarios during tree structure modifications
- Use `TEST_ERROR` for out-of-bounds operations in hierarchical context

### rationale

TreeViewItemBindableRootProvider represents the culmination of provider pattern complexity combining tree hierarchy with data binding. Following the established testing philosophy including Task No.3 learnings:
- Focus on callback behavior for hierarchical operations rather than internal tree storage
- Skip image property testing as it's impractical to create image objects in unit tests (lesson from Task No.3)
- Skip duplicate item detection as bindable items are Value objects that cannot be reliably detected as duplicates
- Test practical scenarios like childrenProperty binding and expansion state coordination
- Use minimal test infrastructure leveraging existing mock callbacks
- Emphasize edge cases around hierarchical data binding that reflect real usage
- Verify interface method contracts work correctly through property bindings across tree levels
- This completes the provider testing pattern progression from simple (TextItemProvider) to complex hierarchical data binding

# Impact to the Knowledge Base

## GacUI

The comprehensive testing of ItemProvider classes may require additions to the knowledge base regarding:

- **ItemProvider Pattern**: A new section explaining the ItemProvider architecture, the difference between concrete providers (like TextItemProvider) and bindable providers (like TextItemBindableProvider), and how they integrate with different list controls.

- **Data Binding in List Controls**: Documentation about property binding mechanisms, ObservableList integration, and manual notification patterns for user-defined item types.

- **Callback Coordination**: Explanation of how multiple callback interfaces work together (IItemProviderCallback, ITextItemProviderCallback, IColumnItemViewCallback, INodeProviderCallback) and their invocation sequences.

- **Tree Provider Architecture**: Details about the hierarchical data management through MemoryNodeProvider and TreeViewItemBindableNode, including expansion state management and parent-child relationship maintenance.

However, most of the implementation details can be understood from the source code, so the knowledge base should focus on high-level patterns and design decisions rather than implementation specifics.

# !!!FINISHED!!!
