# !!!LEARNING!!!

# Orders

- `vl::collections::ObservableList<T>` element access and replacement [2]
- `TreeViewItemBindableRootProvider::UpdateBindingProperties` is root-scoped [1]

# Refinements

## `TreeViewItemBindableRootProvider::UpdateBindingProperties` is root-scoped

`TreeViewItemBindableRootProvider::UpdateBindingProperties(true)` only unprepares and re-prepares the root node's immediate children. It does not recursively process prepared descendants (grandchildren and deeper), so tests and features should not expect descendant callbacks from this API.

## `vl::collections::ObservableList<T>` element access and replacement

If a field is typed as `ObservableList<Ptr<T>>`, indexing returns `Ptr<T>` (by reference) directly; do not use `UnboxValue` (which is for `Value`) on `children[i]`.

`ObservableList<T>` does not support copy assignment for "replace whole list" scenarios; replace list contents with `Clear()` + `Add()` (or equivalent per-element operations) instead of `operator=`.
