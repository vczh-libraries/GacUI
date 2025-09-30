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

# TASKS

- [x] TASK No.1: Create comprehensive unit test plan for TextItemProvider
- [ ] TASK No.2: Create comprehensive unit test plan for TextItemBindableProvider  
- [ ] TASK No.3: Create comprehensive unit test plan for ListViewItemProvider
- [ ] TASK No.4: Create comprehensive unit test plan for ListViewItemBindableProvider
- [ ] TASK No.5: Create comprehensive unit test plan for TreeViewItemRootProvider
- [ ] TASK No.6: Create comprehensive unit test plan for TreeViewItemBindableRootProvider

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

**Test Case Design Quality:**
- Create test fixtures with pre-populated providers for complex scenarios
- Use parameterized tests for different TextItem property combinations
- Implement callback verification helpers to ensure correct parameters and ordering
- Create stress tests with large numbers of items to verify performance and stability
- Use RAII patterns to ensure proper cleanup in all test scenarios
- Implement helper methods for common operations (create item, verify callbacks, cleanup)

### rationale

TextItemProvider is foundational for text-based list controls like GuiVirtualTextList. The existing test only covers basic item addition. A comprehensive test plan is needed because:
- TextItemProvider has dual callback interfaces that need coordination testing
- The TextItem-TextItemProvider relationship through the owner field is critical for proper callbacks
- Property changes in TextItem need to propagate correctly to the provider's callbacks
- This is the simplest provider pattern, so understanding it thoroughly will help with more complex providers

## TASK No.2: Create comprehensive unit test plan for TextItemBindableProvider

The TextItemBindableProvider is a bindable version that works with user-defined item types through property bindings. It implements ITextItemView and manages an ObservableList, with property mappings for text and checked states. This task also includes fixing the `Prop_checked` function to create a `WritableItemProperty` instead of `ItemProperty`.

### what to be done

- Fix the `Prop_checked` function implementation to create `WritableItemProperty` instead of `ItemProperty` for proper writable binding
- Expand the existing `TestItemProviders_TextItemBindableProvider.cpp` file with comprehensive test cases
- Test the property binding system with different property configurations (textProperty, checkedProperty)
- Test ObservableList integration and verify callbacks when items are added/removed from the source list
- Test property change notifications and manual callback triggering for user-defined properties
- Test SetItemSource with different IValueEnumerable implementations
- Test UpdateBindingProperties method and its effects
- Test the interaction between ObservableList changes and IItemProviderCallback notifications

### how to test it

**Core Functionality Tests:**
- Test property binding setup with ItemProperty and WritableItemProperty (ensuring fixed checkedProperty)
- Test SetItemSource with ObservableList<Ptr<BindableItem>> and verify callbacks
- Test adding/removing items to/from the source ObservableList
- Test property changes on BindableItem objects and manual callback triggering
- Test UpdateBindingProperties method at different states
- Test GetChecked/SetChecked methods with and without checkedProperty binding

**Code Coverage and Corner Cases:**
- Test null item source handling and verify graceful degradation
- Test unbound properties (textProperty or checkedProperty not set) and ensure safe fallbacks
- Test empty ObservableList behavior and transitions to/from empty state
- Test WritableItemProperty vs ItemProperty behavior for checked state (read-only vs writable)
- Test property binding with invalid property paths or types
- Test UpdateBindingProperties called multiple times and verify idempotency
- Test concurrent access patterns if applicable (multiple callbacks in sequence)
- Test memory management: verify proper cleanup when item source changes
- Test boundary conditions: very large lists, items with null/empty text properties
- Test NotifyUpdate method for manual property change notifications in various states
- Test SetChecked on read-only vs writable properties and verify correct error handling
- Test callback sequence verification: ensure callbacks fire in correct order for bulk operations

**Test Case Design Quality:**
- Use parameterized tests for different property configurations
- Create helper methods for common setup patterns (ObservableList creation, property binding)
- Implement test fixtures for complex scenarios with shared state
- Use mock objects to verify callback invocation patterns and parameters
- Test data-driven scenarios with various item types and property combinations

### rationale

TextItemBindableProvider enables data binding scenarios where list items are user-defined types. This is more complex than TextItemProvider because:
- It requires proper property binding setup and management
- The provider cannot automatically detect property changes in user objects
- ObservableList integration must work correctly for container changes
- Manual notification mechanisms must work for property changes
- This pattern is fundamental for MVVM-style data binding in GacUI

## TASK No.3: Create comprehensive unit test plan for ListViewItemProvider

The ListViewItemProvider manages ListViewItem objects and implements IListViewItemView and IColumnItemView interfaces. It supports images, text, subitems, and column management for ListView controls.

### what to be done

- Expand the existing `TestItemProviders_ListViewItemProvider.cpp` file with comprehensive test cases
- Test ListViewItem management with text, images (small/large), and subitems
- Test column management through IColumnItemView interface (add, remove, resize columns)
- Test callback mechanisms for both IItemProviderCallback and IColumnItemViewCallback
- Test image property management and retrieval
- Test subitem access and modification
- Test column header text and sizing operations
- Test the relationship between data columns and display columns

### how to test it

**Core Functionality Tests:**
- Test ListViewItem creation with various configurations (text, images, subitems)
- Test adding/removing ListViewItems and verify callbacks
- Test ListViewItem property modifications (text, subitems, images)
- Test column operations (add, remove, resize) and verify IColumnItemViewCallback notifications
- Test IListViewItemView methods (GetSmallImage, GetLargeImage, GetText, GetSubItem)
- Test IColumnItemView methods (GetColumnCount, GetColumnText, SetColumnSize)
- Test data column mappings and subitem access patterns
- Test callback attachment/detachment for column callbacks

**Code Coverage and Corner Cases:**
- Test ListViewItem with maximum number of subitems and verify boundary handling
- Test subitem access with invalid indices and verify safe error handling
- Test image property handling with null/invalid image references
- Test column operations with invalid indices or duplicate column operations
- Test column resizing with invalid sizes (negative, zero, extremely large)
- Test simultaneous item and column modifications and verify callback coordination
- Test ListViewItem text and subitem text with null/empty strings
- Test GetSubItem with column indices beyond available subitems
- Test column removal when items have subitems referencing those columns
- Test provider reset with both items and columns present
- Test large-scale operations: many items with many columns and verify performance
- Test IColumnItemViewCallback detachment during column operations
- Test column header text modifications and verify proper notifications
- Test data column vs display column consistency across operations

**Test Case Design Quality:**
- Create test fixtures with pre-configured column layouts for complex scenarios
- Use parameterized tests for different image type combinations (small/large/both/none)
- Implement callback verification systems to ensure proper sequencing of item vs column callbacks
- Create helper methods for common column management patterns
- Use data-driven tests for various subitem configurations and access patterns
- Implement stress tests with large numbers of items and columns
- Create mock image objects to test image property management without external dependencies

### rationale

ListViewItemProvider is more complex than TextItemProvider because:
- It supports multi-column data with headers and sizing
- Image management adds another dimension of complexity
- Multiple callback interfaces need coordination (item and column callbacks)
- Column management operations have their own lifecycle and notifications
- ListView is a commonly used control requiring robust provider implementation

## TASK No.4: Create comprehensive unit test plan for ListViewItemBindableProvider

The ListViewItemBindableProvider is a bindable version for ListView that works with user-defined item types through property bindings. It implements multiple interfaces including IListViewItemView and IColumnItemView, managing both data binding and column structures.

### what to be done

- Expand the existing `TestItemProviders_ListViewItemBindableProvider.cpp` file with comprehensive test cases
- Test property binding for images (largeImageProperty, smallImageProperty) and text
- Test column management with data binding integration
- Test ObservableList integration with complex item types
- Test the coordination between column definitions and property bindings
- Test data column and display column relationship in bindable context
- Test manual property change notifications for complex scenarios

### how to test it

**Core Functionality Tests:**
- Test property binding setup for image and text properties
- Test SetItemSource with ObservableList containing complex bindable items
- Test column management operations with bindable data
- Test GetDataColumns and GetColumns integration with binding
- Test image property retrieval through largeImageProperty and smallImageProperty
- Test subitem access through property bindings and column mappings
- Test RebuildAllItems and RefreshAllItems methods
- Test coordination between column callbacks and item callbacks

**Code Coverage and Corner Cases:**
- Test property binding with invalid property paths or types for images
- Test image property bindings with null references and verify graceful handling
- Test largeImageProperty and smallImageProperty independently and together
- Test column mappings with bindable items that have missing properties
- Test ObservableList modifications concurrent with column operations
- Test RebuildAllItems vs RefreshAllItems behavior differences and use cases
- Test bindable items with dynamic property sets (properties added/removed at runtime)
- Test column binding with items that have heterogeneous property sets
- Test property change notifications in complex column scenarios
- Test SetItemSource with null list followed by non-null list transitions
- Test UpdateBindingProperties interaction with existing column configurations
- Test manual notification scenarios when properties change outside provider control
- Test image property caching behavior and memory management
- Test column header changes coordination with item property bindings
- Test GetDataColumns consistency when underlying data structure changes

**Test Case Design Quality:**
- Create complex bindable item classes with various property configurations for testing
- Use mock property systems to simulate different binding scenarios
- Implement test fixtures that combine column management with data binding
- Create helper methods for validating property binding consistency across operations
- Use parameterized tests for different image property binding combinations
- Implement stress tests with frequent ObservableList and column changes
- Create validation helpers to verify data integrity between columns and bound properties
- Test memory usage patterns with large bindable datasets

### rationale

ListViewItemBindableProvider combines the complexity of ListView functionality with data binding:
- Multiple property bindings must work correctly with column mappings
- Image binding adds complexity over simple text binding
- Column structure must integrate with bindable data sources
- Both item-level and column-level changes need proper notification
- This provider pattern is essential for data-bound ListView scenarios in GacUI applications

## TASK No.5: Create comprehensive unit test plan for TreeViewItemRootProvider

The TreeViewItemRootProvider manages hierarchical TreeViewItem data through MemoryNodeProvider nodes. It implements INodeRootProvider and uses the node-based tree structure for representing hierarchical data.

### what to be done

- Expand the existing `TestItemProviders_TreeViewItemRootProvider.cpp` file with comprehensive test cases
- Test hierarchical node management with MemoryNodeProvider
- Test tree structure operations (add nodes, remove nodes, expand/collapse)
- Test INodeProviderCallback notifications for tree modifications
- Test parent-child relationships and navigation
- Test TreeViewItem data management within nodes
- Test node expansion state management and callbacks
- Test tree traversal and node visibility calculations

### how to test it

**Core Functionality Tests:**
- Test adding MemoryNodeProvider nodes to the root and verify callbacks
- Test creating multi-level tree structures with parent-child relationships
- Test node expansion/collapse operations and verify callbacks
- Test OnBeforeItemModified and OnAfterItemModified callback sequences
- Test OnItemExpanded and OnItemCollapsed callbacks
- Test node removal and tree structure updates
- Test GetMemoryNode operations and node access patterns
- Test tree navigation methods (GetParent, GetChild, GetChildCount)

**Code Coverage and Corner Cases:**
- Test maximum tree depth scenarios and verify performance doesn't degrade
- Test node expansion state persistence across tree modifications
- Test expanding/collapsing nodes that have no children vs those with children
- Test rapid expansion/collapse sequences and verify callback ordering
- Test node removal from middle of tree and verify child node reparenting or removal
- Test removing expanded nodes and verify proper cleanup of expansion state
- Test Before/After callback pairs under error conditions (verify After always follows Before)
- Test tree navigation at boundaries (root node, leaf nodes, invalid indices)
- Test GetChildCount on leaf nodes vs parent nodes
- Test parent-child relationship consistency after complex tree manipulations
- Test MemoryNodeProvider lifecycle management during tree restructuring
- Test node addition to expanded vs collapsed parents
- Test simultaneous tree modifications from multiple sources
- Test tree structure validation after each major operation
- Test TreeViewItem data consistency when nodes are moved or removed

**Test Case Design Quality:**
- Create test fixtures with pre-built tree structures of various shapes (balanced, skewed, deep)
- Implement tree validation helpers to verify structural integrity after operations
- Use recursive test patterns to verify tree operations at all levels
- Create callback verification systems that track Before/After pairs and expansion state changes
- Implement stress tests with large tree structures and frequent modifications
- Use parameterized tests for different tree topology configurations
- Create helper methods for common tree building and navigation patterns
- Implement visualization helpers for debugging complex tree state issues

### rationale

TreeViewItemRootProvider manages complex hierarchical data structures:
- Tree topology changes require careful callback management
- Node expansion state affects visibility and requires separate callbacks
- Parent-child relationships must be maintained correctly
- MemoryNodeProvider integration adds another layer of complexity
- Tree-based controls are commonly used and require robust hierarchical data management
- The callback sequence for tree modifications (Before/After) is critical for UI updates

## TASK No.6: Create comprehensive unit test plan for TreeViewItemBindableRootProvider

The TreeViewItemBindableRootProvider is the most complex provider, combining hierarchical tree structure with data binding. It uses TreeViewItemBindableNode for bindable hierarchical data and supports property bindings for text, images, and children relationships.

### what to be done

- Expand the existing `TestItemProviders_TreeViewItemBindableRootProvider.cpp` file with comprehensive test cases
- Test hierarchical data binding with childrenProperty mappings
- Test TreeViewItemBindableNode creation and management
- Test property bindings for text, images, and hierarchical relationships
- Test ObservableList integration at multiple tree levels
- Test expansion state management in bindable context
- Test manual notification mechanisms for tree structure changes
- Test reverse mapping functionality for node-to-data relationships

### how to test it

**Core Functionality Tests:**
- Test property binding setup for textProperty, imageProperty, and childrenProperty
- Test SetItemSource with hierarchical bindable data structures
- Test TreeViewItemBindableNode creation and parent-child relationships
- Test childrenProperty binding with nested ObservableList structures
- Test node expansion/collapse with bindable data
- Test property changes in hierarchical bindable items
- Test UpdateBindingProperties with tree structure considerations
- Test reverseMappingProperty functionality for data-to-node mapping

**Code Coverage and Corner Cases:**
- Test childrenProperty binding with circular references and verify cycle detection/prevention
- Test property binding with deep hierarchical structures (10+ levels)
- Test ObservableList modifications at different tree levels simultaneously
- Test childrenProperty with null or empty child collections at various tree levels
- Test TreeViewItemBindableNode lifecycle during data source changes
- Test reverseMappingProperty with duplicate data items and verify handling
- Test property changes in parent items affecting child node structure
- Test expansion state management when childrenProperty changes dynamically
- Test UpdateBindingProperties called during active tree traversal
- Test memory management with large hierarchical datasets and frequent changes
- Test textProperty and imageProperty changes in deeply nested nodes
- Test childrenProperty with heterogeneous child types or structures
- Test node-to-data mapping consistency after complex tree manipulations
- Test concurrent property changes at multiple tree levels
- Test SetItemSource transitions between different hierarchical data structures
- Test bindable item property changes when nodes are collapsed vs expanded

**Test Case Design Quality:**
- Create complex hierarchical data classes with various property binding scenarios
- Implement tree data generators for creating test hierarchies of different shapes and sizes
- Use mock property systems to simulate complex binding relationships
- Create test fixtures with pre-configured hierarchical bindable data
- Implement callback verification systems for multi-level tree notifications
- Use parameterized tests for different childrenProperty configurations
- Create stress tests with frequent hierarchical data changes
- Implement helpers for validating tree-data consistency across all binding scenarios
- Test performance characteristics with very large hierarchical datasets
- Create debugging helpers for visualizing complex bindable tree states

### rationale

TreeViewItemBindableRootProvider is the most complex provider combining all challenges:
- Hierarchical structure complexity with data binding
- Multiple levels of property bindings (parent and children)
- ObservableList integration at each tree level
- Node state management (expansion) with bindable data
- Reverse mapping for efficient data-to-node lookups
- Manual notification requirements for complex hierarchical changes
- This provider pattern represents the most advanced data binding scenario in GacUI
- Proper testing ensures robustness for complex tree-based data scenarios

# Impact to the Knowledge Base

## GacUI

The comprehensive testing of ItemProvider classes may require additions to the knowledge base regarding:

- **ItemProvider Pattern**: A new section explaining the ItemProvider architecture, the difference between concrete providers (like TextItemProvider) and bindable providers (like TextItemBindableProvider), and how they integrate with different list controls.

- **Data Binding in List Controls**: Documentation about property binding mechanisms, ObservableList integration, and manual notification patterns for user-defined item types.

- **Callback Coordination**: Explanation of how multiple callback interfaces work together (IItemProviderCallback, ITextItemProviderCallback, IColumnItemViewCallback, INodeProviderCallback) and their invocation sequences.

- **Tree Provider Architecture**: Details about the hierarchical data management through MemoryNodeProvider and TreeViewItemBindableNode, including expansion state management and parent-child relationship maintenance.

However, most of the implementation details can be understood from the source code, so the knowledge base should focus on high-level patterns and design decisions rather than implementation specifics.

# !!!FINISHED!!!
