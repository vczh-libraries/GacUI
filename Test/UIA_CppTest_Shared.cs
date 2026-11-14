using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Web.Script.Serialization;
using System.Windows.Automation;
using System.Windows.Automation.Text;

public static class GacUIShowcaseTests
{
    public static string ExchangeDocument;
    [ComImport, Guid("d22108aa-8ac5-49a5-837b-37bbb3d7591e"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface INativeAutomationElement {
        void SetFocus();
        [return: MarshalAs(UnmanagedType.SafeArray, SafeArraySubType=VarEnum.VT_I4)] int[] GetRuntimeId();
    }
    [ComVisible(true), Guid("146c3c17-f12e-4e22-8c27-f894b9b79c69"), InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    public interface INativeAutomationEventHandler {
        void HandleAutomationEvent(INativeAutomationElement sender, int eventId);
    }
    // System.Windows.Automation synthesizes WindowClosed from HWND events and
    // deliberately does not subscribe to provider WindowClosed. Use native UIA
    // for the hosted-window contract. Slots are from IUIAutomation in the SDK.
    [ComVisible(true), ClassInterface(ClassInterfaceType.None)]
    public sealed class NativeWindowEvents : INativeAutomationEventHandler, IDisposable {
        [DllImport("ole32.dll")] static extern int CoCreateInstance(ref Guid clsid, IntPtr outer, uint context, ref Guid iid, out IntPtr instance);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int GetRoot(IntPtr self, out IntPtr root);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int ElementFromHandle(IntPtr self, IntPtr hwnd, out IntPtr root);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int AddHandler(IntPtr self, int id, IntPtr element, int scope, IntPtr cache, [MarshalAs(UnmanagedType.Interface)] INativeAutomationEventHandler handler);
        [UnmanagedFunctionPointer(CallingConvention.StdCall)] delegate int RemoveHandler(IntPtr self, int id, IntPtr element, [MarshalAs(UnmanagedType.Interface)] INativeAutomationEventHandler handler);
        IntPtr instance, desktop;
        string expected;
        public int Closed;
        static T Method<T>(IntPtr self, int slot) where T : class {
            return (T)(object)Marshal.GetDelegateForFunctionPointer(Marshal.ReadIntPtr(Marshal.ReadIntPtr(self), slot * IntPtr.Size), typeof(T));
        }
        public NativeWindowEvents(int[] runtimeId, IntPtr hwnd) {
            expected = string.Join(",", Array.ConvertAll(runtimeId, id => id.ToString()));
            var clsid = new Guid("ff48dba4-60ef-4201-aa87-54103eef594e");
            var iid = new Guid("30cbe57d-d9d0-452a-ab13-7ac5ac4825ee");
            Marshal.ThrowExceptionForHR(CoCreateInstance(ref clsid, IntPtr.Zero, 1, ref iid, out instance));
            Marshal.ThrowExceptionForHR(hwnd == IntPtr.Zero
                ? Method<GetRoot>(instance, 5)(instance, out desktop)
                : Method<ElementFromHandle>(instance, 6)(instance, hwnd, out desktop));
            Marshal.ThrowExceptionForHR(Method<AddHandler>(instance, 32)(instance, 20017, desktop, 7, IntPtr.Zero, this));
        }
        public void HandleAutomationEvent(INativeAutomationElement sender, int eventId) {
            try {
                if (eventId == 20017 && string.Join(",", Array.ConvertAll(sender.GetRuntimeId(), id => id.ToString())) == expected) Interlocked.Increment(ref Closed);
            }
            finally { Marshal.ReleaseComObject(sender); }
        }
        public void Dispose() {
            Marshal.ThrowExceptionForHR(Method<RemoveHandler>(instance, 33)(instance, 20017, desktop, this));
            Marshal.Release(desktop); Marshal.Release(instance);
        }
    }
    delegate bool EnumProc(IntPtr hwnd, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumChildWindows(IntPtr parent, EnumProc callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern int GetDlgCtrlID(IntPtr hwnd);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint process);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr hwnd, StringBuilder text, int count);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr hwnd);
    [DllImport("user32.dll")] static extern IntPtr GetForegroundWindow();
    [StructLayout(LayoutKind.Sequential)] struct NativeRect { public int Left, Top, Right, Bottom; }
    [DllImport("user32.dll")] static extern bool GetWindowRect(IntPtr hwnd, out NativeRect rectangle);
    delegate bool MonitorProc(IntPtr monitor, IntPtr dc, ref NativeRect rectangle, IntPtr parameter);
    [StructLayout(LayoutKind.Sequential)] struct MonitorInfo { public int Size; public NativeRect Monitor, Work; public uint Flags; }
    [DllImport("user32.dll")] static extern bool EnumDisplayMonitors(IntPtr dc, IntPtr clip, MonitorProc callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool GetMonitorInfo(IntPtr monitor, ref MonitorInfo info);
    [StructLayout(LayoutKind.Sequential)] struct NativePoint { public int X, Y; }
    [DllImport("user32.dll")] static extern bool ScreenToClient(IntPtr hwnd, ref NativePoint point);
    [DllImport("user32.dll")] static extern bool ClientToScreen(IntPtr hwnd, ref NativePoint point);
    [DllImport("user32.dll")] static extern uint GetDpiForWindow(IntPtr hwnd);
    [DllImport("user32.dll", SetLastError=true)] static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("kernel32.dll", CharSet=CharSet.Unicode)] static extern IntPtr OpenEvent(uint access, bool inherit, string name);
    [DllImport("kernel32.dll")] static extern uint WaitForSingleObject(IntPtr handle, uint milliseconds);
    [DllImport("kernel32.dll")] static extern bool SetEvent(IntPtr handle);
    [DllImport("kernel32.dll")] static extern bool CloseHandle(IntPtr handle);
    [DllImport("wtsapi32.dll", CharSet=CharSet.Unicode)] static extern bool WTSQuerySessionInformation(IntPtr server, int session, int info, out IntPtr buffer, out int bytes);
    [DllImport("wtsapi32.dll")] static extern void WTSFreeMemory(IntPtr buffer);
    static string inputEndpoint;
    static void Press(string key) { Input("!KeyPress:" + key); }
    static void Click(AutomationElement element)
    {
        var bounds = System.Windows.Rect.Empty;
        int stable = 0;
        Wait(() => {
            var current = element.Current.BoundingRectangle;
            // GacUI derives IsOffscreen from this same rectangle. Avoid a
            // second provider query while a large virtual font list is open.
            bool same = current.Width > 0 && current.Height > 0 && current == bounds;
            stable = same ? stable + 1 : 0;
            bounds = current;
            return stable >= 3;
        }, "click target geometry settles: " + element.Current.Name, 60000);
        Pointer("LeftClick", bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2);
    }
    static void Pointer(string action, double x, double y)
    {
        var point = UiaToClientPoint(x, y);
        Input("!" + action + ":" + point.X + "," + point.Y);
    }
    static double uiaClientWidth, uiaClientHeight, uiaScaleX, uiaScaleY;
    static NativePoint UiaToClientPoint(double x, double y)
    {
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        uint dpi = GetDpiForWindow(hwnd);
        Check(dpi != 0, "read pointer coordinate DPI");
        if (isHosted)
        {
            // UIA scales the DPI-unaware GDI provider to physical pixels even
            // though GetDpiForWindow reports 96. Use the observed provider/client
            // size ratio and refresh it after a resize or DPI change.
            var origin = root.Current.BoundingRectangle;
            if (uiaClientWidth != origin.Width || uiaClientHeight != origin.Height)
            {
                var window = (Dictionary<string, object>)ControlsSnapshot()["MainWindow"];
                var client = SnapshotBounds((Dictionary<string, object>)window["composition"]);
                Check(client.Width > 0 && client.Height > 0, "hosted client has usable bounds");
                uiaScaleX = client.Width / origin.Width;
                uiaScaleY = client.Height / origin.Height;
                uiaClientWidth = origin.Width;
                uiaClientHeight = origin.Height;
            }
            return new NativePoint { X = (int)Math.Round((x - origin.Left) * uiaScaleX), Y = (int)Math.Round((y - origin.Top) * uiaScaleY) };
        }
        var point = new NativePoint { X = (int)x, Y = (int)y };
        Check(ScreenToClient(hwnd, ref point), "convert pointer to client coordinates");
        return new NativePoint { X = (int)Math.Round(point.X * 96.0 / dpi), Y = (int)Math.Round(point.Y * 96.0 / dpi) };
    }
    static void Drag(double x, double y, double dx, double dy)
    {
        Pointer("LeftDown", x, y);
        Pointer("MouseMove", x + dx, y + dy);
        Pointer("LeftUp", x + dx, y + dy);
    }
    static void Input(string command, string endpoint = null)
    {
        using (var client = new System.Net.WebClient())
        {
            client.Headers[System.Net.HttpRequestHeader.ContentType] = "application/json; charset=utf8";
            var response = Encoding.UTF8.GetString(client.UploadData(endpoint ?? inputEndpoint, Encoding.UTF8.GetBytes(command)));
            Check(response == "Queued", "input accepted (" + command + "): " + response);
        }
    }
    static AutomationElement root;
    static int assertions;
    static int structureEvents;
    static int propertyEvents;
    static int textEvents;
    static int selectionEvents;
    static int ownedProcess;
    public static string BusyCommand;
    public static string PersistenceOutput;
    static bool isHosted;
    static bool isWorkflowBinary;
    // The Windows event IDs exist in UIAutomationTypes but its managed public
    // surface omits these two identifiers. Register them for this client.
    static AutomationEvent NativeEvent(int id, string name)
    {
        var register = typeof(AutomationEvent).GetMethod("Register", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic);
        return (AutomationEvent)register.Invoke(null, new object[] { Enum.ToObject(register.GetParameters()[0].ParameterType, id), name });
    }
    static readonly AutomationEvent MenuModeStartEvent = NativeEvent(20018, "MenuModeStart");
    static readonly AutomationEvent MenuModeEndEvent = NativeEvent(20019, "MenuModeEnd");
    static List<AutomationElement> OwnedWindows()
    {
        var result = new List<AutomationElement>();
        if (isHosted) result.Add(root);
        else EnumWindows((window, parameter) => {
            uint process; GetWindowThreadProcessId(window, out process);
            // Ordinary mode also opens operating-system dialogs such as #32770.
            if (process == ownedProcess && IsWindowVisible(window)) result.Add(AutomationElement.FromHandle(window));
            return true;
        }, IntPtr.Zero);
        return result;
    }
    static AutomationElement Popup(ControlType type, AutomationElement selectionOwner = null)
    {
        var windows = new List<AutomationElement>();
        if (selectionOwner != null) windows.Add(selectionOwner);
        windows.AddRange(Find(root, type));
        windows.AddRange(Find(root, ControlType.Menu));
        windows.AddRange(isHosted ? Find(root, ControlType.Window, TreeScope.Children) : OwnedWindows());
        foreach (var window in windows)
        {
            if (Automation.Compare(window, root) || window.Current.IsOffscreen) continue;
            if (!Automation.Compare(window, root) && window.Current.ControlType == type) return window;
            foreach (var found in Find(window, type))
            {
                if (selectionOwner == null) return found;
                var items = Items(found);
                if (items.Count > 0 && Automation.Compare(Pattern<SelectionItemPattern>(items[0], SelectionItemPattern.Pattern).Current.SelectionContainer, selectionOwner)) return found;
            }
        }
        return null;
    }
    static void Check(bool condition, string message) { if (!condition) throw new Exception(message); assertions++; }
    static void Test(string name) { Console.WriteLine("TEST " + name); }
    static void Wait(Func<bool> predicate, string message, int timeout = 10000)
    {
        var timer = Stopwatch.StartNew();
        while (timer.ElapsedMilliseconds < timeout) { if (predicate()) { assertions++; return; } Thread.Sleep(30); }
        if (!string.IsNullOrEmpty(PersistenceOutput))
        {
            try { System.IO.Directory.CreateDirectory(PersistenceOutput); System.IO.File.WriteAllText(System.IO.Path.Combine(PersistenceOutput, "uia-timeout.json"), new JavaScriptSerializer { MaxJsonLength = int.MaxValue, RecursionLimit = 512 }.Serialize(ControlsSnapshot())); }
            catch (Exception error) { Console.WriteLine("NOTE timeout snapshot unavailable: " + error.Message); }
        }
        throw new Exception("Timed out: " + message);
    }
    static List<AutomationElement> Find(AutomationElement parent, ControlType type, TreeScope scope = TreeScope.Descendants)
    {
        var result = new List<AutomationElement>();
        foreach (AutomationElement element in parent.FindAll(scope, new PropertyCondition(AutomationElement.ControlTypeProperty, type))) result.Add(element);
        return result;
    }
    static AutomationElement Named(AutomationElement parent, string name, ControlType type)
    {
        var found = parent.FindFirst(TreeScope.Descendants, new AndCondition(new PropertyCondition(AutomationElement.NameProperty, name), new PropertyCondition(AutomationElement.ControlTypeProperty, type)));
        Check(found != null, "Missing " + type.ProgrammaticName + ": " + name);
        return found;
    }
    static AutomationElement FindNamed(AutomationElement parent, string name, ControlType type)
    {
        return parent.FindFirst(TreeScope.Descendants, new AndCondition(new PropertyCondition(AutomationElement.NameProperty, name), new PropertyCondition(AutomationElement.ControlTypeProperty, type)));
    }
    static T Pattern<T>(AutomationElement element, AutomationPattern pattern) { object value; Check(element.TryGetCurrentPattern(pattern, out value), "Missing " + pattern.ProgrammaticName + " on " + element.Current.Name); return (T)value; }
    static void Absent(AutomationElement element, AutomationPattern pattern) { object value; Check(!element.TryGetCurrentPattern(pattern, out value), "Unexpected " + pattern.ProgrammaticName + " on " + element.Current.Name); }
    static void Invoke(AutomationElement parent, string name) { Pattern<InvokePattern>(Named(parent, name, ControlType.Button), InvokePattern.Pattern).Invoke(); }
    static AutomationElement Page(AutomationElement parent, string name)
    {
        var page = Named(parent, name, ControlType.TabItem);
        var selection = Pattern<SelectionItemPattern>(page, SelectionItemPattern.Pattern);
        selection.Select(); Wait(() => selection.Current.IsSelected, name + " selection");
        if (name == "Layout")
        {
            ControlsSnapshot(); // Read on the UI thread after the deferred template installation.
            page = Named(parent, name, ControlType.TabItem);
        }
        return page;
    }
    static AutomationElement RebuiltLayoutPage(string name)
    {
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        // Rebuilding detaches controls and retires their UIA attachments.
        // Wait for the queued input/Invoke and provider disconnections before enumeration.
        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000), "owning layout rebuild reaches idle");
        root = AutomationElement.FromHandle(hwnd);
        return Page(Page(root, "Layout"), name);
    }
    static List<AutomationElement> Items(AutomationElement list) { return Find(list, ControlType.ListItem, TreeScope.Children); }
    static void Same(AutomationElement actual, AutomationElement expected, string message) { Check(Automation.Compare(actual, expected), message); }
    static void CheckFocus(AutomationElement element, string message)
    {
        Check(element.Current.HasKeyboardFocus, message + " property");
        var focused = root.FindAll(TreeScope.Descendants, new AndCondition(
            new PropertyCondition(AutomationElement.HasKeyboardFocusProperty, true),
            new PropertyCondition(AutomationElement.FrameworkIdProperty, "GacUI")));
        var focusNames = new List<string>();
        foreach (AutomationElement candidate in focused) focusNames.Add(candidate.Current.ControlType.ProgrammaticName + "/" + candidate.Current.Name + "/offscreen=" + candidate.Current.IsOffscreen);
        Check(focused.Count == 1 && Automation.Compare(focused[0], element), message + " unique reachable logical focus: " + string.Join("; ", focusNames.ToArray()));
        IntPtr buffer; int bytes;
        Check(WTSQuerySessionInformation(IntPtr.Zero, -1, 8, out buffer, out bytes), "query session connection state");
        int connectionState;
        try { connectionState = Marshal.ReadInt32(buffer); }
        finally { WTSFreeMemory(buffer); }
        uint foregroundProcess;
        GetWindowThreadProcessId(GetForegroundWindow(), out foregroundProcess);
        if (connectionState == 4) Console.WriteLine("UNVERIFIED desktop focus lookup: WTSDisconnected session");
        else if (foregroundProcess != ownedProcess) Console.WriteLine("UNVERIFIED desktop focus lookup: test application is not foreground");
        else
        {
            var actual = AutomationElement.FocusedElement;
            string description = actual == null ? "null" : actual.Current.ControlType.ProgrammaticName + "/" + actual.Current.ClassName + "/PID=" + actual.Current.ProcessId + "/" + actual.Current.Name;
            Same(actual, element, message + " desktop lookup (actual=" + description + ", expected PID=" + ownedProcess + ")");
        }
    }
    static void Names(List<AutomationElement> items, params string[] names)
    {
        Check(items.Count == names.Length, "Item count: expected " + names.Length + ", got " + items.Count);
        for (int i = 0; i < names.Length; i++) Check(items[i].Current.Name == names[i], "Item " + i + " expected " + names[i] + ", got " + items[i].Current.Name);
    }
    static void SelectCombo(AutomationElement container, AutomationElement combo, string name)
    {
        var expand = Pattern<ExpandCollapsePattern>(combo, ExpandCollapsePattern.Pattern);
        expand.Expand();
        AutomationElement item = null;
        Wait(() => { item = combo.FindFirst(TreeScope.Descendants, new AndCondition(new PropertyCondition(AutomationElement.NameProperty, name), new PropertyCondition(AutomationElement.ControlTypeProperty, ControlType.ListItem))); return item != null; }, "popup item under combo: " + name);
        Pattern<SelectionItemPattern>(item, SelectionItemPattern.Pattern).Select();
        expand.Collapse();
    }
    static void TextLists()
    {
        Test("List / TextList: model values, Invoke, selection, Toggle, source replacement, events");
        var page = Page(Page(root, "List"), "TextList");
        var lists = Find(page, ControlType.List);
        Check(lists.Count == 2, "TextList must contain exactly two semantic lists");
        int before = structureEvents;
        Invoke(page, "Add 10 items"); Wait(() => Items(lists[0]).Count == 10, "ten items");
        Names(Items(lists[0]), "0","1","2","3","4","5","6","7","8","9");
        Names(Items(lists[1]), "0","1","2","3","4","5","6","7","8","9");
        Wait(() => structureEvents > before, "item insertion structure event");
        var first = Items(lists[0])[0];
        var retained = Items(lists[0])[1];
        var selection = Pattern<SelectionItemPattern>(retained, SelectionItemPattern.Pattern);
        selection.Select(); Check(selection.Current.IsSelected, "selection changed");
        Same(selection.Current.SelectionContainer, lists[0], "selection container");
        Absent(retained, TogglePattern.Pattern);
        var combo = Find(page, ControlType.ComboBox)[0];
        foreach (string mode in new [] { "Check", "Radio" })
        {
            SelectCombo(page, combo, mode);
            var toggle = Pattern<TogglePattern>(retained, TogglePattern.Pattern);
            var old = toggle.Current.ToggleState; toggle.Toggle();
            Check(toggle.Current.ToggleState != old, "check changed in " + mode);
            Check(selection.Current.IsSelected, "checking preserves selection in " + mode);
        }
        Invoke(page, "Remove odd items"); Wait(() => Items(lists[0]).Count == 5, "odd removal");
        Names(Items(lists[0]), "1","3","5","7","9");
        Check(retained.Current.Name == "1", "retained item identity across deletion");
        bool stale = false; try { var ignored = first.Current.Name; } catch (ElementNotAvailableException) { stale = true; }
        Check(stale, "removed item must be unavailable");
        Invoke(page, "Remove even items"); Wait(() => Items(lists[0]).Count == 3, "even removal"); Names(Items(lists[0]), "1","5","9");
        Invoke(page, "Rotate Item Source -->"); Wait(() => Items(lists[1]).Count == 0, "source detach");
        Invoke(page, "Rotate Item Source -->"); Wait(() => Items(lists[1]).Count == 3, "source attach");
        Invoke(page, "Clear"); Wait(() => Items(lists[0]).Count == 0 && Items(lists[1]).Count == 0, "clear");
        var check = Find(page, ControlType.CheckBox)[0];
        var checkPattern = Pattern<TogglePattern>(check, TogglePattern.Pattern); var state = checkPattern.Current.ToggleState; checkPattern.Toggle(); Check(checkPattern.Current.ToggleState != state, "checkbox Toggle");
        var radio = Find(page, ControlType.RadioButton)[0]; Pattern<SelectionItemPattern>(radio, SelectionItemPattern.Pattern).Select(); Check(Pattern<SelectionItemPattern>(radio, SelectionItemPattern.Pattern).Current.IsSelected, "radio selection"); Absent(radio, TogglePattern.Pattern);
    }
    static void ListViews()
    {
        Test("List / ListView: six views, headers, Grid/Table, scrolling and retained items");
        var page = Page(Page(root, "List"), "ListView");
        var grids = Find(page, ControlType.DataGrid); Check(grids.Count == 2, "two detail lists");
        foreach (var list in grids)
        {
            var grid = Pattern<GridPattern>(list, GridPattern.Pattern);
            Check(grid.Current.ColumnCount == 4 && grid.Current.RowCount > 0, "detail dimensions");
            var headers = Pattern<TablePattern>(list, TablePattern.Pattern).Current.GetColumnHeaders();
            Names(new List<AutomationElement>(headers), "Id", "Category", "Size", "File");
            var headerBounds = headers[0].Current.BoundingRectangle;
            var headerTransform = Pattern<TransformPattern>(headers[0], TransformPattern.Pattern);
            Check(headerTransform.Current.CanResize && !headerTransform.Current.CanMove && !headerTransform.Current.CanRotate, "header resize capabilities");
            headerTransform.Resize(headerBounds.Width + 11, headerBounds.Height);
            Wait(() => headers[0].Current.BoundingRectangle.Width > headerBounds.Width, "column header resized");
            headerTransform.Resize(headerBounds.Width, headerBounds.Height);
            var cell = grid.GetItem(0, 2); var item = Pattern<GridItemPattern>(cell, GridItemPattern.Pattern);
            Check(item.Current.Row == 0 && item.Current.Column == 2 && item.Current.RowSpan == 1, "cell coordinates"); Same(item.Current.ContainingGrid, list, "containing grid");
            var rows = Find(list, ControlType.DataItem, TreeScope.Children); var retained = rows[0];
            Pattern<SelectionItemPattern>(retained, SelectionItemPattern.Pattern).Select();
            Pattern<ScrollItemPattern>(rows[rows.Count - 1], ScrollItemPattern.Pattern).ScrollIntoView();
            Pattern<ScrollPattern>(list, ScrollPattern.Pattern);
            var views = Pattern<MultipleViewPattern>(list, MultipleViewPattern.Pattern);
            Check(views.Current.GetSupportedViews().Length == 6, "six supported views");
            foreach (int view in views.Current.GetSupportedViews())
            {
                Test("ListView / " + views.GetViewName(view));
                views.SetCurrentView(view); Check(views.Current.CurrentView == view, "view changed");
                if (view == 4) Absent(list, GridPattern.Pattern); else Pattern<GridPattern>(list, GridPattern.Pattern);
                if (view == 5) Pattern<TablePattern>(list, TablePattern.Pattern); else Absent(list, TablePattern.Pattern);
                Check(Pattern<SelectionItemPattern>(retained, SelectionItemPattern.Pattern).Current.IsSelected, "selection survives view change");
            }
        }
    }
    static void Trees()
    {
        Test("List / TreeView: hierarchy, expand/collapse, leaf state and selection");
        var page = Page(Page(root, "List"), "TreeView");
        foreach (var tree in Find(page, ControlType.Tree))
        {
            var blue = Named(tree, "Blue+", ControlType.TreeItem); var expand = Pattern<ExpandCollapsePattern>(blue, ExpandCollapsePattern.Pattern);
            expand.Expand(); Check(expand.Current.ExpandCollapseState == ExpandCollapseState.Expanded, "expanded root");
            var children = Find(blue, ControlType.TreeItem, TreeScope.Children); Check(children.Count > 0, "nested tree children");
            Same(TreeWalker.RawViewWalker.GetParent(children[0]), blue, "nested parent");
            var selection = Pattern<SelectionItemPattern>(children[0], SelectionItemPattern.Pattern); selection.Select(); Check(selection.Current.IsSelected, "tree selection"); Same(selection.Current.SelectionContainer, tree, "tree selection container");
            expand.Collapse(); Check(Find(blue, ControlType.TreeItem, TreeScope.Children).Count == 0, "collapsed children hidden");
            expand.Expand();
        }
        var bound = Find(page, ControlType.Tree)[1]; var retained = Find(bound, ControlType.TreeItem, TreeScope.Children)[0];
        Invoke(page, "Rotate Item Source -->"); Wait(() => Find(bound, ControlType.TreeItem, TreeScope.Children).Count == 0, "tree source detached");
        bool stale = false; try { var ignored = retained.Current.Name; } catch (ElementNotAvailableException) { stale = true; }
        Check(stale, "old bound tree node unavailable");
        Invoke(page, "Rotate Item Source -->"); Wait(() => Find(bound, ControlType.TreeItem, TreeScope.Children).Count > 0, "tree source restored");
    }
    static void Grids()
    {
        Test("List / BindableDataGrid: five rows, headers, active text editor and immediate save");
        var page = Page(Page(root, "List"), "BindableDataGrid");
        var element = Find(page, ControlType.DataGrid)[0]; var grid = Pattern<GridPattern>(element, GridPattern.Pattern);
        Check(grid.Current.RowCount == 5 && grid.Current.ColumnCount == 5, "five by five grid");
        Names(new List<AutomationElement>(Pattern<TablePattern>(element, TablePattern.Pattern).Current.GetColumnHeaders()), "Name","Gender","Category","Birthday","Website");
        var cell = grid.GetItem(0,0); Pattern<InvokePattern>(cell, InvokePattern.Pattern).Invoke();
        Wait(() => Find(cell, ControlType.Edit).Count == 1, "one active text editor");
        var editor = Find(cell, ControlType.Edit)[0]; var value = Pattern<ValuePattern>(editor, ValuePattern.Pattern);
        Same(TreeWalker.RawViewWalker.GetParent(editor), cell, "editor parent is cell");
        value.SetValue("UIA edited name"); Wait(() => grid.GetItem(0,0).Current.Name == "UIA edited name", "immediate cell save");
        editor.SetFocus(); Check(editor.Current.HasKeyboardFocus, "text editor focus");
        // The single-line text box consumes Enter. The grid handles its own
        // Enter/Escape commands when focus is on the grid.
        element.SetFocus();
        Press("Enter");
        Wait(() => Find(cell, ControlType.Edit).Count == 0, "old editor removed");
        Check(cell.Current.Name == "UIA edited name", "Enter preserves saved text");
        Pattern<InvokePattern>(cell, InvokePattern.Pattern).Invoke();
        Wait(() => Find(cell, ControlType.Edit).Count == 1, "reopened editor");
        editor = Find(cell, ControlType.Edit)[0]; Pattern<ValuePattern>(editor, ValuePattern.Pattern).SetValue("UIA saved before Escape"); element.SetFocus();
        Press("Esc");
        Wait(() => Find(cell, ControlType.Edit).Count == 0, "Escape closes editor");
        Check(cell.Current.Name == "UIA saved before Escape", "Escape does not roll back immediate save");
        Check(Pattern<GridItemPattern>(cell, GridItemPattern.Pattern).Current.Column == 0, "cell identity survives editor replacement");
        Test("grid / selection editor");
        var gender = grid.GetItem(0,1); Pattern<InvokePattern>(gender, InvokePattern.Pattern).Invoke();
        Wait(() => Find(gender, ControlType.ComboBox).Count == 1, "one gender editor");
        var combo = Find(gender, ControlType.ComboBox)[0];
        var expand = Pattern<ExpandCollapsePattern>(combo, ExpandCollapsePattern.Pattern); expand.Expand();
        AutomationElement popup = null;
        Wait(() => { popup = Popup(ControlType.List, combo); return popup != null; }, "gender popup list");
        var choices = Items(popup);
        Check(choices.Count == 2, "two gender choices, got " + choices.Count);
        var oldGender = gender.Current.Name; Pattern<SelectionItemPattern>(choices[oldGender == "Male" ? 1 : 0], SelectionItemPattern.Pattern).Select();
        Wait(() => gender.Current.Name != oldGender, "gender saved immediately");
        Test("grid / date editor");
        var birthday = grid.GetItem(0,3); Pattern<InvokePattern>(birthday, InvokePattern.Pattern).Invoke();
        Wait(() => Find(birthday, ControlType.ComboBox).Count == 1, "one date editor");
        var date = Find(birthday, ControlType.ComboBox)[0]; Pattern<ExpandCollapsePattern>(date, ExpandCollapsePattern.Pattern).Expand();
        AutomationElement calendar = null;
        Wait(() => { calendar = Popup(ControlType.Calendar); return calendar != null; }, "date popup calendar");
        var oldDate = birthday.Current.Name;
        Pattern<SelectionItemPattern>(Pattern<GridPattern>(calendar, GridPattern.Pattern).GetItem(2,3), SelectionItemPattern.Pattern).Select();
        Wait(() => birthday.Current.Name != oldDate, "birthday saved immediately");
        Pattern<SelectionItemPattern>(grid.GetItem(1,0), SelectionItemPattern.Pattern).Select();
        Check(Find(birthday, ControlType.ComboBox).Count == 0, "date editor removed");
    }
    static void TextControls()
    {
        Test("Control / TextBox: Value, Text, range movement, selection, attributes and events");
        var page = Page(Page(root, "Control"), "TextBox");
        foreach (string name in new [] { "TextBox", "TextBox (No Tab)", "Document", "Document (No Tab)" })
        {
            Test("text / " + name);
            var subpage = Page(page, name);
            var inputs = new List<AutomationElement>(Find(subpage, ControlType.Edit));
            foreach (var doc in Find(subpage, ControlType.Document)) { object valuePattern; if (doc.TryGetCurrentPattern(ValuePattern.Pattern, out valuePattern)) inputs.Add(doc); }
            foreach (var edit in inputs)
            {
                var value = Pattern<ValuePattern>(edit, ValuePattern.Pattern); if (edit.Current.IsPassword) { Absent(edit, TextPattern.Pattern); continue; }
                var text = Pattern<TextPattern>(edit, TextPattern.Pattern);
                if (!value.Current.IsReadOnly)
                {
                    int before = textEvents; value.SetValue("UIA text test"); Check(value.Current.Value == "UIA text test", "Value edit");
                    Check(text.DocumentRange.GetText(-1) == "UIA text test", "Text matches Value"); Check(edit.Current.Name != "UIA text test", "input contents are not the accessible label"); Wait(() => textEvents > before, "text changed event");
                    var range = text.DocumentRange.FindText("text", false, false); Check(range != null && range.GetText(-1) == "text", "FindText");
                    before = selectionEvents;
                    range.Select(); Check(text.GetSelection().Length == 1 && text.GetSelection()[0].GetText(-1) == "text", "text selection");
                    Wait(() => selectionEvents > before, "text selection notification");
                    var clone = range.Clone(); Check(clone.Compare(range), "range clone"); Check(clone.Move(TextUnit.Character, 1) == 1, "range move");
                    value.SetValue(""); Check(text.DocumentRange.GetText(-1) == "", "empty document remains empty");
                    value.SetValue("UIA text test");
                }
            }
            foreach (var doc in Find(subpage, ControlType.Document))
            {
                var text = Pattern<TextPattern>(doc, TextPattern.Pattern); var range = text.DocumentRange;
                Check(range.GetText(-1).Length > 0, "rich text content");
                Check(range.GetAttributeValue(TextPattern.FontNameAttribute) != AutomationElement.NotSupported, "font attribute");
                if (range.GetText(-1).Contains("Size Title")) Check(range.GetAttributeValue(TextPattern.FontSizeAttribute) == TextPattern.MixedAttributeValue, "mixed font sizes");
                Check(range.GetAttributeValue(TextPattern.AnimationStyleAttribute) == AutomationElement.NotSupported, "unsupported attribute sentinel");
                range.Clone().ExpandToEnclosingUnit(TextUnit.Paragraph);
                Check(text.GetVisibleRanges().Length > 0, "visible rich-text ranges"); range.GetChildren();
            }
        }
        var embeddedPage = Page(Page(root, "Control"), "Embedded Controls");
        var embedded = Find(embeddedPage, ControlType.Document)[0]; var embeddedText = Pattern<TextPattern>(embedded, TextPattern.Pattern);
        var children = embeddedText.DocumentRange.GetChildren(); Check(children.Length == 17, "embedded document children include offscreen table labels, combo and button");
        foreach (var child in children) Same(TreeWalker.RawViewWalker.GetParent(child), embedded, "embedded child belongs to document");
        Check(embeddedText.RangeFromChild(children[0]).GetText(-1).Contains("\uFFFC"), "embedded child range");
    }
    static void RefreshLists()
    {
        Test("Refresh List / same-count names, cells, columns and tree values");
        var refresh = Page(root, "Refresh List");
        var page = Page(refresh, "BindableTextList"); var list = Find(page, ControlType.List)[0];
        Invoke(page, "Use Title"); Wait(() => Items(list)[0].Current.Name == "1st", "bound Title text");
        Invoke(page, "Use Name"); Wait(() => Items(list)[0].Current.Name == "First", "bound Name text");
        page = Page(refresh, "TreeView"); var tree = Find(page, ControlType.Tree)[0]; var first = Named(tree, "First", ControlType.TreeItem);
        int before = propertyEvents; Invoke(page, "*First"); Wait(() => first.Current.Name == "One", "same tree node renamed"); Wait(() => propertyEvents > before, "tree property notification");
        page = Page(refresh, "BindableDataGrid"); var element = Find(page, ControlType.DataGrid)[0]; var grid = Pattern<GridPattern>(element, GridPattern.Pattern);
        Invoke(page, "*Sub1"); Wait(() => grid.GetItem(0,1).Current.Name == "SubColumn", "same-count cell update");
        var cell = grid.GetItem(0,1); before = structureEvents;
        Invoke(page, "*Column"); Wait(() => Pattern<TablePattern>(element, TablePattern.Pattern).Current.GetColumnHeaders()[1].Current.Name == "What?", "column caption update");
        Invoke(page, "*Column"); Wait(() => grid.Current.ColumnCount == 3, "column removed");
        Wait(() => structureEvents > before, "column structure notification");
        bool stale = false; try { var ignored = cell.Current.Name; } catch (ElementNotAvailableException) { stale = true; } Check(stale, "column replacement retires cell");
    }
    static Dictionary<string, object> RawClockSnapshot()
    {
        Dictionary<string, object> result = null;
        Wait(() =>
        {
            foreach (var node in SnapshotNodes(ControlsSnapshot()))
            {
                object id;
                if (node.TryGetValue("elementId", out id) && (string)id == "FullControlTest.Clock") { result = node; return true; }
            }
            return false;
        }, "raw clock becomes visible", 120000);
        return result;
    }
    static System.Windows.Rect RenderedCaptionBounds(string caption)
    {
        var result = System.Windows.Rect.Empty;
        Wait(() =>
        {
            foreach (var node in SnapshotNodes(ControlsSnapshot()))
            {
                object text;
                if (!node.TryGetValue("elementText", out text) || (string)text != caption) continue;
                var bounds = SnapshotBounds(node);
                if (bounds.Width <= 0 || bounds.Height <= 0) continue;
                result = bounds;
                return true;
            }
            return false;
        }, "rendered caption " + caption, 120000);
        return result;
    }
    static void ClickRenderedCaption(string caption)
    {
        var bounds = RenderedCaptionBounds(caption);
        Input("!LeftClick:" + (int)(bounds.Left + bounds.Width / 2) + "," + (int)(bounds.Top + bounds.Height / 2));
    }
    static void OpenRawClock()
    {
        Wait(() =>
        {
            bool hasPage = false;
            foreach (var node in SnapshotNodes(ControlsSnapshot()))
            {
                object value;
                if (node.TryGetValue("elementId", out value) && (string)value == "FullControlTest.Clock") return true;
                if (node.TryGetValue("elementText", out value) && (string)value == "Raw Clock") hasPage = true;
            }
            ClickRenderedCaption(hasPage ? "Raw Clock" : "Misc");
            return false;
        }, "Raw Clock page opens through automation", 120000);
    }
    static void RawClockAutomation()
    {
        Test("Raw Clock / automation navigation, data timer and useful bounds");
        OpenRawClock();
        var clock = RawClockSnapshot();
        string initial = (string)clock["elementData"];
        Check(System.Text.RegularExpressions.Regex.IsMatch(initial, "^[0-2][0-9]:[0-5][0-9]:[0-5][0-9]$"), "clock has HH:mm:ss data");
        var bounds = SnapshotBounds(clock);
        Check(bounds.Width >= 200 && bounds.Height >= 200, "raw clock has explicit useful bounds");
        Wait(() => (string)RawClockSnapshot()["elementData"] != initial, "clock data changes without input");

        Test("Raw Clock / automation palette refresh preserves raw element");
        ClickRenderedCaption("Window Manager");
        ClickRenderedCaption("Aurora");
        ControlsSnapshot();
        OpenRawClock();
        initial = (string)RawClockSnapshot()["elementData"];
        Wait(() => (string)RawClockSnapshot()["elementData"] != initial, "clock continues after palette refresh");
    }

    static void RawClock(bool runAutomation)
    {
        if (runAutomation) RawClockAutomation();
        var bounds = SnapshotBounds(RawClockSnapshot());
        Test("Raw Clock / UIA names, page selection and resize");
        root = AutomationElement.FromHandle(new IntPtr(root.Current.NativeWindowHandle));
        var manager = Page(root, "Window Manager");
        Check(Pattern<SelectionItemPattern>(Named(manager, "Aurora", ControlType.RadioButton), SelectionItemPattern.Pattern).Current.IsSelected, "Aurora palette selected through automation");
        var page = Page(Page(root, "Misc"), "Raw Clock");
        Check(Pattern<SelectionItemPattern>(page, SelectionItemPattern.Pattern).Current.IsSelected, "Raw Clock page selected through UIA");
        var label = Named(page, "Clock data: HH:mm:ss, updated once per second", ControlType.Text);
        Check(label.Current.BoundingRectangle.Width > 0 && label.Current.BoundingRectangle.Height > 0, "clock description has visible UIA bounds");
        var original = root.Current.BoundingRectangle;
        var transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        transform.Resize(original.Width + 100, original.Height + 80);
        Wait(() => SnapshotBounds(RawClockSnapshot()).Width > bounds.Width, "clock follows resized bounds");
        transform.Resize(original.Width, original.Height);
        Page(root, "Control");
        Page(Page(root, "Misc"), "Raw Clock");
        string initial = (string)RawClockSnapshot()["elementData"];
        Wait(() => (string)RawClockSnapshot()["elementData"] != initial, "clock continues after switching away and back");

    }
    static Dictionary<string, object> TouchingSnapshot()
    {
        Dictionary<string, object> result = null;
        Wait(() =>
        {
            foreach (var node in SnapshotNodes(ControlsSnapshot()))
            {
                object id;
                if (node.TryGetValue("elementId", out id) && (string)id == "FullControlTest.Touching") { result = node; return true; }
            }
            return false;
        }, "Touching raw element becomes visible", 120000);
        return result;
    }
    static double[] TouchingData()
    {
        return Array.ConvertAll(((string)TouchingSnapshot()["elementData"]).Split(','), value => double.Parse(value, System.Globalization.CultureInfo.InvariantCulture));
    }
    static void TouchingState(int count, double[] transform, string description)
    {
        double[] values = null;
        try
        {
            Wait(() =>
            {
                values = TouchingData();
                if (values.Length != 5 + count * 3 || values[4] != count) return false;
                // Integral translation carries fractional residuals between updates.
                for (int i = 0; i < 4; i++) if (Math.Abs(values[i] - transform[i]) > (i < 2 ? 1.0 : 0.001)) return false;
                return true;
            }, description);
        }
        catch (Exception error)
        {
            throw new Exception(description + ": expected " + string.Join(",", transform) + "; actual "
                + (values == null ? "unavailable" : string.Join(",", values)), error);
        }
    }
    static void PointInput(string operation, int token, System.Windows.Rect bounds, int x, int y)
    {
        Input("!Point" + operation + ":" + token + "," + ((int)bounds.Left + x) + "," + ((int)bounds.Top + y));
    }
    static void PointInput(string operation, int token, AutomationElement element, double relativeX, double relativeY)
    {
        var bounds = element.Current.BoundingRectangle;
        var point = UiaToClientPoint(bounds.Left + bounds.Width * relativeX, bounds.Top + bounds.Height * relativeY);
        Input("!Point" + operation + ":" + token + "," + point.X + "," + point.Y);
    }
    static void OpenTouching()
    {
        Wait(() =>
        {
            bool hasPage = false;
            foreach (var node in SnapshotNodes(ControlsSnapshot()))
            {
                object value;
                if (node.TryGetValue("elementId", out value) && (string)value == "FullControlTest.Touching") return true;
                if (node.TryGetValue("elementText", out value) && (string)value == "Touching") hasPage = true;
            }
            ClickRenderedCaption(hasPage ? "Touching" : "Misc");
            return false;
        }, "Touching page opens through automation", 120000);
    }
    static void TouchingListAutomation()
    {
        Test("TouchingList / list-owned swipes, deferred identity removal and recycled rows");
        var page = Page(Page(root, "Misc"), "TouchingList");
        Check(page.Current.ControlType == ControlType.TabItem, "TouchingList tab role");
        var momentum = Named(page, "Momentum Scrolling", ControlType.CheckBox);
        var momentumToggle = Pattern<TogglePattern>(momentum, TogglePattern.Pattern);
        Check(momentumToggle.Current.ToggleState == ToggleState.Off, "momentum defaults unchecked");
        Check(momentum.Current.BoundingRectangle.Right <= Named(page, "Clear", ControlType.Button).Current.BoundingRectangle.Left,
            "momentum checkbox precedes Clear");
        var list = Find(page, ControlType.List)[0];
        Action settleItems = () =>
        {
            Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000), "item mutation reaches idle");
            list = Find(page, ControlType.List)[0];
        };
        Invoke(page, "Clear"); settleItems(); Wait(() => Items(list).Count == 0, "Clear populated list");
        Invoke(page, "Clear"); settleItems(); Check(Items(list).Count == 0, "Clear empty list");
        Invoke(page, "Add 5 Items"); settleItems(); Wait(() => Items(list).Count == 5, "Add creates exactly five items");
        Invoke(page, "Add 5 Items"); settleItems(); Wait(() => Items(list).Count == 10, "second Add creates five more identities");
        var help = Named(page, "Swipe left fully and release to delete.", ControlType.Text).Current.BoundingRectangle;
        var add = Named(page, "Add 5 Items", ControlType.Button).Current.BoundingRectangle;
        Check(help.Left > add.Right && help.Top < add.Bottom && help.Bottom > add.Top, "one gesture hint beside Add");
        var bodies = new HashSet<string>();
        foreach (var item in Items(list))
        {
            var text = item.Current.Name;
            Check(!text.Contains("Swipe") && !text.Contains("DELETE IT"), "rows contain sample content only");
            Check(bodies.Add(text.Substring(text.IndexOf('\u2014') + 2)), "lorem ipsum bodies differ");
        }
        var scroll = Pattern<ScrollPattern>(list, ScrollPattern.Pattern);
        Check(scroll.Current.VerticallyScrollable && !scroll.Current.HorizontallyScrollable, "wrapped rows scroll only vertically");
        scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
        var name = Items(list)[1].Current.Name;
        var label = RenderedCaptionBounds(name);
        Check(label.Height >= 70 && label.Width > 150, "large usable row content");
        Action<string> closed = description =>
        {
            foreach (var node in SnapshotNodes(ControlsSnapshot()))
            {
                object element;
                Check(!node.TryGetValue("element", out element) || !((string)element).Contains("#B82030"), description);
            }
        };
        closed("closed rows never render a red panel");
        Pattern<SelectionItemPattern>(Items(list)[1], SelectionItemPattern.Pattern).Select();
        Check(Pattern<SelectionItemPattern>(Items(list)[1], SelectionItemPattern.Pattern).Current.IsSelected, "row selection is exposed");
        double maximum = 0;
        foreach (var node in SnapshotNodes(ControlsSnapshot()))
        {
            object text;
            if (!node.TryGetValue("elementText", out text) || (string)text != "DELETE IT") continue;
            var bounds = SnapshotBounds(node);
            if (bounds.Top <= label.Top && bounds.Bottom >= label.Bottom) maximum = bounds.Width + 32;
        }
        Check(maximum > 32, "maximum reveal is measured DELETE IT label plus padding");
        int width = (int)Math.Round(maximum), half = width / 2;
        Action<int, int> begin = (token, delta) =>
        {
            PointInput("Down", token, label, 180, 25);
            PointInput("Update", token, label, 180 + delta, 25);
        };
        Action<double, string> reveal = (distance, description) => Wait(() =>
            Math.Abs(RenderedCaptionBounds(name).Left - (label.Left - distance)) <= 1, description);
        begin(1001, 50); reveal(0, "right-only drag does not reveal");
        PointInput("Up", 1001, label, 230, 25);
        begin(1002, -half); reveal(half, "partial left swipe exposes red region");
        Check(Items(list).Count == 10, "partial reveal does not delete");
        PointInput("Update", 1002, label, 180 - half + 15, 25); reveal(half - 15, "reversal retracts the reveal");
        PointInput("Up", 1002, label, 180 - half + 15, 25); reveal(0, "partial release resets presentation");
        begin(1003, -width - 50); reveal(width, "full reveal clamps at measured maximum");
        Check(Items(list).Count == 10, "full reveal waits for successful completion");
        PointInput("Down", 1004, label, 200, 25); reveal(0, "second contact clears swipe state");
        PointInput("Up", 1004, label, 200, 25);
        PointInput("Update", 1003, label, 180 - width - 60, 25);
        PointInput("Up", 1003, label, 180 - width - 60, 25);
        Check(Items(list).Count == 10, "scrolling never resumes after a second contact");
        begin(1005, -width); reveal(width, "full reveal before cancellation");
        Input("!PointCancel:1005,PlatformCanceled"); reveal(0, "cancellation resets presentation");
        closed("canceled rows hide red completely");
        Check(Items(list).Count == 10, "cancellation retains the identity");
        Test("TouchingList / final Up and deferred middle-row removal");
        PointInput("Down", 1006, label, 180, 25);
        PointInput("Up", 1006, label, 180 - width - 50, 25);
        settleItems();
        Wait(() => Items(list).Count == 9, "final-Up-only full swipe removes exactly one item");
        foreach (var item in Items(list)) Check(item.Current.Name != name, "middle row identity is removed");
        var remaining = Items(list)[1].Current.Name;
        PointInput("Down", 1007, list, 0.5, 0.8);
        PointInput("Update", 1007, list, 0.5, 0.2);
        PointInput("Up", 1007, list, 0.5, 0.2);
        Wait(() => scroll.Current.VerticalScrollPercent > 0, "vertical gesture uses built-in list scrolling");
        scroll.SetScrollPercent(ScrollPattern.NoScroll, 100);
        scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
        Wait(() => Math.Abs(RenderedCaptionBounds(remaining).Left - label.Left) <= 1, "recycled row has no inherited reveal");
        for (int pass = 0; pass < 10; pass++)
        {
            scroll.SetScrollPercent(ScrollPattern.NoScroll, pass % 2 == 0 ? 100 : 0);
            closed("rapidly recycled rows never render red");
        }
        Test("TouchingList / completion after the starting row scrolls outside the window");
        for (int completion = 0; completion < 3; completion++)
        {
            scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
            var first = RenderedCaptionBounds(Items(list)[0].Current.Name);
            int token = 1301 + completion * 2;
            PointInput("Down", token, first, 180, 25);
            PointInput("Update", token, first, 180, -1000);
            Wait(() => scroll.Current.VerticalScrollPercent == 100, "drag outside the window reaches the bottom");
            Check(Items(list)[0].Current.IsOffscreen, "starting row has been virtualized away");
            if (completion == 1) Input("!PointCancel:" + token + ",CaptureLost");
            else
            {
                if (completion == 2)
                {
                    PointInput("Down", token + 1, list, 0.5, 0.5);
                    PointInput("Up", token + 1, list, 0.5, 0.5);
                }
                PointInput("Up", token, first, 180, -1000);
            }
            settleItems();
            Check(Items(list).Count == 9, "offscreen completion preserves every item");
            closed("offscreen completion keeps recycled rows closed");
        }
        Invoke(page, "Clear"); settleItems(); Wait(() => Items(list).Count == 0, "Clear after deletion");
        PointInput("Down", 1008, list, 0.8, 0.5);
        PointInput("Update", 1008, list, 0.2, 0.5);
        PointInput("Up", 1008, list, 0.2, 0.5);
        Invoke(page, "Add 5 Items"); settleItems(); Wait(() => Items(list).Count == 5, "empty-space swipe cannot delete newly added items");

        Test("TouchingList / optional momentum, fresh contact and uncheck interruption");
        for (int i = 0; i < 5; i++) Invoke(page, "Add 5 Items");
        settleItems();
        scroll = Pattern<ScrollPattern>(list, ScrollPattern.Pattern);
        scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
        Pattern<SelectionItemPattern>(Items(list)[0], SelectionItemPattern.Pattern).Select();
        var selection = Pattern<SelectionPattern>(list, SelectionPattern.Pattern);
        var selectedName = selection.Current.GetSelection()[0].Current.Name;
        momentumToggle.Toggle();
        Check(momentumToggle.Current.ToggleState == ToggleState.On, "momentum can be enabled");
        Action<int> flick = token =>
        {
            PointInput("Down", token, list, 0.5, 0.8);
            Thread.Sleep(100);
            PointInput("Update", token, list, 0.5, 0.6);
            Thread.Sleep(100);
            PointInput("Up", token, list, 0.5, 0.4);
            ControlsSnapshot();
        };
        flick(1201);
        var released = scroll.Current.VerticalScrollPercent;
        Wait(() => scroll.Current.VerticalScrollPercent > released, "viewport continues moving after Up");
        PointInput("Down", 1202, list, 0.5, 0.5);
        ControlsSnapshot();
        var interrupted = scroll.Current.VerticalScrollPercent;
        Thread.Sleep(250);
        Check(scroll.Current.VerticalScrollPercent == interrupted, "fresh Down stops continuation");
        Input("!PointCancel:1202,PlatformCanceled");
        flick(1203);
        momentumToggle.Toggle();
        var uncheckedPosition = scroll.Current.VerticalScrollPercent;
        Thread.Sleep(250);
        Check(scroll.Current.VerticalScrollPercent == uncheckedPosition, "unchecking stops continuation");
        Check(selection.Current.GetSelection()[0].Current.Name == selectedName, "momentum preserves selected item identity");
        momentumToggle.Toggle();
        flick(1204);
        Invoke(page, "Clear");
        Invoke(page, "Add 5 Items"); settleItems();
        Thread.Sleep(250);
        Check(Items(list).Count == 5, "Clear and repopulation during playback cannot delete another row");
        closed("momentum does not resurrect an ended row swipe");
        scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
        name = Items(list)[1].Current.Name;
        label = RenderedCaptionBounds(name);
        PointInput("Down", 1205, label, 180, 25);
        Thread.Sleep(100);
        PointInput("Up", 1205, label, 180 - width - 50, 25);
        settleItems();
        Wait(() => Items(list).Count == 4, "full swipe with momentum removes exactly one row at real End");
        Thread.Sleep(500);
        Check(Items(list).Count == 4, "horizontal momentum cannot delete an additional row");
        foreach (var item in Items(list)) Check(item.Current.Name != name, "momentum preserves identity-based removal");
        closed("horizontal momentum cannot reopen a recycled row");
        momentumToggle.Toggle();
    }
    static void TouchTextSelectionAutomation()
    {
        Test("Point input / document press selection and scrolling");
        Func<int[]> counters = () => {
            foreach (var node in SnapshotNodes(ControlsSnapshot())) {
                object value;
                if (!node.TryGetValue("elementText", out value) || !((string)value).StartsWith("Touch: Short=")) continue;
                var parts = ((string)value).Split(' ');
                return new[] { int.Parse(parts[1].Split('=')[1]), int.Parse(parts[2].Split('=')[1]), int.Parse(parts[3].Split('=')[1]) };
            }
            throw new Exception("Document touch counters are missing.");
        };
        for (int pass = 0; pass < 2; pass++)
        {
            var page = Page(Page(root, "Control"), "TextBox");
            page = Page(page, "TextBox");
            AutomationElement editor = null;
            foreach (var candidate in Find(page, ControlType.Document))
            {
                object value;
                if (!candidate.Current.IsPassword && candidate.Current.BoundingRectangle.Height > 80
                    && candidate.TryGetCurrentPattern(TextPattern.Pattern, out value)) { editor = candidate; break; }
            }
            Check(editor != null, "multiline editor exposes text ranges");
            Test("Document touch / palette " + pass + " / install scrollable text");
            var content = new StringBuilder();
            for (int line = 0; line < 80; line++) content.AppendLine("Line " + line + ": Press to select text; drag to scroll while keeping the selected range.");
            Pattern<ValuePattern>(editor, ValuePattern.Pattern).SetValue(content.ToString());
            var scroll = Pattern<ScrollPattern>(editor, ScrollPattern.Pattern);
            Test("Document touch / reset viewport");
            Wait(() => scroll.Current.VerticallyScrollable && scroll.Current.VerticalViewSize < 50, "document fixture has laid out its scrollable text");
            scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
            Wait(() => scroll.Current.VerticalScrollPercent == 0, "document fixture starts at the top");
            var text = Pattern<TextPattern>(editor, TextPattern.Pattern);
            Test("Document touch / press selection and right-click fallback");
            int token = 1101 + pass;
            PointInput("Down", token, editor, 0.05, 0.05);
            PointInput("Up", token, editor, 0.05, 0.05);
            Wait(() => text.GetSelection().Length == 1 && text.GetSelection()[0].GetText(-1).Length == 0, "short press positions a collapsed caret");
            PointInput("Down", token, editor, 0.45, 0.15);
            Thread.Sleep(550);
            Check(text.GetSelection()[0].GetText(-1).Length == 0, "holding a long press does not select before release");
            PointInput("Up", token, editor, 0.45, 0.15);
            Wait(() => text.GetSelection()[0].GetText(-1).Length > 0, "long press extends the caret on release");
            var selected = text.GetSelection()[0].Clone();
            Action<string> unchanged = description => Wait(() => {
                var current = text.GetSelection()[0];
                return current.CompareEndpoints(TextPatternRangeEndpoint.Start, selected, TextPatternRangeEndpoint.Start) == 0
                    && current.CompareEndpoints(TextPatternRangeEndpoint.End, selected, TextPatternRangeEndpoint.End) == 0;
            }, description);
            PointInput("Down", token, editor, 0.45, 0.15);
            PointInput("Up", token, editor, 0.45, 0.15);
            Wait(() => text.GetSelection()[0].GetText(-1).Length == 0, "short press at the active endpoint collapses the range");
            selected.Select();
            ControlsSnapshot();
            var beforeRight = counters();
            PointInput("Down", token, editor, 0.45, 0.15);
            Thread.Sleep(550);
            PointInput("Up", token, editor, 0.45, 0.15);
            unchanged("long press on the selection retains both endpoints for right click");
            Wait(() => { var current = counters(); return current[0] == beforeRight[0] && current[1] == beforeRight[1] + 1 && current[2] == beforeRight[2] + 1; }, "long press delivers one right click and no short-press offer");
            Check(scroll.Current.VerticalViewSize < 50, "document has room for scrolling and momentum");
            double before = scroll.Current.VerticalScrollPercent;
            PointInput("Down", token, editor, 0.6, 0.8);
            PointInput("Update", token, editor, 0.6, 0.3);
            PointInput("Up", token, editor, 0.6, 0.3);
            Wait(() => scroll.Current.VerticalScrollPercent > before, "accepted document drag scrolls the viewport");
            unchanged("document scrolling preserves the selected endpoints");
            var momentumControl = Named(page, "Document Momentum", ControlType.CheckBox);
            Test("Document touch / visible options and momentum");
            var selectableControl = Named(page, "Selection Only", ControlType.CheckBox);
            Check(!momentumControl.Current.IsOffscreen && momentumControl.Current.BoundingRectangle.Height > 0, "document momentum option is visible");
            Check(!selectableControl.Current.IsOffscreen && selectableControl.Current.BoundingRectangle.Height > 0, "document selection mode option is visible");
            var momentum = Pattern<TogglePattern>(momentumControl, TogglePattern.Pattern);
            var selectable = Pattern<TogglePattern>(selectableControl, TogglePattern.Pattern);
            Check(momentum.Current.ToggleState == ToggleState.Off, "document momentum defaults off");
            for (int mode = 0; mode < 2; mode++) {
                if (mode == 1) selectable.Toggle();
                scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
                momentum.Toggle();
                PointInput("Down", token, editor, 0.6, 0.8);
                Thread.Sleep(100);
                PointInput("Update", token, editor, 0.6, 0.6);
                Thread.Sleep(100);
                PointInput("Up", token, editor, 0.6, 0.4);
                var released = scroll.Current.VerticalScrollPercent;
                Check(released < 100, "document flick leaves room for momentum");
                Wait(() => scroll.Current.VerticalScrollPercent > released, "document momentum continues after Up in each edit mode");
                PointInput("Down", token, editor, 0.6, 0.5);
                Thread.Sleep(100);
                var interrupted = scroll.Current.VerticalScrollPercent;
                Thread.Sleep(250);
                Check(scroll.Current.VerticalScrollPercent == interrupted, "fresh document contact interrupts momentum");
                Input("!PointCancel:" + token + ",PlatformCanceled");
                unchanged("document momentum retains the exact selected endpoints");
                momentum.Toggle();
            }
            selectable.Toggle();
            var singlelineCandidates = Find(page, ControlType.Document);
            Test("Document touch / single-line selection and right-click fallback");
            singlelineCandidates.AddRange(Find(page, ControlType.Edit));
            AutomationElement singleline = null;
            foreach (var candidate in singlelineCandidates) {
                object value;
                if (!candidate.Current.IsPassword && candidate.Current.BoundingRectangle.Height < 80
                    && candidate.TryGetCurrentPattern(TextPattern.Pattern, out value)) { singleline = candidate; break; }
            }
            Check(singleline != null, "single-line label family exposes a text range");
            Pattern<ValuePattern>(singleline, ValuePattern.Pattern).SetValue("0123456789 abcdefghijklmnopqrstuvwxyz");
            var lineText = Pattern<TextPattern>(singleline, TextPattern.Pattern);
            PointInput("Down", token, singleline, 0.02, 0.5); PointInput("Up", token, singleline, 0.02, 0.5);
            Wait(() => lineText.GetSelection()[0].GetText(-1).Length == 0, "single-line short press positions the caret");
            PointInput("Down", token, singleline, 0.3, 0.5); Thread.Sleep(550); PointInput("Up", token, singleline, 0.3, 0.5);
            Wait(() => lineText.GetSelection()[0].GetText(-1).Length > 0, "single-line long press extends selection");
            var lineRange = lineText.GetSelection()[0].Clone();
            beforeRight = counters();
            PointInput("Down", token, singleline, 0.3, 0.5); Thread.Sleep(550); PointInput("Up", token, singleline, 0.3, 0.5);
            Wait(() => { var current = counters(); return current[0] == beforeRight[0] && current[1] == beforeRight[1] + 1 && current[2] == beforeRight[2] + 1; }, "single-line long press produces exactly one right fallback");
            Check(lineText.GetSelection()[0].Compare(lineRange), "single-line right fallback preserves both endpoints");
            if (pass == 0)
            {
                var hwnd = new IntPtr(root.Current.NativeWindowHandle);
                ClickRenderedCaption("Window Manager");
                ClickRenderedCaption("Aurora");
                Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000), "palette replacement reaches idle");
                ControlsSnapshot();
                root = AutomationElement.FromHandle(hwnd);
            }
        }
    }
    static void TouchingAutomation()
    {
        Test("Touching / one and two contact transforms, membership, cancellation and source reuse");
        OpenTouching();
        var bounds = SnapshotBounds(TouchingSnapshot());
        Check(bounds.Width >= 320 && bounds.Height >= 240, "Touching raw region has useful bounds");
        int dpi = (int)GetDpiForWindow(new IntPtr(root.Current.NativeWindowHandle));
        Check(dpi > 0, "point fixture reads native DPI");
        // Automation converts integral logical coordinates to native pixels and back.
        Func<int, int, System.Windows.Point> local = (x, y) => new System.Windows.Point(
            ((int)bounds.Left + x) * dpi / 96 * 96 / dpi - bounds.Left,
            ((int)bounds.Top + y) * dpi / 96 * 96 / dpi - bounds.Top);
        var first = local(20, 20);
        var translated = local(50, 40);
        var second = local(150, 40);
        var expanded = local(250, 40);
        var turned = local(50, 240);
        var initial = TouchingData();
        foreach (var invalid in new[] { "!PointDown:abc,1,2", "!PointDown:-1,1,2", "!PointCancel:1,Succeeded", "!PointUpdate:1,1", "!PointDown:1,1,2,pen" })
        {
            using (var client = new System.Net.WebClient())
            {
                client.Headers[System.Net.HttpRequestHeader.ContentType] = "application/json; charset=utf8";
                var response = Encoding.UTF8.GetString(client.UploadData(inputEndpoint, Encoding.UTF8.GetBytes(invalid)));
                Check(response.StartsWith("Syntax Error!", StringComparison.Ordinal), "malformed point command is rejected synchronously: " + invalid);
            }
        }
        Input("!MouseMove:0,0");
        PointInput("Down", 7, bounds, 20, 20);
        TouchingState(1, initial, "Down outside polygon preserves transform");
        PointInput("Update", 7, bounds, 50, 40);
        var moved = new double[] { initial[0] + translated.X - first.X, initial[1] + translated.Y - first.Y, initial[2], initial[3] };
        TouchingState(1, moved, "one contact translates");
        PointInput("Down", 8, bounds, 150, 40);
        TouchingState(2, moved, "second contact rebases without jumping");
        PointInput("Update", 8, bounds, 250, 40);
        double scale = (expanded.X - translated.X) / (second.X - translated.X);
        var scaled = new double[] {
            (translated.X + expanded.X) / 2 + scale * (moved[0] - (translated.X + second.X) / 2),
            translated.Y + scale * (moved[1] - translated.Y), scale * moved[2], scale * moved[3] };
        TouchingState(2, scaled, "two contact scaling and translation");
        PointInput("Update", 8, bounds, 50, 240);
        double turnScale = (turned.Y - translated.Y) / (expanded.X - translated.X);
        var rotated = new double[] {
            translated.X - turnScale * (scaled[1] - translated.Y),
            (translated.Y + turned.Y) / 2 + turnScale * (scaled[0] - (translated.X + expanded.X) / 2),
            -turnScale * scaled[3], turnScale * scaled[2] };
        TouchingState(2, rotated, "two contact rotation");
        PointInput("Down", 9, bounds, 300, 150);
        PointInput("Update", 7, bounds, -30, -30);
        TouchingState(3, rotated, "three contacts freeze while moving outside raw bounds");
        var markers = TouchingData();
        var outside = local(-30, -30);
        Check(markers[6] == outside.X && markers[7] == outside.Y, "marker follows captured contact outside raw bounds");
        PointInput("Up", 9, bounds, 300, 150);
        TouchingState(2, rotated, "three to two rebases");
        PointInput("Up", 7, bounds, -30, -30);
        TouchingState(1, rotated, "original contact can lift without ending sequence");
        PointInput("Update", 8, bounds, 60, 255);
        var remaining = local(60, 255);
        var final = new double[] { rotated[0] + remaining.X - turned.X, rotated[1] + remaining.Y - turned.Y, rotated[2], rotated[3] };
        TouchingState(1, final, "remaining contact translates from new baseline");
        PointInput("Up", 8, bounds, 60, 255);
        TouchingState(0, final, "successful End clears markers");

        PointInput("Down", 42, bounds, 20, 20);
        PointInput("Down", 43, bounds, 20, 20);
        TouchingState(2, final, "coincident contacts preserve transform");
        PointInput("Update", 43, bounds, 40, 20);
        TouchingState(2, final, "first separated sample establishes baseline");
        PointInput("Up", 42, bounds, 20, 20);
        PointInput("Down", 42, bounds, 80, 20);
        TouchingState(2, final, "reused source token does not jump");
        markers = TouchingData();
        Check(markers[5] == 1 && markers[8] == 2, "public point indices retain the hole and never reuse a slot");
        Input("!PointCancel:42,CaptureLost");
        TouchingState(0, final, "canceled End clears showcase markers");
        PointInput("Down", 44, bounds, 100, 20);
        PointInput("Update", 43, bounds, 200, 20);
        Input("!PointCancel:42,CaptureLost");
        TouchingState(0, final, "canceled sequence drains without redispatch or requery");
        PointInput("Up", 43, bounds, 200, 20);
        PointInput("Up", 44, bounds, 100, 20);
        PointInput("Down", 45, bounds, 20, 20);
        TouchingState(1, final, "fresh sequence accepts after drain");
        Check(TouchingData()[5] == 0, "fresh sequence starts at public index zero");
        PointInput("Enter", 45, bounds, 20, 20);
        PointInput("Leave", 45, bounds, -20, -20);
        PointInput("Up", 45, bounds, -20, -20);
        var left = local(-20, -20);
        var boundary = new double[] { final[0] + left.X - first.X, final[1] + left.Y - first.Y, final[2], final[3] };
        TouchingState(0, boundary, "boundary callbacks preserve the captured route");
        Input("!LeftDown:" + ((int)bounds.Left + 20) + "," + ((int)bounds.Top + 20));
        Input("!MouseMove:" + ((int)bounds.Left + 80) + "," + ((int)bounds.Top + 80));
        Input("!LeftUp:" + ((int)bounds.Left + 80) + "," + ((int)bounds.Top + 80));
        TouchingState(0, boundary, "ordinary mouse dragging does not manipulate the polygon");

        Test("Touching / rejected sequence falls back only for the original first contact");
        var clockTab = RenderedCaptionBounds("Raw Clock");
        var touchingTab = RenderedCaptionBounds("Touching");
        int clockX = (int)(clockTab.Width / 2), clockY = (int)(clockTab.Height / 2);
        int touchingX = (int)(touchingTab.Width / 2), touchingY = (int)(touchingTab.Height / 2);
        PointInput("Down", 70, clockTab, clockX, clockY);
        PointInput("Down", 71, touchingTab, touchingX, touchingY);
        PointInput("Up", 70, clockTab, clockX, clockY);
        RawClockSnapshot();
        PointInput("Down", 72, touchingTab, touchingX, touchingY);
        PointInput("Up", 71, touchingTab, touchingX, touchingY);
        PointInput("Up", 72, touchingTab, touchingX, touchingY);
        Check(RawClockSnapshot() != null, "later contacts never replace the original fallback contact");
        PointInput("Down", 73, touchingTab, touchingX, touchingY);
        PointInput("Up", 73, touchingTab, touchingX, touchingY);
        TouchingState(0, boundary, "next rejected sequence can click normally without a stuck mouse button");
        PointInput("Down", 74, bounds, 20, 20);
        TouchingState(1, boundary, "accepted input resumes after rejected sequences");
        PointInput("Up", 74, bounds, 20, 20);
        TouchingState(0, boundary, "final accepted sequence completes");

        var touchingPage = Page(Page(root, "Misc"), "Touching");
        var rawClockTab = Named(Page(root, "Misc"), "Raw Clock", ControlType.TabItem);
        PointInput("Down", 75, bounds, 20, 20);
        PointInput("Update", 75, rawClockTab, 0.50, 0.50);
        Wait(() =>
        {
            var values = TouchingData();
            return values.Length >= 5 && values[4] == 1
                && (Math.Abs(values[0] - boundary[0]) > 0.001 || Math.Abs(values[1] - boundary[1]) > 0.001);
        }, "accepted Touching update is delivered after the point leaves its surface");
        var movedOutside = TouchingData();
        PointInput("Up", 75, rawClockTab, 0.50, 0.50);
        TouchingState(0, new double[] { movedOutside[0], movedOutside[1], movedOutside[2], movedOutside[3] },
            "accepted Touching target remains stable when the point leaves its surface");
        Wait(() => Pattern<SelectionItemPattern>(touchingPage, SelectionItemPattern.Pattern).Current.IsSelected,
            "accepted Touching sequence keeps its page selected without deferred fallback click");

        Test("Point input / built-in ListView scrolling preserves selection and clamps");
        var listPage = Page(Page(root, "List"), "ListView");
        var list = Find(listPage, ControlType.DataGrid)[0];
        var scroll = Pattern<ScrollPattern>(list, ScrollPattern.Pattern);
        Check(scroll.Current.VerticallyScrollable, "ListView is vertically scrollable");
        scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
        Wait(() => scroll.Current.VerticalScrollPercent == 0, "ListView returns to the top");
        var rows = Find(list, ControlType.DataItem, TreeScope.Children);
        Check(rows.Count > 2, "ListView exposes multiple visible rows");
        var selectedRow = rows[0];
        Pattern<SelectionItemPattern>(selectedRow, SelectionItemPattern.Pattern).Select();
        var selection = Pattern<SelectionPattern>(list, SelectionPattern.Pattern);
        var selectedName = selectedRow.Current.Name;
        Action<string> checkSelection = description => Wait(() =>
        {
            try
            {
                var selected = selection.Current.GetSelection();
                return selected.Length == 1 && selected[0].Current.Name == selectedName;
            }
            catch (ElementNotAvailableException) { return false; }
        }, description);
        var beforeScroll = scroll.Current.VerticalScrollPercent;
        PointInput("Down", 901, list, 0.50, 0.50);
        PointInput("Update", 901, list, 0.50, 0.12);
        Wait(() => scroll.Current.VerticalScrollPercent > beforeScroll, "point drag scrolls ListView vertically");
        PointInput("Up", 901, list, 0.50, 0.12);
        checkSelection("point drag preserves ListView selection");
        var afterForward = scroll.Current.VerticalScrollPercent;
        PointInput("Down", 902, list, 0.50, 0.20);
        PointInput("Update", 902, list, 0.50, 0.90);
        Wait(() => scroll.Current.VerticalScrollPercent < afterForward, "reversed point drag scrolls ListView upward");
        PointInput("Up", 902, list, 0.50, 0.90);
        PointInput("Down", 903, list, 0.50, 0.50);
        PointInput("Update", 903, list, 0.50, -2.00);
        PointInput("Up", 903, list, 0.50, -2.00);
        Wait(() => scroll.Current.VerticalScrollPercent == 100, "point drag clamps ListView at the bottom");
        checkSelection("clamped point drag preserves ListView selection");
        TouchingListAutomation();
        TouchTextSelectionAutomation();
        Page(Page(root, "Misc"), "Touching");
    }
    static void Touching(bool runAutomation)
    {
        if (runAutomation) TouchingAutomation();
        Test("Touching / UIA tab name, selection, visible bounds and resize");
        var page = Page(Page(root, "Misc"), "Touching");
        Check(page.Current.Name == "Touching" && page.Current.ControlType == ControlType.TabItem, "Touching tab role and name");
        Check(Pattern<SelectionItemPattern>(page, SelectionItemPattern.Pattern).Current.IsSelected, "Touching selected through UIA");
        Check(page.Current.BoundingRectangle.Width > 0 && page.Current.BoundingRectangle.Height > 0, "Touching tab has visible UIA bounds");
        var label = Named(page, "One finger: move. Two fingers: move, rotate and zoom. More: pause.", ControlType.Text);
        Check(label.Current.BoundingRectangle.Width > 0 && label.Current.BoundingRectangle.Height > 0, "Touching instructions have visible UIA bounds");
        var state = TouchingData();
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        // Replace the template before manual resizing warms the GDI backing bitmap's capacity.
        ClickRenderedCaption("Window Manager");
        ClickRenderedCaption("Aurora");
        OpenTouching();
        TouchingState(0, state, "palette template replacement retains Touching state");
        root = AutomationElement.FromHandle(hwnd);
        var bounds = SnapshotBounds(TouchingSnapshot());
        var original = root.Current.BoundingRectangle;
        var transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        transform.Resize(original.Width + 100, original.Height + 80);
        Wait(() => SnapshotBounds(TouchingSnapshot()).Width > bounds.Width, "Touching follows resized bounds");
        transform.Resize(original.Width, original.Height);
        state = TouchingData();
        Page(Page(root, "Misc"), "Raw Clock");
        Page(Page(root, "Misc"), "Touching");
        TouchingState(0, state, "switching tabs retains rectangle transform");
    }
    static void ActivatePersistenceMenuItem(AutomationElement item, bool submenu = false)
    {
        if (isHosted) { Click(item); return; }
        string name = item.Current.Name;
        foreach (var window in SnapshotNodes(ControlsSnapshot()))
        {
            object windowId, composition, role;
            if (!window.TryGetValue("windowId", out windowId) || !window.TryGetValue("composition", out composition)) continue;
            var content = composition as Dictionary<string, object>;
            if (content == null || !content.TryGetValue("control", out role) || (string)role != "Menu") continue;
            foreach (var node in SnapshotNodes(composition))
            {
                object caption;
                if (!node.TryGetValue("elementText", out caption) || (string)caption != name) continue;
                var bounds = SnapshotBounds(node);
                Check(bounds.Width > 0 && bounds.Height > 0, "native persistence command has rendered bounds: " + name);
                // Native cascading headers open on hover; a subsequent click closes the menu chain.
                string action = submenu ? "MouseMove" : "LeftClick";
                Input("!" + action + ":" + (int)(bounds.Left + bounds.Width / 2) + "," + (int)(bounds.Top + bounds.Height / 2), inputEndpoint + "/" + (string)windowId);
                return;
            }
        }
        throw new Exception("Rendered native persistence command is missing: " + name);
    }
    static AutomationElement PersistenceCommand(AutomationElement page, bool ribbon, string command, bool automation = false)
    {
        Action openMenu = () => {
            if (ribbon)
            {
                if (automation) Click(Named(page, "  HOME  ", ControlType.Button)); else Invoke(page, "  HOME  ");
            }
            else
            {
                var file = Named(page, "File", ControlType.MenuItem);
                if (automation) Click(file); else Pattern<ExpandCollapsePattern>(file, ExpandCollapsePattern.Pattern).Expand();
                if (command != "Load as Private Format")
                {
                    AutomationElement save = null;
                    Wait(() => {
                        var menu = Popup(ControlType.Menu);
                        save = menu == null ? null : FindNamed(menu, "Save as", ControlType.MenuItem);
                        return save != null && !save.Current.IsOffscreen;
                    }, "File popup exposes Save as");
                    if (automation) ActivatePersistenceMenuItem(save, true); else Pattern<ExpandCollapsePattern>(save, ExpandCollapsePattern.Pattern).Expand();
                }
            }
        };
        openMenu();
        AutomationElement item = null;
        // Search popup windows first. Walking the rebuilt showcase's complete
        // provider tree can outlast the timeout before reaching its popup.
        var retryMenu = Stopwatch.StartNew();
        Wait(() => {
            ControlsSnapshot();
            var windows = OwnedWindows();
            windows.Sort((a, b) => Automation.Compare(a, root).CompareTo(Automation.Compare(b, root)));
            foreach (var window in windows)
            {
                foreach (var candidate in Find(window, ControlType.MenuItem))
                {
                    if (candidate.Current.Name == command && !candidate.Current.IsOffscreen)
                    {
                        item = candidate;
                        return true;
                    }
                }
            }
            if (retryMenu.ElapsedMilliseconds >= 500)
            {
                openMenu();
                retryMenu.Restart();
            }
            return false;
        }, "persistence menu command: " + command, 30000);
        if (automation) ActivatePersistenceMenuItem(item); else Pattern<InvokePattern>(item, InvokePattern.Pattern).Invoke();
        AutomationElement dialog = null;
        string title = command == "Load as Private Format" ? "Load a GacUI Document" : "Save a GacUI Document";
        Wait(() => {
            foreach (var window in OwnedWindows())
            {
                if (window.Current.Name == title) { dialog = window; return true; }
                dialog = FindNamed(window, title, ControlType.Window);
                if (dialog != null && !dialog.Current.IsOffscreen) return true;
            }
            return false;
        }, "persistence dialog: " + title);
        return dialog;
    }
    static AutomationElement NativeFileDialogControl(AutomationElement dialog, string controlClass, int controlId)
    {
        if (dialog.Current.FrameworkId == "GacUI") return null;
        var hwnd = new IntPtr(dialog.Current.NativeWindowHandle);
        uint process; GetWindowThreadProcessId(hwnd, out process);
        var name = new StringBuilder(256); GetClassName(hwnd, name, name.Capacity);
        Check(process == ownedProcess && name.ToString() == "#32770", "owned native file dialog");
        IntPtr found = IntPtr.Zero;
        EnumChildWindows(hwnd, (child, parameter) => {
            GetClassName(child, name, name.Capacity);
            if (GetDlgCtrlID(child) == controlId && name.ToString() == controlClass && IsWindowVisible(child)) found = child;
            return true;
        }, IntPtr.Zero);
        if (found == IntPtr.Zero) return null;
        Console.WriteLine("NOTE native dialog footer omitted from UIA descendants; querying its owned " + controlClass + " HWND directly");
        return AutomationElement.FromHandle(found);
    }
    static void PersistenceFile(AutomationElement page, bool ribbon, string command, string path, bool cancel, bool automation = false)
    {
        var dialog = PersistenceCommand(page, ribbon, command, automation);
        Console.WriteLine("Persistence command: " + command + (cancel ? " / cancel" : " / " + path));
        if (!cancel)
        {
            var edit = FindNamed(dialog, "File name:", ControlType.Edit)
                ?? FindNamed(dialog, "File Name:", ControlType.Edit)
                ?? NativeFileDialogControl(dialog, "Edit", 1148);
            if (edit == null)
            {
                var edits = Find(dialog, ControlType.Edit);
                Check(edits.Count > 0, "filename edit exposed");
                edit = edits[edits.Count - 1];
            }
            if (automation && edit.Current.FrameworkId == "GacUI") EditorSetValue(edit, path, false);
            else Pattern<ValuePattern>(edit, ValuePattern.Pattern).SetValue(path);
        }
        var button = cancel ? "Cancel" : command == "Load as Private Format" ? "Open" : "Save";
        AutomationElement confirm = null;
        Wait(() =>
        {
            confirm = FindNamed(dialog, button, ControlType.Button) ?? FindNamed(dialog, "OK", ControlType.Button);
            if (confirm != null) return confirm.Current.IsEnabled;
            foreach (var candidate in Find(dialog, ControlType.Button))
            {
                if (candidate.Current.AutomationId == (cancel ? "2" : "1")) confirm = candidate;
            }
            if (confirm == null) confirm = NativeFileDialogControl(dialog, "Button", cancel ? 2 : 1);
            if (confirm == null)
            {
                // The fake dialog localizes its action buttons independently of
                // the OS. FileDialog.xml puts OK/Cancel after folder/delete actions.
                var actions = Find(dialog, ControlType.Button).FindAll(candidate => candidate.Current.FrameworkId == "GacUI" && candidate.Current.Name.Length > 0);
                if (actions.Count >= 2) confirm = actions[actions.Count - (cancel ? 1 : 2)];
            }
            return confirm != null && confirm.Current.IsEnabled;
        }, "file dialog confirmation button");
        bool nativeClick = false;
        object invoke;
        if (!confirm.TryGetCurrentPattern(InvokePattern.Pattern, out invoke))
        {
            Check(confirm.Current.FrameworkId != "GacUI", "GacUI file confirmation exposes Invoke");
            nativeClick = true;
        }
        try
        {
            if (automation && confirm.Current.FrameworkId == "GacUI") Click(confirm);
            else if (!nativeClick) ((InvokePattern)invoke).Invoke();
        }
        catch (InvalidOperationException error)
        {
            if (confirm.Current.FrameworkId == "GacUI" || error.StackTrace == null || !error.StackTrace.Contains("SendInput")) throw;
            nativeClick = true;
        }
        catch (System.ComponentModel.Win32Exception error)
        {
            if (confirm.Current.FrameworkId == "GacUI" || error.NativeErrorCode != 1409 || error.StackTrace == null || !error.StackTrace.Contains("AutomationProxies.Misc.SetFocus")) throw;
            Console.WriteLine("NOTE native file-dialog UIA focus failed with ERROR_HOTKEY_ALREADY_REGISTERED in this session");
            nativeClick = true;
        }
        if (nativeClick)
        {
            var handle = new IntPtr(confirm.Current.NativeWindowHandle);
            uint process; GetWindowThreadProcessId(handle, out process);
            var className = new StringBuilder(256); GetClassName(handle, className, className.Capacity);
            Check(process == ownedProcess && className.ToString() == "Button", "owned native file dialog button");
            Console.WriteLine("NOTE native file-dialog Invoke unavailable; using owned button BM_CLICK");
            Check(PostMessage(handle, 0x00F5, IntPtr.Zero, IntPtr.Zero), "native file confirmation accepted");
        }
        Wait(() => { try { return dialog.Current.IsOffscreen; } catch (ElementNotAvailableException) { return true; } }, "file dialog completed");
        // The native dialog disappears before its caller resumes and dispatches
        // the document event. A UI-thread snapshot waits for that handler too.
        ControlsSnapshot();
    }
    static void PersistencePalette(IntPtr hwnd, string name)
    {
        var manager = Page(root, "Window Manager");
        var palette = Named(manager, name, ControlType.RadioButton);
        var selection = Pattern<SelectionItemPattern>(palette, SelectionItemPattern.Pattern);
        if (selection.Current.IsSelected) return;
        selection.Select();
        Wait(() => PaletteAttachmentRetired(palette), "persistence palette replaced", 30000);
        // Retired providers can still have queued event-cache work. Allow
        // it to drain before requesting the rebuilt tree and layout.
        Thread.Sleep(2000);
        Wait(() => {
            root = AutomationElement.FromHandle(hwnd);
            return FindNamed(root, "Control", ControlType.TabItem) != null;
        }, "palette rebuilt Control tab", 30000);
        var bounds = root.Current.BoundingRectangle;
        var transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        transform.Resize(bounds.Width + 1, bounds.Height + 1);
        ControlsSnapshot();
        transform.Resize(bounds.Width, bounds.Height);
        ControlsSnapshot();
    }
    static Dictionary<string, object> StylePopupSnapshot()
    {
        foreach (var node in SnapshotNodes(ControlsSnapshot()))
        {
            object composition;
            if (node.TryGetValue("composition", out composition)
                && SnapshotHas(composition, "elementText", "Headers")
                && SnapshotHas(composition, "elementText", "Header 1")) return node;
        }
        return null;
    }
    static void ApplyPopupStyle(AutomationElement gallery, int index, string name, bool collapsed)
    {
        if (collapsed) ClickRenderedCaption("Style");
        else Pattern<ExpandCollapsePattern>(gallery, ExpandCollapsePattern.Pattern).Expand();
        Dictionary<string, object> snapshot = null;
        Wait(() => { snapshot = StylePopupSnapshot(); return snapshot != null; }, "actual style popup opens");
        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000), "popup layout reaches idle");
        AutomationElement popup = null;
        Wait(() => { popup = Popup(ControlType.Menu); return popup != null; }, "UIA exposes the style popup");
        var items = Find(popup, ControlType.CheckBox);
        Check(items.Count == 8, "popup has eight separate style controls in two groups");
        Pattern<ScrollItemPattern>(items[index], ScrollItemPattern.Pattern).ScrollIntoView();
        // These selectable backgrounds currently expose Toggle, which changes
        // only their highlight. Drive their real click path through automation IO.
        var bounds = System.Windows.Rect.Empty;
        Wait(() => {
            snapshot = StylePopupSnapshot();
            if (snapshot == null) return false;
            foreach (var node in SnapshotNodes(snapshot["composition"]))
            {
                object caption;
                if (!node.TryGetValue("elementText", out caption) || (string)caption != name) continue;
                bounds = SnapshotBounds(node);
                return bounds.Width > 0 && bounds.Height > 0 && SnapshotBounds(snapshot).Contains(bounds);
            }
            return false;
        }, "popup item scrolled into view: " + name);
        string endpoint = inputEndpoint;
        object windowId;
        if (snapshot.TryGetValue("windowId", out windowId)) endpoint += "/" + (string)windowId;
        Input("!LeftClick:" + (int)(bounds.Left + bounds.Width / 2) + "," + (int)(bounds.Top + bounds.Height / 2), endpoint);
        Wait(() => StylePopupSnapshot() == null, "popup application closes the menu");
        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000), "popup closure reaches idle");
    }
    static List<Dictionary<string, object>> DocumentContextMenus()
    {
        var menus = new List<Dictionary<string, object>>();
        foreach (var node in SnapshotNodes(ControlsSnapshot()))
        {
            object composition, role;
            if (!node.TryGetValue("composition", out composition)) continue;
            var content = composition as Dictionary<string, object>;
            if (content != null && content.TryGetValue("control", out role) && (string)role == "Menu"
                && SnapshotHas(content, "elementText", "Select All") && SnapshotHas(content, "elementText", "Object")) menus.Add(node);
        }
        return menus;
    }
    static void ContextMenuCommand(string name, bool uia)
    {
        if (uia)
        {
            var popup = Popup(ControlType.Menu);
            Check(popup != null, "UIA context menu is exposed");
            Pattern<InvokePattern>(Named(popup, name, ControlType.MenuItem), InvokePattern.Pattern).Invoke();
        }
        else
        {
            var item = Named(Popup(ControlType.Menu), name, ControlType.MenuItem);
            Check(item.Current.IsEnabled, "context command is enabled: " + name);
            var previous = System.Windows.Rect.Empty;
            int stable = 0;
            Wait(() => {
                var rectangle = item.Current.BoundingRectangle;
                bool same = !rectangle.IsEmpty && rectangle.Width > 0 && rectangle.Height > 0 && rectangle == previous;
                stable = same ? stable + 1 : 0;
                previous = rectangle;
                return stable >= 3;
            }, "context menu command geometry settles: " + name);
            var menus = DocumentContextMenus();
            Check(menus.Count == 1, "one document context menu before command");
            var menu = menus[0];
            foreach (var node in SnapshotNodes(menu["composition"]))
            {
                object caption;
                if (!node.TryGetValue("elementText", out caption) || (string)caption != name) continue;
                var bounds = SnapshotBounds(node);
                string endpoint = inputEndpoint;
                object windowId;
                if (menu.TryGetValue("windowId", out windowId)) endpoint += "/" + (string)windowId;
                string action = !isHosted && name == "Object" ? "MouseMove" : "LeftClick";
                Input("!" + action + ":" + (int)(bounds.Left + bounds.Width / 2) + "," + (int)(bounds.Top + bounds.Height / 2), endpoint);
                return;
            }
            throw new Exception("Rendered context command is missing: " + name);
        }
    }
    static void DismissDocumentContextMenu()
    {
        var menus = DocumentContextMenus();
        Check(menus.Count == 1, "one document context menu before dismissal");
        string endpoint = inputEndpoint;
        object windowId;
        if (menus[0].TryGetValue("windowId", out windowId)) endpoint += "/" + (string)windowId;
        Input("!KeyPress:Esc", endpoint);
        if (DocumentContextMenus().Count != 0)
        {
            Console.WriteLine("LIMITATION Synthetic Escape did not dismiss the nonactivating menu; using the main-window corner outside the document and popup.");
            Input("!LeftClick:1,1");
        }
        Wait(() => DocumentContextMenus().Count == 0, "context menu dismisses");
    }
    static void DocumentContextMenu(IntPtr hwnd)
    {
        Test("Document context menus / automation IO followed by UIA Invoke");
        string fixture = System.IO.Path.GetFullPath(System.IO.Path.Combine(PersistenceOutput, "..", "..", "TestDocument.bin"));
        var expected = new[] { "Undo", "Redo", "Copy", "Cut", "Paste", "Delete", "Select All", "Object", "Set Font ...", "Text Color ...", "Background Color ...", "Edit Hyperlink ...", "Remove Hyperlink" };
        for (int pass = 0; pass < 2; pass++)
        {
            PersistencePalette(hwnd, pass == 0 ? "Default" : "Aurora");
            for (int editor = 0; editor < 2; editor++)
            {
                bool ribbon = editor == 0;
                bool uia = pass == 1;
                var page = Page(Page(root, "Control"), ribbon ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
                PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
                var document = Find(page, ControlType.Document)[0];
                var text = Pattern<TextPattern>(document, TextPattern.Pattern);
                Wait(() => text.DocumentRange.GetText(-1).Contains("GacUI Remote Document"), "context fixture loaded");
                document.SetFocus();
                if (ribbon)
                {
                    Press("Ctrl+Home");
                    Press("Ctrl+Shift+Right");
                }
                else
                {
                    EditorSelect(document, true, "GacUI Remote");
                }
                Wait(() => text.GetSelection()[0].GetText(-1).Length > 0, "context test has a selected range");
                Action open = () => {
                    var selection = text.GetSelection()[0].Clone();
                    if (ribbon)
                    {
                        var bounds = document.Current.BoundingRectangle;
                        double x = bounds.Left + bounds.Width * 0.4, y = bounds.Top + bounds.Height * 0.15;
                        Pointer("RightDown", x, y);
                        Check(DocumentContextMenus().Count == 0, "right down does not open the menu");
                        Pointer("RightUp", x, y);
                    }
                    else
                    {
                        PointInput("Down", 1302, document, 0.25, 0.02);
                        Thread.Sleep(550);
                        Check(DocumentContextMenus().Count == 0, "long hold does not open the menu before release");
                        PointInput("Up", 1302, document, 0.25, 0.02);
                    }
                    Wait(() => DocumentContextMenus().Count == 1, "one context popup opens on release");
                    Check(text.GetSelection()[0].Compare(selection), "context opening preserves both caret endpoints");
                };
                open();
                var popup = Popup(ControlType.Menu);
                var items = Find(popup, ControlType.MenuItem).FindAll(item => !item.Current.IsOffscreen);
                Check(items.Count == expected.Length, "context menu has exactly thirteen top-level items");
                for (int index = 0; index < expected.Length; index++) Check(items[index].Current.Name == expected[index], "context item order " + index + ": " + expected[index]);
                Check(Find(popup, ControlType.Separator).Count == 5, "context menu has five group separators");
                var shortcutKeys = new Dictionary<string, string> {
                    { "Undo", "Ctrl+Z" }, { "Redo", "Ctrl+Y" }, { "Copy", "Ctrl+C" },
                    { "Cut", "Ctrl+X" }, { "Paste", "Ctrl+V" }, { "Select All", "Ctrl+A" }
                };
                foreach (var shortcut in shortcutKeys)
                {
                    string name = shortcut.Key;
                    if (ribbon) Check(Named(popup, name, ControlType.MenuItem).Current.AcceleratorKey == shortcut.Value, "context shortcut: " + name);
                    var equivalent = FindNamed(page, name, ControlType.Button);
                    if (equivalent != null)
                    {
                        var item = Named(popup, name, ControlType.MenuItem);
                        Check(item.Current.IsEnabled == equivalent.Current.IsEnabled, "shared enabled state: " + name);
                        Check(item.Current.AcceleratorKey == equivalent.Current.AcceleratorKey, "shared shortcut: " + name);
                    }
                }
                var objectItem = Named(popup, "Object", ControlType.MenuItem);
                Absent(objectItem, InvokePattern.Pattern);
                if (uia) Pattern<ExpandCollapsePattern>(objectItem, ExpandCollapsePattern.Pattern).Expand();
                else ContextMenuCommand("Object", false);
                Wait(() => Pattern<ExpandCollapsePattern>(objectItem, ExpandCollapsePattern.Pattern).Current.ExpandCollapseState == ExpandCollapseState.Expanded, "Object submenu opens");
                foreach (string name in new[] { "Insert Image ...", "Edit Hyperlink ...", "Remove Hyperlink" })
                {
                    Wait(() => {
                        foreach (var window in OwnedWindows())
                        foreach (var item in Find(window, ControlType.MenuItem))
                        {
                            if (!item.Current.IsOffscreen && item.Current.Name == name) return true;
                        }
                        return false;
                    }, "Object child is exposed: " + name);
                }
                var selectionBeforeDismiss = text.GetSelection()[0].Clone();
                DismissDocumentContextMenu();
                Check(text.GetSelection()[0].Compare(selectionBeforeDismiss), "dismissal preserves the selected endpoints");
                open();
                string before = text.DocumentRange.GetText(-1);
                string selectedText = text.GetSelection()[0].GetText(-1);
                ContextMenuCommand("Copy", uia);
                Wait(() => DocumentContextMenus().Count == 0, "Copy dismisses the menu");
                Check(text.DocumentRange.GetText(-1) == before && text.GetSelection()[0].GetText(-1) == selectedText, "Copy preserves text and selection");
                open(); ContextMenuCommand("Cut", uia);
                Wait(() => text.DocumentRange.GetText(-1).Length == before.Length - selectedText.Length, "Cut removes the selected text");
                // A short press is unnecessary here: Cut leaves its insertion caret.
                var boundsAfterCut = document.Current.BoundingRectangle;
                Pointer("RightClick", boundsAfterCut.Left + 20, boundsAfterCut.Top + 20);
                Wait(() => DocumentContextMenus().Count == 1, "menu opens at the cut caret");
                var caretMenu = Popup(ControlType.Menu);
                foreach (string name in new[] { "Copy", "Cut", "Delete", "Set Font ...", "Text Color ...", "Background Color ..." })
                    Check(!Named(caretMenu, name, ControlType.MenuItem).Current.IsEnabled, "collapsed caret disables " + name);
                ContextMenuCommand("Paste", uia);
                Wait(() => text.DocumentRange.GetText(-1) == before, "Paste restores the copied text at the cut caret");
                Pointer("RightClick", boundsAfterCut.Left + 20, boundsAfterCut.Top + 20);
                Wait(() => DocumentContextMenus().Count == 1, "menu reopens for Undo");
                ContextMenuCommand("Undo", uia);
                Wait(() => text.DocumentRange.GetText(-1).Length == before.Length - selectedText.Length, "Undo removes the pasted text");
                Pointer("RightClick", boundsAfterCut.Left + 20, boundsAfterCut.Top + 20);
                Wait(() => DocumentContextMenus().Count == 1, "menu reopens for Redo");
                ContextMenuCommand("Redo", uia);
                Wait(() => text.DocumentRange.GetText(-1) == before, "Redo restores the pasted text");
                Pointer("RightClick", boundsAfterCut.Left + 20, boundsAfterCut.Top + 20);
                Wait(() => DocumentContextMenus().Count == 1, "menu reopens for Select All");
                ContextMenuCommand("Select All", uia);
                Wait(() => text.GetSelection()[0].GetText(-1) == before, "Select All covers the document");
                open(); ContextMenuCommand("Delete", uia);
                Wait(() => text.DocumentRange.GetText(-1) == "", "Delete clears the selected document");
                Pointer("RightClick", boundsAfterCut.Left + 20, boundsAfterCut.Top + 20);
                Wait(() => DocumentContextMenus().Count == 1, "menu opens in the empty document");
                ContextMenuCommand("Undo", uia);
                Wait(() => text.DocumentRange.GetText(-1) == before, "Undo restores the deleted document");
                Console.WriteLine("PASS context editing / " + (ribbon ? "Ribbon" : "Toolstrip synthetic point selection") + " / " + (uia ? "UIA Invoke after Aurora refresh" : "automation IO"));
                PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
            }
        }
        Console.WriteLine("LIMITATION UIA has no point-injection or context-menu-open pattern; opening uses synthetic automation IO in both passes, never physical touch.");
        PersistencePalette(hwnd, "Default");
    }
    static string EditorDocumentXml()
    {
        foreach (var node in SnapshotNodes(ControlsSnapshot()))
        {
            object control;
            if (!node.TryGetValue("control", out control) || (string)control != "DocumentViewer") continue;
            foreach (var child in SnapshotNodes(node))
            {
                object xml;
                if (child.TryGetValue("elementDocument", out xml)) return (string)xml;
            }
        }
        throw new Exception("Visible document viewer is missing from the automation snapshot.");
    }
    static void EditorSelect(AutomationElement document, bool point, string phrase, bool collapse = false)
    {
        var text = Pattern<TextPattern>(document, TextPattern.Pattern);
        document.SetFocus();
        ControlsSnapshot();
        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "document focus and pending reload become idle");
        var range = text.DocumentRange.FindText(phrase, false, false);
        Check(range != null, "selection phrase exists: " + phrase);
        range.ScrollIntoView(true);
        ControlsSnapshot();
        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "document scrolling becomes idle");
        System.Windows.Rect[] rectangles = null;
        int stable = 0;
        Wait(() => {
            var current = range.GetBoundingRectangles();
            bool same = rectangles != null && current.Length > 0 && current.Length == rectangles.Length;
            if (same) for (int i = 0; i < current.Length; i++) if (current[i] != rectangles[i]) same = false;
            stable = same ? stable + 1 : 0;
            rectangles = current;
            return stable >= 3;
        }, "SOP selection geometry settles after focus and scrolling");
        Check(rectangles.Length > 0, "selection phrase has rendered bounds");
        double x = rectangles[0].Left + 1, y = rectangles[0].Top + rectangles[0].Height / 2;
        Console.WriteLine("SELECTION / " + phrase + " / point=" + point + " / collapse=" + collapse + " / text bounds=" + rectangles[0] + " / document=" + document.Current.BoundingRectangle);
        if (point)
        {
            var bounds = document.Current.BoundingRectangle;
            double relativeX = (x - bounds.Left) / bounds.Width, relativeY = (y - bounds.Top) / bounds.Height;
            PointInput("Down", 1401, document, relativeX, relativeY);
            PointInput("Up", 1401, document, relativeX, relativeY);
            if (!collapse)
            {
                relativeX = (rectangles[0].Right - 1 - bounds.Left) / bounds.Width;
                PointInput("Down", 1401, document, relativeX, relativeY);
                Thread.Sleep(550);
                Check(text.GetSelection()[0].GetText(-1).Length == 0, "SOP point range waits for release");
                PointInput("Up", 1401, document, relativeX, relativeY);
            }
        }
        else
        {
            Pointer("LeftClick", x, y);
            if (!collapse) Press("Ctrl+Shift+Right");
        }
        // Nonempty selection alone can still describe the previous queued click.
        ControlsSnapshot();
        Wait(() => {
            var selected = text.GetSelection()[0];
            return (selected.GetText(-1).Length == 0) == collapse
                && selected.CompareEndpoints(TextPatternRangeEndpoint.Start, range, TextPatternRangeEndpoint.Start) == 0;
        }, "SOP establishes " + (collapse ? "a caret" : "a range") + " at " + phrase + (point ? " through point input" : " through mouse and keyboard input"));
    }
    static void EditorPointCaret(IntPtr hwnd)
    {
        Test("Document point regression / mouse-equivalent caret, focus and popup dismissal");
        string fixture = System.IO.Path.GetFullPath(System.IO.Path.Combine(PersistenceOutput, "..", "..", "TestDocument.bin"));
        Action<string, double, double> point = (operation, x, y) => {
            var p = UiaToClientPoint(x, y);
            Input("!Point" + operation + ":1601," + p.X + "," + p.Y);
        };
        Func<TextPatternRange, System.Windows.Rect[]> settledBounds = range => {
            System.Windows.Rect[] previous = null;
            int stable = 0;
            Wait(() => {
                var current = range.GetBoundingRectangles();
                bool same = previous != null && current.Length > 0 && current.Length == previous.Length;
                if (same) for (int i = 0; i < current.Length; i++) if (current[i] != previous[i]) same = false;
                stable = same ? stable + 1 : 0;
                previous = current;
                return stable >= 3;
            }, "paragraph bounds settle after scrolling and layout");
            return previous;
        };
        Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1200, 700);
        foreach (string palette in new[] { "Default", "Aurora" })
        {
            PersistencePalette(hwnd, palette);
            foreach (bool ribbon in new[] { true, false })
            {
                Console.WriteLine("TEST editor point fixture / ribbon=" + ribbon + " / palette=" + palette);
                var page = Page(Page(root, "Control"), ribbon ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
                PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, true);
                AutomationElement document = null;
                Wait(() => {
                    var documents = Find(page, ControlType.Document);
                    if (documents.Count == 0) return false;
                    document = documents[0];
                    return true;
                }, "editor document is accessible after loading the fixture");
                var text = Pattern<TextPattern>(document, TextPattern.Pattern);
                var heading = text.DocumentRange.FindText("GacUI Remote Document", false, false);
                Func<TextPatternRange, string> rangeOffsets = range => {
                    var prefix = text.DocumentRange.Clone();
                    prefix.MoveEndpointByRange(TextPatternRangeEndpoint.End, range, TextPatternRangeEndpoint.Start);
                    int begin = prefix.GetText(-1).Length;
                    prefix.MoveEndpointByRange(TextPatternRangeEndpoint.End, range, TextPatternRangeEndpoint.End);
                    return begin + "-" + prefix.GetText(-1).Length;
                };
                Check(heading != null, "point regression fixture loaded");
                var allText = text.DocumentRange.GetText(-1).TrimEnd('\r', '\n');
                string lastLine = allText.Substring(allText.LastIndexOf('\n') + 1).TrimEnd('\r');
                var last = text.DocumentRange.FindText(lastLine, false, true);
                Check(last != null, "last fixture paragraph located");
                string leftLine = Array.Find(allText.Split('\n'), line => line.StartsWith("Private save preserves")).TrimEnd('\r');
                var leftParagraph = text.DocumentRange.FindText(leftLine, false, false);
                foreach (bool reversed in new[] { false, true })
                for (int target = 0; target < 6; target++)
                {
                    Console.WriteLine("TEST editor point / ribbon=" + ribbon + " / palette=" + palette + " / target=" + target + " / reversed=" + reversed);
                    Func<bool, System.Windows.Point> prepare = forPoint => {
                        Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1200, target == 5 ? 1000 : 700);
                        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "point fixture resize reaches idle");
                        heading.ScrollIntoView(true); ControlsSnapshot();
                        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "point fixture scrolling reaches idle");
                        var h = settledBounds(heading)[0];
                        double y = (h.Top + h.Bottom) / 2;
                        double begin = h.Left + 1, end = h.Right - 1;
                        Drag(reversed ? end : begin, y, reversed ? begin - end : end - begin, 0);
                        ControlsSnapshot();
                        Check(text.GetSelection()[0].GetText(-1).Length > 0, "mouse drag establishes directed range");
                        if (target >= 4)
                        {
                            (target == 4 ? leftParagraph : last).ScrollIntoView(target == 4); ControlsSnapshot();
                            Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "point target scrolling reaches idle");
                        }
                        if (forPoint)
                        {
                            Named(page, "Copy", ControlType.Button).SetFocus();
                            Thread.Sleep(600); // Isolate generated clicks, then read the current viewport.
                        }
                        var targetBounds = settledBounds(target < 4 ? heading : target == 4 ? leftParagraph : last);
                        var bounds = document.Current.BoundingRectangle;
                        double rightEdge = bounds.Right, bottomEdge = bounds.Bottom;
                        foreach (var node in SnapshotNodes(ControlsSnapshot()))
                        {
                            object role;
                            if (!node.TryGetValue("control", out role) || (string)role != "DocumentViewer") continue;
                            var logical = SnapshotBounds(node);
                            foreach (var child in SnapshotNodes(node))
                            {
                                object cursor;
                                if (!child.TryGetValue("cursor", out cursor) || (string)cursor != "IBeam") continue;
                                var viewport = SnapshotBounds(child);
                                double scale = bounds.Width / logical.Width;
                                rightEdge = bounds.Left + (viewport.Right - logical.Left) * scale;
                                bottomEdge = bounds.Top + (viewport.Bottom - logical.Top) * scale;
                                break;
                            }
                            break;
                        }
                        if (target < 4)
                        {
                            h = targetBounds[0];
                            return new System.Windows.Point(target == 0 ? h.Left + h.Width / 2 : target == 1 ? h.Left + 1 : target == 2 ? h.Right - 1 : Math.Min(h.Right + 30, rightEdge - 8), (h.Top + h.Bottom) / 2);
                        }
                        var tail = targetBounds;
                        Check(tail.Length > 0, "last paragraph has visible bounds");
                        var t = tail[tail.Length - 1];
                        double hitY = target == 4 ? (t.Top + t.Bottom) / 2 : Math.Min(t.Bottom + 10, bottomEdge - 4);
                        if (target == 5) Check(hitY > t.Bottom, "below-final-paragraph point is inside blank viewport space");
                        return new System.Windows.Point(Math.Min(t.Right + 30, rightEdge - 8), hitY);
                    };
                    var hit = prepare(false);
                    Pointer("LeftClick", hit.X, hit.Y); ControlsSnapshot();
                    Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "point regression mouse baseline reaches idle");
                    var mouseHit = hit;
                    var expected = text.GetSelection()[0].Clone();
                    if (expected.GetText(-1).Length != 0)
                        System.IO.File.WriteAllText(System.IO.Path.Combine(PersistenceOutput, "point-failure.json"), new JavaScriptSerializer { MaxJsonLength = 16 * 1024 * 1024, RecursionLimit = 512 }.Serialize(ControlsSnapshot()));
                    Check(expected.GetText(-1).Length == 0, "ordinary mouse collapses the range / hit=" + hit + " / document=" + document.Current.BoundingRectangle + " / selection=" + expected.GetText(-1));
                    hit = prepare(true);
                    point("Down", hit.X, hit.Y); point("Up", hit.X, hit.Y);
                    ControlsSnapshot();
                    try
                    {
                        Wait(() => text.GetSelection()[0].Compare(expected), "short press matches mouse caret / target=" + target + " / reversed=" + reversed);
                    }
                    catch
                    {
                        Console.WriteLine("Caret mismatch: expected=" + rangeOffsets(expected) + " / actual=" + rangeOffsets(text.GetSelection()[0]) + " / mouse=" + mouseHit + " / point=" + hit + " / document=" + document.Current.BoundingRectangle);
                        System.IO.File.WriteAllText(System.IO.Path.Combine(PersistenceOutput, "point-failure.json"), new JavaScriptSerializer { MaxJsonLength = 16 * 1024 * 1024, RecursionLimit = 512 }.Serialize(ControlsSnapshot()));
                        throw;
                    }
                    CheckFocus(document, "short press focuses the document");
                }
                EditorSelect(document, true, "GacUI Remote");
                var selected = text.GetSelection()[0].Clone();
                var headingBounds = heading.GetBoundingRectangles()[0];
                double menuX = headingBounds.Right - 2, menuY = (headingBounds.Top + headingBounds.Bottom) / 2;
                point("Down", menuX, menuY); Thread.Sleep(550);
                Check(DocumentContextMenus().Count == 0, "holding does not open a popup");
                point("Up", menuX, menuY);
                Wait(() => DocumentContextMenus().Count == 1, "long press opens exactly one context menu");
                Check(text.GetSelection()[0].Compare(selected), "long press retains both range endpoints");
                double caretX = headingBounds.Left + 1;
                point("Down", caretX, menuY);
                Check(DocumentContextMenus().Count == 1, "raw Down leaves popup open");
                Input("!PointCancel:1601,PlatformCanceled"); ControlsSnapshot();
                Check(DocumentContextMenus().Count == 1, "cancellation leaves popup open");
                Thread.Sleep(600);
                point("Down", caretX, menuY); point("Up", caretX, menuY); ControlsSnapshot();
                Wait(() => DocumentContextMenus().Count == 0 && text.GetSelection()[0].GetText(-1).Length == 0,
                    "eligible short press closes popup and collapses caret");
                CheckFocus(document, "popup dismissal returns document focus");
                Console.WriteLine("PASS editor point caret / ribbon=" + ribbon + " / palette=" + palette + " / input=" + inputEndpoint);
            }
        }
        PersistencePalette(hwnd, "Default");
    }
    static AutomationElement EditorModal()
    {
        AutomationElement result = null;
        Wait(() => {
            foreach (var window in Find(root, ControlType.Window, TreeScope.Children))
            {
                object pattern;
                if (!window.Current.IsOffscreen && window.TryGetCurrentPattern(WindowPattern.Pattern, out pattern)
                    && ((WindowPattern)pattern).Current.IsModal) { result = window; return true; }
            }
            return false;
        }, "hosted editor modal opens");
        return result;
    }
    static void EditorDialogAction(AutomationElement dialog, bool cancel, bool uia)
    {
        var captions = cancel ? new[] { "Cancel", "\u53d6\u6d88" } : new[] { "OK", "Open", "Save", "\u786e\u5b9a", "\u6253\u5f00", "\u4fdd\u5b58" };
        AutomationElement button = null;
        Wait(() => {
            button = Find(dialog, ControlType.Button, TreeScope.Children).Find(item => Array.IndexOf(captions, item.Current.Name) >= 0 && !item.Current.IsOffscreen);
            if (button == null) button = EditorDialogControls(dialog, ControlType.Button).Find(item => Array.IndexOf(captions, item.Current.Name) >= 0 && !item.Current.IsOffscreen);
            return button != null;
        }, "requested dialog action becomes visible: " + dialog.Current.Name + " / cancel=" + cancel, 60000);
        Console.WriteLine("DIALOG action / " + dialog.Current.Name + " / cancel=" + cancel + " / " + button.Current.Name + " / UIA=" + uia);
        if (uia) Pattern<InvokePattern>(button, InvokePattern.Pattern).Invoke(); else Click(button);
        Wait(() => { try { return dialog.Current.IsOffscreen; } catch (ElementNotAvailableException) { return true; } }, "editor dialog closes");
        ControlsSnapshot();
        if (uia) Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "editor dialog completion becomes idle");
    }
    static List<AutomationElement> EditorDialogControls(AutomationElement dialog, ControlType type)
    {
        var result = new List<AutomationElement>();
        var parents = new List<AutomationElement> { dialog };
        for (int i = 0; i < parents.Count; i++)
        foreach (AutomationElement child in parents[i].FindAll(TreeScope.Children, Condition.TrueCondition))
        {
            var role = child.Current.ControlType;
            if (role == type) result.Add(child);
            // Font lists expose every virtual item. Dialog control lookup must
            // not enumerate their item or document subtrees.
            if (role != ControlType.List && role != ControlType.ListItem && role != ControlType.Document) parents.Add(child);
        }
        return result;
    }
    static void EditorSetValue(AutomationElement edit, string value, bool uia)
    {
        var pattern = Pattern<ValuePattern>(edit, ValuePattern.Pattern);
        if (uia) pattern.SetValue(value);
        else
        {
            Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "dialog input becomes idle before clicking a field");
            var bounds = edit.Current.BoundingRectangle;
            var client = UiaToClientPoint(bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2);
            Console.WriteLine("FIELD click / value=" + value + " / bounds=" + bounds + " / client=" + client.X + "," + client.Y + " / root=" + root.Current.BoundingRectangle);
            Click(edit);
            try { Wait(() => edit.Current.HasKeyboardFocus, "dialog field has focus before typing " + value); }
            catch
            {
                foreach (var field in EditorDialogControls(EditorModal(), ControlType.Edit))
                    Console.WriteLine("FIELD focus / value=" + Pattern<ValuePattern>(field, ValuePattern.Pattern).Current.Value + " / focused=" + field.Current.HasKeyboardFocus + " / bounds=" + field.Current.BoundingRectangle);
                throw;
            }
            Press("Ctrl+A");
            Input("!Type:" + value);
        }
        Wait(() => pattern.Current.Value == value, "dialog field contains " + value);
    }
    static void EditorContextOpen(AutomationElement document)
    {
        var text = Pattern<TextPattern>(document, TextPattern.Pattern);
        var selection = text.GetSelection()[0].Clone();
        var bounds = document.Current.BoundingRectangle;
        Pointer("RightClick", bounds.Left + bounds.Width * 0.5, bounds.Top + bounds.Height * 0.3);
        Wait(() => DocumentContextMenus().Count == 1, "SOP context menu opens");
        Check(text.GetSelection()[0].Compare(selection), "SOP context opening preserves both endpoints");
    }
    static AutomationElement EditorMenuItem(string name)
    {
        AutomationElement result = null;
        Wait(() => {
            foreach (var item in Find(root, ControlType.MenuItem))
            {
                if (item.Current.Name == name && !item.Current.IsOffscreen) { result = item; return true; }
            }
            return false;
        }, "visible editor menu item: " + name);
        return result;
    }
    static void EditorCommand(AutomationElement page, AutomationElement document, bool ribbon, string command, bool context, bool uia, bool objectMenu = false)
    {
        if (context)
        {
            EditorContextOpen(document);
            if (!objectMenu)
            {
                ContextMenuCommand(command, uia);
                Wait(() => DocumentContextMenus().Count == 0, "SOP context command closes its menu: " + command);
                ControlsSnapshot();
                return;
            }
            var header = Named(Popup(ControlType.Menu), "Object", ControlType.MenuItem);
            if (uia) Pattern<ExpandCollapsePattern>(header, ExpandCollapsePattern.Pattern).Expand(); else ContextMenuCommand("Object", false);
            AutomationElement child = null;
            Wait(() => {
                foreach (var menu in Find(root, ControlType.Menu))
                {
                    if (FindNamed(menu, "Insert Image ...", ControlType.MenuItem) == null || FindNamed(menu, "Select All", ControlType.MenuItem) != null) continue;
                    child = FindNamed(menu, command, ControlType.MenuItem);
                    if (child != null && !child.Current.IsOffscreen) return true;
                }
                return false;
            }, "SOP Object command: " + command);
            if (uia) Pattern<InvokePattern>(child, InvokePattern.Pattern).Invoke(); else Click(child);
        }
        else
        {
            if (ribbon && command == "Select All")
            {
                document.SetFocus();
                Press("Ctrl+A");
                ControlsSnapshot();
                return;
            }
            bool mode = command == "Preview" || command == "Selectable" || command == "Editable";
            if (ribbon)
            {
                var tab = Named(page, command == "Insert Image ..." ? "Insert" : "Edit", ControlType.TabItem);
                if (uia) Pattern<SelectionItemPattern>(tab, SelectionItemPattern.Pattern).Select(); else Click(tab);
                ControlsSnapshot();
                if (uia) Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(10000), "editor ribbon tab becomes idle");
            }
            if (mode || (!ribbon && (command == "Left" || command == "Center" || command == "Right" || command == "Set Alignment to Default (Left)" || command == "Edit Hyperlink ..." || command == "Remove Hyperlink" || command == "Select All")))
            {
                var header = Named(page, ribbon ? "Edit Mode" : mode ? "View" : "Edit", ControlType.MenuItem);
                if (uia) Pattern<ExpandCollapsePattern>(header, ExpandCollapsePattern.Pattern).Expand(); else Click(header);
                if (!mode && command != "Select All")
                {
                    var group = EditorMenuItem(command.Contains("Hyperlink") ? "Object" : "Paragram Alignment");
                    if (uia) Pattern<ExpandCollapsePattern>(group, ExpandCollapsePattern.Pattern).Expand(); else Click(group);
                }
                var item = EditorMenuItem(command);
                if (uia) Pattern<InvokePattern>(item, InvokePattern.Pattern).Invoke(); else Click(item);
                ControlsSnapshot();
                return;
            }
            var button = Named(page, command, !ribbon && command == "Insert Image ..." ? ControlType.SplitButton : ControlType.Button);
            Console.WriteLine("COMMAND / " + command + " / UIA=" + uia + " / enabled=" + button.Current.IsEnabled + " / offscreen=" + button.Current.IsOffscreen + " / bounds=" + button.Current.BoundingRectangle);
            if (uia) Pattern<InvokePattern>(button, InvokePattern.Pattern).Invoke(); else Click(button);
        }
        if (context) Wait(() => DocumentContextMenus().Count == 0, "SOP Object command closes its menu: " + command);
        ControlsSnapshot();
    }
    static void EditorClipboard(AutomationElement page, AutomationElement document, bool ribbon, bool uia, string fixture)
    {
        var text = Pattern<TextPattern>(document, TextPattern.Pattern);
        var shortcuts = new Dictionary<string, string> {
            { "Copy", "Ctrl+C" }, { "Cut", "Ctrl+X" }, { "Paste", "Ctrl+V" },
            { "Undo", "Ctrl+Z" }, { "Redo", "Ctrl+Y" }, { "Delete", "Delete" }, { "Select All", "Ctrl+A" }
        };
        Func<int> selectionStart = () => {
            var prefix = text.DocumentRange.Clone();
            prefix.MoveEndpointByRange(TextPatternRangeEndpoint.End, text.GetSelection()[0], TextPatternRangeEndpoint.Start);
            return prefix.GetText(-1).Length;
        };
        for (int route = 0; route < (ribbon ? 3 : 2); route++)
        {
            Test("SOP clipboard / " + (ribbon ? "ribbon" : "toolstrip") + " / route=" + route + " / UIA=" + uia);
            Action<string> command = name => {
                if (route == 2) { document.SetFocus(); Press(shortcuts[name]); ControlsSnapshot(); }
                else EditorCommand(page, document, ribbon, name, route == 0, uia);
            };
            PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
            EditorSelect(document, !ribbon, "Private save preserves");
            string copied = text.GetSelection()[0].GetText(-1);
            Console.WriteLine("TEST SOP clipboard source / offset=" + selectionStart() + " / text=" + copied);
            string sourceText = text.DocumentRange.GetText(-1);
            var copiedRange = text.GetSelection()[0].Clone();
            command("Copy");
            Check(text.DocumentRange.GetText(-1) == sourceText && text.GetSelection()[0].Compare(copiedRange), "Copy preserves document text and both endpoints");
            EditorSelect(document, !ribbon, "GacUI Remote", true);
            string original = text.DocumentRange.GetText(-1);
            string inserted = original.Insert(selectionStart(), copied);
            command("Paste");
            Wait(() => text.DocumentRange.GetText(-1) == inserted, "Copy/Paste inserts the selected text at the new caret");
            command("Undo");
            Wait(() => text.DocumentRange.GetText(-1) == original, "Undo removes the pasted text");
            command("Redo");
            Wait(() => text.DocumentRange.GetText(-1) == inserted, "Redo restores the pasted text");

            PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
            EditorSelect(document, !ribbon, "Private save preserves");
            original = text.DocumentRange.GetText(-1);
            string removed = original.Remove(selectionStart(), text.GetSelection()[0].GetText(-1).Length);
            command("Cut");
            Wait(() => text.DocumentRange.GetText(-1) == removed, "Cut removes exactly the selected range");
            command("Paste");
            Wait(() => text.DocumentRange.GetText(-1) == original, "Paste restores the cut range");
            EditorSelect(document, !ribbon, "Private save preserves");
            removed = original.Remove(selectionStart(), text.GetSelection()[0].GetText(-1).Length);
            command("Delete");
            Wait(() => text.DocumentRange.GetText(-1) == removed, "Delete removes exactly the selected range");
            command("Undo");
            Wait(() => text.DocumentRange.GetText(-1) == original, "Undo restores the deleted range");
            command("Select All");
            Check(text.GetSelection()[0].GetText(-1) == original, "Select All selects the complete document");
            Console.WriteLine("PASS SOP Copy/Cut/Paste/Delete/Undo/Redo/Select All / route=" + route + " / ribbon=" + ribbon + " / UIA=" + uia);
        }
        Console.WriteLine("NOTE Ribbon Select All has no separate ribbon button; its equivalent is Ctrl+A.");
        PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
    }
    // Four steps per editor: clipboard, formatting, objects, modes/samples.
    // Groups are automation Ribbon/Toolstrip, then UIA Ribbon/Toolstrip.
    public static int EditorSopStartStep;
    public static bool EditorSopResumeAtImages;
    static void DocumentEditorSop(IntPtr hwnd)
    {
        Check(isHosted, "SOP sections 3.1 and 3.2 execute only in CppTest");
        Test("CppTest SOP 3.1/3.2 / observable document commands");
        Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1400, 950);
        string fixture = System.IO.Path.GetFullPath(System.IO.Path.Combine(PersistenceOutput, "..", "..", "TestDocument.bin"));
        string folder = System.IO.Path.Combine(PersistenceOutput, "SOP-" + ownedProcess);
        System.IO.Directory.CreateDirectory(folder);
        for (int pass = 0; pass < 2; pass++)
        for (int editor = 0; editor < 2; editor++)
        {
            int step = (pass * 2 + editor) * 4;
            if (step + 3 < EditorSopStartStep) continue;
            bool ribbon = editor == 0, uia = pass == 1;
            var page = Page(Page(root, "Control"), ribbon ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
            PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
            var document = Find(page, ControlType.Document)[0];
            var text = Pattern<TextPattern>(document, TextPattern.Pattern);
            if (step >= EditorSopStartStep) EditorClipboard(page, document, ribbon, uia, fixture);
            if (step + 1 >= EditorSopStartStep)
            {
                EditorSelect(document, !ribbon, "Private save preserves");
                var selection = text.GetSelection()[0].Clone();
                foreach (var attribute in new[] { TextPattern.FontWeightAttribute, TextPattern.IsItalicAttribute, TextPattern.UnderlineStyleAttribute, TextPattern.StrikethroughStyleAttribute })
                {
                    string command = attribute == TextPattern.FontWeightAttribute ? "Bold" : attribute == TextPattern.IsItalicAttribute ? "Italic" : attribute == TextPattern.UnderlineStyleAttribute ? "Underline" : "Strike";
                    var before = selection.GetAttributeValue(attribute);
                    EditorCommand(page, document, ribbon, command, false, uia);
                    Wait(() => !selection.GetAttributeValue(attribute).Equals(before), command + " changes effective formatting");
                    Check(text.GetSelection()[0].Compare(selection), command + " preserves the range");
                    EditorCommand(page, document, ribbon, command, false, uia);
                    Wait(() => selection.GetAttributeValue(attribute).Equals(before), command + " restores effective formatting");
                    Console.WriteLine("PASS SOP " + command + " on/off / editor=" + editor + " / UIA=" + uia);
                }
                foreach (string alignment in new[] { "Left", "Center", "Right" })
                {
                    EditorCommand(page, document, ribbon, alignment, false, uia);
                    Wait(() => {
                        foreach (System.Text.RegularExpressions.Match paragraph in System.Text.RegularExpressions.Regex.Matches(EditorDocumentXml(), "<p(?: [^>]*)?>.*?</p>", System.Text.RegularExpressions.RegexOptions.Singleline))
                        {
                            string content = System.Net.WebUtility.HtmlDecode(System.Text.RegularExpressions.Regex.Replace(paragraph.Value.Replace("<sp/>", " "), "<[^>]*>", ""));
                            if (content.Contains("Private save preserves")) return paragraph.Value.Contains("align=\"" + alignment + "\"");
                        }
                        return false;
                    }, "paragraph alignment is serialized as " + alignment);
                    Wait(() => Convert.ToInt32(selection.GetAttributeValue(TextPattern.HorizontalTextAlignmentAttribute)) == (alignment == "Left" ? 0 : alignment == "Center" ? 1 : 2), "effective paragraph alignment agrees with " + alignment);
                }
                for (int route = 0; route < 2; route++)
                foreach (string command in new[] { "Set Font ...", "Text Color ...", "Background Color ..." })
                {
                    bool context = route == 0;
                    Test("SOP dialog / " + command + " / context=" + context + " / editor=" + editor + " / UIA=" + uia);
                    string before = EditorDocumentXml();
                    EditorCommand(page, document, ribbon, command, context, uia);
                    EditorDialogAction(EditorModal(), true, uia);
                    Check(EditorDocumentXml() == before, command + " cancellation preserves formatting");
                    EditorCommand(page, document, ribbon, command, context, uia);
                    var dialog = EditorModal();
                    if (command == "Set Font ...")
                    {
                        var lists = EditorDialogControls(dialog, ControlType.List);
                        Check(lists.Count == 2, "font dialog exposes family and size lists");
                        string oldFamily = (string)selection.GetAttributeValue(TextPattern.FontNameAttribute);
                        string dialogTitle = dialog.Current.Name;
                        string family = null;
                        Wait(() => {
                            foreach (var window in SnapshotNodes(ControlsSnapshot()))
                            {
                                object title, composition;
                                if (!window.TryGetValue("title", out title) || (string)title != dialogTitle || !window.TryGetValue("composition", out composition)) continue;
                                foreach (var list in SnapshotNodes(composition))
                                {
                                    object role;
                                    if (!list.TryGetValue("control", out role) || (string)role != "TextList") continue;
                                    foreach (var caption in SnapshotNodes(list))
                                    {
                                        object name;
                                        if (caption.TryGetValue("elementText", out name) && ((string)name).Length > 0 && (string)name != oldFamily) { family = (string)name; break; }
                                    }
                                    break;
                                }
                                break;
                            }
                            return family != null;
                        }, "another visible font family is available", 60000);
                        var choice = Pattern<ItemContainerPattern>(lists[0], ItemContainerPattern.Pattern).FindItemByProperty(null, AutomationElement.NameProperty, family);
                        Check(choice != null, "font family resolves through ItemContainer: " + family);
                        Pattern<ScrollItemPattern>(choice, ScrollItemPattern.Pattern).ScrollIntoView();
                        Wait(() => !choice.Current.IsOffscreen, "font family is visible before selection: " + family);
                        if (uia) Pattern<SelectionItemPattern>(choice, SelectionItemPattern.Pattern).Select(); else Click(choice);
                        var edits = EditorDialogControls(dialog, ControlType.Edit).FindAll(edit => !Pattern<ValuePattern>(edit, ValuePattern.Pattern).Current.IsReadOnly);
                        Check(edits.Count == 1, "font size is the editable font field");
                        int size = 20 + route * 2 + pass * 4;
                        EditorSetValue(edits[0], size.ToString(), uia);
                        EditorDialogAction(dialog, false, uia);
                        Wait(() => (string)selection.GetAttributeValue(TextPattern.FontNameAttribute) == family
                            && Math.Abs(Convert.ToDouble(selection.GetAttributeValue(TextPattern.FontSizeAttribute)) - size * 0.75) < 0.01, "font family and size apply to the selection");
                    }
                    else
                    {
                        var edits = EditorDialogControls(dialog, ControlType.Edit);
                        Check(edits.Count == 3, "color dialog exposes RGB components");
                        int[] color = command == "Text Color ..." ? new[] { 34 + route + pass, 150, 90 } : new[] { 40 + route + pass, 60, 180 };
                        for (int index = 0; index < 3; index++) EditorSetValue(edits[index], color[index].ToString(), uia);
                        EditorDialogAction(dialog, false, uia);
                        var attribute = command == "Text Color ..." ? TextPattern.ForegroundColorAttribute : TextPattern.BackgroundColorAttribute;
                        Wait(() => Convert.ToInt32(selection.GetAttributeValue(attribute)) == color[0] + (color[1] << 8) + (color[2] << 16), command + " applies the chosen RGB value");
                    }
                    Check(text.GetSelection()[0].Compare(selection), command + " preserves both selection endpoints");
                    Console.WriteLine("PASS SOP " + command + " cancel/apply / context=" + context + " / editor=" + editor + " / UIA=" + uia);
                }
                string formattedText = text.GetSelection()[0].GetText(-1);
                var formatAttributes = new[] { TextPattern.FontNameAttribute, TextPattern.FontSizeAttribute, TextPattern.ForegroundColorAttribute, TextPattern.BackgroundColorAttribute, TextPattern.HorizontalTextAlignmentAttribute };
                var formatting = new object[formatAttributes.Length];
                for (int index = 0; index < formatting.Length; index++) formatting[index] = selection.GetAttributeValue(formatAttributes[index]);
                string formattedPath = System.IO.Path.Combine(folder, "formatting-" + editor + "-" + pass + ".bin");
                PersistenceFile(page, ribbon, "Save as Private Format", formattedPath, false, !uia);
                PersistenceFile(page, ribbon, "Load as Private Format", formattedPath, false, !uia);
                var restoredFormat = text.DocumentRange.FindText(formattedText, false, false);
                Check(restoredFormat != null, "formatted text survives private reload");
                for (int index = 0; index < formatting.Length; index++)
                    Check(restoredFormat.GetAttributeValue(formatAttributes[index]).Equals(formatting[index]), "effective formatting survives private reload: " + formatAttributes[index].ProgrammaticName);
                PersistenceFile(page, ribbon, "Save as RTF", formattedPath + ".rtf", false, !uia);
                PersistenceFile(page, ribbon, "Save as HTML", formattedPath + ".html", false, !uia);
                string formattedRtf = System.IO.File.ReadAllText(formattedPath + ".rtf", Encoding.UTF8);
                string formattedHtml = System.IO.File.ReadAllText(formattedPath + ".html", Encoding.UTF8);
                Check(formattedRtf.Contains((string)formatting[0]) && formattedRtf.Contains("\\qr{"), "RTF retains the chosen family and paragraph alignment");
                Check(formattedHtml.Contains("font-family:" + formatting[0]) && formattedHtml.Contains("font-size:" + (22 + pass * 4) + "px;"), "HTML retains the chosen family and size");
                for (int index = 2; index < 4; index++)
                {
                    int color = Convert.ToInt32(formatting[index]);
                    int red = color & 255, green = (color >> 8) & 255, blue = (color >> 16) & 255;
                    Check(formattedRtf.Contains("\\red" + red + "\\green" + green + "\\blue" + blue), "RTF retains the chosen color");
                    string cssColor = "#" + red.ToString("X2") + green.ToString("X2") + blue.ToString("X2");
                    Check(formattedHtml.Contains((index == 2 ? "color:" : "background-color:") + cssColor), "HTML retains the chosen color");
                }
            }
            // RTF/HTML exports select the entire document; reload before point selection.
            PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
            if (step + 2 >= EditorSopStartStep)
            {
                for (int route = EditorSopResumeAtImages && step + 2 == EditorSopStartStep ? 3 : 0; route < 3; route++)
                {
                    EditorSelect(document, !ribbon, "Private save preserves");
                    string selectedText = text.GetSelection()[0].GetText(-1);
                    string before = EditorDocumentXml();
                    EditorCommand(page, document, ribbon, "Edit Hyperlink ...", route < 2, uia, route == 1);
                    EditorDialogAction(EditorModal(), true, uia);
                    Check(EditorDocumentXml() == before, "hyperlink cancellation preserves the document");
                    EditorCommand(page, document, ribbon, "Edit Hyperlink ...", route < 2, uia, route == 1);
                    var dialog = EditorModal();
                    var edits = Find(dialog, ControlType.Edit);
                    Check(edits.Count == 1, "hyperlink dialog exposes its URL field");
                    string url = "https://example.invalid/context-" + editor + "-" + pass + "-" + route;
                    EditorSetValue(edits[0], url, uia);
                    Console.WriteLine("HYPERLINK value / " + Pattern<ValuePattern>(edits[0], ValuePattern.Pattern).Current.Value + " / selection=" + text.GetSelection()[0].GetText(-1));
                    EditorDialogAction(dialog, false, uia);
                    Wait(() => EditorDocumentXml().Contains(url), "hyperlink URL is stored in the document");
                    Check(text.GetSelection()[0].GetText(-1) == selectedText, "hyperlink applies to the selected text");
                    if (route == 0)
                    {
                        string linked = System.IO.Path.Combine(folder, "hyperlink-" + editor + "-" + pass + ".bin");
                        string beforeSave = text.DocumentRange.GetText(-1);
                        PersistenceFile(page, ribbon, "Save as Private Format", linked, false, !uia);
                        PersistenceFile(page, ribbon, "Load as Private Format", linked, false, !uia);
                        Check(EditorDocumentXml().Contains(url) && text.DocumentRange.GetText(-1) == beforeSave, "private reload preserves the hyperlink and document text");
                    }
                    EditorCommand(page, document, ribbon, "Preview", false, uia);
                    string originalText = text.DocumentRange.GetText(-1);
                    document.SetFocus(); Input("!Type:blocked"); Press("Delete"); ControlsSnapshot();
                    Check(text.DocumentRange.GetText(-1) == originalText, "Preview rejects typing and deletion");
                    var hyperlink = text.DocumentRange.FindText(selectedText, false, false);
                    if (uia)
                    {
                        var links = Find(document, ControlType.Hyperlink);
                        Check(links.Count > 0, "UIA exposes the document hyperlink");
                        Pattern<InvokePattern>(links[0], InvokePattern.Pattern).Invoke();
                    }
                    else
                    {
                        var rectangles = hyperlink.GetBoundingRectangles();
                        Check(rectangles.Length > 0, "hyperlink has rendered bounds");
                        Pointer("LeftClick", rectangles[0].Left + rectangles[0].Width / 2, rectangles[0].Top + rectangles[0].Height / 2);
                    }
                    dialog = EditorModal();
                    Check(dialog.Current.Name == "You Clicked a Hyperlink!", "hyperlink activation opens the sample message");
                    Check(FindNamed(dialog, url, ControlType.Text) != null, "hyperlink message contains the chosen URL");
                    EditorDialogAction(dialog, false, uia);
                    EditorCommand(page, document, ribbon, "Editable", false, uia);
                    EditorSelect(document, !ribbon, "Private save preserves");
                    EditorCommand(page, document, ribbon, "Remove Hyperlink", route < 2, uia, route == 1);
                    Wait(() => !EditorDocumentXml().Contains(url), "Remove Hyperlink clears the selected link");
                    Check(text.DocumentRange.GetText(-1) == originalText, "hyperlink removal preserves text");
                    Console.WriteLine("PASS SOP hyperlink cancel/edit/activate/remove / route=" + route + " / editor=" + editor + " / UIA=" + uia);
                }
                for (int route = 0; route < 2; route++)
                {
                    PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
                    EditorSelect(document, !ribbon, "GacUI Remote", true);
                    string before = EditorDocumentXml();
                    int images = System.Text.RegularExpressions.Regex.Matches(before, "<img ").Count;
                    EditorCommand(page, document, ribbon, "Insert Image ...", route == 0, uia, route == 0);
                    EditorDialogAction(EditorModal(), true, uia);
                    Check(EditorDocumentXml() == before, "image dialog cancellation preserves the document");
                    EditorCommand(page, document, ribbon, "Insert Image ...", route == 0, uia, route == 0);
                    var dialog = EditorModal();
                    var edits = Find(dialog, ControlType.Edit);
                    Check(edits.Count > 0, "image dialog exposes a filename field");
                    string imagePath = System.IO.Path.GetFullPath(System.IO.Path.Combine(PersistenceOutput, "..", "..", "Resources", "App", "FullControlTest", "ToolbarImages", "s_Font.png"));
                    if (route == 0)
                    {
                        EditorSetValue(edits[edits.Count - 1], System.IO.Path.GetDirectoryName(System.IO.Path.GetDirectoryName(imagePath)), uia);
                        edits[edits.Count - 1].SetFocus();
                        Wait(() => edits[edits.Count - 1].Current.HasKeyboardFocus, "image picker filename focus before folder navigation");
                        Press("Enter");
                        AutomationElement folderItem = null;
                        Wait(() => { folderItem = FindNamed(dialog, "ToolbarImages", ControlType.ListItem); return folderItem != null; }, "image picker exposes folder list items", 60000);
                        var fileList = TreeWalker.ControlViewWalker.GetParent(folderItem);
                        Check(fileList.Current.ControlType == ControlType.List, "non-Detail data grid exposes a list");
                        Absent(fileList, TablePattern.Pattern);
                        Pattern<SelectionItemPattern>(folderItem, SelectionItemPattern.Pattern).Select();
                        fileList.SetFocus();
                        Wait(() => Pattern<SelectionItemPattern>(folderItem, SelectionItemPattern.Pattern).Current.IsSelected && fileList.Current.HasKeyboardFocus, "image picker folder selection and list focus");
                        var selected = Pattern<SelectionPattern>(fileList, SelectionPattern.Pattern).Current.GetSelection();
                        Check(selected.Length == 1, "image picker returns one selected item");
                        Same(selected[0], folderItem, "image picker selection returns the live folder item");
                        CheckFocus(fileList, "image picker list-mode focus");
                        Console.WriteLine("PASS image picker list-mode UIA selection/focus / editor=" + editor + " / UIA=" + uia);
                    }
                    EditorSetValue(edits[edits.Count - 1], imagePath, uia);
                    EditorDialogAction(dialog, false, uia);
                    Wait(() => System.Text.RegularExpressions.Regex.Matches(EditorDocumentXml(), "<img ").Count == images + 1, "image insertion adds one embedded image at the caret");
                    EditorCommand(page, document, ribbon, "Undo", false, uia);
                    Wait(() => System.Text.RegularExpressions.Regex.Matches(EditorDocumentXml(), "<img ").Count == images, "Undo removes the image");
                    EditorCommand(page, document, ribbon, "Redo", false, uia);
                    Wait(() => System.Text.RegularExpressions.Regex.Matches(EditorDocumentXml(), "<img ").Count == images + 1, "Redo restores the image");
                    string stem = System.IO.Path.Combine(folder, "image-" + editor + "-" + pass + "-" + route);
                    PersistenceFile(page, ribbon, "Save as Private Format", stem + ".bin", false, !uia);
                    PersistenceFile(page, ribbon, "Load as Private Format", stem + ".bin", false, !uia);
                    Check(System.Text.RegularExpressions.Regex.Matches(EditorDocumentXml(), "<img ").Count == images + 1, "private reload retains the inserted image");
                    PersistenceFile(page, ribbon, "Save as RTF", stem + ".rtf", false, !uia);
                    PersistenceFile(page, ribbon, "Save as HTML", stem + ".html", false, !uia);
                    string rtf = System.IO.File.ReadAllText(stem + ".rtf");
                    string html = System.IO.File.ReadAllText(stem + ".html", Encoding.UTF8);
                    Check(rtf.StartsWith("{\\rtf") && rtf.Contains("\\pict") && rtf.Contains("\\u20320"), "RTF export contains Unicode and embedded pictures");
                    Check(html.Contains("data:image/") && System.Net.WebUtility.HtmlDecode(html).Contains("\u4f60\u597d"), "HTML export contains Unicode and embedded image data");
                    Console.WriteLine("PASS SOP image cancel/insert/undo/redo/private/RTF/HTML / context=" + (route == 0) + " / editor=" + editor + " / UIA=" + uia);
                }
            }
            PersistenceFile(page, ribbon, "Load as Private Format", fixture, false, !uia);
            EditorSelect(document, !ribbon, "Private save preserves");
            EditorCommand(page, document, ribbon, "Selectable", false, uia);
            string selectableText = text.DocumentRange.GetText(-1);
            document.SetFocus(); Input("!Type:blocked"); Press("Delete"); ControlsSnapshot();
            Check(text.DocumentRange.GetText(-1) == selectableText, "Selectable rejects typing and deletion");
            EditorContextOpen(document);
            var readOnlyMenu = Popup(ControlType.Menu);
            Check(Named(readOnlyMenu, "Copy", ControlType.MenuItem).Current.IsEnabled, "Selectable retains Copy");
            foreach (string command in new[] { "Cut", "Delete", "Set Font ...", "Text Color ...", "Background Color ...", "Edit Hyperlink ...", "Remove Hyperlink" })
                Check(!Named(readOnlyMenu, command, ControlType.MenuItem).Current.IsEnabled, "Selectable disables " + command);
            ContextMenuCommand("Copy", uia);
            EditorCommand(page, document, ribbon, "Editable", false, uia);
            EditorSelect(document, !ribbon, "GacUI Remote", true);
            string editableText = "SOP editable ";
            document.SetFocus(); Input("!Type:" + editableText);
            Wait(() => text.DocumentRange.GetText(-1).Contains(editableText), "Editable accepts text");
            // Automation typing creates one edit step for each character.
            for (int index = 0; index < editableText.Length; index++)
            {
                if (ribbon) { document.SetFocus(); Press("Ctrl+Z"); }
                else EditorCommand(page, document, false, "Undo", false, uia);
            }
            Wait(() => text.DocumentRange.GetText(-1) == selectableText, "Undo restores the exact document");
            var modes = new[] { "Preview", "Selectable", "Editable" };
            var modeKeys = new[] { "Ctrl+Shift+P", "Ctrl+Shift+S", "Ctrl+Shift+E" };
            for (int mode = 0; mode < modes.Length; mode++)
            {
                if (ribbon) { document.SetFocus(); Press(modeKeys[mode]); ControlsSnapshot(); }
                else EditorCommand(page, document, false, modes[mode], false, uia);
                Check(text.DocumentRange.GetText(-1) == selectableText, "mode preserves text: " + modes[mode]);
                Check((bool)text.DocumentRange.GetAttributeValue(TextPattern.IsReadOnlyAttribute) == (mode != 2), "mode changes editability: " + modes[mode]);
                Check(text.SupportedTextSelection == (mode == 0 ? SupportedTextSelection.None : SupportedTextSelection.Single), "mode changes selection support: " + modes[mode]);
            }
            PersistenceFile(page, ribbon, "Load as Private Format", fixture, true, !uia);
            PersistenceFile(page, ribbon, "Save as Private Format", System.IO.Path.Combine(folder, "canceled.bin"), true, !uia);
            Check(text.DocumentRange.GetText(-1) == selectableText, "canceled persistence dialogs retain the fixture");
            Console.WriteLine("PASS SOP modes/shortcuts/persistence cancellation / editor=" + editor + " / UIA=" + uia);
            if (!ribbon)
            {
                Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1100, 600);
                ControlsSnapshot();
                page = Page(Page(root, "Control"), "Document Editor (Toolstrip)");
                document = Find(page, ControlType.Document)[0];
                text = Pattern<TextPattern>(document, TextPattern.Pattern);
                EditorSelect(document, true, "GacUI Remote");
                var selected = text.GetSelection()[0].Clone();
                var scroll = Pattern<ScrollPattern>(document, ScrollPattern.Pattern);
                Check(scroll.Current.VerticallyScrollable, "SOP point-scroll fixture exceeds its viewport");
                double beforeScroll = scroll.Current.VerticalScrollPercent;
                PointInput("Down", 1403, document, 0.6, 0.8);
                PointInput("Update", 1403, document, 0.6, 0.3);
                PointInput("Up", 1403, document, 0.6, 0.3);
                Wait(() => scroll.Current.VerticalScrollPercent > beforeScroll, "SOP content dragging scrolls");
                Check(text.GetSelection()[0].Compare(selected), "SOP scrolling retains the selected endpoints");
                PointInput("Down", 1404, document, 0.6, 0.4);
                Thread.Sleep(550);
                Check(DocumentContextMenus().Count == 0, "scrolled document hold waits for release");
                PointInput("Up", 1404, document, 0.6, 0.4);
                Wait(() => DocumentContextMenus().Count == 1, "scrolled document long press opens one menu");
                Check(text.GetSelection()[0].Compare(selected), "scrolled context menu preserves the range");
                DismissDocumentContextMenu();
                Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1400, 950);
                Console.WriteLine("PASS SOP 3.2 synthetic point scrolling and long-press menu; no physical touch was injected.");
            }
            else
            {
                string beforeSample = EditorDocumentXml();
                var edits = Find(page, ControlType.Edit).FindAll(edit => !edit.Current.IsOffscreen);
                Check(edits.Count == 7, "ribbon exposes Search and six Icon Labels sample fields");
                for (int index = 0; index < edits.Count; index++)
                {
                    string value = "sample" + index;
                    EditorSetValue(edits[index], value, uia);
                    Check(Pattern<ValuePattern>(edits[index], ValuePattern.Pattern).Current.Value == value, "sample field accepts text");
                }
                Check(EditorDocumentXml() == beforeSample, "Search and Icon Labels do not invent document commands");
                var groups = Find(page, ControlType.Group).FindAll(group => group.Current.Name == "Alignment");
                Check(groups.Count == 1, "Alignment group is exposed");
                var expandButtons = Find(groups[0], ControlType.Button).FindAll(button => button.Current.Name.Length == 0 && !button.Current.IsOffscreen);
                Check(expandButtons.Count == 1, "Alignment has one unnamed expansion button");
                if (uia) Pattern<InvokePattern>(expandButtons[0], InvokePattern.Pattern).Invoke(); else Click(expandButtons[0]);
                var dialog = EditorModal();
                Check(dialog.Current.Name == "You Expanded a Group!", "ribbon group expansion executes its sample action");
                EditorDialogAction(dialog, false, uia);
                Console.WriteLine("PASS SOP Search/Icon Labels fields and Alignment group sample action / UIA=" + uia);
            }
        }
        Console.WriteLine("SOP outputs: " + folder);
    }
    static void DocumentStyles(IntPtr hwnd, string folder, double? defaultFontSize = null)
    {
        Test("Document styles / fixture load, all built-ins, unused definitions and palette replacement");
        var names = new[] { "Header 1", "Header 2", "Header 3", "Strong", "Quote", "Emphasis", "Intense Emphasis", "Deleted" };
        var factors = new[] { 2.0, 1.6, 1.3, 1.0, 1.0, 1.0, 1.0, 1.0 };
        Action<string, string> checkCatalog = (xml, context) => {
            foreach (string name in names) Check(xml.Contains("<Style name=\"" + name + "\""), context + " contains " + name);
        };
        PersistencePalette(hwnd, "Default");
        var page = Page(Page(root, "Control"), "Document Editor (Ribbon)");
        var document = Find(page, ControlType.Document)[0];
        var text = Pattern<TextPattern>(document, TextPattern.Pattern);
        double baseSize = defaultFontSize ?? Convert.ToDouble(text.DocumentRange.GetAttributeValue(TextPattern.FontSizeAttribute));
        var originalBounds = root.Current.BoundingRectangle;
        Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1400, 900);
        string fixture = System.IO.Path.GetFullPath(System.IO.Path.Combine(PersistenceOutput, "..", "..", "TestDocument.bin"));
        string title = "GacUI Remote Document \u2014 \u4e2d\u6587 \u03c0";
        for (int pass = 0; pass < 2; pass++)
        {
            if (pass == 1) PersistencePalette(hwnd, "Aurora");
            page = Page(Page(root, "Control"), "Document Editor (Ribbon)");
            PersistenceFile(page, true, "Load as Private Format", fixture, false);
            document = Find(page, ControlType.Document)[0];
            text = Pattern<TextPattern>(document, TextPattern.Pattern);
            Wait(() => text.DocumentRange.GetText(-1).Contains(title), "rich fixture loaded for gallery styles");
            foreach (string sample in new[] { title, "Self-contained persistence fixture", "Private save preserves text", "Right aligned / Consolas / orange italic", "Gaclib embedded image:", "Video embedded image:" })
            {
                var sampleRange = text.DocumentRange.FindText(sample, false, false);
                Check(sampleRange != null, "fixture paragraph is present: " + sample);
                int color = Convert.ToInt32(sampleRange.GetAttributeValue(TextPattern.ForegroundColorAttribute));
                int brightness = (color & 255) * 299 + ((color >> 8) & 255) * 587 + ((color >> 16) & 255) * 114;
                Check(brightness >= 160000, "fixture text is bright on the dark editor background");
            }
            string saved = System.IO.Path.Combine(folder, "style-catalog-" + pass + ".bin");
            PersistenceFile(page, true, "Save as Private Format", saved, false);
            checkCatalog(Encoding.UTF8.GetString(System.IO.File.ReadAllBytes(saved)), "private save before using gallery styles");
            // Verify file definitions in the toolstrip editor, which does not
            // install ribbon defaults and therefore cannot hide a missing catalog.
            var toolstrip = Page(Page(root, "Control"), "Document Editor (Toolstrip)");
            PersistenceFile(toolstrip, false, "Load as Private Format", saved, false);
            bool foundCatalog = false;
            foreach (var node in SnapshotNodes(ControlsSnapshot()))
            {
                object xml;
                if (!node.TryGetValue("elementDocument", out xml) || !((string)xml).Contains(title)) continue;
                checkCatalog((string)xml, "toolstrip loaded catalog");
                foundCatalog = true;
            }
            Check(foundCatalog, "loaded document catalog exposed");
            page = Page(Page(root, "Control"), "Document Editor (Ribbon)");
            document = Find(page, ControlType.Document)[0];
            text = Pattern<TextPattern>(document, TextPattern.Pattern);
            var styleLists = Find(page, ControlType.List).FindAll(candidate => Items(candidate).Count == names.Length);
            Check(styleLists.Count == 1, "one gallery exposes all eight built-in styles");
            var heading = text.DocumentRange.FindText(title, false, false);
            heading.Select();
            foreach (string mode in new[] { "Selectable", "Preview", "Editable" })
            {
                EditorCommand(page, document, true, mode, false, true);
                Check(styleLists[0].Current.IsEnabled == (mode == "Editable"), mode + " sets gallery editability");
                foreach (var item in Items(styleLists[0]))
                    Check(item.Current.IsEnabled == (mode == "Editable"), mode + " sets style item editability");
            }
            Action<int, TextPatternRange> checkStyle = (index, range) => {
                double expectedSize = Math.Floor(baseSize / 0.75 * factors[index]) * 0.75;
                int expectedColor = index < 2 ? 0x0080FF : index == 6 ? 0xFF8080 : 0xFFFFFF;
                int expectedWeight = index == 3 ? 700 : 400;
                bool expectedItalic = index >= 4 && index <= 6;
                bool expectedUnderline = index == 5 || index == 6;
                Wait(() => Math.Abs(Convert.ToDouble(range.GetAttributeValue(TextPattern.FontSizeAttribute)) - expectedSize) < 0.01
                    && Convert.ToInt32(range.GetAttributeValue(TextPattern.ForegroundColorAttribute)) == expectedColor
                    && Convert.ToInt32(range.GetAttributeValue(TextPattern.FontWeightAttribute)) == expectedWeight
                    && Convert.ToBoolean(range.GetAttributeValue(TextPattern.IsItalicAttribute)) == expectedItalic
                    && (Convert.ToInt32(range.GetAttributeValue(TextPattern.UnderlineStyleAttribute)) != 0) == expectedUnderline
                    && (Convert.ToInt32(range.GetAttributeValue(TextPattern.StrikethroughStyleAttribute)) != 0) == (index == 7),
                    names[index] + " applies its dedicated font, color and decorations");
                Check(text.GetSelection()[0].GetText(-1) == title, "gallery applies to the selected heading");
            };
            for (int index = 0; index < names.Length; index++)
            {
                var range = text.DocumentRange.FindText(title, false, false);
                Check(range != null, "fixture heading remains available");
                range.Select();
                var item = Items(styleLists[0])[index];
                if (pass == 0)
                {
                    Pattern<ScrollItemPattern>(item, ScrollItemPattern.Pattern).ScrollIntoView();
                    ControlsSnapshot();
                    ClickRenderedCaption(names[index]);
                }
                else Pattern<SelectionItemPattern>(item, SelectionItemPattern.Pattern).Select();
                checkStyle(index, range);
                Console.WriteLine("PASS style " + names[index] + " / " + (pass == 0 ? "automation pointer" : "UIA SelectionItem"));
            }
            foreach (int width in new[] { 1400, 650, 1250 })
            {
                Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(width, 900);
                ControlsSnapshot();
                bool collapsed = width == 650;
                AutomationElement gallery = null;
                if (!collapsed)
                {
                    // Responsive template replacement retires old UIA providers.
                    page = Page(Page(root, "Control"), "Document Editor (Ribbon)");
                    styleLists = Find(page, ControlType.List).FindAll(candidate => Items(candidate).Count == names.Length);
                    Check(styleLists.Count == 1, "resized ribbon exposes its inline gallery");
                    gallery = TreeWalker.RawViewWalker.GetParent(styleLists[0]);
                    object expand;
                    while (!gallery.TryGetCurrentPattern(ExpandCollapsePattern.Pattern, out expand)) gallery = TreeWalker.RawViewWalker.GetParent(gallery);
                }
                for (int index = 0; index < names.Length; index++)
                {
                    var range = text.DocumentRange.FindText(title, false, false);
                    range.Select();
                    ApplyPopupStyle(gallery, index, names[index], collapsed);
                    checkStyle(index, range);
                    if (index == 0)
                    {
                        ApplyPopupStyle(gallery, index, names[index], collapsed);
                        checkStyle(index, range);
                    }
                    Console.WriteLine("PASS popup style " + names[index] + " / width " + width + " / palette " + pass);
                }
            }
            Console.WriteLine("LIMITATION popup backgrounds expose unnamed UIA Toggle controls without Invoke; UIA locates and scrolls them, automation IO applies them.");
            page = Page(Page(root, "Control"), "Document Editor (Ribbon)");
            styleLists = Find(page, ControlType.List).FindAll(candidate => Items(candidate).Count == names.Length);
            var sampleGallery = TreeWalker.RawViewWalker.GetParent(styleLists[0]);
            object sampleExpansion;
            while (!sampleGallery.TryGetCurrentPattern(ExpandCollapsePattern.Pattern, out sampleExpansion)) sampleGallery = TreeWalker.RawViewWalker.GetParent(sampleGallery);
            string beforeSamples = EditorDocumentXml();
            foreach (string sample in new[] { "Item A1", "Item A2", "Item A3", "Item B1", "Item B2", "Item B3" })
            {
                Pattern<ExpandCollapsePattern>(sampleGallery, ExpandCollapsePattern.Pattern).Expand();
                Wait(() => StylePopupSnapshot() != null, "sample gallery popup opens");
                if (pass == 1)
                {
                    AutomationElement item = null;
                    Wait(() => {
                        foreach (var window in OwnedWindows())
                        {
                            item = FindNamed(window, sample, ControlType.MenuItem);
                            if (item != null && !item.Current.IsOffscreen) return true;
                        }
                        return false;
                    }, "sample menu item is exposed: " + sample);
                    Pattern<InvokePattern>(item, InvokePattern.Pattern).Invoke();
                }
                else
                {
                    var snapshot = StylePopupSnapshot();
                    bool clicked = false;
                    foreach (var node in SnapshotNodes(snapshot["composition"]))
                    {
                        object caption;
                        if (!node.TryGetValue("elementText", out caption) || (string)caption != sample) continue;
                        var bounds = SnapshotBounds(node);
                        string endpoint = inputEndpoint;
                        object windowId;
                        if (snapshot.TryGetValue("windowId", out windowId)) endpoint += "/" + (string)windowId;
                        Input("!LeftClick:" + (int)(bounds.Left + bounds.Width / 2) + "," + (int)(bounds.Top + bounds.Height / 2), endpoint);
                        clicked = true;
                        break;
                    }
                    Check(clicked, "sample menu label was rendered: " + sample);
                }
                Wait(() => StylePopupSnapshot() == null, "sample menu item dismisses its popup");
                Check(EditorDocumentXml() == beforeSamples, "sample entry has no document formatting command: " + sample);
                Console.WriteLine("PASS sample entry " + sample + " / " + (pass == 0 ? "automation pointer" : "UIA Invoke"));
            }
            string styled = System.IO.Path.Combine(folder, "styled-document-" + pass + ".bin");
            PersistenceFile(page, true, "Save as Private Format", styled, false);
            PersistenceFile(page, true, "Load as Private Format", styled, false);
            checkCatalog(Encoding.UTF8.GetString(System.IO.File.ReadAllBytes(styled)), "styled private file");
            var reloaded = text.DocumentRange.FindText(title, false, false);
            Check(Convert.ToInt32(reloaded.GetAttributeValue(TextPattern.StrikethroughStyleAttribute)) != 0, "applied style survives private reload");
        }
        Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(originalBounds.Width, originalBounds.Height);
    }
    static void DocumentExchange(IntPtr hwnd)
    {
        Test("Document exchange / both GUI editors preserve a supplied private document");
        Check(!string.IsNullOrEmpty(ExchangeDocument) && System.IO.File.Exists(ExchangeDocument), "exchange fixture exists");
        var original = System.IO.File.ReadAllBytes(ExchangeDocument);
        string serialized = Encoding.UTF8.GetString(original);
        int start = serialized.IndexOf("<Doc>", StringComparison.Ordinal);
        int end = serialized.IndexOf("</Doc>", StringComparison.Ordinal);
        Check(start >= 0 && end > start, "exchange fixture contains document XML");
        var expected = new System.Xml.XmlDocument();
        expected.LoadXml(serialized.Substring(start, end + 6 - start));
        string folder = System.IO.Path.Combine(PersistenceOutput, "Exchange-" + ownedProcess);
        System.IO.Directory.CreateDirectory(folder);
        for (int index = 0; index < 2; index++)
        {
            bool ribbon = index == 0;
            var page = Page(Page(root, "Control"), ribbon ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
            PersistenceFile(page, ribbon, "Load as Private Format", ExchangeDocument, false);
            var document = Find(page, ControlType.Document)[0];
            var text = Pattern<TextPattern>(document, TextPattern.Pattern);
            foreach (System.Xml.XmlNode node in expected.SelectNodes("/Doc/Content//text()"))
                Check(text.DocumentRange.GetText(-1).Contains(node.Value), "exchange text is present: " + node.Value);
            string before = text.DocumentRange.GetText(-1);
            document.SetFocus();
            Press("Ctrl+End");
            Input("!Type:\u00a7");
            Wait(() => text.DocumentRange.GetText(-1) == before + "\u00a7", "exchange document edit");
            text.DocumentRange.FindText("\u00a7", true, false).Select();
            PersistencePalette(hwnd, ribbon ? "Aurora" : "Default");
            page = Page(Page(root, "Control"), ribbon ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
            document = Find(page, ControlType.Document)[0];
            text = Pattern<TextPattern>(document, TextPattern.Pattern);
            Check(text.DocumentRange.GetText(-1) == before + "\u00a7" && text.GetSelection()[0].GetText(-1) == "\u00a7", "theme refresh preserves edited document and selection");
            if (ribbon) { document.SetFocus(); Press("Ctrl+Z"); }
            else EditorCommand(page, document, false, "Undo", false, true);
            Wait(() => text.DocumentRange.GetText(-1) == before, "theme refresh preserves document undo history");
            string saved = System.IO.Path.Combine(folder, "editor-" + index + ".bin");
            PersistenceFile(page, ribbon, "Save as Private Format", saved, false);
            Check(Convert.ToBase64String(original) == Convert.ToBase64String(System.IO.File.ReadAllBytes(saved)), "exchange preserves complete model and embedded image bytes in editor " + index);
            Console.WriteLine("PASS exchange / " + (ribbon ? "Ribbon" : "Toolstrip") + " / " + saved);
        }
    }

    static void DocumentPersistence(IntPtr hwnd)
    {
        Test("Document persistence / independent viewers, private round trip, RTF, HTML and cancellation");
        string folder = System.IO.Path.Combine(PersistenceOutput, "UIA-" + ownedProcess);
        System.IO.Directory.CreateDirectory(folder);
        var paths = new string[2];
        var contents = new[] { "Ribbon persistence Unicode \u4E2D\u6587", "Toolstrip independent document \u03C0" };
        for (int pass = 0; pass < 2; pass++)
        {
            if (pass == 1) PersistencePalette(hwnd, "Aurora");
            for (int index = 0; index < 2; index++)
            {
                bool ribbon = index == 0;
                var page = Page(Page(root, "Control"), ribbon ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
                var document = Find(page, ControlType.Document)[0];
                var text = Pattern<TextPattern>(document, TextPattern.Pattern);
                document.SetFocus();
                text.DocumentRange.Select();
                Input("!Type:" + contents[index]);
                Wait(() => text.DocumentRange.GetText(-1) == contents[index], "seed independent editor " + index);
                string stem = System.IO.Path.Combine(folder, "editor-" + index + "-" + pass);
                paths[index] = stem + ".bin";
                PersistenceFile(page, ribbon, "Save as Private Format", paths[index], false);
                Check(System.IO.File.ReadAllBytes(paths[index]).Length > 0, "private bytes written before command returns");
                PersistenceFile(page, ribbon, "Save as RTF", stem + ".rtf", false);
                string rtf = System.IO.File.ReadAllText(stem + ".rtf");
                var encodedText = new StringBuilder();
                foreach (char character in contents[index]) encodedText.Append("\\u").Append((int)character);
                Check(rtf.StartsWith("{\\rtf") && rtf.Contains(encodedText.ToString()), "RTF has document content and Unicode");
                PersistenceFile(page, ribbon, "Save as HTML", stem + ".html", false);
                string html = System.IO.File.ReadAllText(stem + ".html", Encoding.UTF8);
                Check(System.Net.WebUtility.HtmlDecode(html).Replace('\u00a0', ' ').Contains(contents[index]) && html.Contains("<"), "HTML has Unicode content and markup");
                document.SetFocus(); text.DocumentRange.Select(); Input("!Type:changed after save");
                Wait(() => text.DocumentRange.GetText(-1) == "changed after save", "mutate editor before reload");
                PersistenceFile(page, ribbon, "Load as Private Format", paths[index], false);
                Wait(() => text.DocumentRange.GetText(-1) == contents[index], "private reload restores matching editor");
                var original = System.IO.File.ReadAllBytes(paths[index]);
                PersistenceFile(page, ribbon, "Save as Private Format", paths[index], true);
                PersistenceFile(page, ribbon, "Load as Private Format", paths[index], true);
                Check(text.DocumentRange.GetText(-1) == contents[index], "canceled dialogs preserve the document");
                Check(Convert.ToBase64String(original) == Convert.ToBase64String(System.IO.File.ReadAllBytes(paths[index])), "canceled save preserves output bytes");
            }
            for (int index = 0; index < 2; index++)
            {
                var page = Page(Page(root, "Control"), index == 0 ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
                PersistenceFile(page, index == 0, "Load as Private Format", paths[1 - index], false);
                var text = Pattern<TextPattern>(Find(page, ControlType.Document)[0], TextPattern.Pattern);
                Wait(() => text.DocumentRange.GetText(-1) == contents[1 - index], "cross-editor private file load");
            }
        }
        DocumentStyles(hwnd, folder);
        Console.WriteLine("Persistence outputs: " + folder);
        // Leave the showcase's original empty editors for later All scenarios.
        for (int index = 0; index < 2; index++)
        {
            var page = Page(Page(root, "Control"), index == 0 ? "Document Editor (Ribbon)" : "Document Editor (Toolstrip)");
            var document = Find(page, ControlType.Document)[0];
            document.SetFocus();
            var text = Pattern<TextPattern>(document, TextPattern.Pattern);
            text.DocumentRange.Select();
            Press("Backspace");
            Wait(() => text.DocumentRange.GetText(-1) == "", "clear persistence test editor");
        }
        PersistencePalette(hwnd, "Default");
    }
    static void Calendars()
    {
        Test("Misc / DatePicker: selection and calendar grid/table");
        var page = Page(Page(root, "Misc"), "DatePicker");
        var calendars = Find(page, ControlType.Calendar); Check(calendars.Count == 2, "two calendars");
        foreach (var calendar in calendars)
        {
            var grid = Pattern<GridPattern>(calendar, GridPattern.Pattern); Check(grid.Current.RowCount == 6 && grid.Current.ColumnCount == 7, "calendar dimensions");
            Check(Pattern<TablePattern>(calendar, TablePattern.Pattern).Current.GetColumnHeaders().Length == 7, "weekday headers");
            var day = grid.GetItem(2,2); var selection = Pattern<SelectionItemPattern>(day, SelectionItemPattern.Pattern); selection.Select();
            Check(selection.Current.IsSelected, "calendar day selected"); Same(selection.Current.SelectionContainer, calendar, "calendar container");
            day.SetFocus();
            Check(day.Current.HasKeyboardFocus, "calendar logical focus");
            CheckFocus(day, "calendar focus");
        }
    }
    static readonly string[] ResponsiveAccountNames = {
        "Penguin Random House", "HarperCollins", "Simon & Schuster", "Hachette Book Group",
        "Macmillan Publishers", "Scholastic", "Bloomsbury", "Oxford University Press"
    };
    static void ResponsiveRegressions()
    {
        Test("responsive regressions / ribbon document");
        var ribbon = Page(Page(root, "Control"), "Document Editor (Ribbon)");
        var search = Find(ribbon, ControlType.Edit)[0];
        Pattern<ValuePattern>(search, ValuePattern.Pattern).SetValue("responsive resize");
        var document = Find(ribbon, ControlType.Document)[0];
        var text = Pattern<TextPattern>(document, TextPattern.Pattern).DocumentRange.GetText(-1);
        var original = root.Current.BoundingRectangle;
        var transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        foreach (var width in new[] { 1000, 750, 550, 1000 })
        {
            transform.Resize(width, original.Height);
            Wait(() => document.Current.BoundingRectangle.Width > 0 && !document.Current.IsOffscreen, "ribbon document survives resize");
            Check(Pattern<ValuePattern>(search, ValuePattern.Pattern).Current.Value == "responsive resize", "ribbon search survives resize");
            Check(Pattern<TextPattern>(document, TextPattern.Pattern).DocumentRange.GetText(-1) == text, "ribbon text survives resize");
            Wait(() => {
                var home = FindNamed(ribbon, "  HOME  ", ControlType.Button);
                return home != null && home.Current.BoundingRectangle.Width > 0 && !home.Current.IsOffscreen;
            }, "ribbon header remains visible");
        }
        transform.Resize(original.Width, original.Height);

        Test("responsive regressions / view, stack and group levels");
        var page = Page(Page(root, "Layout"), "Responsive");
        var names = new[] { "GuiResponsiveViewComposition", "GuiResponsiveStackComposition", "GuiResponsiveGroupComposition" };
        var highest = new[] { 3, 6, 3 };
        for (int i = 0; i < names.Length; i++)
        {
            var group = Named(page, names[i], ControlType.Group);
            for (int level = highest[i] - 1; level >= 0; level--)
            {
                Invoke(group, "LevelDown();");
                int expected = level;
                Wait(() => FindNamed(group, "CurrentLevel: " + expected, ControlType.Text) != null, "responsive level down " + names[i]);
            }
            for (int level = 1; level <= highest[i]; level++)
            {
                Invoke(group, "LevelUp();");
                int expected = level;
                Wait(() => FindNamed(group, "CurrentLevel: " + expected, ControlType.Text) != null, "responsive level up " + names[i]);
            }
        }
    }

    static void ResponsiveLayout(bool includePalette = true)
    {
        Test("Layout / vertical headers and responsive modal action");
        if (isWorkflowBinary) Console.WriteLine("COVERAGE LIMIT: Workflow UIA tab items expose page-content bounds; header geometry uses Controls. C++ consumers require UIA header bounds.");
        var layout = Page(root, "Layout");
        layout = Named(root, "Layout", ControlType.TabItem);
        var headers = new[] { "Repeat", "Responsive", "Easy Layout", "Easy Layout (Table)" };
        var previous = System.Windows.Rect.Empty;
        foreach (var name in headers)
        {
            var header = Named(layout, name, ControlType.TabItem);
            Wait(() => LayoutHeaderBounds(layout, name).Width > 0 && LayoutHeaderBounds(layout, name).Height > 0, "settled Layout header " + name);
            var bounds = LayoutHeaderBounds(layout, name);
            Check(bounds.Width > 0 && bounds.Height > 0, "visible Layout header " + name);
            if (!previous.IsEmpty) Check(bounds.Top >= previous.Bottom && Math.Abs(bounds.Left - previous.Left) <= 2, "vertical header order " + name);
            previous = bounds;
            if (includePalette)
            {
                Pattern<SelectionItemPattern>(header, SelectionItemPattern.Pattern).Select();
                Check(Pattern<SelectionItemPattern>(header, SelectionItemPattern.Pattern).Current.IsSelected, "Layout selection " + name);
            }
        }
        var launch = Named(layout, "Easy Layout (Responsive)", ControlType.Button);
        Absent(launch, SelectionItemPattern.Pattern);
        double scale = GetDpiForWindow(new IntPtr(root.Current.NativeWindowHandle)) / 96.0;
        Check(Math.Abs(launch.Current.BoundingRectangle.Top - previous.Bottom - 5 * scale) <= scale, "five-pixel launch gap");
        Check(FindNamed(layout, "Easy Layout (Responsive)", ControlType.TabItem) == null, "responsive action is outside the page collection");
        var lastHeader = Named(layout, "Easy Layout (Table)", ControlType.TabItem);
        var nestedTab = Pattern<SelectionItemPattern>(lastHeader, SelectionItemPattern.Pattern).Current.SelectionContainer;
        Check(nestedTab.Current.Orientation == OrientationType.Vertical, "vertical tab orientation");
        Check(lastHeader.Current.AccessKey == "T" && launch.Current.AccessKey == "W", "header and launch ALT keys retained");
        if (includePalette)
        {
            nestedTab.SetFocus();
            Press("Up");
            Wait(() => Pattern<SelectionItemPattern>(Named(layout, "Easy Layout", ControlType.TabItem), SelectionItemPattern.Pattern).Current.IsSelected, "Up follows vertical tab order");
            Press("Down");
            Wait(() => Pattern<SelectionItemPattern>(lastHeader, SelectionItemPattern.Pattern).Current.IsSelected, "Down follows vertical tab order");
            Press("Alt");
            PressRenderedAlt("CheckBox", "Easy Layout");
            Wait(() => Pattern<SelectionItemPattern>(Named(layout, "Easy Layout", ControlType.TabItem), SelectionItemPattern.Pattern).Current.IsSelected, "ALT selects a vertical header");
            Page(layout, "Easy Layout (Table)");
        }
        else Page(layout, "Responsive");

        foreach (var close in new[] { "OK", "Cancel", "Title bar" })
        {
            Test("responsive modal / " + close);
            if (close == "Cancel")
            {
                Press("Alt");
                PressRenderedAlt("Button", "Easy Layout (Responsive)");
            }
            else Pattern<InvokePattern>(launch, InvokePattern.Pattern).Invoke();
            AutomationElement dialog = null;
            Wait(() => { dialog = ResponsiveDialog(); return dialog != null; }, "one responsive modal opens");
            Check(Pattern<WindowPattern>(dialog, WindowPattern.Pattern).Current.IsModal, "responsive window is modal");
            Check(!root.Current.IsEnabled, "owner disabled while modal is open");
            Wait(() => { var client = SnapshotBounds(ResponsiveSnapshot()); return client.Width == 240 && client.Height == 320; }, "initial 240 by 320 client area");
            dialog = SettledResponsiveDialog();
            Check(Pattern<TransformPattern>(dialog, TransformPattern.Pattern).Current.CanResize, "responsive modal can resize");
            CheckResponsiveAppearance(dialog, 0);

            if (close == "OK")
            {
                var list = Named(dialog, "Movies", ControlType.List);
                double compactMovieHeight = ResponsiveMovieRowHeight(dialog);
                Console.WriteLine("COMPACT MOVIE HEIGHT " + compactMovieHeight);
                CheckRejectedScrollRequests(list);
                var items = Items(list);
                Check(items.Count == 24, "real movie list exposes all 24 items");
                Check(items[0].Current.Name == "Jeanne Dielman, 23 quai du Commerce, 1080 Bruxelles" && items[1].Current.Name == "Vertigo" && items[23].Current.Name == "Do the Right Thing", "movie sample uses the BFI critics' poll");
                Pattern<SelectionItemPattern>(items[1], SelectionItemPattern.Pattern).Select();
                var menu = Pattern<ExpandCollapsePattern>(Named(dialog, "Subscribed Accounts", ControlType.MenuItem), ExpandCollapsePattern.Pattern);
                menu.Expand();
                AutomationElement accountPopup = null;
                Wait(() => { accountPopup = Popup(ControlType.Menu); return accountPopup != null; }, "account menu opens");
                Names(Find(accountPopup, ControlType.MenuItem), ResponsiveAccountNames);
                Pattern<InvokePattern>(Named(accountPopup, "HarperCollins", ControlType.MenuItem), InvokePattern.Pattern).Invoke();
                Wait(() => menu.Current.ExpandCollapseState == ExpandCollapseState.Collapsed, "account choice closes menu");
                string selectedAccount = "HarperCollins";
                // Direct compact/detailed jumps must settle without an additional resize.
                foreach (var size in new[] { new[] { 680, 320, 3 }, new[] { 240, 320, 0 }, new[] { 350, 320, 1 }, new[] { 440, 320, 2 }, new[] { 680, 320, 3 }, new[] { 680, 220, 3 }, new[] { 680, 520, 3 }, new[] { 440, 320, 2 }, new[] { 350, 320, 1 }, new[] { 240, 320, 0 }, new[] { 350, 320, 1 }, new[] { 240, 320, 0 } })
                {
                    ResizeResponsive(dialog, size[0], size[1]);
                    CheckResponsiveAppearance(dialog, size[2]);
                    Wait(() => size[2] == 0
                        ? Math.Abs(ResponsiveMovieRowHeight(dialog) - compactMovieHeight) < 1
                        : ResponsiveMovieRowHeight(dialog) > compactMovieHeight,
                        "movie row height follows responsive appearance #" + size[2]);
                    Console.WriteLine("MOVIE HEIGHT #" + size[2] + " " + ResponsiveMovieRowHeight(dialog));
                    list = Named(dialog, "Movies", ControlType.List);
                    var selected = Pattern<SelectionPattern>(list, SelectionPattern.Pattern).Current.GetSelection();
                    Check(selected.Length == 1 && selected[0].Current.Name == "Vertigo", "movie selection survives view change");
                    var scroll = Pattern<ScrollPattern>(list, ScrollPattern.Pattern);
                    scroll.SetScrollPercent(ScrollPattern.NoScroll, 100);
                    Wait(() => !Named(list, "Do the Right Thing", ControlType.ListItem).Current.IsOffscreen, "last movie is realized after scrolling down");
                    CheckResponsiveAppearance(dialog, size[2]);
                    Check(scroll.Current.VerticalScrollPercent > 0, "movie list scrolls at every appearance");
                    scroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
                    Wait(() => !Named(list, "Jeanne Dielman, 23 quai du Commerce, 1080 Bruxelles", ControlType.ListItem).Current.IsOffscreen, "first movie is realized after scrolling back");
                    CheckResponsiveAppearance(dialog, size[2]);
                    if (size[2] >= 2)
                    {
                        var accounts = Named(dialog, "Accounts", ControlType.List);
                        Names(Items(accounts), ResponsiveAccountNames);
                        var accountSelection = Pattern<SelectionPattern>(accounts, SelectionPattern.Pattern);
                        var selectedAccounts = accountSelection.Current.GetSelection();
                        Check(selectedAccounts.Length == 1 && selectedAccounts[0].Current.Name == selectedAccount, "account selection survives navigation changes");
                        var accountScroll = Pattern<ScrollPattern>(accounts, ScrollPattern.Pattern);
                        if (size[1] == 220) Check(accountScroll.Current.VerticallyScrollable, "short account list needs its scrollbar");
                        if (size[1] == 520) Check(!accountScroll.Current.VerticallyScrollable, "tall account list hides its scrollbar");
                        if (accountScroll.Current.VerticallyScrollable)
                        {
                            accountScroll.SetScrollPercent(ScrollPattern.NoScroll, 100);
                            Wait(() => !Named(accounts, "Oxford University Press", ControlType.ListItem).Current.IsOffscreen, "last account is realized after scrolling down");
                            Check(accountScroll.Current.VerticalScrollPercent > 0, "account list scrolls independently");
                            CheckResponsiveAppearance(dialog, size[2]);
                            accountScroll.SetScrollPercent(ScrollPattern.NoScroll, 0);
                            Wait(() => !Named(accounts, "Penguin Random House", ControlType.ListItem).Current.IsOffscreen, "first account is realized after scrolling back");
                            CheckResponsiveAppearance(dialog, size[2]);
                        }
                        if (size[1] == 520)
                        {
                            CheckRejectedScrollRequests(accounts);
                            selectedAccount = "Simon & Schuster";
                            Pattern<SelectionItemPattern>(Named(accounts, selectedAccount, ControlType.ListItem), SelectionItemPattern.Pattern).Select();
                            Wait(() => { var current = accountSelection.Current.GetSelection(); return current.Length == 1 && current[0].Current.Name == selectedAccount; }, "direct account list selection replaces the menu selection");
                        }
                    }
                    // Leave the last movie realized for the next resize and row-height transition.
                    scroll.SetScrollPercent(ScrollPattern.NoScroll, 100);
                    Wait(() => !Named(list, "Do the Right Thing", ControlType.ListItem).Current.IsOffscreen, "resize starts near the end of the movie list");
                }
                menu = Pattern<ExpandCollapsePattern>(Named(dialog, "Subscribed Accounts", ControlType.MenuItem), ExpandCollapsePattern.Pattern);
                menu.Expand();
                Wait(() => { accountPopup = Popup(ControlType.Menu); return accountPopup != null; }, "account menu reopens after list selection");
                Names(Find(accountPopup, ControlType.MenuItem), ResponsiveAccountNames);
                Pattern<InvokePattern>(Named(accountPopup, selectedAccount, ControlType.MenuItem), InvokePattern.Pattern).Invoke();
                Wait(() => menu.Current.ExpandCollapseState == ExpandCollapseState.Collapsed, "invoking the selected account closes the menu");
                ResizeResponsive(dialog, 680, 320);
                CheckResponsiveAppearance(dialog, 3);
                var finalAccounts = Pattern<SelectionPattern>(Named(dialog, "Accounts", ControlType.List), SelectionPattern.Pattern).Current.GetSelection();
                Check(finalAccounts.Length == 1 && finalAccounts[0].Current.Name == selectedAccount, "reinvoking the selected account preserves one selection");
            }
            if (close == "Title bar") CloseResponsiveTitleBar();
            else Invoke(dialog, close);
            Wait(() => ResponsiveDialog() == null && root.Current.IsEnabled, "modal closes and owner input returns");
            layout = Named(root, "Layout", ControlType.TabItem);
            launch = Named(layout, "Easy Layout (Responsive)", ControlType.Button);
        }
        if (includePalette) ResponsivePalette();
    }

    static void CloseResponsiveTitleBar()
    {
        var window = ResponsiveSnapshot();
        var bounds = SnapshotBounds(window);
        var close = System.Windows.Rect.Empty;
        foreach (var node in SnapshotNodes(window["composition"]))
        {
            object control;
            if (!node.TryGetValue("control", out control) || (string)control != "Button") continue;
            var candidate = SnapshotBounds(node);
            if (candidate.Width > 0 && candidate.Top < bounds.Top + 40 && (close.IsEmpty || candidate.Right > close.Right)) close = candidate;
        }
        Check(!close.IsEmpty, "rendered title-bar close button");
        if (isHosted)
        {
            Input("!LeftClick:" + (int)(close.Left + close.Width / 2) + "," + (int)(close.Top + close.Height / 2));
        }
        else
        {
            // Native caption hit testing is handled by Windows, outside automation IO.
            var hwnd = new IntPtr(ResponsiveDialog().Current.NativeWindowHandle);
            Check(hwnd != IntPtr.Zero && PostMessage(hwnd, 0x0112, new IntPtr(0xF060), IntPtr.Zero), "native caption SC_CLOSE accepted");
        }
    }

    static System.Windows.Rect LayoutHeaderBounds(AutomationElement layout, string name)
    {
        if (!isWorkflowBinary) return Named(layout, name, ControlType.TabItem).Current.BoundingRectangle;
        var bounds = SnapshotBounds(SnapshotControl(ControlsSnapshot(), "CheckBox", name));
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        double scale = GetDpiForWindow(hwnd) / 96.0;
        var origin = new NativePoint();
        Check(ClientToScreen(hwnd, ref origin), "convert Workflow header bounds to screen");
        return new System.Windows.Rect(origin.X + bounds.X * scale, origin.Y + bounds.Y * scale, bounds.Width * scale, bounds.Height * scale);
    }

    static void PressRenderedAlt(string theme, string caption)
    {
        string key = null;
        Wait(() =>
        {
            var control = SnapshotControl(ControlsSnapshot(), theme, caption);
            foreach (var node in SnapshotNodes(control))
            {
                object role;
                if (!node.TryGetValue("control", out role) || (string)role != "ShortcutKey") continue;
                foreach (var label in SnapshotNodes(node))
                {
                    object text;
                    if (label.TryGetValue("elementText", out text)) { key = ((string)text).Replace("[", "").Replace("]", ""); return key.Length > 0; }
                }
            }
            return false;
        }, "rendered ALT sequence for " + caption);
        Console.WriteLine("ALT " + caption + " = " + key);
        foreach (char letter in key) Press(letter.ToString());
    }

    static bool SnapshotHas(object tree, string key, string expected)
    {
        foreach (var node in SnapshotNodes(tree))
        {
            object value;
            if (node.TryGetValue(key, out value) && (string)value == expected) return true;
        }
        return false;
    }

    static Dictionary<string, object> SnapshotControl(object tree, string theme, string text)
    {
        foreach (var node in SnapshotNodes(tree))
        {
            object control;
            if (node.TryGetValue("control", out control) && (string)control == theme && SnapshotHas(node, "elementText", text)) return node;
        }
        throw new Exception("Missing rendered " + theme + ": " + text);
    }

    static bool PaletteAttachmentRetired(AutomationElement radio)
    {
        try { var name = radio.Current.Name; return false; }
        catch (ElementNotAvailableException) { return true; }
        // UIA can also report a COM failure when a template provider is retired.
        // Callers must reacquire the window and verify its rebuilt controls.
        catch (COMException error) { Console.WriteLine("NOTE retired palette provider: " + error.ErrorCode.ToString("X8")); return true; }
    }

    static void SkinItems(AutomationElement manager, string selected)
    {
        var names = new[] { "Default", "Aurora", "Ember", "Moonstone", "Lagoon", "Rosewood" };
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        var radios = new List<AutomationElement>();
        var previous = new List<System.Windows.Rect>();
        int stable = 0;
        Wait(() => {
            Thread.Sleep(100);
            root = AutomationElement.FromHandle(hwnd);
            manager = Named(root, "Window Manager", ControlType.TabItem);
            radios = Find(manager, ControlType.RadioButton);
            if (radios.Count != names.Length) return false;
            var current = radios.ConvertAll(radio => radio.Current.BoundingRectangle);
            bool settled = previous.Count == current.Count;
            for (int i = 0; i < current.Count; i++)
                settled &= !current[i].IsEmpty && current[i].Width > 0 && current[i].Height > 0 && previous.Count == current.Count && previous[i] == current[i];
            stable = settled ? stable + 1 : 0;
            previous = current;
            return stable >= 3;
        }, "repeated skin item bounds settle", 30000);
        Check(radios.Count == names.Length, "repeated skin item count");
        int checkedCount = 0;
        for (int i = 0; i < names.Length; i++)
        {
            Check(radios[i].Current.Name == names[i], "repeated skin item order " + i);
            bool isSelected = Pattern<SelectionItemPattern>(radios[i], SelectionItemPattern.Pattern).Current.IsSelected;
            Check(isSelected == (names[i] == selected), "skin item checked state " + names[i]);
            if (isSelected) checkedCount++;
            if (i > 0) Check(radios[i].Current.BoundingRectangle.Top > radios[i - 1].Current.BoundingRectangle.Top, "skin items remain vertical");
        }
        Check(checkedCount == 1, "skin items share one selection group");
    }

    static void SkinRefreshListState()
    {
        Test("skin refresh / hidden list selection and scrolling");
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        var page = Page(Page(root, "List"), "TextList");
        var lists = Find(page, ControlType.List);
        Check(lists.Count == 2 && Items(lists[0]).Count == 0, "fresh paired TextLists");
        for (int i = 0; i < 5; i++) Invoke(page, "Add 10 items");
        Wait(() => Items(lists[0]).Count == 50 && Items(lists[1]).Count == 50, "both lists populated for scrolling");
        var selected = new[] { "13", "37" };
        var positions = new double[2];
        for (int i = 0; i < lists.Count; i++)
        {
            Pattern<SelectionItemPattern>(Named(lists[i], selected[i], ControlType.ListItem), SelectionItemPattern.Pattern).Select();
            var scroll = Pattern<ScrollPattern>(lists[i], ScrollPattern.Pattern);
            Check(scroll.Current.VerticallyScrollable, "populated list scrolls " + i);
            scroll.SetScrollPercent(ScrollPattern.NoScroll, i == 0 ? 50 : 70);
            positions[i] = scroll.Current.VerticalScrollPercent;
            Check(positions[i] > 0, "retained list offset starts away from zero " + i);
            Console.WriteLine("LIST BEFORE " + i + " selected=" + selected[i] + " scroll=" + positions[i]);
        }
        foreach (var palette in new[] { "Aurora", "Default" })
        {
            var radio = Named(Page(root, "Window Manager"), palette, ControlType.RadioButton);
            Pattern<SelectionItemPattern>(radio, SelectionItemPattern.Pattern).Select();
            Wait(() => PaletteAttachmentRetired(radio), "list palette replaced", 30000);
            Wait(() => {
                root = AutomationElement.FromHandle(hwnd);
                return FindNamed(root, "Window Manager", ControlType.TabItem) != null;
            }, "list palette window rebuilt", 30000);
            SkinItems(Page(root, "Window Manager"), palette);
            page = Page(Page(root, "List"), "TextList");
            lists = Find(page, ControlType.List);
            Check(lists.Count == 2 && Items(lists[0]).Count == 50 && Items(lists[1]).Count == 50, "both list models survive " + palette);
            for (int i = 0; i < lists.Count; i++)
            {
                var list = lists[i];
                int index = i;
                string lastState = null;
                Wait(() => {
                    var selection = Pattern<SelectionPattern>(list, SelectionPattern.Pattern).Current.GetSelection();
                    double position = Pattern<ScrollPattern>(list, ScrollPattern.Pattern).Current.VerticalScrollPercent;
                    string state = selection.Length + ":" + (selection.Length == 1 ? selection[0].Current.Name : "") + ":" + position;
                    if (lastState != state) { Console.WriteLine("LIST AFTER " + palette + " " + index + " " + state); lastState = state; }
                    return selection.Length == 1 && selection[0].Current.Name == selected[index] && Math.Abs(position - positions[index]) < 1;
                }, "hidden list selection and scroll retained after " + palette + " / " + i, 30000);
            }
        }
    }

    static void ResponsivePalette()
    {
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        var tablePage = Page(Page(root, "Layout"), "Easy Layout (Table)");
        var editor = Pattern<ValuePattern>(Find(tablePage, ControlType.Edit)[0], ValuePattern.Pattern);
        string originalText = editor.Current.Value;
        editor.SetValue("Vertical template retained state");
        foreach (var colors in new[] {
            new[] { "Default", "#007ACCFF", "#1C97EAFF", "#3399FFFF" },
            new[] { "Ember", "#B66332FF", "#D2834AFF", "#C77540FF" },
            new[] { "Aurora", "#168A7AFF", "#31AA97FF", "#299C89FF" },
            new[] { "Default", "#007ACCFF", "#1C97EAFF", "#3399FFFF" }
        })
        {
            Test("vertical template palette / " + colors[0]);
            Input("!MouseMove:0,0");
            var manager = Page(root, "Window Manager");
            var radio = Named(manager, colors[0], ControlType.RadioButton);
            var selection = Pattern<SelectionItemPattern>(radio, SelectionItemPattern.Pattern);
            bool changed = !selection.Current.IsSelected;
            selection.Select();
            if (changed) Wait(() => PaletteAttachmentRetired(radio), "palette replaced");
            root = AutomationElement.FromHandle(hwnd);
            var layout = Page(root, "Layout");
            // A Debug palette refresh rebuilds the complete showcase before laying out its new templates.
            Wait(() => {
                // Leave time for layout between full-tree snapshots on Debug builds.
                Thread.Sleep(250);
                return SnapshotBounds(SnapshotControl(ControlsSnapshot(), "CheckBox", "Repeat")).Width > 0;
            }, "palette header layout settles", 60000);
            var selected = Named(layout, "Easy Layout (Table)", ControlType.TabItem);
            Check(Pattern<SelectionItemPattern>(selected, SelectionItemPattern.Pattern).Current.IsSelected, "vertical template refresh retains selected page");
            Check(Pattern<ValuePattern>(Find(selected, ControlType.Edit)[0], ValuePattern.Pattern).Current.Value == "Vertical template retained state", "vertical template refresh retains editor state");
            var tab = SnapshotControl(ControlsSnapshot(), "RibbonTab", "Easy Layout (Responsive)");
            Check(SnapshotHas(tab, "element", "Background:#2D2D30FF,Rectangle") && SnapshotHas(tab, "element", "Border:#434346FF,Rectangle"), "vertical template background and border");
            Wait(() => SnapshotHas(SnapshotControl(ControlsSnapshot(), "CheckBox", "Easy Layout (Table)"), "element", "Background:" + colors[1] + ",Rectangle"), "selected header uses current palette before hover");
            var lastHoverBounds = System.Windows.Rect.Empty;
            Wait(() => {
                var repeat = SnapshotBounds(SnapshotControl(ControlsSnapshot(), "CheckBox", "Repeat"));
                if (repeat != lastHoverBounds) { Console.WriteLine("HOVER " + colors[0] + " " + repeat); lastHoverBounds = repeat; }
                if (repeat.Width <= 0 || repeat.Height <= 0) return false;
                Input("!MouseMove:" + (int)(repeat.Left + repeat.Width / 2) + "," + (int)(repeat.Top + repeat.Height / 2));
                return SnapshotHas(SnapshotControl(ControlsSnapshot(), "CheckBox", "Repeat"), "element", "Background:" + colors[2] + ",Rectangle");
            }, "hovered header uses current palette");
            var launch = Named(layout, "Easy Layout (Responsive)", ControlType.Button);
            double scale = GetDpiForWindow(hwnd) / 96.0;
            Check(Math.Abs(launch.Current.BoundingRectangle.Top - LayoutHeaderBounds(layout, "Easy Layout (Table)").Bottom - 5 * scale) <= scale, "launch gap survives palette refresh");
            Pattern<InvokePattern>(launch, InvokePattern.Pattern).Invoke();
            AutomationElement dialog = null;
            Wait(() => { dialog = ResponsiveDialog(); return dialog != null; }, "modal opens under current palette");
            dialog = SettledResponsiveDialog();
            CheckResponsiveAppearance(dialog, 0);
            Check(SnapshotHas(ResponsiveSnapshot(), "element", "Background:#252526FF,Rectangle"), "modal list uses current content background");
            ResizeResponsive(dialog, 680, 320);
            CheckResponsiveAppearance(dialog, 3);
            var accounts = Named(dialog, "Accounts", ControlType.List);
            var selectedAccounts = Pattern<SelectionPattern>(accounts, SelectionPattern.Pattern).Current.GetSelection();
            Check(selectedAccounts.Length == 1 && selectedAccounts[0].Current.Name == "Penguin Random House", "fresh account list selects its first publisher");
            var movieNames = new HashSet<string>();
            foreach (var item in Items(Named(dialog, "Movies", ControlType.List))) movieNames.Add(item.Current.Name);
            var accountList = FindResponsiveList(ResponsiveSnapshot(), movieNames, false);
            Check(accountList != null && SnapshotHas(accountList, "element", "Background:" + colors[3] + ",Rectangle"), "selected account row uses current palette");
            Invoke(dialog, "Cancel");
            Wait(() => ResponsiveDialog() == null && root.Current.IsEnabled, "palette modal restores owner");
        }
        tablePage = Page(Page(root, "Layout"), "Easy Layout (Table)");
        Pattern<ValuePattern>(Find(tablePage, ControlType.Edit)[0], ValuePattern.Pattern).SetValue(originalText);
    }

    static AutomationElement ResponsiveDialog()
    {
        var windows = isHosted ? Find(root, ControlType.Window) : OwnedWindows();
        AutomationElement found = null;
        foreach (var window in windows)
        {
            string name;
            try { name = window.Current.Name; }
            catch (ElementNotAvailableException) { continue; } // A closing HWND can disappear after enumeration.
            if (name != "Easy Layout (Responsive)") continue;
            Check(found == null, "only one responsive modal is active");
            found = window;
        }
        return found;
    }

    static AutomationElement SettledResponsiveDialog()
    {
        AutomationElement dialog = null;
        Wait(() => {
            dialog = ResponsiveDialog();
            if (dialog == null) return false;
            var ok = FindNamed(dialog, "OK", ControlType.Button);
            var cancel = FindNamed(dialog, "Cancel", ControlType.Button);
            return ok != null && cancel != null && !ok.Current.BoundingRectangle.IsEmpty && !cancel.Current.BoundingRectangle.IsEmpty;
        }, "responsive modal publishes its laid-out UIA controls");
        return dialog;
    }

    static IEnumerable<Dictionary<string, object>> SnapshotNodes(object value)
    {
        var map = value as Dictionary<string, object>;
        if (map != null)
        {
            yield return map;
            foreach (var child in map.Values) foreach (var node in SnapshotNodes(child)) yield return node;
        }
        else
        {
            var array = value as object[];
            if (array != null) foreach (var child in array) foreach (var node in SnapshotNodes(child)) yield return node;
        }
    }

    static Dictionary<string, object> ResponsiveSnapshot()
    {
        foreach (var node in SnapshotNodes(ControlsSnapshot()))
        {
            object title;
            if (node.TryGetValue("title", out title) && (string)title == "Easy Layout (Responsive)") return node;
        }
        throw new Exception("Responsive window missing from Controls");
    }

    static Dictionary<string, object> ControlsSnapshot()
    {
        using (var client = new System.Net.WebClient())
        {
            client.Encoding = Encoding.UTF8;
            var json = new JavaScriptSerializer { MaxJsonLength = 16 * 1024 * 1024, RecursionLimit = 512 };
            return (Dictionary<string, object>)json.DeserializeObject(client.DownloadString(inputEndpoint.Substring(0, inputEndpoint.Length - 3) + "/Controls"));
        }
    }

    static System.Windows.Rect SnapshotBounds(Dictionary<string, object> node)
    {
        var bounds = (Dictionary<string, object>)node["bounds"];
        double x = Convert.ToDouble(bounds["x1"]), y = Convert.ToDouble(bounds["y1"]);
        return new System.Windows.Rect(x, y, Convert.ToDouble(bounds["x2"]) - x, Convert.ToDouble(bounds["y2"]) - y);
    }

    static void ResizeResponsive(AutomationElement dialog, int width, int height)
    {
        var client = SnapshotBounds(ResponsiveSnapshot());
        var outer = dialog.Current.BoundingRectangle;
        double scale = GetDpiForWindow(new IntPtr(root.Current.NativeWindowHandle)) / 96.0;
        Pattern<TransformPattern>(dialog, TransformPattern.Pattern).Resize(outer.Width + (width - client.Width) * scale, outer.Height + (height - client.Height) * scale);
        Wait(() => { var settled = SnapshotBounds(ResponsiveSnapshot()); return settled.Width == width && settled.Height == height; }, "settled responsive client " + width + "x" + height);
    }

    static double ResponsiveMovieRowHeight(AutomationElement dialog)
    {
        var list = Named(dialog, "Movies", ControlType.List);
        double height = 0;
        foreach (var item in Items(list))
        {
            if (!item.Current.IsOffscreen) height = Math.Max(height, item.Current.BoundingRectangle.Height);
        }
        return height;
    }

    static Dictionary<string, object> FindResponsiveList(object tree, HashSet<string> movieNames, bool movies)
    {
        // Controls snapshots expose theme roles and rendered labels, not GuiControl.Text.
        // The movie list always has a title, including in its compact appearance.
        foreach (var node in SnapshotNodes(tree))
        {
            object value;
            if (!node.TryGetValue("control", out value) || (string)value != "TextList") continue;
            bool containsMovie = false;
            foreach (var child in SnapshotNodes(node))
            {
                if (child.TryGetValue("elementText", out value) && movieNames.Contains((string)value)) { containsMovie = true; break; }
            }
            if (containsMovie == movies) return node;
        }
        return null;
    }

    static ScrollPattern CheckScrollProperties(AutomationElement element)
    {
        var scroll = Pattern<ScrollPattern>(element, ScrollPattern.Pattern);
        var current = scroll.Current;
        var scrollable = new[] { current.HorizontallyScrollable, current.VerticallyScrollable };
        var percent = new[] { current.HorizontalScrollPercent, current.VerticalScrollPercent };
        var viewSize = new[] { current.HorizontalViewSize, current.VerticalViewSize };
        var scrollableProperties = new[] { ScrollPattern.HorizontallyScrollableProperty, ScrollPattern.VerticallyScrollableProperty };
        var percentProperties = new[] { ScrollPattern.HorizontalScrollPercentProperty, ScrollPattern.VerticalScrollPercentProperty };
        var viewSizeProperties = new[] { ScrollPattern.HorizontalViewSizeProperty, ScrollPattern.VerticalViewSizeProperty };
        for (int axis = 0; axis < 2; axis++)
        {
            string name = element.Current.Name + (axis == 0 ? " horizontal" : " vertical");
            Check((bool)element.GetCurrentPropertyValue(scrollableProperties[axis]) == scrollable[axis], name + " scrollability property matches pattern");
            Check((double)element.GetCurrentPropertyValue(percentProperties[axis]) == percent[axis], name + " scroll percent property matches pattern");
            Check((double)element.GetCurrentPropertyValue(viewSizeProperties[axis]) == viewSize[axis], name + " view size property matches pattern");
            if (!scrollable[axis]) Check(percent[axis] == ScrollPattern.NoScroll && viewSize[axis] == 100, name + " fitting axis reports NoScroll and a full view");
        }
        return scroll;
    }

    static void CheckRejectedScrollRequests(AutomationElement element)
    {
        var scroll = CheckScrollProperties(element);
        double horizontal = scroll.Current.HorizontalScrollPercent, vertical = scroll.Current.VerticalScrollPercent;
        var requests = new List<Action>();
        if (!scroll.Current.HorizontallyScrollable)
        {
            requests.Add(() => scroll.Scroll(ScrollAmount.SmallIncrement, ScrollAmount.NoAmount));
            requests.Add(() => scroll.SetScrollPercent(0, ScrollPattern.NoScroll));
        }
        if (!scroll.Current.VerticallyScrollable)
        {
            requests.Add(() => scroll.Scroll(ScrollAmount.NoAmount, ScrollAmount.SmallIncrement));
            requests.Add(() => scroll.SetScrollPercent(ScrollPattern.NoScroll, 0));
        }
        Check(requests.Count > 0, "scroll rejection fixture has a fitting axis");
        requests.Add(() => scroll.Scroll(ScrollAmount.SmallIncrement, ScrollAmount.SmallIncrement));
        requests.Add(() => scroll.SetScrollPercent(100, 100));
        foreach (var request in requests)
        {
            bool rejected = false;
            try { request(); } catch (InvalidOperationException) { rejected = true; }
            Check(rejected, "scroll request on a fitting axis is rejected");
            Check(scroll.Current.HorizontalScrollPercent == horizontal && scroll.Current.VerticalScrollPercent == vertical, "rejected scrolling preserves both axes");
        }
        scroll.Scroll(ScrollAmount.NoAmount, ScrollAmount.NoAmount);
        scroll.SetScrollPercent(ScrollPattern.NoScroll, ScrollPattern.NoScroll);
        Check(scroll.Current.HorizontalScrollPercent == horizontal && scroll.Current.VerticalScrollPercent == vertical, "no-op scrolling preserves both axes");
    }

    static void CheckResponsiveAppearance(AutomationElement dialog, int level)
    {
        var movieNames = new HashSet<string>();
        foreach (var item in Items(Named(dialog, "Movies", ControlType.List))) movieNames.Add(item.Current.Name);
        var publisherNames = new HashSet<string>(ResponsiveAccountNames);
        string observed = "";
        Wait(() =>
        {
            var snapshot = ResponsiveSnapshot();
            var movieList = FindResponsiveList(snapshot, movieNames, true);
            var accountList = FindResponsiveList(snapshot, movieNames, false);
            int movies = 0, authors = 0, images = 0, accountImages = 0, accountNames = 0;
            bool menu = SnapshotHas(snapshot["composition"], "elementText", "Subscribed Accounts");
            foreach (var node in SnapshotNodes(movieList))
            {
                object value;
                if (node.TryGetValue("elementText", out value))
                {
                    string text = (string)value;
                    if (movieNames.Contains(text)) movies++;
                    if (text.StartsWith("by: ")) authors++;
                }
                if (node.TryGetValue("element", out value) && (string)value == "Image") images++;
            }
            foreach (var node in SnapshotNodes(accountList))
            {
                object value;
                if (node.TryGetValue("elementText", out value) && publisherNames.Contains((string)value)) accountNames++;
                if (node.TryGetValue("element", out value) && (string)value == "Image") accountImages++;
            }
            string counts = "menu=" + menu + "; movies=" + movies + "; authors=" + authors + "; movie images=" + images + "; account images=" + accountImages + "; account names=" + accountNames;
            if (counts != observed) { Console.WriteLine("RENDER " + counts); observed = counts; }
            return movies > 0 && menu == (level < 2)
                && (level == 0 ? authors == 0 && images == 0 : authors == movies && images == movies)
                && (level < 2 ? accountList == null : accountImages > 0 && accountImages <= ResponsiveAccountNames.Length)
                && (level == 3 ? accountNames == accountImages : accountNames == 0);
        }, "responsive appearance #" + level);
        string footerBounds = "";
        Wait(() => {
            var ok = Named(dialog, "OK", ControlType.Button).Current.BoundingRectangle;
            var cancel = Named(dialog, "Cancel", ControlType.Button).Current.BoundingRectangle;
            var list = Named(dialog, "Movies", ControlType.List).Current.BoundingRectangle;
            string current = "OK=" + ok + "; Cancel=" + cancel + "; Movies=" + list;
            bool valid = ok.Width > 0 && cancel.Width > 0 && Math.Abs(ok.Width - cancel.Width) < 2 && Math.Abs(ok.Top - cancel.Top) < 2 && ok.Right <= cancel.Left && list.Bottom <= ok.Top;
            if (!valid && current != footerBounds) Console.WriteLine("FOOTER " + current);
            footerBounds = current;
            return valid;
        }, "equal-width footer buttons below the movie viewport");
        Check(!CheckScrollProperties(Named(dialog, "Movies", ControlType.List)).Current.HorizontallyScrollable, "movie rows fit without horizontal scrolling");
        if (level >= 2) Check(!CheckScrollProperties(Named(dialog, "Accounts", ControlType.List)).Current.HorizontallyScrollable, "account rows fit without horizontal scrolling");
        if (level > 0)
        {
            var titles = new List<System.Windows.Rect>();
            var authors = new List<System.Windows.Rect>();
            var icons = new List<System.Windows.Rect>();
            var listNode = FindResponsiveList(ResponsiveSnapshot(), movieNames, true);
            Check(listNode != null, "rendered movie list viewport");
            foreach (var node in SnapshotNodes(listNode))
            {
                object value;
                if (node.TryGetValue("elementText", out value))
                {
                    if (movieNames.Contains((string)value)) titles.Add(SnapshotBounds(node));
                    if (((string)value).StartsWith("by: ")) authors.Add(SnapshotBounds(node));
                }
                if (node.TryGetValue("element", out value) && (string)value == "Image") icons.Add(SnapshotBounds(node));
            }
            Check(titles.Count == authors.Count && titles.Count == icons.Count && titles.Count > 0, "one image and author for each rich movie row");
            for (int i = 0; i < titles.Count; i++)
            {
                Check(icons[i].Width == 96 && icons[i].Height == 96 && icons[i].Right < titles[i].Left && titles[i].Left == authors[i].Left && titles[i].Top < authors[i].Top && icons[i].Top <= titles[i].Top && icons[i].Bottom >= authors[i].Bottom, "96-pixel square image spans title and author rows without overlap: image=" + icons[i] + "; title=" + titles[i] + "; author=" + authors[i]);
            }
        }
        Console.WriteLine("LAYOUT #" + level + " client=" + SnapshotBounds(ResponsiveSnapshot()).Size);
    }

    static void EasyLayout()
    {
        Test("Layout / Easy Layout / mixed docking controls");
        var page = Page(Page(root, "Layout"), "Easy Layout");
        double scale = GetDpiForWindow(new IntPtr(root.Current.NativeWindowHandle)) / 96.0;
        double absoluteWidth = 120;
        var left = new[] { Named(page, "Left one", ControlType.Button), Named(page, "Left two", ControlType.Button), Named(page, "Left three", ControlType.Button) };
        var check = Named(page, "Right check", ControlType.CheckBox);
        var combos = Find(page, ControlType.ComboBox);
        Check(combos.Count == 1, "one Easy Layout choice control");
        var combo = combos[0];
        // A combo's selected text is Selection, not its accessible Name.
        Check(combo.Current.Name == "", "unlabeled combo does not use its changing value as Name");
        var selection = Pattern<SelectionPattern>(combo, SelectionPattern.Pattern);
        var expansion = Pattern<ExpandCollapsePattern>(combo, ExpandCollapsePattern.Pattern);
        var toggle = Pattern<TogglePattern>(check, TogglePattern.Pattern);
        Check(selection.Current.GetSelection().Length == 1 && selection.Current.GetSelection()[0].Current.Name == "First choice", "initial static choice");
        var row = new[] { left[0], left[1], left[2], combo, check };
        Action geometry = () => {
            left = new[] { Named(page, "Left one", ControlType.Button), Named(page, "Left two", ControlType.Button), Named(page, "Left three", ControlType.Button) };
            check = Named(page, "Right check", ControlType.CheckBox);
            combo = Find(page, ControlType.ComboBox)[0];
            selection = Pattern<SelectionPattern>(combo, SelectionPattern.Pattern);
            expansion = Pattern<ExpandCollapsePattern>(combo, ExpandCollapsePattern.Pattern);
            toggle = Pattern<TogglePattern>(check, TogglePattern.Pattern);
            row = new[] { left[0], left[1], left[2], combo, check };
            Wait(() => {
                var first = row[0].Current.BoundingRectangle;
                double right = first.Left - 1;
                foreach (var control in row) {
                    var bounds = control.Current.BoundingRectangle;
                    if (control.Current.IsOffscreen || bounds.Width <= 0 || bounds.Height <= 0 || bounds.Top != first.Top || bounds.Bottom != first.Bottom || bounds.Left <= right) return false;
                    right = bounds.Right;
                }
                return true;
            }, "five nonoverlapping controls share their outer top and bottom");
            Check(combo.Current.BoundingRectangle.Width >= 120, "choice control leaves room for its selected text");
            Check(combo.Current.BoundingRectangle.Left - left[2].Current.BoundingRectangle.Right > left[1].Current.BoundingRectangle.Left - left[0].Current.BoundingRectangle.Right, "left and right docking groups leave unused space");
            var firstColumn = Named(page, "First column", ControlType.Text).Current.BoundingRectangle;
            var secondColumn = Named(page, "Second column", ControlType.Text).Current.BoundingRectangle;
            var shared = Named(page, "Shared tracks", ControlType.Text).Current.BoundingRectangle;
            var rebuild = Named(page, "Rebuild", ControlType.Button).Current.BoundingRectangle;
            var edit = Find(page, ControlType.Edit)[0];
            var editBounds = edit.Current.BoundingRectangle;
            var label = Named(page, Pattern<ValuePattern>(edit, ValuePattern.Pattern).Current.Value, ControlType.Text).Current.BoundingRectangle;
            Check(Math.Abs(firstColumn.Width - absoluteWidth * scale) <= scale && Math.Abs(firstColumn.Height - 24 * scale) <= scale, "absolute column and row sizes");
            Check(firstColumn.Top == secondColumn.Top && firstColumn.Bottom == secondColumn.Bottom, "independent grid stays horizontal");
            foreach (double gap in new[] {
                left[1].Current.BoundingRectangle.Left - left[0].Current.BoundingRectangle.Right,
                left[2].Current.BoundingRectangle.Left - left[1].Current.BoundingRectangle.Right,
                check.Current.BoundingRectangle.Left - combo.Current.BoundingRectangle.Right,
                editBounds.Top - rebuild.Bottom,
                left[0].Current.BoundingRectangle.Top - Math.Max(editBounds.Bottom, label.Bottom),
                firstColumn.Top - check.Current.BoundingRectangle.Bottom,
                secondColumn.Left - firstColumn.Right,
                shared.Top - firstColumn.Bottom
            }) Check(Math.Abs(gap - 5 * scale) <= scale, "one padding gap within and between descriptor groups");
            Check(rebuild.Left == editBounds.Left && editBounds.Left == firstColumn.Left && firstColumn.Left == shared.Left && rebuild.Right == secondColumn.Right && secondColumn.Right == shared.Right, "nested containers add no horizontal inset");
        };
        geometry();
        foreach (var button in left) Pattern<InvokePattern>(button, InvokePattern.Pattern).Invoke();
        Check(toggle.Current.ToggleState == ToggleState.Off && selection.Current.GetSelection()[0].Current.Name == "First choice", "inert buttons preserve local state");
        toggle.Toggle(); Wait(() => toggle.Current.ToggleState == ToggleState.On, "new checkbox toggles");
        expansion.Expand(); Wait(() => expansion.Current.ExpandCollapseState == ExpandCollapseState.Expanded, "new combo opens");
        AutomationElement popup = null;
        Wait(() => { popup = Popup(ControlType.List, combo); return popup != null; }, "static choice popup");
        var choices = Items(popup);
        Check(choices.Count == 3 && choices[0].Current.Name == "First choice" && choices[1].Current.Name == "Second choice" && choices[2].Current.Name == "Third choice", "static choice names");
        Pattern<SelectionItemPattern>(choices[1], SelectionItemPattern.Pattern).Select();
        Wait(() => selection.Current.GetSelection().Length == 1 && selection.Current.GetSelection()[0].Current.Name == "Second choice", "new combo selection");
        expansion.Collapse(); Wait(() => expansion.Current.ExpandCollapseState == ExpandCollapseState.Collapsed, "new combo dismisses");
        geometry();
        Action reopenCombo = () => {
            expansion.Expand();
            Wait(() => expansion.Current.ExpandCollapseState == ExpandCollapseState.Expanded, "retained combo opens");
            Wait(() => { popup = Popup(ControlType.List, combo); return popup != null; }, "retained choice popup");
            choices = Items(popup);
            Check(choices.Count == 3 && choices[0].Current.Name == "First choice" && choices[1].Current.Name == "Second choice" && choices[2].Current.Name == "Third choice", "all choices remain available");
            Check(selection.Current.GetSelection()[0].Current.Name == "Second choice", "opening retains selected choice");
            expansion.Collapse();
            Wait(() => expansion.Current.ExpandCollapseState == ExpandCollapseState.Collapsed, "retained combo dismisses");
        };

        var editor = Find(page, ControlType.Edit)[0];
        var value = Pattern<ValuePattern>(editor, ValuePattern.Pattern);
        value.SetValue("UIA Easy Layout retained text");
        Action<bool> arrangement = vertical => {
            Wait(() => {
                editor = Find(page, ControlType.Edit)[0];
                var label = FindNamed(page, "UIA Easy Layout retained text", ControlType.Text);
                if (label == null) return false;
                var a = editor.Current.BoundingRectangle;
                var b = label.Current.BoundingRectangle;
                return vertical
                    ? a.Left == b.Left && a.Right == b.Right && Math.Abs(b.Top - a.Bottom - 5 * scale) <= 1 && Math.Abs(b.Height - 2 * a.Height) <= 2 * scale
                    : a.Top == b.Top && a.Bottom == b.Bottom && Math.Abs(b.Left - a.Right - 5 * scale) <= 1 && Math.Abs(b.Width - 2 * a.Width) <= 2 * scale;
            }, vertical ? "vertical 1:2 payloads with one gap" : "horizontal 1:2 payloads with one gap");
        };
        arrangement(false);
        for (int i = 0; i < 2; i++) {
            var direction = Pattern<TogglePattern>(Named(page, "Store vertical arrangement (press Rebuild to apply)", ControlType.CheckBox), TogglePattern.Pattern);
            direction.Toggle();
            Wait(() => direction.Current.ToggleState == (i == 0 ? ToggleState.On : ToggleState.Off), "direction stored");
            arrangement(i != 0);
            if (i == 0) Click(Named(page, "Rebuild", ControlType.Button)); else Invoke(page, "Rebuild");
            page = RebuiltLayoutPage("Easy Layout");
            Wait(() => {
                var editors = Find(page, ControlType.Edit);
                if (editors.Count != 1) return false;
                editor = editors[0]; value = Pattern<ValuePattern>(editor, ValuePattern.Pattern);
                return value.Current.Value == "UIA Easy Layout retained text" && Find(page, ControlType.Text).Exists(text => text.Current.Name == "UIA Easy Layout retained text");
            }, "rebuild preserves editor and label binding");
            arrangement(i == 0);
            geometry();
            Check(toggle.Current.ToggleState == ToggleState.On && selection.Current.GetSelection()[0].Current.Name == "Second choice", "rebuild retains docking state");
            reopenCombo();
        }
        var transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        var original = root.Current.BoundingRectangle;
        var resizeWindowHandle = new IntPtr(root.Current.NativeWindowHandle);
        var beforeDrag = Named(page, "First column", ControlType.Text).Current.BoundingRectangle;
        var dockingBefore = left[0].Current.BoundingRectangle;
        var editorBefore = Find(page, ControlType.Edit)[0].Current.BoundingRectangle;
        Drag(beforeDrag.Right + 2 * scale, beforeDrag.Top + beforeDrag.Height / 2, 20 * scale, 0);
        absoluteWidth = 140;
        Wait(() => Math.Abs(Named(page, "First column", ControlType.Text).Current.BoundingRectangle.Width - absoluteWidth * scale) <= scale, "relocated splitter changes adjacent absolute column");
        Check(left[0].Current.BoundingRectangle == dockingBefore && Find(page, ControlType.Edit)[0].Current.BoundingRectangle == editorBefore, "splitter leaves separate descriptor groups unchanged");
        geometry();
        transform.Resize(original.Width + 160, original.Height + 100);
        Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000);
        root = AutomationElement.FromHandle(resizeWindowHandle);
        transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        var expanded = root.Current.BoundingRectangle;
        Check(expanded.Width > original.Width && expanded.Height > original.Height, "larger Transform.Resize increases both dimensions");
        geometry();
        reopenCombo();
        Invoke(page, "Rebuild");
        page = RebuiltLayoutPage("Easy Layout");
        absoluteWidth = 120;
        Wait(() => Math.Abs(Named(page, "First column", ControlType.Text).Current.BoundingRectangle.Width - absoluteWidth * scale) <= scale, "rebuild resets relocated splitter adjustment");
        geometry();
        transform.Resize(original.Width, original.Height);
        Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000);
        root = AutomationElement.FromHandle(resizeWindowHandle);
        transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        geometry();
        transform.Resize(Math.Max(320, original.Width - 80), Math.Max(240, original.Height - 40));
        Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000);
        root = AutomationElement.FromHandle(resizeWindowHandle);
        transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        var smaller = root.Current.BoundingRectangle;
        Check(smaller.Width < expanded.Width && smaller.Height < expanded.Height, "smaller Transform.Resize reduces both dimensions from expanded size (minimum client size may clamp the request)");
        geometry();
        transform.Resize(original.Width, original.Height);
        geometry();
        var mainWindowHandle = new IntPtr(root.Current.NativeWindowHandle);
        foreach (var palette in new[] { "Aurora", "Ember", "Moonstone", "Lagoon", "Rosewood", "Default" }) {
            Test("palette refresh / " + palette);
            var manager = Page(root, "Window Manager");
            var oldRadio = Named(manager, palette, ControlType.RadioButton);
            Pattern<SelectionItemPattern>(oldRadio, SelectionItemPattern.Pattern).Select();
            Wait(() => PaletteAttachmentRetired(oldRadio), "old palette attachment retired after " + palette);
            root = AutomationElement.FromHandle(mainWindowHandle);
            manager = Page(root, "Window Manager");
            var currentRadio = Named(manager, palette, ControlType.RadioButton);
            Check(Pattern<SelectionItemPattern>(currentRadio, SelectionItemPattern.Pattern).Current.IsSelected, "palette " + palette + " selected after theme refresh");
            SkinItems(manager, palette);
            page = Page(Page(root, "Layout"), "Easy Layout");
            Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000);
            root = AutomationElement.FromHandle(mainWindowHandle);
            var currentLayoutPage = FindNamed(root, "Layout", ControlType.TabItem);
            Check(currentLayoutPage != null, "Layout page reacquired after palette refresh " + palette);
            var currentEasyPage = FindNamed(currentLayoutPage, "Easy Layout", ControlType.TabItem);
            Check(currentEasyPage != null, "Easy Layout page reacquired after palette refresh " + palette);
            Check(Pattern<SelectionItemPattern>(currentLayoutPage, SelectionItemPattern.Pattern).Current.IsSelected, "Layout remains selected after palette refresh " + palette);
            Check(Pattern<SelectionItemPattern>(currentEasyPage, SelectionItemPattern.Pattern).Current.IsSelected, "Easy Layout remains selected after palette refresh " + palette);
            page = currentEasyPage;
            left = new[] { Named(page, "Left one", ControlType.Button), Named(page, "Left two", ControlType.Button), Named(page, "Left three", ControlType.Button) };
            check = Named(page, "Right check", ControlType.CheckBox);
            var currentCombos = Find(page, ControlType.ComboBox);
            var editors = Find(page, ControlType.Edit);
            Check(currentCombos.Count == 1 && editors.Count == 1, "palette refresh retains current controls");
            combo = currentCombos[0]; editor = editors[0];
            selection = Pattern<SelectionPattern>(combo, SelectionPattern.Pattern);
            toggle = Pattern<TogglePattern>(check, TogglePattern.Pattern);
            value = Pattern<ValuePattern>(editor, ValuePattern.Pattern);
            row = new[] { left[0], left[1], left[2], combo, check };
            Check(toggle.Current.ToggleState == ToggleState.On && value.Current.Value == "UIA Easy Layout retained text", "palette refresh preserves editor and checkbox state");
            var comboSelection = selection.Current.GetSelection();
            Check(comboSelection.Length == 1 && comboSelection[0].Current.Name == "Second choice", "palette refresh preserves combo selection");
            Check(FindNamed(page, "UIA Easy Layout retained text", ControlType.Text) != null && FindNamed(page, "Shared tracks", ControlType.Text) != null, "palette refresh preserves bound labels and shared tracks");
            geometry();
            reopenCombo();
        }
        toggle.Toggle();
    }
    static void EasyLayoutTables()
    {
        Test("Layout / Easy Layout (Table) / independent shared grids");
        var page = Page(Page(root, "Layout"), "Easy Layout (Table)");
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        double scale = GetDpiForWindow(hwnd) / 96.0;
        Func<string, System.Windows.Rect> bounds = name => Named(page, name, ControlType.Text).Current.BoundingRectangle;
        Action<double, double, string> equal = (actual, expected, message) => Check(Math.Abs(actual - expected) <= scale, message + ": " + actual + " versus " + expected);
        Action geometry = () => {
            var previous = new Dictionary<string, System.Windows.Rect>();
            var captured = new Dictionary<string, System.Windows.Rect>();
            var previousEdit = System.Windows.Rect.Empty;
            var capturedEdit = System.Windows.Rect.Empty;
            string capturedText = "";
            int stableSamples = 0;
            Wait(() => {
                Thread.Sleep(250);
                try {
                    page = FindNamed(root, "Easy Layout (Table)", ControlType.TabItem);
                    if (page == null) { stableSamples = 0; return false; }
                    var anchor = FindNamed(page, "Row A", ControlType.Text);
                    if (anchor == null) { stableSamples = 0; return false; }
                    var windowBounds = root.Current.BoundingRectangle;
                    var anchorBounds = anchor.Current.BoundingRectangle;
                    var current = new Dictionary<string, System.Windows.Rect>();
                    foreach (var payload in Find(page, ControlType.Text)) {
                        var rectangle = payload.Current.BoundingRectangle;
                        if (payload.Current.IsOffscreen || rectangle.IsEmpty || rectangle.Width <= 0 || rectangle.Height <= 0) { stableSamples = 0; return false; }
                        current.Add(payload.Current.Name, rectangle);
                    }
                    var edits = Find(page, ControlType.Edit);
                    if (edits.Count != 1) { stableSamples = 0; return false; }
                    var editBounds = edits[0].Current.BoundingRectangle;
                    if (edits[0].Current.IsOffscreen || editBounds.IsEmpty || editBounds.Width <= 0 || editBounds.Height <= 0
                        || root.Current.BoundingRectangle != windowBounds || anchor.Current.BoundingRectangle != anchorBounds) { stableSamples = 0; return false; }
                    bool stable = current.Count > 0 && current.Count == previous.Count && editBounds == previousEdit;
                    if (stable) foreach (var item in current) {
                        System.Windows.Rect old;
                        if (!previous.TryGetValue(item.Key, out old) || item.Value != old) { stable = false; break; }
                    }
                    previous = current; previousEdit = editBounds;
                    stableSamples = stable ? stableSamples + 1 : 0;
                    if (stableSamples < 2) return false;
                    captured = current; capturedEdit = editBounds;
                    capturedText = ((ValuePattern)edits[0].GetCurrentPattern(ValuePattern.Pattern)).Current.Value;
                    return true;
                }
                catch (ElementNotAvailableException) { stableSamples = 0; previous.Clear(); return false; }
            }, "table payload geometry settles", 60000);
            var a = captured["Row A"]; var b = captured["Row B"];
            var fixedColumn = captured["Shared 120"];
            var one = captured["Column 1x"]; var two = captured["Column 2x"];
            var edit = capturedEdit;
            equal(a.Left, b.Left, "minimum column shares left edge");
            equal(a.Right, b.Right, "minimum column shares right edge");
            equal(edit.Left, fixedColumn.Left, "nondefault column overrides default declaration");
            equal(edit.Width, fixedColumn.Width, "shared absolute width");
            equal(edit.Width, 120 * scale, "configured 120-pixel column");
            equal(a.Height, 40 * scale, "configured 40-pixel outer row");
            Check(Math.Abs(two.Width - 2 * one.Width) <= 2 * scale, "percentage columns split remaining width 1:2 with pixel rounding");
            equal(edit.Left - a.Right, 5 * scale, "minimum to absolute gap");
            equal(one.Left - edit.Right, 5 * scale, "column splitter occupies one gap");
            equal(two.Left - one.Right, 5 * scale, "percentage column gap");
            equal(b.Top - a.Bottom, 5 * scale, "row splitter occupies one gap");
            var spanning = captured[capturedText];
            equal(spanning.Left, one.Left, "two-column span begins at shared track");
            equal(spanning.Right, two.Right, "two-column span ends at shared track");
            var outerSpan = captured["Two rows"]; var shortRow = captured["Short row"];
            equal(outerSpan.Top - b.Bottom, 5 * scale, "outer span starts at next row");
            equal(shortRow.Top - outerSpan.Bottom, 5 * scale, "outer span covers two existing rows");
            equal(shortRow.Left, a.Left, "ragged row uses the same minimum column");

            var ca = captured["Column A"]; var cb = captured["Column B"];
            var fixedRow = captured["Shared 40"]; var sameRow = captured["Same row"];
            var r1 = captured["Row 1x"]; var r2 = captured["Row 2x"];
            equal(ca.Width, 140 * scale, "independent configured 140-pixel column");
            equal(ca.Top, cb.Top, "transposed minimum row shares top");
            equal(ca.Bottom, cb.Bottom, "transposed minimum row shares bottom");
            equal(fixedRow.Top, sameRow.Top, "nondefault row overrides default declaration");
            equal(fixedRow.Bottom, sameRow.Bottom, "shared absolute row ends together");
            equal(fixedRow.Height, 40 * scale, "configured 40-pixel shared row");
            Check(Math.Abs(r2.Height - 2 * r1.Height) <= 2 * scale, "percentage rows split remaining height 1:2 with pixel rounding");
            equal(fixedRow.Top - ca.Bottom, 5 * scale, "minimum to absolute row gap");
            equal(r1.Top - fixedRow.Bottom, 5 * scale, "shared row splitter occupies one gap");
            equal(r2.Top - r1.Bottom, 5 * scale, "percentage row gap");
            equal(cb.Left - ca.Right, 5 * scale, "outer column splitter occupies one gap");
            var innerSpan = captured["Two shared rows"]; var wide = captured["Two columns"];
            equal(innerSpan.Top, r1.Top, "transposed inner span begins at shared row");
            equal(innerSpan.Bottom, r2.Bottom, "transposed inner span ends at shared row");
            equal(wide.Left - cb.Right, 5 * scale, "outer column span starts at next column");
            equal(wide.Width, 125 * scale, "outer span covers two 60-pixel columns and one gap");
            equal(a.Left, ca.Left, "nested grid groups add no horizontal inset");
            foreach (var label in captured) {
                Check(label.Value.Width > 0 && label.Value.Height > 0, "visible table payload: " + label.Key);
            }
        };
        Wait(() => { var label = FindNamed(page, "Row 2x", ControlType.Text); return label != null && label.Current.BoundingRectangle.Height > 0; }, "table layout settled");
        geometry();
        var editor = Find(page, ControlType.Edit)[0];
        string retainedText = "Kept table text";
        Pattern<ValuePattern>(editor, ValuePattern.Pattern).SetValue(retainedText);
        Wait(() => FindNamed(page, retainedText, ControlType.Text) != null, "table payload binding updated");
        Test("table outer row splitter / original window size");
        var rowA = bounds("Row A");
        Drag(rowA.Left + rowA.Width / 2, rowA.Bottom + 2 * scale, 0, 10 * scale);
        Wait(() => Math.Abs(bounds("Row A").Height - 50 * scale) <= scale, "outer row splitter drags down at original size");
        rowA = bounds("Row A");
        Drag(rowA.Left + rowA.Width / 2, rowA.Bottom + 2 * scale, 0, -10 * scale);
        Wait(() => Math.Abs(bounds("Row A").Height - 40 * scale) <= scale, "outer row splitter reverses at original size");
        var initial = root.Current.BoundingRectangle;
        var transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
        transform.Resize(initial.Width + 180, initial.Height + 160);
        Wait(() => root.Current.BoundingRectangle.Width > initial.Width, "table window enlarged");
        Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000);
        geometry();
        Test("table splitters / both axes and repeated shared boundaries");
        var shared = bounds("Shared 120");
        var unaffected = bounds("Column A");
        Drag(shared.Right + 2 * scale, shared.Top + shared.Height / 2, 20 * scale, 0);
        Wait(() => Math.Abs(bounds("Shared 120").Width - 140 * scale) <= scale, "repeated shared column boundary drags once");
        equal(bounds("Column A").Width, unaffected.Width, "row grid drag leaves transposed table unchanged");
        rowA = bounds("Row A");
        Drag(rowA.Left + rowA.Width / 2, rowA.Bottom + 2 * scale, 0, 10 * scale);
        Wait(() => Math.Abs(bounds("Row A").Height - 50 * scale) <= scale, "outer row splitter drags down at enlarged size");
        rowA = bounds("Row A");
        Drag(rowA.Left + rowA.Width / 2, rowA.Bottom + 2 * scale, 0, -20 * scale);
        Wait(() => Math.Abs(bounds("Row A").Height - 30 * scale) <= scale, "outer row splitter drags up past its initial position");
        rowA = bounds("Row A");
        Drag(rowA.Left + rowA.Width / 2, rowA.Bottom + 2 * scale, 0, 20 * scale);
        Wait(() => Math.Abs(bounds("Row A").Height - 50 * scale) <= scale, "outer row splitter reverses downward again");
        equal(bounds("Column A").Width, unaffected.Width, "outer row drag leaves transposed table unchanged");
        var columnA = bounds("Column A");
        Drag(columnA.Right + 2 * scale, columnA.Top + columnA.Height / 2, 15 * scale, 0);
        Wait(() => Math.Abs(bounds("Column A").Width - 155 * scale) <= scale, "transposed outer column splitter changes absolute width");
        var same = bounds("Same row");
        Drag(same.Left + same.Width / 2, same.Bottom + 2 * scale, 0, 12 * scale);
        Wait(() => Math.Abs(bounds("Shared 40").Height - 52 * scale) <= scale, "repeated shared row boundary drags once");
        equal(bounds("Shared 120").Width, 140 * scale, "transposed drags leave row grid adjustment intact");
        transform.Resize(initial.Width + 220, initial.Height + 200);
        Wait(() => root.Current.BoundingRectangle.Width >= initial.Width + 220, "second resize applied");
        equal(bounds("Shared 120").Width, 140 * scale, "resize retains first grid adjustment");
        equal(bounds("Row A").Height, 50 * scale, "resize retains outer row adjustment");
        equal(bounds("Shared 40").Height, 52 * scale, "resize retains transposed adjustment");
        for (int iteration = 0; iteration < 2; iteration++) {
            Test("table rebuild / " + (iteration == 0 ? "mouse" : "Invoke"));
            if (iteration == 0) Click(Named(page, "Rebuild tables", ControlType.Button)); else Invoke(page, "Rebuild tables");
            page = RebuiltLayoutPage("Easy Layout (Table)");
            Wait(() => {
                var column = FindNamed(page, "Shared 120", ControlType.Text);
                var row = FindNamed(page, "Shared 40", ControlType.Text);
                return column != null && row != null && Math.Abs(column.Current.BoundingRectangle.Width - 120 * scale) <= scale && Math.Abs(row.Current.BoundingRectangle.Height - 40 * scale) <= scale;
            }, "rebuild restores descriptor sizes");
            geometry();
            var retainedValue = Pattern<ValuePattern>(Find(page, ControlType.Edit)[0], ValuePattern.Pattern);
            Check(retainedValue.Current.Value == retainedText, "repeated rebuild retains table text");
            if (iteration == 0) {
                retainedText = "Updated table";
                retainedValue.SetValue(retainedText);
                Wait(() => FindNamed(page, retainedText, ControlType.Text) != null, "binding remains live after owning rebuild");
            }
        }
        Page(Page(root, "Layout"), "Easy Layout");
        page = Page(Page(root, "Layout"), "Easy Layout (Table)");
        geometry();
        foreach (var palette in new[] { "Ember", "Default" }) {
            Test("table palette refresh / " + palette);
            var manager = Page(root, "Window Manager");
            var radio = Named(manager, palette, ControlType.RadioButton);
            Pattern<SelectionItemPattern>(radio, SelectionItemPattern.Pattern).Select();
            Wait(() => PaletteAttachmentRetired(radio), "table palette refresh completed");
            root = AutomationElement.FromHandle(hwnd);
            page = Page(Page(root, "Layout"), "Easy Layout (Table)");
            Check(Pattern<ValuePattern>(Find(page, ControlType.Edit)[0], ValuePattern.Pattern).Current.Value == retainedText, "palette refresh retains table payload");
            geometry();
        }
        Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(initial.Width, initial.Height);
        Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(30000);
        geometry();
    }
    static void WalkTabs(AutomationElement container, int depth)
    {
        if (depth > 5) return;
        if (container.Current.ControlType == ControlType.TabItem)
        {
            var bodies = Find(container, ControlType.Pane, TreeScope.Children);
            Check(bodies.Count == 1, "selected tab exposes one body Pane");
            container = bodies[0];
        }
        var tabs = Find(container, ControlType.Tab, TreeScope.Children);
        foreach (var tab in tabs) foreach (var initialPage in Find(tab, ControlType.TabItem, TreeScope.Children))
        {
            var name = initialPage.Current.Name;
            Test("coverage / " + name);
            var page = Page(tab, name);
            var seen = new HashSet<string>();
            foreach (AutomationElement element in page.FindAll(TreeScope.Descendants, Condition.TrueCondition))
            {
                var current = element.Current;
                Check(current.FrameworkId == "GacUI", "native semantic descendant");
                Check(seen.Add(string.Join(",", Array.ConvertAll(element.GetRuntimeId(), id => id.ToString()))), "duplicate logical node: " + current.Name);
                foreach (var pattern in element.GetSupportedPatterns()) element.GetCurrentPattern(pattern);
                if (current.ControlType == ControlType.Slider && current.IsEnabled)
                {
                    var range = Pattern<RangeValuePattern>(element, RangeValuePattern.Pattern);
                    if (!range.Current.IsReadOnly) { var value = range.Current.Value == range.Current.Minimum ? range.Current.Maximum : range.Current.Minimum; range.SetValue(value); Check(range.Current.Value == value, "RangeValue changed"); }
                }
            }
            WalkTabs(page, depth+1);
        }
    }
    static void ReviewContracts()
    {
        var failures = new List<string>();
        Action<bool,string> verify = (condition, message) => {
            Console.WriteLine((condition ? "PASS " : "FAIL ") + message);
            if (condition) assertions++; else failures.Add(message);
        };
        Test("review / text range contracts");
        var textPage = Page(Page(Page(root, "Control"), "TextBox"), "TextBox");
        var edit = Find(textPage, ControlType.Edit)[0];
        var value = Pattern<ValuePattern>(edit, ValuePattern.Pattern);
        var text = Pattern<TextPattern>(edit, TextPattern.Pattern);
        value.SetValue("");
        var visible = text.GetVisibleRanges();
        verify(visible.Length == 1 && visible[0].GetText(-1) == "", "X3 empty document has one degenerate visible range");
        value.SetValue("0123456789");
        foreach (bool add in new [] {true, false})
        {
            var selected = text.DocumentRange.FindText("123", false, false); selected.Select();
            var caret = text.DocumentRange.Clone();
            caret.MoveEndpointByRange(TextPatternRangeEndpoint.End, caret, TextPatternRangeEndpoint.Start);
            caret.Move(TextUnit.Character, 6);
            bool accepted = true;
            try { if (add) caret.AddToSelection(); else caret.RemoveFromSelection(); }
            catch (InvalidOperationException) { accepted = false; }
            var current = text.GetSelection();
            verify(accepted && current.Length == 1 && current[0].Compare(caret), "X2 degenerate " + (add ? "Add" : "Remove") + " moves insertion point");
        }
        var rectangle = edit.Current.BoundingRectangle;
        Check(!rectangle.IsEmpty && rectangle.Width > 0 && rectangle.Height > 0, "visible editor geometry");
        var hit = AutomationElement.FromPoint(new System.Windows.Point(rectangle.Left + rectangle.Width / 2, rectangle.Top + rectangle.Height / 2));
        verify(Automation.Compare(hit, edit), "T5 root hit test reaches selected tab body editor");

        var embeddedPage = Page(Page(root, "Control"), "Embedded Controls");
        var document = Pattern<TextPattern>(Find(embeddedPage, ControlType.Document)[0], TextPattern.Pattern);
        var child = document.DocumentRange.GetChildren()[0];
        var childRange = document.RangeFromChild(child);
        verify(Automation.Compare(childRange.GetEnclosingElement(), child), "X4 embedded range enclosure identifies child");
        bool repeatsChild = false;
        foreach (var nested in childRange.GetChildren()) if (Automation.Compare(nested, child)) repeatsChild = true;
        verify(!repeatsChild, "X4 embedded range children do not repeat enclosing child");
        var childBounds = child.Current.BoundingRectangle;
        verify(document.RangeFromPoint(new System.Windows.Point(childBounds.Left + childBounds.Width / 2, childBounds.Top + childBounds.Height / 2)).Compare(childRange), "R2-20 point within embedded control returns its object range");

        Test("review / combo ownership and atomic rejection");
        var listPage = Page(Page(root, "List"), "TextList");
        var combo = Find(listPage, ControlType.ComboBox)[0];
        var expansion = Pattern<ExpandCollapsePattern>(combo, ExpandCollapsePattern.Pattern);
        expansion.Expand();
        Wait(() => expansion.Current.ExpandCollapseState == ExpandCollapseState.Expanded, "review combo opens");
        verify(Find(combo, ControlType.ListItem).Count > 0, "T1 combo dropdown items descend from combo");
        var dropdownItems = Find(combo, ControlType.ListItem);
        foreach (var item in dropdownItems)
        {
            var bounds = item.Current.BoundingRectangle;
            if (bounds.IsEmpty || bounds.Width <= 0 || bounds.Height <= 0) continue;
            verify(Automation.Compare(item, AutomationElement.FromPoint(new System.Windows.Point(bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2))), "T1 popup hit test follows native stacking");
            break;
        }
        var menuParent = TreeWalker.RawViewWalker.GetParent(Find(combo, ControlType.Menu)[0]);
        verify(Automation.Compare(menuParent, combo), "L1 popup parent navigation returns combo");
        var selection = Pattern<SelectionPattern>(combo, SelectionPattern.Pattern);
        var before = selection.Current.GetSelection();
        Check(before.Length == 1, "review combo initially selected");
        bool rejected = false;
        try { Pattern<SelectionItemPattern>(before[0], SelectionItemPattern.Pattern).RemoveFromSelection(); }
        catch (InvalidOperationException) { rejected = true; }
        var after = selection.Current.GetSelection();
        verify(rejected && after.Length == 1 && Automation.Compare(before[0], after[0]), "T1 rejected combo removal preserves selection");
        expansion.Collapse();

        Test("review / grid selection, focus and header metadata");
        var gridPage = Page(Page(root, "List"), "BindableDataGrid");
        var grid = Find(gridPage, ControlType.DataGrid)[0];
        var cell = Pattern<GridPattern>(grid, GridPattern.Pattern).GetItem(0, 0);
        Pattern<SelectionItemPattern>(cell, SelectionItemPattern.Pattern).Select();
        var selectedCells = Pattern<SelectionPattern>(grid, SelectionPattern.Pattern).Current.GetSelection();
        verify(selectedCells.Length == 1 && Automation.Compare(selectedCells[0], cell), "T4 data grid selection returns active cell");
        var anotherCell = Pattern<GridPattern>(grid, GridPattern.Pattern).GetItem(0, 1);
        verify(cell.Current.IsKeyboardFocusable && !anotherCell.Current.IsKeyboardFocusable, "P2 grid focusability follows the active keyboard cell");
        rejected = false;
        try { Pattern<SelectionItemPattern>(anotherCell, SelectionItemPattern.Pattern).AddToSelection(); }
        catch (InvalidOperationException) { rejected = true; }
        selectedCells = Pattern<SelectionPattern>(grid, SelectionPattern.Pattern).Current.GetSelection();
        bool preserved = selectedCells.Length == 1 && Automation.Compare(selectedCells[0], cell);
        verify(rejected && preserved, "T4 rejected second-cell Add preserves single selection (rejected=" + rejected + ", preserved=" + preserved + ", count=" + selectedCells.Length + ")");
        cell.SetFocus();
        int focused = grid.Current.HasKeyboardFocus ? 1 : 0;
        foreach (AutomationElement item in grid.FindAll(TreeScope.Descendants, new PropertyCondition(AutomationElement.HasKeyboardFocusProperty, true))) focused++;
        verify(focused == 1, "P2 one focused semantic element in grid");
        var headers = Find(grid, ControlType.Header);
        verify(headers.Count == 1 && headers[0].Current.Orientation == OrientationType.Horizontal, "P2 header orientation is horizontal");
        var cellValue = Pattern<ValuePattern>(cell, ValuePattern.Pattern);
        var originalValue = cellValue.Current.Value;
        cellValue.SetValue("UIA grid value");
        verify(cellValue.Current.Value == "UIA grid value" && cell.Current.Name == "UIA grid value", "R2-11 editable cell Value updates the model");
        cellValue.SetValue(originalValue);
        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(2000), "grid value restoration is idle");
        var gridPattern = Pattern<GridPattern>(grid, GridPattern.Pattern);
        var gridHeaders = Pattern<TablePattern>(grid, TablePattern.Pattern).Current.GetColumnHeaders();
        var gridScroll = Pattern<ScrollPattern>(grid, ScrollPattern.Pattern);
        double originalScroll = gridScroll.Current.HorizontalScrollPercent;
        var firstHeader = Pattern<TransformPattern>(gridHeaders[0], TransformPattern.Pattern);
        double originalWidth = gridHeaders[0].Current.BoundingRectangle.Width;
        double originalHeight = gridHeaders[0].Current.BoundingRectangle.Height;
        firstHeader.Resize(grid.Current.BoundingRectangle.Width + 100, originalHeight);
        Wait(() => gridHeaders[0].Current.BoundingRectangle.Width > originalWidth, "grid column resize is laid out");
        var farCell = gridPattern.GetItem(0, gridPattern.Current.ColumnCount - 1);
        Wait(() => farCell.Current.IsOffscreen, "R2-10 distant grid column begins offscreen after column layout");
        Pattern<ScrollItemPattern>(farCell, ScrollItemPattern.Pattern).ScrollIntoView();
        Wait(() => !farCell.Current.IsOffscreen, "R2-10 horizontal scrolling reveals the distant cell");
        selectedCells = Pattern<SelectionPattern>(grid, SelectionPattern.Pattern).Current.GetSelection();
        verify(selectedCells.Length == 1 && Automation.Compare(selectedCells[0], cell), "R2-10 horizontal cell scrolling preserves selection");
        firstHeader.Resize(originalWidth, originalHeight);
        gridScroll.SetScrollPercent(originalScroll, ScrollPattern.NoScroll);

        var calendarPage = Page(Page(root, "Misc"), "DatePicker");
        var calendar = Find(calendarPage, ControlType.Calendar)[0];
        var weekdayHeaders = Pattern<TablePattern>(calendar, TablePattern.Pattern).Current.GetColumnHeaders();
        bool visibleHeaders = weekdayHeaders.Length == 7;
        foreach (var header in weekdayHeaders) { var bounds = header.Current.BoundingRectangle; visibleHeaders &= !bounds.IsEmpty && bounds.Width > 0 && bounds.Height > 0; }
        verify(visibleHeaders, "T6 calendar weekday headers have visible geometry");
        var calendarSelection = Pattern<SelectionPattern>(calendar, SelectionPattern.Pattern);
        var selectedDay = calendarSelection.Current.GetSelection()[0];
        var days = Pattern<GridPattern>(calendar, GridPattern.Pattern);
        var differentDay = days.GetItem(2, 3);
        if (Automation.Compare(selectedDay, differentDay)) differentDay = days.GetItem(2, 4);
        rejected = false;
        try { Pattern<SelectionItemPattern>(differentDay, SelectionItemPattern.Pattern).AddToSelection(); }
        catch (InvalidOperationException) { rejected = true; }
        verify(rejected && Automation.Compare(calendarSelection.Current.GetSelection()[0], selectedDay), "R2-07 calendar Add rejects a conflicting day without mutation");
        selectedDay.SetFocus();
        Check(Pattern<WindowPattern>(root, WindowPattern.Pattern).WaitForInputIdle(2000), "previous calendar focus transition settles");
        AutomationElement focusedDay = null;
        AutomationFocusChangedEventHandler focusHandler = (s,e) => { var element = s as AutomationElement; if (element != null && Automation.Compare(element, differentDay)) focusedDay = element; };
        Automation.AddAutomationFocusChangedEventHandler(focusHandler);
        try
        {
            verify(differentDay.Current.IsKeyboardFocusable, "R2-03 logical calendar day is focusable");
            differentDay.SetFocus();
            verify(differentDay.Current.HasKeyboardFocus, "R2-03 logical day reports keyboard focus after SetFocus");
            Wait(() => focusedDay != null, "R2-03 focus event identifies logical calendar day");
            CheckFocus(differentDay, "R2-03 logical calendar day");
        }
        finally { Automation.RemoveAutomationFocusChangedEventHandler(focusHandler); }
        var tabs = Find(root, ControlType.Tab, TreeScope.Children);
        verify(tabs.Count > 0 && tabs[0].Current.Orientation == OrientationType.Horizontal, "P2 tab orientation is horizontal");
        var tabSelection = Pattern<SelectionPattern>(tabs[0], SelectionPattern.Pattern);
        var originalTab = tabSelection.Current.GetSelection()[0];
        var otherTab = Find(tabs[0], ControlType.TabItem, TreeScope.Children)[0];
        if (Automation.Compare(originalTab, otherTab)) otherTab = Find(tabs[0], ControlType.TabItem, TreeScope.Children)[1];
        rejected = false;
        try { Pattern<SelectionItemPattern>(otherTab, SelectionItemPattern.Pattern).AddToSelection(); }
        catch (InvalidOperationException) { rejected = true; }
        verify(rejected && Automation.Compare(tabSelection.Current.GetSelection()[0], originalTab), "R2-07 tab Add rejects another page without mutation");

        Test("review / ribbon headers and menu mode events");
        var ribbonPage = Page(Page(root, "Control"), "Document Editor (Ribbon)");
        var home = Named(ribbonPage, "  HOME  ", ControlType.Button);
        var search = Find(ribbonPage, ControlType.Edit)[0];
        foreach (var headerControl in new [] { home, search })
        {
            var bounds = headerControl.Current.BoundingRectangle;
            verify(Automation.Compare(AutomationElement.FromPoint(new System.Windows.Point(bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2)), headerControl), "R2-02 ribbon header hit test: " + headerControl.Current.ControlType.ProgrammaticName);
            headerControl.SetFocus();
            CheckFocus(headerControl, "R2-02 ribbon header");
        }
        var menuPage = Page(Page(root, "Control"), "Document Editor (Toolstrip)");
        var menuEvents = new List<int>();
        AutomationEventHandler menuHandler = (s,e) => { lock(menuEvents) menuEvents.Add(e.EventId.Id); };
        var eventRoot = isHosted ? root : AutomationElement.RootElement;
        foreach (var eventId in new [] { MenuModeStartEvent, AutomationElement.MenuOpenedEvent, AutomationElement.MenuClosedEvent, MenuModeEndEvent })
            Automation.AddAutomationEventHandler(eventId, eventRoot, TreeScope.Subtree, menuHandler);
        try
        {
            var fileItem = Named(menuPage, "File", ControlType.MenuItem);
            var fileMenu = Pattern<ExpandCollapsePattern>(fileItem, ExpandCollapsePattern.Pattern);
            foreach (var entry in new [] { "ExpandCollapse", "keyboard", "pointer" })
            {
                lock(menuEvents) menuEvents.Clear();
                if (entry == "keyboard") { fileItem.SetFocus(); Press("Space"); }
                else if (entry == "pointer") Click(fileItem);
                else fileMenu.Expand();
                Wait(() => { lock(menuEvents) return menuEvents.Contains(AutomationElement.MenuOpenedEvent.Id); }, "R2-23 menu opened by " + entry);
                var nestedMenu = Pattern<ExpandCollapsePattern>(Named(menuPage, "Save as", ControlType.MenuItem), ExpandCollapsePattern.Pattern);
                nestedMenu.Expand();
                Wait(() => { lock(menuEvents) return menuEvents.FindAll(id => id == AutomationElement.MenuOpenedEvent.Id).Count == 2; }, "R2-23 nested menu opens in the same menu mode");
                fileMenu.Collapse();
                Wait(() => { lock(menuEvents) return menuEvents.Contains(MenuModeEndEvent.Id); }, "R2-23 menu mode ended event");
                lock(menuEvents) verify(menuEvents.Count == 6 && menuEvents[0] == MenuModeStartEvent.Id && menuEvents[1] == AutomationElement.MenuOpenedEvent.Id && menuEvents[2] == AutomationElement.MenuOpenedEvent.Id && menuEvents[3] == AutomationElement.MenuClosedEvent.Id && menuEvents[4] == AutomationElement.MenuClosedEvent.Id && menuEvents[5] == MenuModeEndEvent.Id, "R2-23 nested menu boundary events are paired and ordered: " + entry);
            }
        }
        finally
        {
            foreach (var eventId in new [] { MenuModeStartEvent, AutomationElement.MenuOpenedEvent, AutomationElement.MenuClosedEvent, MenuModeEndEvent })
                Automation.RemoveAutomationEventHandler(eventId, eventRoot, menuHandler);
        }

        Check(failures.Count == 0, "UIA review contract failures: " + string.Join("; ", failures.ToArray()));
    }
    static void TransitionContracts()
    {
        Test("review transitions / password, read-only, disabled text and horizontal scrolling");
        var edit = Find(root, ControlType.Edit)[0];
        var value = Pattern<ValuePattern>(edit, ValuePattern.Pattern);
        var text = Pattern<TextPattern>(edit, TextPattern.Pattern);
        var retained = text.DocumentRange;
        int passwordEvents = 0, readOnlyEvents = 0, leakedValues = 0;
        Automation.AddAutomationPropertyChangedEventHandler(edit, TreeScope.Element, (s,e) => {
            if (e.Property == AutomationElement.IsPasswordProperty) Interlocked.Increment(ref passwordEvents);
            if (e.Property == ValuePattern.IsReadOnlyProperty) Interlocked.Increment(ref readOnlyEvents);
            if (e.Property == ValuePattern.ValueProperty && ((AutomationElement)s).Current.IsPassword &&
                ((e.OldValue as string) == "public-before-password" || (e.NewValue as string) == "public-before-password")) Interlocked.Increment(ref leakedValues);
        }, AutomationElement.IsPasswordProperty, ValuePattern.ValueProperty, ValuePattern.IsReadOnlyProperty);
        Invoke(root, "Password on");
        Wait(() => edit.Current.IsPassword && passwordEvents > 0, "password transition notification");
        Absent(edit, TextPattern.Pattern);
        bool unavailable = false;
        try { retained.GetText(-1); } catch (ElementNotAvailableException) { unavailable = true; }
        Check(unavailable, "retained text range is unavailable while protected");
        try { Check(value.Current.Value != "public-before-password", "password Value property excludes plaintext"); }
        catch (UnauthorizedAccessException) { assertions++; }
        Check(leakedValues == 0, "password event payload excludes cached plaintext");
        Invoke(root, "Password off");
        Wait(() => !edit.Current.IsPassword && passwordEvents >= 2, "password removal notification");
        text = Pattern<TextPattern>(edit, TextPattern.Pattern);
        Invoke(root, "Read only");
        Wait(() => value.Current.IsReadOnly && readOnlyEvents > 0, "read-only transition notification");
        bool rejected = false;
        try { value.SetValue("must not replace"); } catch (InvalidOperationException) { rejected = true; }
        Check(rejected && value.Current.Value == "public-before-password", "read-only Value rejection is atomic");
        Invoke(root, "Editable");
        Wait(() => !value.Current.IsReadOnly && readOnlyEvents >= 2, "editable transition notification");
        var caret = text.DocumentRange; caret.MoveEndpointByRange(TextPatternRangeEndpoint.End, caret, TextPatternRangeEndpoint.Start);
        Invoke(root, "Disabled"); Wait(() => !edit.Current.IsEnabled, "disabled text");
        foreach (Action operation in new Action[] { caret.Select, caret.AddToSelection, caret.RemoveFromSelection })
        {
            rejected = false;
            try { operation(); } catch (ElementNotEnabledException) { rejected = true; }
            Check(rejected, "disabled text selection mutation rejected");
        }
        Invoke(root, "Enabled"); Wait(() => edit.Current.IsEnabled, "enabled text");
        value.SetValue(new string('x', 300) + "TARGET");
        caret = text.DocumentRange; caret.MoveEndpointByRange(TextPatternRangeEndpoint.End, caret, TextPatternRangeEndpoint.Start); caret.Select();
        var target = text.DocumentRange.FindText("TARGET", false, false);
        target.ScrollIntoView(false);
        Wait(() => target.GetBoundingRectangles().Length > 0, "horizontal text scroll reaches target");
        Check(text.GetSelection()[0].Compare(caret), "scrolling preserves insertion point");
        var progress = Pattern<RangeValuePattern>(Find(root, ControlType.ProgressBar)[0], RangeValuePattern.Pattern).Current;
        Check(progress.IsReadOnly && Double.IsNaN(progress.SmallChange) && Double.IsNaN(progress.LargeChange), "ProgressBar read-only step values are NaN");

        Test("review transitions / selection event cardinality and ordinary command sources");
        var list = Find(root, ControlType.List)[0];
        var items = Items(list);
        var events = new List<string>();
        Action<AutomationEvent,string> subscribe = (kind, name) => Automation.AddAutomationEventHandler(kind, list, TreeScope.Subtree, (s,e) => {
            lock (events) events.Add(name + ":" + ((AutomationElement)s).Current.Name);
        });
        subscribe(SelectionItemPattern.ElementSelectedEvent, "selected");
        subscribe(SelectionItemPattern.ElementAddedToSelectionEvent, "added");
        subscribe(SelectionItemPattern.ElementRemovedFromSelectionEvent, "removed");
        Action<string> expected = name => Wait(() => { lock (events) return events.Contains(name); }, name + " event");
        Pattern<SelectionItemPattern>(items[0], SelectionItemPattern.Pattern).Select(); expected("selected:0");
        Invoke(root, "Select second"); expected("added:1");
        Check(Pattern<SelectionPattern>(list, SelectionPattern.Pattern).Current.GetSelection().Length == 2, "application selection mutation adds second item");
        Invoke(root, "Remove first"); expected("selected:1");
        Invoke(root, "Clear selection"); expected("removed:1");
        lock (events) Check(events.Count == 4, "exact selection event types and sources across cardinalities");
        int invoked = 0;
        var command = Named(root, "Select second", ControlType.Button);
        Automation.AddAutomationEventHandler(InvokePattern.InvokedEvent, command, TreeScope.Element, (s,e) => Interlocked.Increment(ref invoked));
        command.SetFocus(); Press("Space");
        Wait(() => invoked == 1, "keyboard activation raises one Invoked event");
        Check(Pattern<SelectionItemPattern>(items[1], SelectionItemPattern.Pattern).Current.IsSelected, "keyboard activation executes application command");
        var checks = Find(root, ControlType.CheckBox);
        Check(checks.Count == 2, "non-mutex groups keep CheckBox semantics");
        foreach (var check in checks) { Absent(check, SelectionItemPattern.Pattern); Pattern<TogglePattern>(check, TogglePattern.Pattern).Toggle(); }
        foreach (var check in checks) Check(Pattern<TogglePattern>(check, TogglePattern.Pattern).Current.ToggleState == ToggleState.On, "non-mutex group permits independent selection");

        Test("review transitions / concurrent MTA readers share retained providers and ranges");
        var errors = new List<Exception>();
        var readers = new List<Thread>();
        for (int worker = 0; worker < 4; worker++)
        {
            var thread = new Thread(() => {
                try {
                    for (int i = 0; i < 30; i++) {
                        if (root.Current.FrameworkId != "GacUI" || text.DocumentRange.GetText(-1).Length != 306 || target.Clone().GetText(-1) != "TARGET") throw new Exception("Concurrent read changed semantic identity or text");
                    }
                } catch (Exception error) { lock (errors) errors.Add(error); }
            });
            thread.SetApartmentState(ApartmentState.MTA); thread.IsBackground = true;
            readers.Add(thread); thread.Start();
        }
        foreach (var reader in readers) Check(reader.Join(30000), "concurrent MTA reader completes");
        lock (errors) Check(errors.Count == 0, errors.Count == 0 ? "concurrent MTA reads agree" : errors[0].ToString());
    }

    static void BusyIdleContract()
    {
        var rootWindow = Pattern<WindowPattern>(root, WindowPattern.Pattern);
        Check(rootWindow.WaitForInputIdle(2000), "R2-12 settled application reaches idle");
        string eventName = "Local\\GacUI.UiaBusy." + ownedProcess;
        IntPtr started = OpenEvent(0x100000, false, eventName + ".Started");
        IntPtr release = OpenEvent(2, false, eventName + ".Release");
        Check(started != IntPtr.Zero && release != IntPtr.Zero, "busy synchronization handles");
        try
        {
            Input(BusyCommand);
            Check(WaitForSingleObject(started, 3000) == 0, "UI thread entered controlled busy state");
            var timer = Stopwatch.StartNew();
            bool idle = rootWindow.WaitForInputIdle(60);
            Console.WriteLine("Busy WaitForInputIdle: " + timer.ElapsedMilliseconds + " ms; idle=" + idle);
            Check(!idle && timer.ElapsedMilliseconds < 1000, "R2-12 busy wait honors timeout and returns false");
        }
        finally { SetEvent(release); CloseHandle(started); CloseHandle(release); }
        Check(rootWindow.WaitForInputIdle(2000), "R2-12 idle wait succeeds after busy work completes");
    }

    static void Review2Contracts()
    {
        var failures = new List<string>();
        Action<bool,string> verify = (condition, message) => {
            Console.WriteLine((condition ? "PASS " : "FAIL ") + message);
            if (condition) assertions++; else failures.Add(message);
        };
        var edit = Find(root, ControlType.Edit)[0];
        var value = Pattern<ValuePattern>(edit, ValuePattern.Pattern);
        var text = Pattern<TextPattern>(edit, TextPattern.Pattern);
        value.SetValue("\uD83D\uDE00");
        var found = text.DocumentRange.FindAttribute(TextPattern.FontWeightAttribute, 400, false);
        verify(found != null && found.GetText(-1) == "\uD83D\uDE00", "R2-15 attribute search preserves supplementary character");
        var range = text.DocumentRange.Clone();
        range.MoveEndpointByRange(TextPatternRangeEndpoint.End, range, TextPatternRangeEndpoint.Start);
        range.ExpandToEnclosingUnit(TextUnit.Format);
        verify(range.GetText(-1) == "\uD83D\uDE00", "R2-15 Format preserves supplementary character");
        value.SetValue("abc");
        range = text.DocumentRange.Clone();
        range.MoveEndpointByRange(TextPatternRangeEndpoint.End, range, TextPatternRangeEndpoint.Start);
        verify(Object.Equals(range.GetAttributeValue(TextPattern.FontWeightAttribute), 400), "R2-16 caret inherits uniform formatting");
        range.Move(TextUnit.Character, 2);
        int moved = range.Move(TextUnit.Document, -1);
        verify(moved == -1 && range.CompareEndpoints(TextPatternRangeEndpoint.Start, text.DocumentRange, TextPatternRangeEndpoint.Start) == 0, "R2-17 degenerate movement counts actual boundary");
        range = text.DocumentRange.FindText("b", false, false);
        moved = range.Move(TextUnit.Document, -1);
        verify(moved == 0 && range.GetText(-1) == "b", "R2-17 failed nondegenerate movement preserves endpoints");
        value.SetValue("one two");
        range = text.DocumentRange.Clone();
        range.MoveEndpointByRange(TextPatternRangeEndpoint.End, range, TextPatternRangeEndpoint.Start);
        range.ExpandToEnclosingUnit(TextUnit.Word);
        verify(range.GetText(-1) == "one ", "R2-18 Word includes trailing separator");

        verify(edit.Current.Name == "Account name" && edit.Current.LabeledBy != null && edit.Current.LabeledBy.Current.Name == "Account name", "R2-01 explicit label remains independent of value");
        verify(edit.Current.HelpText == "Your account identifier", "R2-05 tooltip text is owner HelpText before opening");
        Invoke(root, "Show tooltip");
        AutomationElement tooltip = null;
        Wait(() => { tooltip = Popup(ControlType.ToolTip); return tooltip != null; }, "tooltip opens");
        verify(tooltip.Current.Name == "Your account identifier" && !tooltip.Current.IsContentElement, "R2-05 ordinary tooltip name and content view");
        Invoke(root, "Hide tooltip");
        Wait(() => tooltip.Current.IsOffscreen, "tooltip closes");
        var combo = Named(root, "Review choices", ControlType.ComboBox);
        var comboLabel = combo.Current.LabeledBy;
        SelectCombo(root, combo, "Second choice");
        verify(combo.Current.Name == "Review choices" && Automation.Compare(combo.Current.LabeledBy, comboLabel), "R2-01 combo label remains stable across selection");
        var namedEdit = Named(root, "Explicit editor name", ControlType.Edit);
        Pattern<ValuePattern>(namedEdit, ValuePattern.Pattern).SetValue("new contents");
        verify(namedEdit.Current.Name == "Explicit editor name", "R2-01 application name remains independent of editor value");
        Invoke(root, "Interactive tooltip");
        Wait(() => { tooltip = Popup(ControlType.ToolTip); return tooltip != null; }, "interactive tooltip opens");
        verify(tooltip.Current.IsContentElement && tooltip.Current.Name == "Tooltip command", "R2-05 interactive tooltip belongs to content view");
        Invoke(root, "Hide interactive");
        Wait(() => tooltip.Current.IsOffscreen, "interactive tooltip closes");
        Invoke(root, "Plain popup");
        AutomationElement plainPopup = null;
        Wait(() => { foreach (var candidate in isHosted ? Find(root, ControlType.Pane) : OwnedWindows()) if (candidate.Current.Name == "Review plain popup") { plainPopup = candidate; return true; } return false; }, "plain popup opens");
        verify(plainPopup.Current.ControlType == ControlType.Pane, "R2-06 plain popup has Pane role");
        Absent(plainPopup, WindowPattern.Pattern);
        Absent(plainPopup, TransformPattern.Pattern);
        Invoke(root, "Hide popup");
        Wait(() => plainPopup.Current.IsOffscreen, "plain popup closes");

        var radio0 = Named(root, "Radio 0", ControlType.RadioButton);
        var radio1 = Named(root, "Radio 1", ControlType.RadioButton);
        var radio2 = Named(root, "Radio 2", ControlType.RadioButton);
        var selection0 = Pattern<SelectionItemPattern>(radio0, SelectionItemPattern.Pattern);
        var selection1 = Pattern<SelectionItemPattern>(radio1, SelectionItemPattern.Pattern);
        var selection2 = Pattern<SelectionItemPattern>(radio2, SelectionItemPattern.Pattern);
        var group0 = selection0.Current.SelectionContainer;
        verify(group0 != null && Automation.Compare(group0, selection1.Current.SelectionContainer) && !Automation.Compare(group0, selection2.Current.SelectionContainer), "R2-08 mutex identity spans layout parents and separates independent groups");
        var selected = Pattern<SelectionPattern>(group0, SelectionPattern.Pattern).Current.GetSelection();
        verify(selected.Length == 1 && Automation.Compare(selected[0], radio0), "R2-08 group Selection returns actual selected peer");
        bool rejected = false;
        try { selection1.AddToSelection(); } catch (InvalidOperationException) { rejected = true; }
        verify(rejected && selection0.Current.IsSelected && !selection1.Current.IsSelected, "R2-07 radio Add rejection preserves selection");
        selection1.Select();
        verify(selection1.Current.IsSelected && selection2.Current.IsSelected, "R2-08 selection affects only actual mutex peers");

        var farButton = Named(root, "Far button", ControlType.Button);
        edit.SetFocus();
        verify(farButton.Current.IsOffscreen, "R2-09 ordinary target begins outside scroll viewport");
        Pattern<ScrollItemPattern>(farButton, ScrollItemPattern.Pattern).ScrollIntoView();
        verify(!farButton.Current.IsOffscreen && edit.Current.HasKeyboardFocus, "R2-09 ScrollItem reveals ordinary control without changing focus");

        var documentElement = Named(root, "Review objects", ControlType.Document);
        var document = Pattern<TextPattern>(documentElement, TextPattern.Pattern);
        var children = document.DocumentRange.GetChildren();
        verify(children.Length == 3 && children[0].Current.ControlType == ControlType.Hyperlink && children[1].Current.ControlType == ControlType.Image, "R2-19 hyperlinks and images have text-child identities");
        foreach (var child in children)
        {
            var objectRange = document.RangeFromChild(child);
            verify(Automation.Compare(objectRange.GetEnclosingElement(), child), "R2-19 object range enclosure: " + child.Current.Name);
            var bounds = child.Current.BoundingRectangle;
            verify(!bounds.IsEmpty && bounds.Width > 0 && bounds.Height > 0, "R2-19 object bounds: " + child.Current.Name);
            var pointRange = document.RangeFromPoint(new System.Windows.Point(bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2));
            verify(pointRange.Compare(objectRange), "R2-20 point range encloses object: " + child.Current.Name);
            objectRange.ExpandToEnclosingUnit(TextUnit.Format);
            verify(objectRange.Compare(document.RangeFromChild(child)), "R2-18 Format stops at object boundary: " + child.Current.Name);
        }
        Pattern<InvokePattern>(children[0], InvokePattern.Pattern).Invoke();
        Wait(() => edit.Current.LabeledBy.Current.Name == "review://link", "R2-19 hyperlink invokes its document action");
        var nestedDocumentElement = Named(root, "Nested review objects", ControlType.Document);
        var nestedText = Pattern<TextPattern>(nestedDocumentElement, TextPattern.Pattern);
        // The entire stream is inside this link, so its text range's enclosing
        // element is the link and GetChildren already returns the nested objects.
        var nestedLink = Find(nestedDocumentElement, ControlType.Hyperlink, TreeScope.Children)[0];
        var nestedChildren = nestedText.RangeFromChild(nestedLink).GetChildren();
        verify(nestedChildren.Length == 2 && nestedChildren[0].Current.ControlType == ControlType.Image && nestedChildren[1].Current.ControlType == ControlType.Button, "R2-19 nested image and button remain reachable under hyperlink");
        foreach (var child in nestedChildren)
        {
            Same(TreeWalker.ControlViewWalker.GetParent(child), nestedLink, "R2-19 nested semantic parent");
            var objectRange = nestedText.RangeFromChild(child);
            Same(objectRange.GetEnclosingElement(), child, "R2-19 nested object enclosure");
            var bounds = child.Current.BoundingRectangle;
            verify(nestedText.RangeFromPoint(new System.Windows.Point(bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2)).Compare(objectRange), "R2-20 nested object point range");
            verify(objectRange.GetChildren().Length == 0, "R2-19 nested object does not repeat itself");
        }
        int documentTextEvents = 0, documentSelectionEvents = 0;
        Automation.AddAutomationEventHandler(TextPattern.TextChangedEvent, documentElement, TreeScope.Element, (s,e) => Interlocked.Increment(ref documentTextEvents));
        Automation.AddAutomationEventHandler(TextPattern.TextSelectionChangedEvent, documentElement, TreeScope.Element, (s,e) => Interlocked.Increment(ref documentSelectionEvents));
        range = document.DocumentRange;
        range.MoveEndpointByRange(TextPatternRangeEndpoint.Start, range, TextPatternRangeEndpoint.End);
        range.Select();
        Wait(() => documentSelectionEvents > 0, "fixture caret selection event");
        int selectionBefore = documentSelectionEvents;
        Invoke(root, "Replace document");
        Wait(() => documentTextEvents > 0 && documentSelectionEvents > selectionBefore, "R2-21 document replacement publishes text and caret changes");
        int textBefore = documentTextEvents;
        Invoke(root, "Update paragraph");
        Wait(() => documentTextEvents > textBefore && document.DocumentRange.GetText(-1).EndsWith("updated"), "R2-21 direct paragraph update publishes text event");

        AutomationElement views = null;
        foreach (AutomationElement candidate in root.FindAll(TreeScope.Descendants, Condition.TrueCondition))
        {
            object pattern;
            if (candidate.TryGetCurrentPattern(MultipleViewPattern.Pattern, out pattern)) { views = candidate; break; }
        }
        var multipleView = Pattern<MultipleViewPattern>(views, MultipleViewPattern.Pattern);
        verify(multipleView.GetViewName(0) == "Grandes icones" && multipleView.Current.GetSupportedViews()[0] == 0, "R2-24 localized view names preserve numeric IDs");
        verify(Find(views, ControlType.Header)[0].Current.Name == "En-tetes", "R2-24 header metadata uses application locale");

        Invoke(root, "Open secondary");
        AutomationElement secondary = null;
        Wait(() => { foreach (var candidate in isHosted ? Find(root, ControlType.Window) : OwnedWindows()) if (candidate.Current.Name == "Review secondary") { secondary = candidate; return true; } return false; }, "secondary window opens");
        var windowPattern = Pattern<WindowPattern>(secondary, WindowPattern.Pattern);
        Invoke(root, "Disable secondary");
        Wait(() => !secondary.Current.IsEnabled, "secondary disabled by application");
        verify(!windowPattern.Current.IsModal && windowPattern.Current.WindowInteractionState == WindowInteractionState.Running, "R2-14 manually disabled window is not modal-blocked");
        Invoke(root, "Enable secondary");
        Wait(() => secondary.Current.IsEnabled, "secondary enabled");
        int boundsEvents = 0;
        Automation.AddAutomationPropertyChangedEventHandler(secondary, TreeScope.Element, (s,e) => Interlocked.Increment(ref boundsEvents), AutomationElement.BoundingRectangleProperty);
        var secondaryChild = Find(secondary, ControlType.Button).Find(candidate => !candidate.Current.IsOffscreen);
        Check(secondaryChild != null, "visible secondary-window descendant");
        int childBoundsEvents = 0;
        Automation.AddAutomationPropertyChangedEventHandler(secondaryChild, TreeScope.Element, (s,e) => Interlocked.Increment(ref childBoundsEvents), AutomationElement.BoundingRectangleProperty);
        var childOriginal = secondaryChild.Current.BoundingRectangle;
        var transform = Pattern<TransformPattern>(secondary, TransformPattern.Pattern);
        var original = secondary.Current.BoundingRectangle;
        transform.Move(original.Left + 15, original.Top + 12);
        Wait(() => boundsEvents > 0 && secondary.Current.BoundingRectangle.Left != original.Left, "R2-22 translation publishes bounding rectangle event");
        Wait(() => childBoundsEvents > 0 && secondaryChild.Current.BoundingRectangle.Left != childOriginal.Left, "R2-22 translation publishes descendant bounds event");
        transform.Move(-100000, -100000);
        var movedBounds = secondary.Current.BoundingRectangle;
        var rootBounds = root.Current.BoundingRectangle;
        verify(!movedBounds.IsEmpty && movedBounds.Right > (isHosted ? rootBounds.Left : 0) && movedBounds.Bottom > (isHosted ? rootBounds.Top : 0), "R2-13 inaccessible Move is clamped into window container");
        windowPattern.Close();
        Wait(() => secondary.Current.IsOffscreen, "secondary closes before modal session");

        Invoke(root, "Open modal");
        Wait(() => !secondary.Current.IsOffscreen && windowPattern.Current.IsModal, "R2-14 actual modal session reports IsModal");
        verify(Pattern<WindowPattern>(root, WindowPattern.Pattern).Current.WindowInteractionState == WindowInteractionState.BlockedByModalWindow, "R2-14 modal owner reports blocked interaction state");
        windowPattern.Close();
        Wait(() => root.Current.IsEnabled && !windowPattern.Current.IsModal, "R2-14 closing modal restores owner and clears session state");

        Check(failures.Count == 0, "UIA review 2 failures: " + string.Join("; ", failures.ToArray()));
    }

    static void Review3Contracts()
    {
        Invoke(root, "Open review 3");
        AutomationElement review = null;
        Wait(() => { foreach (var candidate in isHosted ? Find(root, ControlType.Window) : OwnedWindows()) if (candidate.Current.Name == "UIA Review 3") { review = candidate; return true; } return false; }, "review 3 opens");
        var idle = Pattern<WindowPattern>(review, WindowPattern.Pattern);
        Action settle = () => Check(idle.WaitForInputIdle(2000), "review fixture settles");
        var failures = new List<string>();
        Action<string, Action> regression = (name, action) => {
            Test(name);
            try { settle(); action(); Console.WriteLine("PASS " + name); }
            catch (Exception error) { failures.Add(name + ": " + error.Message); Console.WriteLine("FAIL " + name + ": " + error.Message); }
        };
        regression("R3-01 protected rich document children", () => {
            var edit = Named(review, "Rich password", ControlType.Edit);
            var link = Named(edit, "secret", ControlType.Hyperlink);
            Invoke(review, "Mask rich password");
            Wait(() => edit.Current.IsPassword, "rich password becomes protected");
            bool unavailable = false;
            try { Check(link.Current.Name != "secret", "retained hyperlink exposes secret"); }
            catch (ElementNotAvailableException) { unavailable = true; }
            Check(unavailable, "retained protected child is unavailable");
            Check(Find(edit, ControlType.Hyperlink).Count == 0, "protected children omitted from discovery");
        });
        regression("R3-02 grid editor radio selection group", () => {
            var grid = Named(review, "Radio grid", ControlType.DataGrid);
            var cell = Pattern<GridPattern>(grid, GridPattern.Pattern).GetItem(0, 0);
            Pattern<InvokePattern>(cell, InvokePattern.Pattern).Invoke();
            AutomationElement a = null;
            Wait(() => { a = cell.FindFirst(TreeScope.Descendants, new PropertyCondition(AutomationElement.NameProperty, "Editor A")); return a != null; }, "radio editor opens beneath cell");
            var b = Named(cell, "Editor B", ControlType.RadioButton);
            var selectionA = Pattern<SelectionItemPattern>(a, SelectionItemPattern.Pattern);
            var selectionB = Pattern<SelectionItemPattern>(b, SelectionItemPattern.Pattern);
            var container = selectionB.Current.SelectionContainer;
            var selection = Pattern<SelectionPattern>(container, SelectionPattern.Pattern);
            Same(container, selectionA.Current.SelectionContainer, "editor radios share Selection container");
            Same(selection.Current.GetSelection()[0], a, "editor selection reports A");
            bool rejected = false;
            try { selectionB.AddToSelection(); } catch (InvalidOperationException) { rejected = true; }
            Check(rejected && selectionA.Current.IsSelected && !selectionB.Current.IsSelected, "conflicting add rejected without mutation");
        });
        regression("R3-03 spatial list coordinates", () => {
            var icons = Named(review, "Spatial icons", ControlType.List);
            var grid = Pattern<GridPattern>(icons, GridPattern.Pattern);
            Action<bool> verify = columnMajor => {
                int columns = grid.Current.ColumnCount, rows = grid.Current.RowCount;
                Check(columns >= 1 && rows >= 1, "spatial fixture has rows and columns");
                var items = Items(icons);
                Check(items.Count == 24 && (columnMajor ? columns == (24 + rows - 1) / rows : rows == (24 + columns - 1) / columns), "spatial dimensions cover model");
                for (int i = 0; i < items.Count; i++) {
                    var item = Pattern<GridItemPattern>(items[i], GridItemPattern.Pattern).Current;
                    Check(item.Row == (columnMajor ? i % rows : i / columns) && item.Column == (columnMajor ? i / rows : i % columns) && item.RowSpan == 1 && item.ColumnSpan == 1, "spatial item coordinates " + i);
                    Same(item.ContainingGrid, icons, "spatial containing grid");
                    Same(grid.GetItem(item.Row, item.Column), items[i], "GetItem returns existing list item");
                }
                var first = items[0].Current.BoundingRectangle;
                var next = items[columnMajor ? 1 : columns].Current.BoundingRectangle;
                Check(!first.IsEmpty && !next.IsEmpty && next.Top > first.Top && Math.Abs(next.Left - first.Left) < 2, "row coordinates match rendered arrangement");
                Absent(icons, TablePattern.Pattern);
            };
            verify(false); int before = grid.Current.ColumnCount;
            Invoke(review, "Narrow icons"); settle();
            Wait(() => grid.Current.ColumnCount != before, "spatial columns follow resize");
            verify(false);
            var views = Pattern<MultipleViewPattern>(icons, MultipleViewPattern.Pattern);
            foreach (int view in new [] { 1, 2, 3 }) {
                views.SetCurrentView(view); settle();
                grid = Pattern<GridPattern>(icons, GridPattern.Pattern);
                verify(view == 2);
            }
        });
        regression("R3-04 optional stable AutomationId", () => {
            var stable = Named(review, "Vertical tabs", ControlType.Tab);
            Check(stable.Current.AutomationId == "review3.vertical-tabs", "explicit application ID survives native and hosted sessions");
            Check(Named(review, "Spatial icons", ControlType.List).Current.AutomationId == "", "unspecified ID remains optional");
            foreach (AutomationElement element in review.FindAll(TreeScope.Descendants, Condition.TrueCondition))
                Check(!element.Current.AutomationId.StartsWith("gacui-"), "runtime allocation counter exposed as AutomationId");
        });
        regression("R3-05 vertical tab orientation and input", () => {
            var tab = Named(review, "Vertical tabs", ControlType.Tab);
            Check(tab.Current.Orientation == OrientationType.Vertical, "vertical Tab reports Vertical");
            tab.SetFocus();
            var hwnd = new IntPtr(isHosted ? root.Current.NativeWindowHandle : review.Current.NativeWindowHandle);
            Check(PostMessage(hwnd, 0x100, new IntPtr(0x28), IntPtr.Zero) && PostMessage(hwnd, 0x101, new IntPtr(0x28), IntPtr.Zero), "vertical arrow input posted");
            Wait(() => Pattern<SelectionItemPattern>(Named(tab, "Vertical second", ControlType.TabItem), SelectionItemPattern.Pattern).Current.IsSelected, "Down follows vertical tab order");
        });
        regression("R3-06 exclusive paragraph alignment", () => {
            var text = Pattern<TextPattern>(Named(review, "Paragraph alignment", ControlType.Document), TextPattern.Pattern);
            var first = text.DocumentRange.FindText("first", false, false);
            first.ExpandToEnclosingUnit(TextUnit.Paragraph);
            Check(first.GetText(-1) == "first\r\n", "first paragraph includes its separator");
            Check(Object.Equals(first.GetAttributeValue(TextPattern.HorizontalTextAlignmentAttribute), HorizontalTextAlignment.Left), "first paragraph alignment excludes second paragraph");
            var found = text.DocumentRange.FindAttribute(TextPattern.HorizontalTextAlignmentAttribute, HorizontalTextAlignment.Left, false);
            Check(found != null && found.GetText(-1) == "first\r\n", "alignment search includes first separator only");
            first.ExpandToEnclosingUnit(TextUnit.Format);
            Check(first.GetText(-1) == "first\r\n", "Format ends at alignment boundary");
        });
        regression("R3-07 uniform paragraph separator formatting", () => {
            var text = Pattern<TextPattern>(Named(review, "Uniform paragraphs", ControlType.Document), TextPattern.Pattern);
            Check(text.DocumentRange.GetText(-1) == "a\r\nb", "uniform paragraph fixture");
            var separator = text.DocumentRange.FindText("\r\n", false, false);
            Check(Object.Equals(separator.GetAttributeValue(TextPattern.FontWeightAttribute), 400), "separator has uniform font weight");
            Check(separator.GetAttributeValue(TextPattern.FontNameAttribute) is string, "separator has font name");
            var format = text.DocumentRange.FindText("a", false, false);
            format.ExpandToEnclosingUnit(TextUnit.Format);
            Check(format.GetText(-1) == "a\r\nb", "uniform Format crosses paragraph separator");
            foreach (bool backward in new [] { false, true }) {
                var found = text.DocumentRange.FindAttribute(TextPattern.FontWeightAttribute, 400, backward);
                Check(found != null && found.GetText(-1) == "a\r\nb", "uniform attribute search crosses separator");
            }
        });
        regression("R3-08 Direct2D supplementary character geometry", () => {
            var text = Pattern<TextPattern>(Named(review, "Wrapped emoji", ControlType.Document), TextPattern.Pattern);
            var emoji = text.DocumentRange.FindText("\uD83D\uDE00", false, false);
            Check(emoji != null, "emoji exists");
            Console.WriteLine("GEOMETRY unwrapped=" + string.Join(";", Array.ConvertAll(emoji.GetBoundingRectangles(), r => r.ToString())));
            bool wrapped = false;
            for (int i = 0; i < 30; i++) {
                var line = emoji.Clone(); line.ExpandToEnclosingUnit(TextUnit.Line);
                Console.WriteLine("GEOMETRY step=" + i + " line=" + line.GetText(-1));
                if (line.GetText(-1).StartsWith("\uD83D\uDE00")) { wrapped = true; break; }
                Invoke(review, "Narrow emoji"); settle();
            }
            Check(wrapped, "Direct2D soft wrap immediately before emoji");
            var before = text.DocumentRange.FindText("mmmm", false, false).GetBoundingRectangles();
            var rectangles = emoji.GetBoundingRectangles();
            Console.WriteLine("GEOMETRY previous=" + string.Join(";", Array.ConvertAll(before, r => r.ToString())) + " emoji=" + string.Join(";", Array.ConvertAll(rectangles, r => r.ToString())));
            Check(rectangles.Length == 1 && rectangles[0].Top >= before[before.Length - 1].Bottom - 1, "emoji rectangle excludes previous line");
        });
        regression("R3-09 composition bounds and clipping events", () => {
            var button = Named(review, "Moving button", ControlType.Button);
            var original = button.Current.BoundingRectangle;
            int bounds = 0, offscreen = 0;
            Automation.AddAutomationPropertyChangedEventHandler(button, TreeScope.Element, (s,e) => { if (e.Property == AutomationElement.BoundingRectangleProperty) Interlocked.Increment(ref bounds); else Interlocked.Increment(ref offscreen); }, AutomationElement.BoundingRectangleProperty, AutomationElement.IsOffscreenProperty);
            Invoke(review, "Move button");
            Wait(() => bounds > 0 && button.Current.BoundingRectangle != original, "composition-only move emits bounds");
            Invoke(review, "Clip button");
            Wait(() => offscreen > 0 && button.Current.IsOffscreen, "composition-only move emits offscreen");
        });
        regression("R3-10 logical document structure notification", () => {
            var document = Named(review, "Changing objects", ControlType.Document);
            int changes = 0;
            Automation.AddStructureChangedEventHandler(document, TreeScope.Element, (s,e) => Interlocked.Increment(ref changes));
            Invoke(review, "Add hyperlink");
            Wait(() => Find(document, ControlType.Hyperlink).Count == 1, "same text gains hyperlink identity");
            Wait(() => changes > 0, "logical document children emit structure event");
        });
        regression("R3-11 hyperlink pointer and Invoke events", () => {
            var document = Named(review, "Changing objects", ControlType.Document);
            var link = Named(document, "same", ControlType.Hyperlink);
            int events = 0;
            Automation.AddAutomationEventHandler(InvokePattern.InvokedEvent, link, TreeScope.Element, (s,e) => Interlocked.Increment(ref events));
            var rectangle = link.Current.BoundingRectangle;
            var hwnd = new IntPtr(isHosted ? root.Current.NativeWindowHandle : review.Current.NativeWindowHandle);
            var point = new NativePoint { X = (int)(rectangle.Left + rectangle.Width / 2), Y = (int)(rectangle.Top + rectangle.Height / 2) };
            Check(ScreenToClient(hwnd, ref point), "hyperlink pointer coordinates");
            var position = new IntPtr((point.Y << 16) | (point.X & 0xffff));
            foreach (uint message in new uint[] { 0x200, 0x201, 0x202 }) Check(PostMessage(hwnd, message, message == 0x201 ? new IntPtr(1) : IntPtr.Zero, position), "hyperlink pointer input posted");
            Wait(() => review.FindFirst(TreeScope.Descendants, new PropertyCondition(AutomationElement.NameProperty, "Clicks!")) != null, "pointer activates document hyperlink");
            Wait(() => events == 1, "pointer emits one hyperlink Invoked");
            Pattern<InvokePattern>(link, InvokePattern.Pattern).Invoke();
            Wait(() => events == 2, "UIA emits one additional hyperlink Invoked");
            settle(); Thread.Sleep(100);
            Check(events == 2, "hyperlink Invoke does not duplicate events");
        });
        regression("R3-12 modal-and-delete WindowClosed", () => {
            for (int cycle = 0; cycle < 25; cycle++) {
                Invoke(review, "Open deleting modal");
                AutomationElement modal = null;
                Wait(() => { foreach (var candidate in isHosted ? Find(root, ControlType.Window) : OwnedWindows()) if (candidate.Current.Name == "Deleting modal") { modal = candidate; return true; } return false; }, "deleting modal opens");
                using (var events = new NativeWindowEvents(modal.GetRuntimeId(), isHosted ? IntPtr.Zero : new IntPtr(modal.Current.NativeWindowHandle))) {
                    Pattern<WindowPattern>(modal, WindowPattern.Pattern).Close();
                    Wait(() => review.Current.IsEnabled, "modal deletion restores owner");
                    Wait(() => events.Closed > 0, "WindowClosed delivered before provider retirement");
                }
            }
            Console.WriteLine("PASS 25 consecutive modal deletion/subscription cycles");
        });
        regression("R3-13 Resize retains accessible placement", () => {
            Invoke(review, "Place resize window");
            AutomationElement window = null;
            Wait(() => { foreach (var candidate in isHosted ? Find(root, ControlType.Window) : OwnedWindows()) if (candidate.Current.Name == "Review secondary") { window = candidate; return true; } return false; }, "placed window opens");
            var transform = Pattern<TransformPattern>(window, TransformPattern.Pattern);
            transform.Resize(400, 300);
            var rectangle = window.Current.BoundingRectangle;
            Check(!rectangle.IsEmpty && rectangle.Right > (isHosted ? root.Current.BoundingRectangle.Left : 0), "shrunk window remains accessible");
            Pattern<WindowPattern>(window, WindowPattern.Pattern).Close();
        });
        regression("R3-14 off-desktop primary monitor fallback", () => {
            if (isHosted) { Console.WriteLine("Hosted child uses host containment; desktop fallback is covered in native mode."); return; }
            var monitors = new List<MonitorInfo>();
            EnumDisplayMonitors(IntPtr.Zero, IntPtr.Zero, (IntPtr monitor, IntPtr dc, ref NativeRect rectangle, IntPtr parameter) => {
                var info = new MonitorInfo { Size = Marshal.SizeOf(typeof(MonitorInfo)) };
                Check(GetMonitorInfo(monitor, ref info), "monitor geometry"); monitors.Add(info); return true;
            }, IntPtr.Zero);
            var primary = monitors.Find(m => (m.Flags & 1) != 0);
            if (monitors.Count == 1) Console.WriteLine("LIMITATION: secondary-monitor distinction requires a multimonitor desktop.");
            var secondary = monitors.Count > 1 ? monitors.Find(m => (m.Flags & 1) == 0) : primary;
            var transform = Pattern<TransformPattern>(review, TransformPattern.Pattern);
            transform.Move(secondary.Work.Left + 10, secondary.Work.Top + 10);
            var valid = review.Current.BoundingRectangle;
            Check(valid.Left >= secondary.Work.Left && valid.Left < secondary.Work.Right, "valid secondary-monitor placement preserved");
            double x = (secondary.Monitor.Left + secondary.Monitor.Right) / 2.0;
            double y = secondary.Monitor.Top < primary.Monitor.Top ? -100000 : 100000;
            transform.Move(x, y);
            var corrected = review.Current.BoundingRectangle;
            Check(corrected.Left >= primary.Work.Left && corrected.Left < primary.Work.Right && corrected.Top >= primary.Work.Top && corrected.Top < primary.Work.Bottom, "off-desktop request falls back to primary monitor");
            foreach (double extreme in new [] { double.MaxValue, -double.MaxValue }) {
                transform.Move(extreme, extreme);
                corrected = review.Current.BoundingRectangle;
                Check(corrected.Left >= primary.Work.Left && corrected.Left < primary.Work.Right && corrected.Top >= primary.Work.Top && corrected.Top < primary.Work.Bottom, "extreme finite Move remains representable and accessible");
            }
        });
        idle.Close();
        Check(failures.Count == 0, "UIA review 3 failures: " + string.Join("; ", failures.ToArray()));
    }

    public static string FileDialogPlayground;
    static AutomationElement FileMutationWindow(string title, bool input = true)
    {
        AutomationElement result = null;
        Wait(() => {
            var windows = isHosted ? Find(root, ControlType.Window) : OwnedWindows();
            result = windows.FindLast(window => window.Current.Name == title && window.Current.IsEnabled && !window.Current.IsOffscreen && (Find(window, ControlType.Edit).Count > 0) == input);
            return result != null;
        }, "active file dialog: " + title);
        return result;
    }
    static void FileMutationAction(AutomationElement window, string caption, bool automation)
    {
        if (automation) Click(Named(window, caption, ControlType.Button));
        else Invoke(window, caption);
        ControlsSnapshot();
    }
    static void FileMutationText(AutomationElement edit, string value, bool automation)
    {
        Console.WriteLine("File dialog input: " + value);
        if (automation)
        {
            Click(edit);
            Wait(() => edit.Current.HasKeyboardFocus, "file dialog input receives focus");
            Press("Home");
            Press("Shift+End");
            ControlsSnapshot();
            if (value.Length == 0) Press("Delete"); else Input("!Type:" + value);
        }
        else Pattern<ValuePattern>(edit, ValuePattern.Pattern).SetValue(value);
        ControlsSnapshot();
        Wait(() => Pattern<ValuePattern>(edit, ValuePattern.Pattern).Current.Value == value, "file dialog text entered: " + value + "; observed: " + Pattern<ValuePattern>(edit, ValuePattern.Pattern).Current.Value);
    }
    static void FileMutationSelect(AutomationElement cell, bool first, bool automation)
    {
        if (automation)
        {
            if (first) Click(cell);
            else
            {
                var bounds = cell.Current.BoundingRectangle;
                var point = UiaToClientPoint(bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2);
                Input("!LeftClick:" + point.X + "," + point.Y + ",ctrl");
            }
            ControlsSnapshot();
        }
        else
        {
            var selection = Pattern<SelectionItemPattern>(cell, SelectionItemPattern.Pattern);
            if (first) selection.Select(); else selection.AddToSelection();
        }
    }
    static void FileMutationNativeBatch(AutomationElement dialogs, AutomationElement page, string fixture, int sequence, bool automation, bool chinese)
    {
        Test("native file mutation failures / " + (automation ? "automation IO" : "UIA") + " / " + (chinese ? "Chinese" : "English"));
        var batch = System.IO.Path.GetFullPath(System.IO.Path.Combine(fixture, "Native-" + sequence));
        Check(batch.StartsWith(fixture + System.IO.Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase), "native fixture containment");
        System.IO.Directory.CreateDirectory(batch);
        foreach (var directory in new[] { "target", "c-unsafe", "d-partial/removed-child", "e-empty", "f-full" })
            System.IO.Directory.CreateDirectory(System.IO.Path.Combine(batch, directory));
        foreach (var file in new[] { "target/sentinel.txt", "c-unsafe/keep.txt", "d-partial/removed-child/data.txt", "d-partial/a-removable.txt", "d-partial/z-locked.txt", "a-locked.txt", "f-full/nested.txt", "z-success.txt" })
            System.IO.File.WriteAllText(System.IO.Path.Combine(batch, file), "native sentinel");
        var links = new[] { System.IO.Path.Combine(batch, "b-linked"), System.IO.Path.Combine(batch, "c-unsafe/link") };
        var ready = System.IO.Path.Combine(batch, "lock-ready.flag");
        var release = System.IO.Path.Combine(batch, "lock-release.flag");
        Func<string, string> quote = value => "'" + value.Replace("'", "''") + "'";
        string script = "$ErrorActionPreference='Stop'; try { ";
        foreach (var link in links) script += "New-Item -ItemType Junction -Path " + quote(link) + " -Target " + quote(System.IO.Path.Combine(batch, "target")) + " | Out-Null; ";
        script += "$a=[IO.File]::Open(" + quote(System.IO.Path.Combine(batch, "a-locked.txt")) + ",'Open','Read','ReadWrite'); ";
        script += "$b=[IO.File]::Open(" + quote(System.IO.Path.Combine(batch, "d-partial/z-locked.txt")) + ",'Open','Read','ReadWrite'); ";
        script += "[IO.File]::WriteAllText(" + quote(ready) + ",'ready'); while(-not [IO.File]::Exists(" + quote(release) + ")) { Start-Sleep -Milliseconds 20 } } finally { if($a){$a.Dispose()}; if($b){$b.Dispose()} }";
        var start = new ProcessStartInfo("powershell.exe", "-NoProfile -EncodedCommand " + Convert.ToBase64String(Encoding.Unicode.GetBytes(script)));
        start.UseShellExecute = false; start.CreateNoWindow = true; start.WindowStyle = ProcessWindowStyle.Hidden;
        using (var holder = Process.Start(start))
        {
            try
            {
                Wait(() => { Check(!holder.HasExited, "native lock helper remains alive"); return System.IO.File.Exists(ready); }, "native junctions and separate-process locked handles ready", 30000);
                SelectCombo(dialogs, Find(dialogs, ControlType.ComboBox)[0], chinese ? "zh-CN" : "en-US");
                var fields = Find(page, ControlType.Edit);
                FileMutationText(fields[1], batch, automation); FileMutationText(fields[2], "", automation);
                var multiple = Pattern<TogglePattern>(Named(page, "FileDialogAllowMultipleSelection", ControlType.CheckBox), TogglePattern.Pattern);
                if (multiple.Current.ToggleState != ToggleState.On) multiple.Toggle();
                FileMutationAction(page, "Show Open File Dialog", automation);
                var picker = FileMutationWindow("The Title");
                string create = chinese ? "新建文件夹" : "New Folder", delete = chinese ? "删除" : "Delete", ok = chinese ? "确定" : "OK", cancel = chinese ? "取消" : "Cancel";
                Wait(() => Named(picker, create, ControlType.Button).Current.IsEnabled, "native batch loaded");
                Pattern<TransformPattern>(picker, TransformPattern.Pattern).Resize(850, 600);
                var tree = Find(picker, ControlType.Tree)[0];
                var batchNode = Named(tree, System.IO.Path.GetFileName(batch), ControlType.TreeItem);
                Pattern<ExpandCollapsePattern>(batchNode, ExpandCollapsePattern.Pattern).Expand();
                Wait(() => FindNamed(tree, "d-partial", ControlType.TreeItem) != null, "current directory tree expanded");
                var partialNode = Named(tree, "d-partial", ControlType.TreeItem);
                var expansion = Pattern<ExpandCollapsePattern>(partialNode, ExpandCollapsePattern.Pattern);
                expansion.Expand();
                Wait(() => FindNamed(tree, "removed-child", ControlType.TreeItem) != null, "partial directory tree loaded");
                expansion.Collapse();
                var selected = new List<string> { "a-locked.txt", "b-linked", "c-unsafe", "d-partial", "e-empty", "f-full", "z-success.txt" };
                var failed = new List<string> { "a-locked.txt", "b-linked", "c-unsafe", "d-partial" };
                var ordered = new List<string>(); var failures = new List<string>();
                var listing = Find(picker, ControlType.List)[0];
                foreach (var cell in Items(listing))
                {
                    var name = cell.Current.Name;
                    if (!selected.Contains(name)) continue;
                    FileMutationSelect(cell, ordered.Count == 0, automation);
                    ordered.Add(name); if (failed.Contains(name)) failures.Add(name);
                }
                Check(ordered.Count == selected.Count, "all native batch targets selected");
                FileMutationText(Find(picker, ControlType.Edit)[0], "target/sentinel.txt", automation);
                FileMutationAction(picker, delete, automation);
                var confirmation = FileMutationWindow("The Title", false);
                string question = (chinese ? "确定要删除文件夹“" + batch + "”中的下列项目吗？\n" : "Are you sure to delete the following items in folder \"" + batch + "\"?\n") + string.Join("\n", ordered.ToArray());
                Check(FindNamed(confirmation, question, ControlType.Text) != null, "native confirmation matches ordered selected rows");
                FileMutationAction(confirmation, ok, automation);
                var error = FileMutationWindow("The Title", false);
                string message = (chinese ? "文件夹“" + batch + "”中的下列项目删除失败：\n" : "The following items failed to delete in folder \"" + batch + "\":\n") + string.Join("\n", failures.ToArray());
                Wait(() => FindNamed(error, message, ControlType.Text) != null, "combined native failure report");
                Check(System.IO.File.Exists(System.IO.Path.Combine(batch, "a-locked.txt")), "locked top-level file survives");
                Check(System.IO.File.Exists(System.IO.Path.Combine(batch, "d-partial/z-locked.txt")), "locked descendant survives");
                Check(!System.IO.Directory.Exists(System.IO.Path.Combine(batch, "d-partial/removed-child")), "recursive failure reflects actual partial removal");
                Console.WriteLine("Partial directory remaining ordinary file: " + System.IO.File.Exists(System.IO.Path.Combine(batch, "d-partial/a-removable.txt")));
                Check(!System.IO.Directory.Exists(System.IO.Path.Combine(batch, "e-empty")) && !System.IO.Directory.Exists(System.IO.Path.Combine(batch, "f-full")) && !System.IO.File.Exists(System.IO.Path.Combine(batch, "z-success.txt")), "later ordinary targets are attempted after failures");
                Check(System.IO.File.ReadAllText(System.IO.Path.Combine(batch, "target/sentinel.txt")) == "native sentinel" && System.IO.File.Exists(System.IO.Path.Combine(batch, "c-unsafe/keep.txt")), "selected and nested junctions preserve target and subtree sentinels");
                Check(System.IO.Directory.Exists(links[0]) && System.IO.Directory.Exists(links[1]), "rejected junction entries remain intact");
                FileMutationAction(error, ok, automation);
                Wait(() => picker.Current.IsEnabled, "native failure report dismissed");
                expansion = Pattern<ExpandCollapsePattern>(Named(tree, "d-partial", ControlType.TreeItem), ExpandCollapsePattern.Pattern);
                Wait(() => expansion.Current.ExpandCollapseState == ExpandCollapseState.LeafNode, "collapsed tree becomes a leaf after partial removal");
                Wait(() => Pattern<SelectionItemPattern>(batchNode, SelectionItemPattern.Pattern).Current.IsSelected, "tree retains current directory after reconciliation");
                Check(FindNamed(tree, "removed-child", ControlType.TreeItem) == null, "removed descendant stays absent");
                System.IO.File.WriteAllText(release, "release");
                Check(holder.WaitForExit(10000) && holder.ExitCode == 0, "locked handles released normally");
                Pattern<SelectionItemPattern>(Named(tree, "target", ControlType.TreeItem), SelectionItemPattern.Pattern).Select();
                Wait(() => Items(listing).Count == 1 && Items(listing)[0].Current.Name == "sentinel.txt", "remaining sibling navigates to its own contents");
                Pattern<SelectionItemPattern>(batchNode, SelectionItemPattern.Pattern).Select();
                Wait(() => Items(listing).Exists(item => item.Current.Name == "a-locked.txt"), "return to affected directory shows surviving entries");
                bool first = true;
                foreach (var cell in Items(listing))
                {
                    if (cell.Current.Name != "a-locked.txt" && cell.Current.Name != "d-partial") continue;
                    FileMutationSelect(cell, first, automation); first = false;
                }
                FileMutationAction(picker, delete, automation);
                FileMutationAction(FileMutationWindow("The Title", false), ok, automation);
                Wait(() => !System.IO.File.Exists(System.IO.Path.Combine(batch, "a-locked.txt")) && !System.IO.Directory.Exists(System.IO.Path.Combine(batch, "d-partial")), "retry succeeds after releasing native handles");
                Wait(() => Named(picker, create, ControlType.Button).Current.IsEnabled, "retry refresh completed");
                FileMutationAction(picker, cancel, automation);
                Wait(() => page.Current.IsEnabled, "native batch picker closed");
                multiple.Toggle();
            }
            finally
            {
                System.IO.File.WriteAllText(release, "release");
                if (!holder.WaitForExit(10000)) { holder.Kill(); holder.WaitForExit(); }
                foreach (var link in links)
                {
                    Check(System.IO.Path.GetFullPath(link).StartsWith(batch + System.IO.Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase), "fixture link cleanup containment");
                    if (System.IO.Directory.Exists(link))
                    {
                        Check((System.IO.File.GetAttributes(link) & System.IO.FileAttributes.ReparsePoint) != 0, "unlink only the owned junction");
                        System.IO.Directory.Delete(link, false);
                    }
                }
            }
        }
    }
    static void FileDialogMutations(IntPtr hwnd)
    {
        Check(isHosted, "file mutation scenario requires hosted fake dialogs");
        var playground = System.IO.Path.GetFullPath(FileDialogPlayground);
        Check(System.IO.Path.GetFileName(playground) == "FileDialogPlayground", "dedicated mutation playground");
        System.IO.Directory.CreateDirectory(playground);
        Check((System.IO.File.GetAttributes(playground) & System.IO.FileAttributes.ReparsePoint) == 0, "playground is an ordinary directory");
        var fixture = System.IO.Path.GetFullPath(System.IO.Path.Combine(playground, "UIA-" + ownedProcess));
        Check(fixture.StartsWith(playground + System.IO.Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase), "fixture is contained in playground");
        Check(!System.IO.Directory.Exists(fixture), "fresh owned fixture");
        System.IO.Directory.CreateDirectory(fixture);
        System.IO.Directory.CreateDirectory(System.IO.Path.Combine(fixture, "Existing"));
        System.IO.File.WriteAllText(System.IO.Path.Combine(fixture, "sentinel.txt"), "unselected sentinel");
        try
        {
            Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1000, 700);
            var dialogs = Page(Page(root, "Misc"), "Dialogs");
            var page = Page(dialogs, "Open/Save FileDialog");
            int sequence = 0;
            foreach (bool automation in new[] { true, false })
            foreach (bool chinese in new[] { false, true })
            foreach (bool save in new[] { false, true })
            {
                Test("file mutations / " + (automation ? "automation IO" : "UIA") + " / " + (chinese ? "Chinese" : "English") + " / " + (save ? "Save" : "Open"));
                SelectCombo(dialogs, Find(dialogs, ControlType.ComboBox)[0], chinese ? "zh-CN" : "en-US");
                var edits = Find(page, ControlType.Edit);
                Check(edits.Count == 6, "showcase file dialog fields");
                FileMutationText(edits[1], fixture, automation);
                FileMutationText(edits[2], "", automation);
                FileMutationAction(page, save ? "Show Save File Dialog" : "Show Open File Dialog", automation);
                var picker = FileMutationWindow("The Title");
                string create = chinese ? "新建文件夹" : "New Folder", delete = chinese ? "删除" : "Delete";
                string ok = chinese ? "确定" : "OK", cancel = chinese ? "取消" : "Cancel";
                Wait(() => Named(picker, create, ControlType.Button).Current.IsEnabled, "directory load enables creation");
                Check(!Named(picker, delete, ControlType.Button).Current.IsEnabled, "empty selection disables delete");
                Pattern<TransformPattern>(picker, TransformPattern.Pattern).Resize(save ? 560 : 800, save ? 360 : 520);
                foreach (var caption in new[] { create, delete })
                {
                    var command = Named(picker, caption, ControlType.Button);
                    var bounds = command.Current.BoundingRectangle;
                    Check(!command.Current.IsOffscreen && bounds.Width > 0 && bounds.Right <= picker.Current.BoundingRectangle.Right, "header action remains visible after resizing: " + caption);
                }
                var filename = Find(picker, ControlType.Edit)[0];
                FileMutationText(filename, "independent.txt", automation);
                FileMutationAction(picker, create, automation);
                var modal = FileMutationWindow(create);
                var input = Find(modal, ControlType.Edit)[0];
                Check(input.Current.HasKeyboardFocus, "new folder input receives logical focus");
                if (automation)
                {
                    Press("Tab"); ControlsSnapshot();
                    Wait(() => Find(modal, ControlType.Button).Exists(button => button.Current.HasKeyboardFocus), "Tab reaches a new folder action");
                    Press("Shift+Tab"); ControlsSnapshot();
                    Wait(() => input.Current.HasKeyboardFocus, "Shift+Tab restores the new folder input");
                }
                var labelBounds = Named(modal, chinese ? "请输入文件夹名称" : "Enter the folder name", ControlType.Text).Current.BoundingRectangle;
                var inputBounds = input.Current.BoundingRectangle;
                var okBounds = Named(modal, ok, ControlType.Button).Current.BoundingRectangle;
                var cancelBounds = Named(modal, cancel, ControlType.Button).Current.BoundingRectangle;
                Check(labelBounds.Top < inputBounds.Top && okBounds.Top >= inputBounds.Bottom && cancelBounds.Top == okBounds.Top && okBounds.Left < cancelBounds.Left, "new folder label, input and right-aligned action order");
                Check(cancelBounds.Right <= modal.Current.BoundingRectangle.Right && cancelBounds.Right > modal.Current.BoundingRectangle.Left + modal.Current.BoundingRectangle.Width * 0.75, "new folder actions remain on the right");
                FileMutationText(input, "Canceled", automation);
                FileMutationAction(modal, cancel, automation);
                Wait(() => picker.Current.IsEnabled, "cancel restores picker");
                Check(!System.IO.Directory.Exists(System.IO.Path.Combine(fixture, "Canceled")), "cancel creates nothing");
                if (!automation)
                {
                    FileMutationAction(picker, create, false);
                    var closing = FileMutationWindow(create);
                    FileMutationText(Find(closing, ControlType.Edit)[0], "TitleClosed", false);
                    Pattern<WindowPattern>(closing, WindowPattern.Pattern).Close();
                    Wait(() => picker.Current.IsEnabled, "title close restores picker");
                    Check(!System.IO.Directory.Exists(System.IO.Path.Combine(fixture, "TitleClosed")), "title close creates nothing");
                }
                FileMutationAction(picker, create, automation);
                modal = FileMutationWindow(create);
                input = Find(modal, ControlType.Edit)[0];
                foreach (var invalid in new[] { "Existing", "sentinel.txt", "", ".", "..", "child\\escape", "/escaped", "C:\\escaped", new string('x', 300) })
                {
                    FileMutationText(input, invalid, automation);
                    FileMutationAction(modal, ok, automation);
                    var error = FileMutationWindow(create, false);
                    Wait(() => Find(error, ControlType.Edit).Count == 0, "creation failure is a message box");
                    string expected = chinese
                        ? "无法在文件夹“" + fixture + "”中创建文件夹“" + invalid + "”" + (invalid == "Existing" ? "，因为它已经存在。" : "。")
                        : "In folder \"" + fixture + "\", \"" + invalid + "\" folder cannot be created" + (invalid == "Existing" ? " because it already exists." : ".");
                    Check(FindNamed(error, expected, ControlType.Text) != null, "complete localized creation failure");
                    FileMutationAction(error, ok, automation);
                    Wait(() => modal.Current.IsEnabled, "failure leaves input modal open");
                    Check(Pattern<ValuePattern>(input, ValuePattern.Pattern).Current.Value == invalid, "failed input retained");
                }
                string child = "Created 文件夹 " + (++sequence);
                FileMutationText(input, child, automation);
                FileMutationAction(modal, ok, automation);
                Wait(() => picker.Current.IsEnabled && System.IO.Directory.Exists(System.IO.Path.Combine(fixture, child)), "successful retry creates one child and closes modal");
                Check(Pattern<ValuePattern>(filename, ValuePattern.Pattern).Current.Value == "independent.txt", "refresh preserves independently entered filename");
                System.IO.File.WriteAllText(System.IO.Path.Combine(fixture, child, "nested.txt"), "recursive child");
                var listing = Find(picker, ControlType.List)[0];
                AutomationElement cell = null;
                Wait(() => { cell = Items(listing).Find(item => item.Current.Name == child); return cell != null; }, "created folder appears in list");
                FileMutationSelect(cell, true, automation);
                FileMutationText(filename, "sentinel.txt", automation);
                FileMutationAction(picker, delete, automation);
                var confirmation = FileMutationWindow("The Title", false);
                string question = chinese ? "确定要删除文件夹“" + fixture + "”中的下列项目吗？\n" + child : "Are you sure to delete the following items in folder \"" + fixture + "\"?\n" + child;
                Check(FindNamed(confirmation, question, ControlType.Text) != null, "confirmation uses selected row and excludes typed sentinel");
                FileMutationAction(confirmation, cancel, automation);
                Wait(() => picker.Current.IsEnabled, "delete cancellation restores picker");
                Check(System.IO.File.Exists(System.IO.Path.Combine(fixture, child, "nested.txt")), "cancel preserves recursive contents");
                FileMutationAction(picker, delete, automation);
                FileMutationAction(FileMutationWindow("The Title", false), ok, automation);
                Wait(() => picker.Current.IsEnabled && !System.IO.Directory.Exists(System.IO.Path.Combine(fixture, child)), "recursive deletion succeeds");
                Check(System.IO.File.ReadAllText(System.IO.Path.Combine(fixture, "sentinel.txt")) == "unselected sentinel", "unselected sentinel survives");
                Wait(() => !Named(picker, delete, ControlType.Button).Current.IsEnabled, "refresh clears deleted selection");
                var currentDirectory = Named(Find(picker, ControlType.Tree)[0], System.IO.Path.GetFileName(fixture), ControlType.TreeItem);
                Wait(() => Pattern<SelectionItemPattern>(currentDirectory, SelectionItemPattern.Pattern).Current.IsSelected, "tree retains current directory after deletion");
                Check(Pattern<ValuePattern>(filename, ValuePattern.Pattern).Current.Value == "sentinel.txt", "delete refresh preserves typed filename");
                FileMutationAction(picker, chinese ? (save ? "保存" : "打开") : (save ? "Save" : "Open"), automation);
                Wait(() => page.Current.IsEnabled, "ordinary file selection still completes");
            }
            foreach (bool automation in new[] { true, false })
            foreach (bool chinese in new[] { false, true })
                FileMutationNativeBatch(dialogs, page, fixture, ++sequence, automation, chinese);
            foreach (bool automation in new[] { true, false })
            {
                Test("synthetic drive-list command boundaries / " + (automation ? "automation IO" : "UIA"));
                SelectCombo(dialogs, Find(dialogs, ControlType.ComboBox)[0], "en-US");
                var fields = Find(page, ControlType.Edit);
                FileMutationText(fields[1], "", automation); FileMutationText(fields[2], "", automation);
                FileMutationAction(page, "Show Open File Dialog", automation);
                var picker = FileMutationWindow("The Title");
                var listing = Find(picker, ControlType.List)[0];
                AutomationElement drive = null;
                Wait(() => { drive = Items(listing).Find(item => item.Current.Name.Length == 2 && item.Current.Name[1] == ':'); return drive != null; }, "synthetic Windows drive rows loaded");
                Check(!Named(picker, "New Folder", ControlType.Button).Current.IsEnabled && !Named(picker, "Delete", ControlType.Button).Current.IsEnabled, "synthetic root disables both commands");
                FileMutationSelect(drive, true, automation);
                Check(!Named(picker, "Delete", ControlType.Button).Current.IsEnabled, "selecting a drive does not enable deletion");
                FileMutationAction(picker, "Cancel", automation);
                Wait(() => page.Current.IsEnabled, "drive-list inspection closed without mutation");
            }
        }
        finally
        {
            Check(System.IO.Path.GetFullPath(fixture).StartsWith(playground + System.IO.Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase), "cleanup remains in playground");
            foreach (var directory in System.IO.Directory.GetDirectories(fixture, "*", System.IO.SearchOption.AllDirectories))
                Check((System.IO.File.GetAttributes(directory) & System.IO.FileAttributes.ReparsePoint) == 0, "owned fixture has no links before cleanup");
            System.IO.Directory.Delete(fixture, true);
        }
    }

    public static void Run(int processId, bool hosted, string scenario, int port, string application)
    {
        Check(Thread.CurrentThread.GetApartmentState() == ApartmentState.MTA, "client must be MTA");
        IntPtr hwnd = IntPtr.Zero;
        Wait(() => { EnumWindows((h,p) => { uint id; GetWindowThreadProcessId(h,out id); var text=new StringBuilder(256); GetClassName(h,text,256); if (id==processId && text.ToString()=="VczhWindow" && IsWindowVisible(h)) hwnd=h; return true; },IntPtr.Zero); return hwnd!=IntPtr.Zero; }, "owned HWND");
        inputEndpoint = "http://localhost:" + port + "/Automation/" + application + "/IO";
        ownedProcess = processId; isHosted = hosted;
        isWorkflowBinary = application == "GacUI_Host";
        Wait(() => { root = AutomationElement.FromHandle(hwnd); return root.Current.ProcessId == processId && root.Current.FrameworkId == "GacUI"; }, "owned GacUI provider ready");
        if (scenario == "RawClock") RawClockAutomation();
        if (scenario == "Touching") TouchingAutomation();
        if (scenario == "PointRegression")
        {
            EditorPointCaret(hwnd);
            Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1000, 600);
            TouchingListAutomation();
        }
        if (scenario == "TouchingList")
        {
            Pattern<TransformPattern>(root, TransformPattern.Pattern).Resize(1000, 600);
            TouchingListAutomation();
        }
        if (scenario == "FileDialogMutations") FileDialogMutations(hwnd);
        Check(root.Current.ProcessId == processId && root.Current.FrameworkId == "GacUI", "owned native GacUI root; HWND=" + hwnd + "; PID=" + root.Current.ProcessId + "; FrameworkId=" + root.Current.FrameworkId + "; Name=" + root.Current.Name);
        Check(root.Current.Name == (scenario == "Transitions" || scenario == "Review2" || scenario == "Review3" ? "UIA Review Fixture" : "Complete Control Showcase"), "application title");
        if (scenario == "Concurrent")
        {
            Test("concurrent target / independent UIA selection");
            var selectedPage = Page(root, "Control");
            Check(Pattern<SelectionItemPattern>(selectedPage, SelectionItemPattern.Pattern).Current.IsSelected, "target Control page selected");
            selectedPage = Page(root, "List");
            Check(Pattern<SelectionItemPattern>(selectedPage, SelectionItemPattern.Pattern).Current.IsSelected, "target List page restored");
            Console.WriteLine("PASS concurrent target " + processId + " / " + assertions + " assertions");
            return;
        }
        // Isolate idle timing from UIA event-cache work that can block behind the busy UI thread.
        if (scenario == "Review2") BusyIdleContract();
        // Persistence checks use synchronous patterns and output bytes. Run its
        // template replacement before broad notification subscriptions; those
        // exercise event contracts separately and cache the entire rebuilt tree.
        if (scenario == "All" || scenario == "Persistence") DocumentPersistence(hwnd);
        if (scenario == "DocumentExchange") DocumentExchange(hwnd);
        if (scenario == "All" || scenario == "DocumentContextMenu") DocumentContextMenu(hwnd);
        if (scenario == "DocumentEditorSop" || scenario == "DocumentEditorCommands")
        {
            var page = Page(Page(root, "Control"), "Document Editor (Ribbon)");
            var document = Find(page, ControlType.Document)[0];
            var text = Pattern<TextPattern>(document, TextPattern.Pattern);
            // Capture the default before the SOP loads a mixed-format fixture.
            double defaultFontSize = Convert.ToDouble(text.DocumentRange.GetAttributeValue(TextPattern.FontSizeAttribute));
            if (scenario == "DocumentEditorSop") DocumentContextMenu(hwnd);
            DocumentEditorSop(hwnd);
            string folder = System.IO.Path.Combine(PersistenceOutput, "SOP-" + ownedProcess);
            DocumentStyles(hwnd, folder, defaultFontSize);
        }
        if (scenario == "DocumentStyles")
        {
            string folder = System.IO.Path.Combine(PersistenceOutput, "UIA-" + ownedProcess);
            System.IO.Directory.CreateDirectory(folder);
            DocumentStyles(hwnd, folder);
        }
        if (scenario != "RawClock" && scenario != "Touching" && scenario != "Persistence" && scenario != "DocumentExchange" && scenario != "DocumentStyles" && scenario != "DocumentContextMenu" && scenario != "DocumentEditorSop" && scenario != "DocumentEditorCommands" && scenario != "PointRegression" && scenario != "TouchingList" && scenario != "FileDialogMutations")
        {
            Automation.AddStructureChangedEventHandler(root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref structureEvents));
            Automation.AddAutomationPropertyChangedEventHandler(root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref propertyEvents), AutomationElement.NameProperty, SelectionItemPattern.IsSelectedProperty, TogglePattern.ToggleStateProperty);
            Automation.AddAutomationEventHandler(TextPattern.TextChangedEvent, root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref textEvents));
            Automation.AddAutomationEventHandler(TextPattern.TextSelectionChangedEvent, root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref selectionEvents));
        }
        try
        {
            if (scenario == "All" || scenario == "Layout" || scenario == "ShowcaseSkins") SkinItems(Page(root, "Window Manager"), "Default");
            if (scenario == "All" || scenario == "Review") ReviewContracts();
            if (scenario == "Transitions") TransitionContracts();
            if (scenario == "Review2") Review2Contracts();
            if (scenario == "Review3") Review3Contracts();
            if (scenario == "All" || scenario == "List") { TextLists(); ListViews(); Trees(); }
            if (scenario == "All" || scenario == "Grid") Grids();
            if (scenario == "All" || scenario == "Text") TextControls();
            if (scenario == "All" || scenario == "Refresh") RefreshLists();
            if (scenario == "All" || scenario == "Calendar") Calendars();
            // The notification assertions above are complete. Later animation,
            // resize and palette checks use current properties and snapshots;
            // retaining subtree subscriptions makes their rebuilds cache every
            // intermediate template and can starve layout on Debug builds.
            if (scenario == "All" || scenario == "Layout" || scenario == "ShowcaseSkins" || scenario == "ResponsivePalette") Automation.RemoveAllEventHandlers();
            if (scenario == "All" || scenario == "RawClock") RawClock(scenario == "All");
            if (scenario == "All" || scenario == "Touching") Touching(scenario == "All");
            if (scenario == "All" || scenario == "Layout" || scenario == "Responsive") ResponsiveLayout();
            if (scenario == "ResponsiveWindow") { ResponsiveRegressions(); ResponsiveLayout(false); }
            if (scenario == "ResponsivePalette") ResponsivePalette();
            if (scenario == "All" || scenario == "Layout" || scenario == "ShowcaseSkins") { EasyLayout(); EasyLayoutTables(); }
            if (scenario == "ShowcaseSkins") SkinRefreshListState();
            if (scenario == "All" || scenario == "Walk") WalkTabs(root, 0);
            if (scenario == "All" || scenario == "Window") {
            Test("Window / Transform.Move uses physical screen coordinates");
            NativeRect original, moved;
            Check(GetWindowRect(hwnd, out original), "read owned HWND bounds");
            var transform = Pattern<TransformPattern>(root, TransformPattern.Pattern);
            Check(transform.Current.CanMove, "restored root can move");
            transform.Move(original.Left + 7, original.Top + 9);
            Check(GetWindowRect(hwnd, out moved) && moved.Left == original.Left + 7 && moved.Top == original.Top + 9, "root moved by requested screen offset");
            transform.Move(original.Left, original.Top);
            Check(transform.Current.CanResize, "restored root can resize");
            transform.Resize(original.Right-original.Left+13, original.Bottom-original.Top+17);
            Wait(() => GetWindowRect(hwnd, out moved) && moved.Right-moved.Left == original.Right-original.Left+13 && moved.Bottom-moved.Top == original.Bottom-original.Top+17, "root resized in screen pixels");
            transform.Resize(original.Right-original.Left, original.Bottom-original.Top);
            Test("Control / Document Editor / menu popup");
            var menuPage = Page(Page(root, "Control"), "Document Editor (Toolstrip)");
            var richDocument = Find(menuPage, ControlType.Document)[0];
            var richText = Pattern<TextPattern>(richDocument, TextPattern.Pattern);
            richDocument.SetFocus();
            Check(richDocument.Current.HasKeyboardFocus, "rich document focus");
            Input("!Type:UIA command test");
            Wait(() => richText.DocumentRange.GetText(-1) == "UIA command test", "seed editable rich document");
            var toolbars = Find(menuPage, ControlType.ToolBar); Check(toolbars.Count > 0, "toolbar role");
            var commands = Find(toolbars[0], ControlType.Button); Check(commands.Count > 0, "toolbar command roles");
            foreach (var command in commands) { Pattern<InvokePattern>(command, InvokePattern.Pattern); Absent(command, TogglePattern.Pattern); }
            var fileItem = Named(menuPage, "File", ControlType.MenuItem); Absent(fileItem, TogglePattern.Pattern); Absent(fileItem, InvokePattern.Pattern);
            var fileMenu = Pattern<ExpandCollapsePattern>(fileItem, ExpandCollapsePattern.Pattern);
            fileMenu.Expand(); Wait(() => fileMenu.Current.ExpandCollapseState == ExpandCollapseState.Expanded, "File menu expanded");
            var menuPopup = Popup(ControlType.Menu); Check(menuPopup != null && Find(menuPopup, ControlType.MenuItem).Count > 0, "menu items exposed");
            foreach (var command in Find(menuPopup, ControlType.MenuItem)) { object expansion; if (!command.TryGetCurrentPattern(ExpandCollapsePattern.Pattern, out expansion)) { Pattern<InvokePattern>(command, InvokePattern.Pattern); Absent(command, TogglePattern.Pattern); } }
            fileMenu.Collapse(); Check(fileMenu.Current.ExpandCollapseState == ExpandCollapseState.Collapsed, "File menu collapsed");
            var editMenu = Pattern<ExpandCollapsePattern>(Named(menuPage, "Edit", ControlType.MenuItem), ExpandCollapsePattern.Pattern);
            editMenu.Expand(); Wait(() => editMenu.Current.ExpandCollapseState == ExpandCollapseState.Expanded, "Edit menu expanded");
            menuPopup = Popup(ControlType.Menu);
            Pattern<InvokePattern>(Named(menuPopup, "Select All", ControlType.MenuItem), InvokePattern.Pattern).Invoke();
            Wait(() => richText.GetSelection().Length == 1 && richText.GetSelection()[0].GetText(-1) == richText.DocumentRange.GetText(-1), "menu Invoke selects document");
            Wait(() => editMenu.Current.ExpandCollapseState == ExpandCollapseState.Collapsed, "command dismisses menu");
            var originalWeight = richText.DocumentRange.GetAttributeValue(TextPattern.FontWeightAttribute);
            Invoke(toolbars[0], "Bold");
            Wait(() => { var currentWeight = richText.DocumentRange.GetAttributeValue(TextPattern.FontWeightAttribute); return currentWeight is int && !currentWeight.Equals(originalWeight); }, "toolbar Invoke changes to a uniform font weight");
            int weight = (int)richText.DocumentRange.GetAttributeValue(TextPattern.FontWeightAttribute);
            Invoke(toolbars[0], "Bold");
            Wait(() => (int)richText.DocumentRange.GetAttributeValue(TextPattern.FontWeightAttribute) != weight, "toolbar Invoke changes document formatting");
            Test("Misc / Dialogs / modal Invoke returns before dismissal");
            var messagePage = Page(Page(Page(root, "Misc"), "Dialogs"), "MessageDialog");
            var showDialog = Pattern<InvokePattern>(Named(messagePage, "Show Dialog", ControlType.Button), InvokePattern.Pattern);
            var timer = Stopwatch.StartNew(); showDialog.Invoke(); timer.Stop();
            Console.WriteLine("Modal Invoke returned in " + timer.ElapsedMilliseconds + " ms");
            Check(timer.ElapsedMilliseconds < 3000, "Invoke must return before modal dismissal");
            AutomationElement modal = null;
            Wait(() => { var windows = hosted ? Find(root, ControlType.Window, TreeScope.Children) : OwnedWindows(); foreach (var w in windows) if (w.Current.Name == "The Title") { modal = w; return true; } return false; }, "message dialog opened");
            Check(Pattern<WindowPattern>(modal, WindowPattern.Pattern).Current.IsModal, "modal window state");
            var buttons = Find(modal, ControlType.Button); Check(buttons.Count > 0, "dialog dismissal button");
            // The showcase chooses its dialog locale independently of the OS.
            var dismiss = buttons.Find(button => button.Current.Name.Length > 0 && button.Current.IsEnabled);
            Check(dismiss != null, "localized message dialog dismissal button");
            try { Pattern<InvokePattern>(dismiss, InvokePattern.Pattern).Invoke(); }
            catch (InvalidOperationException error) {
                // Windows' native button proxy needs SendInput to focus a message
                // box, which is unavailable on the locked interactive desktop.
                // This fallback does not turn a failing GacUI Invoke into success.
                if (dismiss.Current.FrameworkId == "GacUI" || error.StackTrace == null || !error.StackTrace.Contains("SendInput")) throw;
                var buttonHandle = new IntPtr(dismiss.Current.NativeWindowHandle);
                uint buttonProcess; GetWindowThreadProcessId(buttonHandle, out buttonProcess);
                var buttonClass = new StringBuilder(256); GetClassName(buttonHandle, buttonClass, buttonClass.Capacity);
                Check(buttonProcess == processId && buttonClass.ToString() == "Button", "owned native dismissal button");
                Console.WriteLine("NOTE native message-box Invoke requires unavailable SendInput; dismissing owned button with BM_CLICK");
                Check(PostMessage(buttonHandle, 0x00F5, IntPtr.Zero, IntPtr.Zero), "native BM_CLICK accepted");
            }
            Wait(() => { try { return modal.Current.IsOffscreen; } catch(ElementNotAvailableException) { return true; } }, "modal dismissed");
            Test("Window Manager / asynchronous Invoke and logical subwindow lifecycle");
            var page = Page(root, "Window Manager");
            Invoke(page, "Open New Window");
            AutomationElement subwindow = null;
            Wait(() => { var windows = hosted ? Find(root, ControlType.Window, TreeScope.Children) : OwnedWindows(); foreach (var w in windows) if (w.Current.ControlType == ControlType.Window && !Automation.Compare(w,root)) { subwindow=w; return true; } return false; }, "subwindow opened");
            if (hosted) {
                var bounds = subwindow.Current.BoundingRectangle;
                Pattern<TransformPattern>(subwindow, TransformPattern.Pattern).Move(bounds.Left + 5, bounds.Top + 6);
                Wait(() => Math.Abs(subwindow.Current.BoundingRectangle.Left - bounds.Left - 5) < 1 && Math.Abs(subwindow.Current.BoundingRectangle.Top - bounds.Top - 6) < 1, "hosted subwindow moved in screen coordinates");
            }
            Pattern<WindowPattern>(subwindow, WindowPattern.Pattern).Close();
            Wait(() => { try { return subwindow.Current.IsOffscreen; } catch(ElementNotAvailableException) { return true; } }, "subwindow closed");
            Check(structureEvents > 0 && propertyEvents > 0, "structure and property notifications");
            Test("palette replacement / preserved state");
            var palette = Named(page, "Aurora", ControlType.RadioButton); Pattern<SelectionItemPattern>(palette, SelectionItemPattern.Pattern).Select();
            Wait(() => PaletteAttachmentRetired(palette), "old palette attachment retired by template replacement");
            root = AutomationElement.FromHandle(hwnd);
            page = Page(root, "Window Manager");
            palette = Named(page, "Aurora", ControlType.RadioButton);
            Check(Pattern<SelectionItemPattern>(palette, SelectionItemPattern.Pattern).Current.IsSelected, "palette selection");
            }
            Console.WriteLine("PASS " + assertions + " assertions; structure=" + structureEvents + "; property=" + propertyEvents + "; text=" + textEvents);
        }
        catch (Exception error) { Console.Error.WriteLine(error.ToString()); throw; }
        finally { Automation.RemoveAllEventHandlers(); }
        ControlsSnapshot();
        root = AutomationElement.FromHandle(hwnd);
        Test("normal Window.Close"); Pattern<WindowPattern>(root, WindowPattern.Pattern).Close();
        // Keep the out-of-process client alive until the queued Close executes.
        Wait(() => {
            try { using (var process = Process.GetProcessById(processId)) return process.HasExited; }
            catch (ArgumentException) { return true; }
        }, "owned application exits after Window.Close", 30000);
    }
}
