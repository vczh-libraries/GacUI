# !!!LEARNING!!!

# Orders

- Capture dependent lambdas explicitly [1]
- Don't assume observable changes are batched [1]

# Refinements

## Capture dependent lambdas explicitly

When a C++ lambda uses another local lambda (or any local variable), capture it explicitly (e.g. `[&]` or `[CreateBindableTree]`). Lambdas do not implicitly capture other lambdas, and missing captures can show up as confusing compile errors.

## Don't assume observable changes are batched

When verifying callbacks from an observable collection, do not assume multiple operations collapse into a single notification. For example, `Clear()` followed by multiple `Add()` calls typically triggers one event (and one callback pair) per operation; test expectations should match the actual per-operation granularity.
