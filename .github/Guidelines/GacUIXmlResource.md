# Syntax of GacUI XML Resources

- This is a brief introduction for GacUI XML Resource.
- Detailed document can be found in `REPO-ROOT/.github/KnowledgeBase/Index.md` under `# Copy of Online Manual`:
  - In `## GacUI`, the `GacUI XML Resource` item and all sub items detailed explain the syntax for GacUI XML Resource.
  - In `## Workflow Script`, the `Syntax` item and all sub items detailed explain the syntax for the script language used in GacUI XML Resource.

## Mapping XML Entity to C++ Entity

Most XML tags are calling constructors for classes in the following folder:

- Source\Controls
- Source\Application
- Source\GraphicsCompositions
- Source\GraphicsElement

All mappings are:

- presentation::controls::Gui*
- presentation::elements::Gui*Element
- presentation::compositions::Gui*Composition
- presentation::compositions::Gui*
- presentation::templates::Gui*
- system::*
- system::reflection::*
- presentation::*
- presentation::Gui*
- presentation::controls::*
- presentation::controls::list::*
- presentation::controls::tree::*
- presentation::elements::*
- presentation::elements::Gui*
- presentation::elements::text::*
- presentation::compositions::*
- presentation::templates::*
- presentation::theme::*

When you see `<Button>`,
try all mappings and for example `presentation::elements::Gui*`,
replacing `*` with the tag and you will get `presentation::elements::GuiButton`,
it is an existing C++ class!
So `<Button>` means `presentation::controls::GuiButton`.
Following the same rule, `<Table>` would be `presentation::compositions::GuiTableComposition` and `<SolidLabel>` would be `presentation::elements::GuiSolidLabelElement`.

Take this file `Test\Resources\Metadata\Reflection64.txt` as an index, it collects all valid C++ classes and their members, but written in my own format.
When there is a `@FullName` on top of a class, it means the full name in C++, the class name will be the full name in XML.
When there is no `@FullName` but the class name begins with `presentation::`, the full name in C++ begins with `vl::presentation::`.

## XML in a Single String

To define an empty window, the XML looks like:

```xml
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonGallery" ClientSize="x:480 y:320">
      </Window>
    </Instance>
  </Instance>
</Resource>
```

The first `<Instance>` defines the resource named `MainWindowResource`, the `<Instance>` inside is the content. You can also add a script like this:

```xml
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="GuiRibbonGallery" ClientSize="x:480 y:320">
        <Label Text="This is a demo"/>
      </Window>
    </Instance>
  </Instance>

  <Folder name="Scripts">
    <Script name="ViewModelResource">
      <Workflow>
        <![CDATA[
          module viewmodel;
          
          using system::*;
          using presentation::*;
          
          namespace demo
          {
            enum MyCategory
            {
              Black = 0,
              Red = 1,
              Lime = 2,
              Blue = 3,
              White = 4,
            }

            func ToString(value : DateTime) : string
            {
              return $"$(value.month)/$(value.day)/$(value.year)";
            }
          }
        ]]>
      </Workflow>
    </Script>
  </Folder>
</Resource>
```

One more `Scripts\ViewModelResource` is added to the resource, and the content of the new resource is defined by `<Workflow>`. Code inside `<Workflow>` will always be Workflow Script instead of C++.

## UI Layout

This is the GacUI XML document for UI layout: https://gaclib.net/doc/current/gacui/components/compositions/home.html

To expand a composition to the whole parent client area:

```XML
<AnyComposition AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
```

- `MinSizeLimitation="LimitToElementAndChildren"` for a composition limits its minimum size to the sum of all its children.
- `AlignmentToParent="left:8 top:8 right:8 bottom:8"` for a composition sticks itself to the parent's whole client area with 8 pixels on each side. If the number is -1, it means it doesn't stick to the specific parent's client area side. When both `left` and `right` are -1, it sticks to the left. When both `top` and `bottom` are -1, it sticks to the top. The default value is all -1.

`BoundsComposition` of a control is its boundary composition. To expand a control to the whole parent client area:

```XML
<AnyControl>
  <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
</AnyControl>
```

`<Cell>`, `<RowSplitter>`, `<ColumnSplitter>`, `<StackItem>` and `<FlowItem>` are controlled by its parent composition, no positions or size limits need to be adjusted.

### Bounds

`<Bounds>` is useful to define a space.

### Table

The most useful composition is `<Table>`, it is a grid layout with rows and columns. There are 3 sizing modes for rows and columns:
- `<_>composeType:MinSize</_>`: The size is decided by its content.
- `<_>composeType:Absolute absolute:10</_>`: The size is 10.
- `<_>composeType:Percentage percentage:0.5</_>`: The size is 50% of all space excludes MinSizes and Absolutes.

The `CellPadding` property defines the space between cells, default 0. The `BorderVisible` adds `CellPadding` around the border, default true. Obviously the following two tables are identical.
```XML
<Table AlignmentToParent="left:8 top:8 right:8 bottom:8" CellPadding="5" BorderVisible="false"/>
<Table AlignmentToParent="left:3 top:3 right:3 bottom:3" CellPadding="5" BorderVisible="true"/>
```

In order to make a table expanded to the whole window and placing a button at the center:
- Set rows to Percentage 0.5; MinSize; Percentage 0.5
- Set columns to Percentage 0.5; MinSize; Percentage 0.5
- Put the button to the center cell, which is `Site="row:1 column:1"`

We can also list 3 buttons vertically in the top-left corner of the window:
- Set rows to MinSize; MinSize; MinSize; Percentage 1.0
- Set columns to MinSize; Percentage 1.0
- Put 3 button to all MinSize cells, which is `Site="row:0 column:0"`, `Site="row:1 column:0"`, `Site="row:2 column:0"`

To make a dialog with big content with OK and Cancel buttons at the bottom-right corner:
- Set rows to Percentage 1.0; MinSize
- Set columns to Percentage 1.0; MinSize; MinSize
- Put the content to the cell that automatically expands to the rest of the space, which is `Site="row:0 column:0"`
- Put 2 button to all MinSize cells, which is `Site="row:1 column:1"`, `Site="row:1 column:2"`

### Others

Please read the document of GacUI XML for other compositions: https://gaclib.net/doc/current/gacui/components/compositions/home.html
- Stack/StackItem
- Flow/FlowItem
- SharedSizeRoot/SharedSizeItem
- SideAligned
- PartialView
- etc

## Properties

There are two ways to add a property:

```xml
<Label Text="This is a demo"/>
```

```xml
<Label>
  <att.Text>This is a demo</att.Text>
</Label>
```

If the object assigned to a property cannot be written as a string, the second way is the only way to do that, for example:

```xml
<Label>
</Label>
```

To access properties in the nested level, the `-set` binding is required:

```xml
<Label>
  <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
</Label>
```

This example changes the property `AlignmentToParent` to the object in label's `BoundsComposition` property.

If a property name looks like `Name-binding`, it means the property `Name` should interpret the content using the specified way `-binding`. There are more predefined bindings like `-ref`, `-uri`, `-eval`, `-bind`, etc.

## Events

An event is subscribed like:

```xml
<Button>
  <ev.Clicked-eval><![CDATA[{
    // some code
  }]]></ev.Clicked-eval>
</Button>
```

It means when button's `Clicked` event happens, execute some code.

Code in an event of in the `<Workflow>` resource item should be Workflow Script instead of C++.
