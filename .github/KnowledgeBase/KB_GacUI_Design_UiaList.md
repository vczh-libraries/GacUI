# UiaList: Windows UI Automation Inspector

UiaListApp is the Windows desktop inspector in `GacUI/Tools/UiaList`. Use it to discover application windows, inspect their UI Automation trees, read properties, and exercise supported actions. Run `UiaListApp.exe`; its resources are embedded, so no resource files need to accompany it.

## Inspect a Window

1. In **Processes**, select a process from the hierarchical dropdown beside **Refresh**. The list shows that process's visible top-level windows. An ancestor retained only because it has visible descendants can have an empty window list.
2. Single-click a window row, or select it and press Enter. **UI** opens a captured snapshot of that window.
3. Hover over the snapshot to outline the deepest matching UI Automation node. Click to reveal that node in **Nodes**. Scroll when the snapshot is larger than the preview area.
4. In **Nodes**, right-click a node and choose **Inspect**, or select it and press Enter. Double-click expands or collapses the tree.

The preview is a snapshot. Clicking it selects an inspector node; it does not click the inspected application. A minimized or otherwise unavailable preview does not necessarily prevent inspecting the window's nodes. **Refresh** explicitly refreshes process discovery and preserves surviving selections; selecting a different process only changes the window list.

## Properties and Actions

The inspection window has **Properties** and **Actions** tabs. Properties lists supported values, including false, zero, empty and read-only values. Editable properties offer an embedded editor or a **...** button opening a multiline editor. Unsupported properties are omitted; not every property has a setter.

Actions groups the target's supported operations by provider. Parameterless getters display their results directly. Parameterized getters run when valid edits are committed, such as with Enter or focus loss. Mutations and text-range workspace commands require explicit activation. These actions operate on the inspected application, so invoking them can change its state.

Returned element references can be inspected; references within the selected tree can also be revealed in **Nodes**. Text-range workspaces support inspecting and manipulating ranges. Refresh or target mutation can invalidate an earlier range and require acquiring it again. Availability depends on what the target application's UI Automation providers expose.

The surrounding UI selects English, Simplified Chinese or Japanese from Windows' preferred UI languages. Standard interface names and text from the target retain their original spelling.

## Launch and Automation

The executable is Windows-only. Build and launch instructions are in `GacUI/Tools/UiaList/README.md`; the release tool build instructions are in `Release/Tools/README.md`.

Debug builds expose `http://localhost:8888/Automation/UiaListApp/Controls` and `/IO`. Pass `/AsPort:8891`, for example, when another application uses the default port. One decimal port from 1 through 65535 is accepted. Release builds do not start an HTTP automation endpoint; Windows UI Automation remains available.

For GacUI applications' own UI Automation support, see [Windows UI Automation](./KB_GacUI_Design_UIAutomation.md).

## UiaListCli JSON protocol

`UiaListCli.exe` is the Windows console frontend of the same UiaList library. It uses the same discovery, MTA UIA session, descriptors, setters, actions, range workspaces and capture worker. It runs the owner dispatcher without opening an inspector window or HTTP listener. Both executables are standalone; no resource files accompany them.

Launch from `GacUI/Tools/UiaList` with the repository wrapper:

```powershell
& C:\Code\VczhLibraries\GacUI\.github\Scripts\copilotExecute.ps1 -Mode CLI -Executable UiaListCli -Configuration Debug -Platform x64 -Interactive
```

Use one line per command: `Verb-Target [ID] [JSON arguments object]`. Names and parameter keys are case-sensitive. IDs are opaque, whitespace-free strings. The final object uses JSON escaping; multiline text stays on one command line using `\n`. Omit an empty arguments object or supply `{}`. All named parameters are required unless their schema says otherwise; nullable parameters still require their key. Duplicate or extra keys are errors. Blank lines are ignored. Redirected input/output is UTF-8; interactive console text uses Windows Unicode console handles.

Stdout contains exactly one compact JSON object and newline for each nonblank command. There are no prompts, progress records or banners from the executable. Commands execute sequentially, and the response is flushed before the next command is accepted. Wrapper diagnostic output is separate; when using the PowerShell wrapper as a pipe endpoint, suppress its information stream with `6>$null`.

### Response and identity

Every response, including help and exit, has these fields:

```typescript
type Response = {
  command: string;
  ok: boolean;
  generation: string | null;
  result: unknown | null;
  error: null | {
    code: string; message: string; operation: string;
    target: string | null; hresult: string | null;
    phase: "validation" | "invocation" | "readback";
  };
};
```

Success has a non-error result and `error:null`; failure has `result:null`. `generation` identifies the currently published inspection, or is null without a selection. HRESULTs are hexadecimal strings. An unparseable UTF-8 line has an empty `command`. Local errors include `UnknownCommand`, `UnknownProvider`, `InvalidEncoding`, `InvalidJson`, `InvalidArguments`, `InvalidType`, `InvalidId`, `ExpiredId`, `WrongKind`, `WrongDocument`, `WrongWindow`, `OutsideTree`, `ReadOnly`, `UnsupportedOperation` and `CapabilityChanged`.

Window, element and range IDs have separate kinds and belong to one CLI process. Keep the returned strings intact; do not construct IDs from a PID, HWND, RuntimeId, label or array index. Discovery retains surviving window identities and removes absent windows. Window switches, explicit refresh and target mutations expire all earlier element/range IDs, even when the native session survives. Reacquire IDs from the new tree/readback. Queries and local range Clone/Find/endpoint operations retain the generation and valid workspace operands. Inspecting another element does not change the selected tree root.

Returned elements can be inspected even when outside the selected tree. `treeNodeId:null` means no matching tree node exists; `Select-Node` rejects that reference. Every range carries an owning `documentId`. Comparison and endpoint transfer require the same document, including ranges acquired through different interfaces. ObjectModel and IAccessible results are opaque descriptions, not callable IDs.

Discovery/tree/inspection responses wait for their own publication. A mutation is dispatched once, then the response waits for refresh and actual readback. Errors with `phase:"readback"` do not imply that the mutation was rolled back. Runtime expected UIA failures produce `Unavailable` with context; unexpected provider/infrastructure/invariant failures produce `Fatal` when reporting remains possible, then exit nonzero without a CLI-created dialog. There is no automatic mutation retry. `Window.Close` reports `{closed:true}` and clears a destroyed selection; a surviving root can also return its refreshed tree. Other mutations that remove their original cached tree node return `removed:true` and `readback:null` with the new tree, without querying the retired node.

`Exit-Application` drains workers and releases UIA/capture resources before its final response. EOF drains accepted work and exits normally without an extra response. Listening is canceled by the shared session shutdown.

### Commands

| Command and target | Exact arguments | Result and effect |
| --- | --- | --- |
| `Help-Command [command name]` | `{}` | `{commands:CommandSchema[],schemas:object,syntax:string,identity:string}`. Lists all command families or one definition, with examples and lifetime rules. |
| `List-Process` | `{}` | `{processes:Process[]}`. Explicit discovery refresh; parent links retain the process hierarchy and each process's own qualifying windows. |
| `Print-Window WindowId` | `{}` | `Tree`. Selects the discovered window and waits for complete tree/capture publication. Repeating for the selected window returns its cached generation. |
| `Refresh-Window WindowId` | `{}` | `Tree`. Requires the selected window, rereads tree/capture and expires old element/range IDs. |
| `Select-Node ElementId` | `{}` | `{id:ElementId}`. Selects/reveals a cached node without target focus or input. |
| `Query-Node ElementId` | `{}` | `Inspection`. Reads current properties, supported provider sections and returned references. External elements are accepted. |
| `Query-Properties ElementId` | `{}` | The same typed `Inspection`; supported properties retain false, zero, empty, null, mixed and read-only values. Unsupported property rows are omitted. |
| `Set-Property ElementId` | `{"propertyId":integer,"value":string or number}` | `{readback:Inspection or null,tree:Tree,removed?:true}`. Only an existing mapped setter is allowed; current capability is rechecked before one commit. Numeric values must be finite and within the setter bounds; choices use their integer value. |
| `Query-Providers ElementId` | `{}` | The same `Inspection`, including Element and supported versioned pattern sections, typed readouts and action schemas. |
| `Help-Provider [provider/client name]` | `{}` | `{catalog:ProviderSection[],currentTarget:ProviderSection[],properties:Descriptor[],attributes:Descriptor[],roles:Descriptor[],metadata:Descriptor[],boundary:string}`. Catalog commands have `enabled:null`; cached live sections report their actual enabled state. No target access occurs. Version-specific range names are client interfaces. |
| `Run-ProviderInterface::Method ElementId/RangeId` | Exactly the named fields in that action's `parameters` | Getter/workspace: `{value:Value,range:ProviderSection or null,references:Reference[]}`. Target mutation: `{value:Value,readback:Inspection or null,tree:Tree,removed?:true}`. Close: `{closed:true,tree?:Tree}`. |
| `Query-Range RangeId` | `{}` | `ProviderSection` with document, readouts, references and available operations. The preview text is limited; `Run-IUIAutomationTextRange::GetText` with `{"maxLength":-1}` requests full text. |
| `Query-Preview WindowId` | `{}` | `Preview` for the selected window and current generation. Capture bytes appear only here. |
| `HitTest-Preview WindowId` | `{"x":integer,"y":integer}` | `{id:ElementId or null,bounds:Bounds or null}`. Uses physical image-local coordinates and cached geometry; selects/reveals the match without target input. |
| `Exit-Application` | `{}` | `{exited:true}` followed by normal process exit. |

### Result schemas

The following notation describes exact field names. `WindowId`, `ElementId`, `RangeId`, decimal strings and hexadecimal strings are JSON strings. `Bounds` uses physical screen coordinates.

```typescript
type Bounds = {left:number, top:number, right:number, bottom:number};
type Process = {
  pid:number, parentPid:number|null, creationTime:string|null,
  executable:string, windows:Window[]
};
type Window = {id:WindowId, pid:number, hwnd:string, title:string, class:string};
type Tree = {rootIds:ElementId[], nodes:Node[]};
type Node = {
  id:ElementId, parentId:ElementId|null, runtimeId:string,
  controlType:number, role:string, client:string, name:string,
  automationId:string, providers:string, bounds:Bounds, offscreen:boolean
};
type Inspection = {
  id:ElementId, properties:Property[], providers:ProviderSection[], references:Reference[]
};
type Property = {
  propertyId:number, name:string, value:Value, enumName:string|null,
  sourcePattern:number, setter:Setter|null
};
type Choice = {value:number, name:string};
type Setter = {
  kind:"None"|"Text"|"Number"|"Choice", multiline:boolean,
  minimum:number, maximum:number, choices:Choice[]
};
type Reference = {
  id:ElementId|RangeId, kind:"Element"|"Range", documentId:ElementId|null,
  treeNodeId:ElementId|null, label:string
};
type ProviderSection = {
  patternId:number, provider:string, client:string, readouts:Property[],
  commands:ActionSchema[], references:Reference[], documentId:ElementId|null
};
type ActionSchema = {
  command:string, name:string, enabled:boolean|null, reason:string|null,
  mutation:boolean, pureGetter:boolean, parameters:Parameter[],
  result:string, effects:string, example:string
};
type Parameter = {
  name:string, type:"string"|"integer"|"number"|"choice"|"element"|"range"|"variant"|"attributeIds",
  nullable:boolean, multiline:boolean, minimum:number, maximum:number, choices:Choice[]
};
type Descriptor = {id:number, name:string, vartype:number, elementArray:boolean};
type CommandSchema = {
  command:string, target:string, arguments:string, result:string, effects:string, example:string
};
type Preview = {
  status:"Ready"|"Unavailable"|"GeometryChanged"|"Canceled",
  bounds:Bounds|null, dpi:number|null, width:number, height:number,
  timestamp:string|null, mediaType:"image/bmp", base64:string|null
};
```

Trees are complete flat preorder arrays, with parent before child, sibling order retained and no nesting-depth cutoff. The element's semantic `controlType`/`role` is separate from its highest acquired client interface. `providers` contains the cached provider-name summary; query the node for structured sections. `propertyId:0` denotes a named pattern readout. `sourcePattern:0` denotes a standard property or Element section. No writable setter is implied by an absent setter or `kind:"None"`.

```typescript
type Value = {
  kind:"Null"|"Unsupported"|"Mixed"|"Boolean"|"Signed"|"Unsigned"|
       "Real"|"String"|"Element"|"Range"|"Array"|"Opaque",
  vartype:number,
  value:unknown,
  dimensions:{lower:number,upper:number}[]
};
```

`Value.value` is null for Null/Unsupported/Mixed, boolean for Boolean, a decimal string for Signed/Unsigned, and a JSON number for finite Real. Nonfinite Real uses `{"nonfinite":"NaN"}`, `{"nonfinite":"Infinity"}` or `{"nonfinite":"-Infinity"}`. String holds the complete text delivered by UIA, with JSON escapes for controls and embedded NUL. Element/Range holds an opaque ID (or null); Array holds `Value[]` in the shared converter's element order and retains its dimension bounds; Opaque holds an interface description. `vartype` retains the native VARTYPE. Numeric enum/flag values remain in `value`; `enumName` adds a known canonical interpretation without discarding unknown numeric values.

Input arguments use JSON primitives, not output `Value` wrappers. `choice` accepts an integer, or a boolean only for an explicit false/true choice pair. `element`/`range` accepts an ID, or null when nullable. `attributeIds` is an integer array. A `variant` matches the property or attribute descriptor's VARTYPE: string, boolean, integer, finite real, element/null or array as declared. Strings passed to null-terminated methods reject embedded NUL. Help bounds and choices come from shared descriptors; the worker validates current capabilities again before invoking.

### Provider and range coverage

`Help-Provider` exposes the selected SDK's full property, pattern, text-attribute, role and metadata inventories. It derives commands from the native action schemas also used by the GUI. Catalog support is independent of target support: use `Query-Providers` or `Query-Range` for acquired interfaces and current capability. Parameterless properties/getters remain available as typed readouts even when the GUI has no command button. Provider names alias the corresponding public client patterns; arbitrary custom providers and methods on opaque COM results are not discoverable.

Range operations include acquisition through Text/Text2/TextEdit/TextChild, Clone, Compare, CompareEndpoints, ExpandToEnclosingUnit, Move, MoveEndpointByUnit, MoveEndpointByRange, FindText, FindAttribute, GetAttributeValue, GetText, GetBoundingRectangles, GetEnclosingElement, GetChildren, Select, AddToSelection, RemoveFromSelection and ScrollIntoView. `ReadAllAttributes` is the inspector helper for the full attribute catalog, not a COM method named `GetAttributeValue [44]`. `IUIAutomationTextRange2::ShowContextMenu` is version-qualified. `IUIAutomationTextRange3` exposes GetEnclosingElementBuildCache, GetChildrenBuildCache and GetAttributeValues when acquired; there is no invented TextRange3 provider interface. Element SetFocus/ShowContextMenu and explicit SynchronizedInput StartListening/Cancel retain their distinct effects; querying help never starts listening.

### Multi-command examples

IDs in these line examples are placeholders to replace with returned IDs from the same process:

```text
Help-Command
List-Process
Print-Window WINDOW_ID
Query-Properties ELEMENT_ID
Set-Property ELEMENT_ID {"propertyId":30045,"value":"Hello 日本語\nSecond line"}
Query-Providers NEW_ELEMENT_ID
Help-Provider IGridProvider
Run-IGridProvider::GetItem GRID_ID {"row":0,"column":0}
Query-Node RETURNED_ELEMENT_ID
Run-ITextProvider::DocumentRange DOCUMENT_ID
Query-Range RANGE_ID
Run-IUIAutomationTextRange::Clone RANGE_ID
Run-IUIAutomationTextRange::Compare RANGE_ID {"range":"CLONE_ID"}
Run-IUIAutomationTextRange::FindText RANGE_ID {"text":"Hello","backward":false,"ignoreCase":true}
Run-IUIAutomationTextRange::GetText FOUND_RANGE_ID {"maxLength":-1}
Query-Preview WINDOW_ID
HitTest-Preview WINDOW_ID {"x":120,"y":80}
Refresh-Window WINDOW_ID
Exit-Application
```

This Python example runs the shipped binary and uses returned IDs without relying on their format. Choose a PID belonging to an application you intend to inspect:

```python
import json, subprocess
p = subprocess.Popen([r"C:\Code\VczhLibraries\Release\Tools\UiaListCli.exe"],
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, encoding="utf-8")
def call(command, target=None, args=None):
    line = command + (" " + target if target else "")
    if args is not None:
        line += " " + json.dumps(args, ensure_ascii=False)
    p.stdin.write(line + "\n"); p.stdin.flush()
    reply = json.loads(p.stdout.readline())
    if not reply["ok"]: raise RuntimeError(reply["error"])
    return reply["result"]
processes = call("List-Process")["processes"]
target_pid = 1234  # replace with the intended PID from this response
window = next(x for x in processes if x["pid"] == target_pid)["windows"][0]
tree = call("Print-Window", window["id"])
details = call("Query-Node", tree["rootIds"][0])
print(details["properties"])
call("Exit-Application")
assert p.wait() == 0
```

Capture can be unavailable while node/property inspection remains usable. The CLI never restores, activates or moves a window to obtain pixels. Hit testing uses the cached tree/capture generation and one physical-coordinate mapping. UIA itself may normalize values before returning them; the CLI preserves what its client interfaces deliver, not inaccessible provider-internal bytes.
