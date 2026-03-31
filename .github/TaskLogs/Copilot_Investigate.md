# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Running GacUICompiler may end up changing the generated Workflow/C++/binary resource non-deterministically. The suspected reason is that sometimes members of generated workflow classes change in their order, causing generated C++ source code and binary resource to change.

Observations:
1. Running GacUICompiler multiple times produces different outputs even when no XML source files have changed.
2. The member order in generated workflow classes can change between runs.
3. Sometimes when C++/txt files are NOT changed, binary resource STILL changes, suggesting there might be additional non-determinism in the Workflow-to-binary serialization beyond just member ordering.
4. GacUICompiler generates: txt files (generated Workflow script from XML), C++ source files (generated from Workflow script), and binary resources (containing binary representation of generated Workflow script). The resource part is stable, but binary representation of Workflow script is not.

Verification method: Run GacUICompiler once and commit all files. Then run GacUICompiler 3 more times and check `git status` / `git diff` to see if any files changed.

# UPDATES

# TEST

N/A

The problem cannot be verified by unit tests. Instead, the verification method is:
1. Run GacUICompiler once and commit all output files.
2. Run GacUICompiler 3 more times.
3. After each run, check `git status` to see if any files changed.
4. If files changed, use `git diff` on txt/cpp files to identify what changed.
5. Success criteria: no files should change after repeated runs of GacUICompiler.

# TEST [CONFIRMED]

Before the fix, running GacUICompiler produces 20 changed binary files (.bin) every run, even though txt/cpp files remain stable. After the fix, 3 consecutive runs of GacUICompiler produced zero changes. All 1686 unit test cases pass.

# PROPOSALS

- No.1 Sort metadata collections by deterministic keys instead of pointer addresses [CONFIRMED]

## No.1 Sort metadata collections by deterministic keys instead of pointer addresses

Root cause: Two sources of non-determinism in Workflow binary serialization:

**Source 1: `WfWriterContextPrepare` in `VlppWorkflowRuntime.cpp`**
Uses `SortedList<ITypeDescriptor*>`, `SortedList<IMethodInfo*>`, `SortedList<IPropertyInfo*>`, `SortedList<IEventInfo*>` to collect metadata during serialization. `SortedList` sorts by `operator<=>` which for raw pointers compares memory addresses. These addresses change between runs due to heap allocation order and ASLR. The sorted order determines integer indices assigned to each metadata item, which are embedded throughout the binary stream (in type references, instruction operands, etc.).

**Source 2: `GenerateAssembly` in `VlppWorkflowCompiler.cpp`**
Iterates `manager->declarationTypes.Values()` to populate `assembly->typeImpl->classes/interfaces/structs/enums`. `declarationTypes` is `Dictionary<Ptr<WfDeclaration>, Ptr<ITypeDescriptor>>`, which sorts keys by `Ptr<WfDeclaration>` pointer address. This makes the order of custom types in the assembly non-deterministic, affecting both binary serialization order and custom type index assignment.

### CODE CHANGE

**File: `Import/VlppWorkflowRuntime.cpp`**

1. Added helper functions for deterministic sort keys:
   - `GetMethodSortKey(IMethodInfo*)`: Returns `ownerTypeName::methodName(param1TypeFriendlyName,param2TypeFriendlyName,...)`  
   - `GetPropertySortKey(IPropertyInfo*)`: Returns `ownerTypeName::propertyName`
   - `GetEventSortKey(IEventInfo*)`: Returns `ownerTypeName::eventName`

2. Modified `IOPrepare` (writer path): After collecting metadata into `WfWriterContextPrepare` (which still uses `SortedList` for efficient `Contains` checks during collection), copy contents to `List<>` and sort by deterministic string keys:
   - `sortedTds`: sorted by `GetTypeName()`
   - `sortedMis`: sorted by `GetMethodSortKey()`
   - `sortedPis`: sorted by `GetPropertySortKey()`
   - `sortedEis`: sorted by `GetEventSortKey()`
   Build writer context indices from these sorted lists. Iterate them for binary serialization.

**File: `Import/VlppWorkflowCompiler.cpp`**

3. In `GenerateAssembly`: After collecting custom types from `declarationTypes.Values()`, sort `assembly->typeImpl->classes`, `interfaces`, `structs`, and `enums` by type name using `Sort()`.

### CONFIRMED

- GacUICompiler run once, committed all output (20 binary files changed from previous baseline).
- GacUICompiler run 3 more times: `git status` showed ZERO changes after each run.
- All 1686 unit test cases passed across 84 test files.
- The fix correctly addresses both sources of non-determinism:
  - Source 1: Metadata indices are now assigned based on alphabetical type/method/property/event names instead of pointer addresses.
  - Source 2: Custom types in the assembly are now ordered alphabetically by type name instead of by declaration pointer address.
