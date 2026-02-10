# !!!LEARNING!!!

# Orders

- Use `WString::IndexOf` with `wchar_t` (not `const wchar_t*`) [4]
- Capture dependent lambdas explicitly [2]
- Don't assume observable changes are batched [2]
- Prefer simple calls before interface casts [2]
- Use `collections::BinarySearchLambda` on contiguous buffers (guard empty) [2]
- Validate expectations against implementation and existing tests [2]
- Prefer well-defined tests over ambiguous edge cases [1]
- Prefer `operator<=> = default` for lexicographic key structs [1]
- Prefer two-pointer merge for sorted range maps [1]
- Use `Variant<T...>::Index()` to check active alternative [1]
- Avoid references into containers when mutating them [1]
- Prefer designated initializers for aggregate-like structs [1]
- Construct `Nullable<WString>` explicitly in function calls [1]
- `collections::Dictionary` copy assignment is deleted (use move/swap) [1]
- Dereference `Ptr<T>` via `.Obj()` (not `*ptr`) [1]
- `vl::regex` separator regex: `L"[\\/\\\\]+"` [1]
- Use 2-space indentation in embedded XML/JSON literals [1]

# Refinements

## Capture dependent lambdas explicitly

When a C++ lambda uses another local lambda (or any local variable), capture it explicitly (e.g. `[&]` or `[CreateBindableTree]`). Lambdas do not implicitly capture other lambdas, and missing captures can show up as confusing compile errors.

## Don't assume observable changes are batched

When verifying callbacks from an observable collection, do not assume multiple operations collapse into a single notification. For example, for `vl::collections::ObservableList<T>`, `Clear()` followed by multiple `Add()` calls triggers one callback pair per operation; test expectations should match the actual per-operation granularity.

## Prefer simple calls before interface casts

Do not add explicit interface casting (e.g. `dynamic_cast<IFoo*>(...)`) just because a similar file does it. Start with the simplest direct call, and only introduce interface casts when the compiler or access rules actually require it; unnecessary casts make tests noisier and can obscure which API surface is being exercised.

## Prefer well-defined tests over ambiguous edge cases

When a scenario’s expected behavior is unclear or undocumented (e.g. calling an operation while an object is in an “invisible”/non-effective state), avoid turning it into a test requirement. Prefer fewer tests that validate the public contract and real-world usage; if an edge case is important, first clarify the intended semantics from implementation and/or documentation.

## Validate expectations against implementation and existing tests

Before encoding expectations (especially for return value conventions and error semantics), read the relevant implementation and check existing tests for established patterns. This reduces churn from mismatched assumptions (e.g. public API returning a normalized error value even if internals use different sentinel codes).

This also applies to enums and API surface: verify that enum values and method names/signatures actually exist before using them.

## Prefer `operator<=> = default` for lexicographic key structs

When a struct is a pure lexicographic key (e.g. `{begin, end}`), prefer `auto operator<=>(const T&) const = default;` to generate a correct, consistent ordering and equality set automatically, instead of hand-writing `<`, `==`, etc.

## Use `collections::BinarySearchLambda` on contiguous buffers (guard empty)

`collections::BinarySearchLambda` expects a contiguous buffer pointer and count (e.g. `&keys[0]`, `keys.Count()`), plus a search item, an out `index`, and an orderer that returns `std::strong_ordering`. When using it on `Dictionary::Keys()`, guard the empty-map case before taking `&Keys()[0]`.

If you are searching for “overlap” rather than exact ordering, provide a custom orderer that defines “before / after / overlap” semantics for your ranges.

If multiple entries can satisfy “overlap”, binary search can return any matching entry. Scan backward (and/or forward) from the returned index to locate the first overlapping entry you intend to process.

## Prefer two-pointer merge for sorted range maps

When combining or diffing two maps that are already sorted by range keys, iterate both in one pass using a two-pointer “merge sort merge phase” approach. This avoids nested scans and keeps merge/diff logic linear in the number of runs.

## Use `Variant<T...>::Index()` to check active alternative

For `Variant<T...>`, use `Index()` (active alternative index) to branch on the stored type; do not assume helper methods like `GetType()` exist.

## Avoid references into containers when mutating them

When iterating a container and performing mutations like `Remove()`/`Add()`, avoid holding references (`auto&&`) to elements across mutations, because internal storage can be reallocated or reordered. Copy keys/values you still need to local variables before modifying the container.

## Prefer designated initializers for aggregate-like structs

For small structs used as value objects (especially those with default member initializers), prefer designated initializers like `{ .field = value }` for clarity and to avoid compile-time issues from positional aggregate initialization.

## Construct `Nullable<WString>` explicitly in function calls

When passing string literals to a function parameter typed as `Nullable<WString>`, wrap them in `WString(...)` (or otherwise construct a `WString`) to make the conversion explicit. Direct assignment to a `Nullable<WString>` field may compile via an assignment operator, but function-call argument conversion can require explicit construction.

## `collections::Dictionary` copy assignment is deleted (use move/swap)

`collections::Dictionary` does not support copy assignment. When you need to replace one dictionary with another, use move semantics (when appropriate), or rebuild/swap explicitly instead of `a = b`.

## Dereference `Ptr<T>` via `.Obj()` (not `*ptr`)

`Ptr<T>` is a smart pointer wrapper; to dereference it, use `ptr.Obj()` to get a raw pointer first (e.g. `*ptr.Obj()`). The `*ptr` syntax is not supported.

## Use `WString::IndexOf` with `wchar_t` (not `const wchar_t*`)

`vl::WString::IndexOf` searches for a single character when given a character parameter; pass character literals like `L'\r'` / `L'\n'`, not string literals like `L"\r"` / `L"\n"` (which are `const wchar_t*`).

## Use 2-space indentation in embedded XML/JSON literals

When writing XML or JSON inside a C++ string literal (e.g. `LR"GacUISrc(... )GacUISrc"` resources), indent the XML/JSON with 2 spaces (not tabs) to match the repo’s formatting rules for embedded structured text.

## `vl::regex` separator regex: `L"[\\/\\\\]+"`

In `vl::regex::Regex`, both `/` and `\\` are escaping characters, and incorrect escaping inside `[]` can throw errors like `Illegal character set definition.`

To split paths by either `/` or `\\`, a verified pattern is `L"[\\/\\\\]+"`, and using `Regex::Split(..., keepEmptyMatch=false, ...)` conveniently drops empty components (so `//` behaves like `/`).
