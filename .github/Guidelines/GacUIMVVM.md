# Coding Convention (GacUI and MVVM)

- When using view model and data binding with a UI:
  - The UI's XML and embedded workflow script code are recommended to implement UI visual effect only.
  - Move actual logic into view model implementation.
  - This makes the UI simple enough as well as keeping the view model testable.
    - In MVVM (model-view-viewmodel), view could be UI and unit test.
    - Multiple views connecting to the same view model is a valid design, e.g. CLI application as a view.
  - It is nearly unavoidable to let view model interface's generated C++ code includes GacUI files, but you should keep view model implementation to pure logic only.
- It is recommended to define interfaces for view models.
- Any window or custom controls could add a view model interface as a parameter in its XML description.
- Bidirectional bindings are implemented by creating two binding for each direction. The `-set` binding could be used to access the view model object, and make the view model tracking control data.
- XML's embedded workflow script is a good way to open a sub window, making passing sub view model easier. `$Await` could be used to wait for the sub window's closing.

## Using Container Data Structures

- Workflow Script strong-typed container types are stronger recommended for data binding, e.g., `T[]` or `observe T[]`.
  - Use `observe T[]` when UI needs to automatically updated when the container is changed.
    - `observe T[]` does not track changing inside each `T`, usually it requires any item template to create extra binding to `T`'s properties.
  - It generates weak-typed container types, e.g. `Ptr<IValueObservableList>`, which is expected.
    - But in C++ view model implementation, strong-typed containers are strong recommended to implement these pointers.
    - `vl::__vwsn::UnboxCollection` function, or equivalently calling `BoxParameter` followed by `UnboxParameters`, converts a strong-typed C++ container to reflectable container interfaces like `Ptr<IValueObservableList>`. Such interface uses the reference to the original strong-typed C++ container directly without any lifecycle protection, make sure to keep the strong-typed C++ container alive. An easiest way is to put such strong-typed C++ container in the view model implementation's field, and any view model function could call boxing or unboxing freely.
    - Unless explicitly instructed, you can assume that the view model object itself will live long enough, no need to implement lifecycle management. If the view model interface design breaks such usage, it is recommended to fix the design itself.
  - Built-in LINQ function (starting from `From(collection)`) returns `LazyList<T>` which could be unboxed to a `Ptr<IValueEnumerable>`.

## Cross-Resource XML Authoring

- `GacBuild.ps1` calls the `GacBuild` tool to run incremental build on multiple GacUI XML resources with dependencies.
- View model interfaces, UI definition and resources (especially images) could be used in downstream resource files.
- When the application's size grows, such technique could speed up the resource compiling, but each GacUI XML resource would generates its own C++ files, there is no option to merge everything together in terms of C++ code generation.
