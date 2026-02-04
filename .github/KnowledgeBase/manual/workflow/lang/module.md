# Module

Any Workflow script file should be a module, written as:
```
module NAME;

using system::*;
using presentation::compositions::Gui*Composition;

/* ... */
```


The module name does not matter, but when multiple files are given to the compiler to build an assembly, module names in this assembly are require to be unique.

## Using declaration

A**using**declaration imports names in this module.

**using system::*;**means that names under the namespace**system**can be used directly. For example, you can use**system::DateTime**in a module. But writing the namespace everywhere is annoying, so when**using system::***appears in a module, you can use**DateTime**directly.

The wildcard is not required to be a complete name. When you haveGacUI types`missing document: /home/registered/gacui.html`loaded,**presentation::compositions::GuiTableComposition**is available. But this name is too long, even**GuiTableComposition**is too long. Now when**using presentation::compositions::Gui*Composition;**appears in a module, you can use**Table**for that type.

Because when you replace the wildcard by**Table**, it becomes**presentation::compositions::GuiTableComposition**, which is exactly the full name of the type.

Sometimes wildcards could create ambiguity, for example,
```
using presentation::compositions::Gui*Composition;
using presentation::controls::Gui*;
using presentation::templates::Gui*Template;
```
When we try to use**Window**in this module, the compiler founds that all following types
- presentation::compositions::GuiWindowComposition
- presentation::controls::GuiWindow
- presentation::templates::GuiWindowTemplateexist, it cannot determine which type to use, so an error is generated.

In order to resolve this issue, you are able to just use the full name, or**GuiWindow**if**using presentation::controls::***appears in the module, or even**controls::GuiWindow**if**using presentation::***appears in the module.

## Namespace declaration

You can create namespace by yourself like this:
```
module test;
using system::*;

namespace mynamespace
{
    namespace another
    {
        func MyFunc(): string
        {
            return "Hello, world!";
        }
    }
}
```
Namespaces are allowed to be nested.

**IMPORTANT**:**using**declarations are not allowed to be in namespaces. In fact they should be declared before any other declarations, right below the module name.

## Static Initialization

Static initialization are statements in**static {}**declarations. Multiple**static {}**declarations could appear in a module or a namespace. Statemenets inside them will be executed when the module is loaded.

When loading and executing workflow script in runtime, such initialization happens when running the[<initialize>](../.././workflow/running/running.md)function, after global variables are initialized.

The execution order between multiple**static {}**declarations are not defined.

