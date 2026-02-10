# !!!LEARNING!!!

# Orders

- Capture dependent lambdas explicitly [2]
- Don't assume observable changes are batched [2]
- Prefer simple calls before interface casts [1]

# Refinements

## Capture dependent lambdas explicitly

When a C++ lambda uses another local lambda (or any local variable), capture it explicitly (e.g. `[&]` or `[CreateBindableTree]`). Lambdas do not implicitly capture other lambdas, and missing captures can show up as confusing compile errors.

## Don't assume observable changes are batched

When verifying callbacks from an observable collection, do not assume multiple operations collapse into a single notification. For example, for `vl::collections::ObservableList<T>`, `Clear()` followed by multiple `Add()` calls triggers one callback pair per operation; test expectations should match the actual per-operation granularity.

## Prefer simple calls before interface casts

Do not add explicit interface casting (e.g. `dynamic_cast<IFoo*>(...)`) just because a similar file does it. Start with the simplest direct call, and only introduce interface casts when the compiler or access rules actually require it; unnecessary casts make tests noisier and can obscure which API surface is being exercised.
