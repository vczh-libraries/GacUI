# Knowledge Base

## Guidance

The following data types are preferred:

- For any code interops with Windows API, use Windows API specific types.
- Use signed integer type `vint` or unsigned integer type `vuint` for general purpose. It always has the size of a pointer.
- Use signed integer types when the size is critical: `vint8_t`, `vint16_t`, `vint32_t`, `vint64_t`.
- Use unsigned integer types when the size is critical: `vuint8_t`, `vuint16_t`, `vuint32_t`, `vuint64_t`.
- Use `atomic_vint` for atomic integers, it is a rename of `std::atomic<vint>`.
- Use `DateTime` for date times.

[Learning of Coding Preference](./Learning.md)

## Vlpp

Files from Import:
- Vlpp.h
- Vlpp.cpp
- Vlpp.Windows.cpp
- Vlpp.Linux.cpp

Online documentation: https://gaclib.net/doc/current/vlpp/home.html

Vlpp is the foundational library that provides STL replacements and basic utilities.
It is the cornerstone of the entire framework, offering string handling, collections, lambda expressions, memory management, and primitive data types.
Use this when you need basic data structures without depending on STL.
This project prefers `wchar_t` over other char types and provides immutable string types, smart pointers, collection classes, and LINQ-like operations.

Detailed project guidance: [Index_Vlpp.md](./Index_Vlpp.md)

## VlppOS

Files from Import:
- VlppOS.h
- VlppOS.cpp
- VlppOS.Windows.cpp
- VlppOS.Linux.cpp

Online documentation: https://gaclib.net/doc/current/vlppos/home.html

VlppOS provides cross-platform OS abstraction for file system operations, streams, locale support, and multi-threading.
Use this when you need to interact with the operating system in a portable way.
It offers locale-aware string manipulation, file system access, various stream types with encoding/decoding capabilities, and comprehensive multi-threading support with synchronization primitives.
It offers inter-process communication structure, but actual network protocol implementations are subject for referencing or writing demoes.

Detailed project guidance: [Index_VlppOS.md](./Index_VlppOS.md)

## VlppRegex

Files from Import:
- VlppRegex.h
- VlppRegex.cpp

Online documentation: https://gaclib.net/doc/current/vlppregex/home.html

VlppRegex provides regular expression functionality with .NET-like syntax but with important differences.
Use this when you need pattern matching and text processing capabilities.

Pattern definition with .NET-like syntax but specific escaping and character matching rules.
Check out comments before `class Regex_` for a full syntax description.

The regular expression syntax is mostly compatible with .NET but has important differences:
- Both `/` and `\` perform escaping (prefer `/` to avoid C++ string literal conflicts)
- `.` matches literal '.' character, while `/.` or `\.` matches all characters
- DFA incompatible features significantly impact performance
- Detailed syntax description is available in `Regex_<T>` class comments

Detailed project guidance: [Index_VlppRegex.md](./Index_VlppRegex.md)

## VlppReflection

Files from Import:
- VlppReflection.h
- VlppReflection.cpp

Online documentation: https://gaclib.net/doc/current/vlppreflection/home.html

VlppReflection provides runtime reflection capabilities for C++ classes and functions.
Use this when you need to work with type metadata, register classes for scripting, or implement dynamic behavior.
It supports three compilation levels: full reflection, metadata-only, and no reflection.
Registration must happen in dedicated files and follows specific patterns for enums, structs, classes, and interfaces.

Detailed project guidance: [Index_VlppReflection.md](./Index_VlppReflection.md)

## VlppParser2

Files from Import:
- VlppGlrParser.h
- VlppGlrParser.cpp

Online documentation: https://gaclib.net/doc/current/vlppparser2/home.html

VlppParser2 implements GLR parsers based on customized and enhanced EBNF syntax.
Use this when you need to parse complex grammars or implement domain-specific languages.
The documentation for VlppParser2 is not ready yet.

Detailed project guidance: [Index_VlppParser2.md](./Index_VlppParser2.md)

## Workflow

Files from Import:
- VlppWorkflowLibrary.h
- VlppWorkflowLibrary.cpp
- VlppWorkflowCompiler.h
- VlppWorkflowCompiler.cpp
- VlppWorkflowRuntime.h
- VlppWorkflowRuntime.cpp

Online documentation: https://gaclib.net/doc/current/workflow/home.html

Workflow is a script language based on C++ reflection that can execute scripts at runtime or generate equivalent C++ code.
Use this when you need scripting capabilities, code generation, or when working with GacUI XML files.
It can execute the script if reflection is turned on.
It can generate equivalent C++ source files from the the script.
It can run Workflow with RPC on Workflow declared interfaces, but the user is responsible to offere ability of data transmission.

Detailed project guidance: [Index_Workflow.md](./Index_Workflow.md)

## GacUI

Online documentation: https://gaclib.net/doc/current/gacui/home.html

GacUI is a cross-platform GUI library that comes with an XML-based UI definition system and a compiler.
Use this when you need to create desktop applications with rich user interfaces.
It provides a comprehensive testing framework, XML-to-C++ compilation, and integrates with the Workflow script language for event handling and data binding.

Detailed project guidance: [Index_GacUI.md](./Index_GacUI.md)

## Experiences and Learnings

# Copy of Online Manual

## Vlpp OS

- [Using Streams](./manual/vlppos/using-streams.md)
- [Using Threads and Locks](./manual/vlppos/using-threads.md)

## Vlpp Parser2

- [AST Definition](./manual/vlppparser2/ast.md)
- [Lexer Definition](./manual/vlppparser2/lexer.md)
- [Syntax Definition](./manual/vlppparser2/syntax.md)
- [Generated APIs](./manual/vlppparser2/apis.md)

## Workflow Script

- [Running a Script](./manual/workflow/running.md)
  - [Running Workflow Scripts](./manual/workflow/running/running.md)
  - [Invoking C++ Classes](./manual/workflow/running/invoking1.md)
  - [Inheriting C++ Interfaces](./manual/workflow/running/invoking2.md)
  - [Inheriting C++ Classes](./manual/workflow/running/invoking3.md)
  - [Serializing and Loading Assemblies](./manual/workflow/running/serializing.md)
  - [Generating C++ Code](./manual/workflow/running/generating.md)
  - [Debugging](./manual/workflow/running/debugging.md)
- [Syntax](./manual/workflow/lang.md)
  - [Module](./manual/workflow/lang/module.md)
  - [Functions and Variables](./manual/workflow/lang/funcvar.md)
  - [Types](./manual/workflow/lang/type.md)
    - [Enums](./manual/workflow/lang/enum.md)
    - [Structs](./manual/workflow/lang/struct.md)
    - [Interfaces](./manual/workflow/lang/interface.md)
      - [Properties and Events](./manual/workflow/lang/interface_prop.md)
      - [Inheritance](./manual/workflow/lang/interface_inherit.md)
      - [Implementing](./manual/workflow/lang/interface_new.md)
      - [Generic](./manual/workflow/lang/interface_using.md)
    - [Classes](./manual/workflow/lang/class.md)
      - [Properties and Events](./manual/workflow/lang/class_prop.md)
      - [Inheritance](./manual/workflow/lang/class_inherit.md)
      - [Allocating](./manual/workflow/lang/class_new.md)
  - [Expressions](./manual/workflow/lang/expr.md)
    - [Precedences](./manual/workflow/lang/expr_precedences.md)
    - [Unary Operators](./manual/workflow/lang/expr_unary.md)
    - [Binary Operators](./manual/workflow/lang/expr_binary.md)
  - [Statements](./manual/workflow/lang/stat.md)
  - [Data Bindings](./manual/workflow/lang/bind.md)
    - [Observe events explicitly](./manual/workflow/lang/bind_observe.md)
  - [Coroutines](./manual/workflow/lang/coroutine.md)
    - [Enumerable Coroutine](./manual/workflow/lang/coroutine_enumerable.md)
    - [Async Coroutine](./manual/workflow/lang/coroutine_async.md)
    - [Raw Coroutine](./manual/workflow/lang/coroutine_raw.md)
    - [Design Your Own Coroutine](./manual/workflow/lang/coroutine_custom.md)
  - [State Machines](./manual/workflow/lang/state.md)
    - [Using Input Methods](./manual/workflow/lang/state_input.md)
    - [Handling Unexpected Inputs](./manual/workflow/lang/state_switch.md)
    - [Jumping Between States](./manual/workflow/lang/state_jump.md)
    - [Example: Calculator](./manual/workflow/lang/state_calculator.md)
  - [Index of Keywords](./manual/workflow/lang/index.md)
  - [Index of Errors](./manual/workflow/lang/error.md)
- [Runtime Instructions](./manual/workflow/ins.md)
- [C++ Code Generation](./manual/workflow/codegen.md)

## GacUI

- [Knowledge Base](./manual/gacui/kb/home.md)
  - [GacUI Object Models](./manual/gacui/kb/gom.md)
  - [Elements](./manual/gacui/kb/elements.md)
  - [Compositions](./manual/gacui/kb/compositions.md)
  - [Controls](./manual/gacui/kb/controls.md)
  - [Control Templates](./manual/gacui/kb/ctemplates.md)
  - [Item Templates](./manual/gacui/kb/itemplates.md)
  - [Default Template Registrations](./manual/gacui/kb/dtemplates.md)
  - [Application](./manual/gacui/kb/application.md)
  - [OS Provider](./manual/gacui/kb/osprovider.md)
- [GacUI XML Resource](./manual/gacui/xmlres/home.md)
  - [Code Generation Configuration](./manual/gacui/xmlres/cgc.md)
  - [Cross XML Resource References](./manual/gacui/xmlres/cxrr.md)
  - [Folders and Resources](./manual/gacui/xmlres/dom.md)
  - [\<Text\>](./manual/gacui/xmlres/tag_text.md)
  - [\<Image\>](./manual/gacui/xmlres/tag_image.md)
  - [\<Xml\>](./manual/gacui/xmlres/tag_xml.md)
  - [\<Doc\>](./manual/gacui/xmlres/tag_doc.md)
  - [\<Script\>](./manual/gacui/xmlres/tag_script.md)
  - [\<Instance\>](./manual/gacui/xmlres/tag_instance.md)
    - [Root Instance](./manual/gacui/xmlres/instance/root_instance.md)
    - [Child Instances](./manual/gacui/xmlres/instance/child_instances.md)
    - [Properties](./manual/gacui/xmlres/instance/properties.md)
    - [Events](./manual/gacui/xmlres/instance/events.md)
    - [Namespaces](./manual/gacui/xmlres/instance/namespaces.md)
    - [Instance Inheriting](./manual/gacui/xmlres/instance/inheriting.md)
  - [\<InstanceStyle\>](./manual/gacui/xmlres/tag_instancestyle.md)
  - [\<Animation\>](./manual/gacui/xmlres/tag_animation.md)
  - [\<LocalizedStrings\>](./manual/gacui/xmlres/tag_localizedstrings.md)
  - [\<LocalizedStringsInjection\>](./manual/gacui/xmlres/tag_localizedstringsinjection.md)
- [GacUI Components](./manual/gacui/components/home.md)
  - [Elements](./manual/gacui/components/elements/home.md)
  - [Compositions](./manual/gacui/components/compositions/home.md)
    - [\<Bounds\>](./manual/gacui/components/compositions/bounds.md)
    - [\<Stack\> and \<StackItem\>](./manual/gacui/components/compositions/stack.md)
    - [\<Flow\> and \<Flow\>](./manual/gacui/components/compositions/flow.md)
    - [\<Table\>, \<Cell\>, \<RowSplitter\> and \<ColumnSplitter\>](./manual/gacui/components/compositions/table.md)
    - [Repeat Compositions](./manual/gacui/components/compositions/repeat.md)
      - [\<RepeatStack\> and \<RepeatFlow\>](./manual/gacui/components/compositions/repeat_nonvirtual.md)
      - [Virtual Repeat Compositions](./manual/gacui/components/compositions/repeat_virtual.md)
        - [\<RepeatFreeHeightItem\>](./manual/gacui/components/compositions/repeat_virtual_freeheight.md)
        - [\<RepeatFixedHeightItem\>](./manual/gacui/components/compositions/repeat_virtual_fixedheight.md)
        - [\<RepeatFixedHeightMultiColumnItem\>](./manual/gacui/components/compositions/repeat_virtual_fixedheightmc.md)
        - [\<RepeatFixedSizeMultiColumnItem\>](./manual/gacui/components/compositions/repeat_virtual_fixedsizemc.md)
    - [\<SharedSizeRoot\> and \<SharedSizeItem\>](./manual/gacui/components/compositions/sharedsize.md)
    - [\<SideAligned\>](./manual/gacui/components/compositions/sidealigned.md)
    - [\<PartialView\>](./manual/gacui/components/compositions/partialview.md)
    - [Responsive Design Series](./manual/gacui/components/compositions/responsive.md)
      - [\<ResponsiveContainer\>](./manual/gacui/components/compositions/responsive_container.md)
      - [\<ResponsiveView\>](./manual/gacui/components/compositions/responsive_view.md)
        - [\<ResponsiveFixed\>](./manual/gacui/components/compositions/responsive_fixed.md)
        - [\<ResponsiveShared\>](./manual/gacui/components/compositions/responsive_shared.md)
      - [\<ResponsiveStack\>](./manual/gacui/components/compositions/responsive_stack.md)
      - [\<ResponsiveGroup\>](./manual/gacui/components/compositions/responsive_group.md)
    - [Handling Input Events](./manual/gacui/components/compositions/events.md)
  - [Controls](./manual/gacui/components/controls/home.md)
    - [Basic Controls](./manual/gacui/components/controls/basic/home.md)
      - [ControlHost and \<Window\>](./manual/gacui/components/controls/basic/window.md)
      - [\<CustomControl\>](./manual/gacui/components/controls/basic/customcontrol.md)
      - [\<Label\>](./manual/gacui/components/controls/basic/label.md)
      - [\<Button\>](./manual/gacui/components/controls/basic/Button.md)
      - [GuiSelectableButton](./manual/gacui/components/controls/basic/selectableButton.md)
      - [GuiScroll](./manual/gacui/components/controls/basic/scroll.md)
      - [\<DatePicker\>](./manual/gacui/components/controls/basic/datepicker.md)
      - [\<DateComboBox\>](./manual/gacui/components/controls/basic/datecombobox.md)
    - [Container Controls](./manual/gacui/components/controls/container/home.md)
      - [\<GroupBox\>](./manual/gacui/components/controls/container/groupbox.md)
      - [GuiScrollView and \<ScrollContainer\>](./manual/gacui/components/controls/container/scrollcontainer.md)
      - [\<Tab\> and \<TabPage\>](./manual/gacui/components/controls/container/tab.md)
    - [Editor Controls](./manual/gacui/components/controls/editor/home.md)
      - [\<DocumentViewer\>](./manual/gacui/components/controls/editor/documentviewer.md)
      - [\<DocumentLabel\> and \<DocumentTextBox\>](./manual/gacui/components/controls/editor/documentlabel.md)
      - [\<SinglelineTextBox\>](./manual/gacui/components/controls/editor/singlelinetextbox.md)
      - [\<MultilineTextBox\>](./manual/gacui/components/controls/editor/multilinetextbox.md)
    - [List Controls](./manual/gacui/components/controls/list/home.md)
      - [GuiListControl and GuiSelectableListControl](./manual/gacui/components/controls/list/guilistcontrol.md)
      - [\<TextList\>](./manual/gacui/components/controls/list/textlist.md)
      - [\<TreeView\>](./manual/gacui/components/controls/list/treeview.md)
      - [\<ListView\>](./manual/gacui/components/controls/list/listview.md)
      - [\<ComboButton\> and \<ComboBox\>](./manual/gacui/components/controls/list/combobox.md)
      - [Bindable List Controls](./manual/gacui/components/controls/list/bindable.md)
        - [\<BindableTextList\>](./manual/gacui/components/controls/list/bindabletextlist.md)
        - [\<BindableTreeView\>](./manual/gacui/components/controls/list/bindabletreeview.md)
        - [\<BindableListView\>](./manual/gacui/components/controls/list/bindablelistview.md)
        - [\<BindableDataGrid\>](./manual/gacui/components/controls/list/bindabledatagrid.md)
          - [Column Configuration](./manual/gacui/components/controls/list/datagrid_column.md)
          - [Filter](./manual/gacui/components/controls/list/datagrid_filter.md)
          - [Sorter](./manual/gacui/components/controls/list/datagrid_sorter.md)
          - [Visualizer](./manual/gacui/components/controls/list/datagrid_visualizer.md)
          - [Editor](./manual/gacui/components/controls/list/datagrid_editor.md)
      - [Accessing Items via IItemProvider](./manual/gacui/components/controls/list/itemprovider.md)
    - [Toolstrip Controls](./manual/gacui/components/controls/toolstrip/home.md)
      - [\<ToolstripButton\>](./manual/gacui/components/controls/toolstrip/toolstripbutton.md)
        - [Grouping Multiple \<ToolstripButton\>](./manual/gacui/components/controls/toolstrip/grouping.md)
      - [\<ToolstripMenuBar\>](./manual/gacui/components/controls/toolstrip/toolstripmenubar.md)
      - [\<ToolstripMenu\>](./manual/gacui/components/controls/toolstrip/toolstripmenu.md)
      - [\<ToolstripToolBar\>](./manual/gacui/components/controls/toolstrip/toolstriptoolbar.md)
    - [Ribbon Controls](./manual/gacui/components/controls/ribbon/home.md)
      - [\<ToolstripButton\>](./manual/gacui/components/controls/ribbon/toolstripbutton.md)
        - [Grouping Multiple \<ToolstripButton\>](./manual/gacui/components/controls/ribbon/grouping.md)
      - [\<RibbonTab\>](./manual/gacui/components/controls/ribbon/tab.md)
      - [\<RibbonTabPage\>](./manual/gacui/components/controls/ribbon/tabpage.md)
      - [\<RibbonGroup\>](./manual/gacui/components/controls/ribbon/group.md)
        - [\<RibbonSplitter\>](./manual/gacui/components/controls/ribbon/splitter.md)
        - [\<RibbonButtons\>](./manual/gacui/components/controls/ribbon/buttons.md)
          - [\<RibbonIconLabel\>](./manual/gacui/components/controls/ribbon/iconlabel.md)
        - [\<RibbonToolstrips\>](./manual/gacui/components/controls/ribbon/toolstrips.md)
        - [\<BindableRibbonGalleryList\>](./manual/gacui/components/controls/ribbon/gallerylist.md)
          - [\<RibbonToolstripHeader\>](./manual/gacui/components/controls/ribbon/header.md)
  - [Control Templates](./manual/gacui/components/ctemplates/home.md)
    - [\<CommonScrollBehavior\>](./manual/gacui/components/ctemplates/commonscrollbehavior.md)
    - [\<CommonScrollViewLook\>](./manual/gacui/components/ctemplates/commonscrollviewlook.md)
    - [\<CommonDatePicker\>](./manual/gacui/components/ctemplates/commondatepickerlook.md)
  - [Item Templates](./manual/gacui/components/itemplates/home.md)
    - [\<ListItemTemplate\>](./manual/gacui/components/itemplates/listitemtemplate.md)
    - [\<TextListItemTemplate\>](./manual/gacui/components/itemplates/textlistitemtemplate.md)
    - [\<TreeItemTemplate\>](./manual/gacui/components/itemplates/treeitemtemplate.md)
    - [\<GridVisualizerTemplate\>](./manual/gacui/components/itemplates/gridvisualizertemplate.md)
    - [\<GridEditorTemplate\>](./manual/gacui/components/itemplates/grideditortemplate.md)
  - [Components](./manual/gacui/components/components/home.md)
    - [\<MessageDialog\>](./manual/gacui/components/components/messagedialog.md)
    - [\<ColorDialog\>](./manual/gacui/components/components/colordialog.md)
    - [\<FontDialog\>](./manual/gacui/components/components/fontdialog.md)
    - [\<OpenFileDialog\>](./manual/gacui/components/components/openfiledialog.md)
    - [\<SaveFileDialog\>](./manual/gacui/components/components/savefiledialog.md)
- [Advanced Topics](./manual/gacui/advanced/home.md)
  - [Interop with C++ View Model](./manual/gacui/advanced/vm.md)
  - [Data Bindings](./manual/gacui/advanced/bindings.md)
  - [Animations](./manual/gacui/advanced/animations.md)
  - [Localization](./manual/gacui/advanced/localization.md)
  - [Cross XML Resource References](./manual/gacui/advanced/cxrr.md)
  - [ALT Sequence and Control Focus](./manual/gacui/advanced/alt.md)
  - [TAB and Control Focus](./manual/gacui/advanced/tab.md)
  - [Creating New List Controls](./manual/gacui/advanced/impllistcontrol.md)
  - [Porting GacUI to Other Platforms](./manual/gacui/advanced/implosprovider.md)
- [Hosted Mode and Remote Protocol](./manual/gacui/modes/home.md)
  - [Remote Protocol Core Application](./manual/gacui/modes/remote_core.md)
  - [Remote Protocol Client Application](./manual/gacui/modes/remote_client.md)
  - [Implementing a Communication Protocol](./manual/gacui/modes/remote_communication.md)

## Unit Testing

- [Using Unit Test Framework](./manual/unittest/vlpp.md)
- [Running GacUI in Unit Test Framework](./manual/unittest/gacui.md)
  - [Snapshots and Frames](./manual/unittest/gacui_frame.md)
  - [IO Interaction](./manual/unittest/gacui_io.md)
  - [Accessing Controls](./manual/unittest/gacui_controls.md)
  - [Snapshot Viewer](./manual/unittest/gacui_snapshots.md)
