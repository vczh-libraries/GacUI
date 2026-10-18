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
        var bounds = element.Current.BoundingRectangle;
        Pointer("LeftClick", bounds.Left + bounds.Width / 2, bounds.Top + bounds.Height / 2);
    }
    static void Pointer(string action, double x, double y)
    {
        var point = new NativePoint { X = (int)x, Y = (int)y };
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        Check(ScreenToClient(hwnd, ref point), "convert pointer to client coordinates");
        uint dpi = GetDpiForWindow(hwnd);
        Check(dpi != 0, "read pointer coordinate DPI");
        Input("!" + action + ":" + (point.X * 96 / dpi) + "," + (point.Y * 96 / dpi));
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
            Check(response == "Queued", "input accepted: " + response);
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
        var focused = root.FindAll(TreeScope.Descendants, new PropertyCondition(AutomationElement.HasKeyboardFocusProperty, true));
        Check(focused.Count == 1 && Automation.Compare(focused[0], element), message + " unique reachable logical focus");
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
        Wait(() =>
        {
            var values = TouchingData();
            if (values.Length != 5 + count * 3 || values[4] != count) return false;
            for (int i = 0; i < 4; i++) if (Math.Abs(values[i] - transform[i]) > 0.001) return false;
            return true;
        }, description);
    }
    static void PointInput(string operation, int token, System.Windows.Rect bounds, int x, int y)
    {
        Input("!Point" + operation + ":" + token + "," + ((int)bounds.Left + x) + "," + ((int)bounds.Top + y));
    }
    static void PointInput(string operation, int token, AutomationElement element, double relativeX, double relativeY)
    {
        var bounds = element.Current.BoundingRectangle;
        var point = new NativePoint
        {
            X = (int)Math.Round(bounds.Left + bounds.Width * relativeX),
            Y = (int)Math.Round(bounds.Top + bounds.Height * relativeY),
        };
        var hwnd = new IntPtr(root.Current.NativeWindowHandle);
        Check(ScreenToClient(hwnd, ref point), "convert point coordinate to client coordinates");
        uint dpi = GetDpiForWindow(hwnd);
        Check(dpi != 0, "read point coordinate DPI");
        Input("!Point" + operation + ":" + token + "," + (int)Math.Round(point.X * 96.0 / dpi) + "," + (int)Math.Round(point.Y * 96.0 / dpi));
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
    static void TouchingAutomation()
    {
        Test("Touching / one and two contact transforms, membership, cancellation and source reuse");
        OpenTouching();
        var bounds = SnapshotBounds(TouchingSnapshot());
        Check(bounds.Width >= 320 && bounds.Height >= 240, "Touching raw region has useful bounds");
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
        var moved = new double[] { initial[0] + 30, initial[1] + 20, initial[2], initial[3] };
        TouchingState(1, moved, "one contact translates");
        PointInput("Down", 8, bounds, 150, 40);
        TouchingState(2, moved, "second contact rebases without jumping");
        PointInput("Update", 8, bounds, 250, 40);
        var scaled = new double[] { 150 + 2 * (moved[0] - 100), 40 + 2 * (moved[1] - 40), 2 * moved[2], 2 * moved[3] };
        TouchingState(2, scaled, "two contact scaling and translation");
        PointInput("Update", 8, bounds, 50, 240);
        var rotated = new double[] { 50 - 2 * (moved[1] - 40), 140 + 2 * (moved[0] - 100), -2 * moved[3], 2 * moved[2] };
        TouchingState(2, rotated, "two contact rotation");
        PointInput("Down", 9, bounds, 300, 150);
        PointInput("Update", 7, bounds, -30, -30);
        TouchingState(3, rotated, "three contacts freeze while moving outside raw bounds");
        var markers = TouchingData();
        Check(markers[6] == -30 && markers[7] == -30, "marker follows captured contact outside raw bounds");
        PointInput("Up", 9, bounds, 300, 150);
        TouchingState(2, rotated, "three to two rebases");
        PointInput("Up", 7, bounds, -30, -30);
        TouchingState(1, rotated, "original contact can lift without ending sequence");
        PointInput("Update", 8, bounds, 60, 255);
        var final = new double[] { rotated[0] + 10, rotated[1] + 15, rotated[2], rotated[3] };
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
        var boundary = new double[] { final[0] - 40, final[1] - 40, final[2], final[3] };
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
            if (changed) Wait(() => { try { var name = radio.Current.Name; return false; } catch (ElementNotAvailableException) { return true; } }, "palette replaced");
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
            Wait(() => { try { var name = oldRadio.Current.Name; return false; } catch (ElementNotAvailableException) { return true; } }, "old palette attachment retired after " + palette);
            root = AutomationElement.FromHandle(mainWindowHandle);
            manager = Page(root, "Window Manager");
            var currentRadio = Named(manager, palette, ControlType.RadioButton);
            Check(Pattern<SelectionItemPattern>(currentRadio, SelectionItemPattern.Pattern).Current.IsSelected, "palette " + palette + " selected after theme refresh");
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
            Wait(() => { try { var name = radio.Current.Name; return false; } catch (ElementNotAvailableException) { return true; } }, "table palette refresh completed");
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
        foreach (var tab in tabs) foreach (var page in Find(tab, ControlType.TabItem, TreeScope.Children))
        {
            Test("coverage / " + page.Current.Name);
            Pattern<SelectionItemPattern>(page, SelectionItemPattern.Pattern).Select();
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
        if (scenario != "RawClock" && scenario != "Touching")
        {
            Automation.AddStructureChangedEventHandler(root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref structureEvents));
            Automation.AddAutomationPropertyChangedEventHandler(root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref propertyEvents), AutomationElement.NameProperty, SelectionItemPattern.IsSelectedProperty, TogglePattern.ToggleStateProperty);
            Automation.AddAutomationEventHandler(TextPattern.TextChangedEvent, root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref textEvents));
            Automation.AddAutomationEventHandler(TextPattern.TextSelectionChangedEvent, root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref selectionEvents));
        }
        try
        {
            if (scenario == "All" || scenario == "Review") ReviewContracts();
            if (scenario == "Transitions") TransitionContracts();
            if (scenario == "Review2") Review2Contracts();
            if (scenario == "Review3") Review3Contracts();
            if (scenario == "All" || scenario == "List") { TextLists(); ListViews(); Trees(); }
            if (scenario == "All" || scenario == "Grid") Grids();
            if (scenario == "All" || scenario == "Text") TextControls();
            if (scenario == "All" || scenario == "Refresh") RefreshLists();
            if (scenario == "All" || scenario == "Calendar") Calendars();
            if (scenario == "All" || scenario == "RawClock") RawClock(scenario == "All");
            if (scenario == "All" || scenario == "Touching") Touching(scenario == "All");
            if (scenario == "All" || scenario == "Layout" || scenario == "Responsive") ResponsiveLayout();
            if (scenario == "ResponsiveWindow") { ResponsiveRegressions(); ResponsiveLayout(false); }
            if (scenario == "ResponsivePalette") ResponsivePalette();
            if (scenario == "All" || scenario == "Layout") { EasyLayout(); EasyLayoutTables(); }
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
            Wait(() => { try { var name = palette.Current.Name; return false; } catch (ElementNotAvailableException) { return true; } }, "old palette attachment retired by template replacement");
            root = AutomationElement.FromHandle(hwnd);
            page = Page(root, "Window Manager");
            palette = Named(page, "Aurora", ControlType.RadioButton);
            Check(Pattern<SelectionItemPattern>(palette, SelectionItemPattern.Pattern).Current.IsSelected, "palette selection");
            }
            Console.WriteLine("PASS " + assertions + " assertions; structure=" + structureEvents + "; property=" + propertyEvents + "; text=" + textEvents);
        }
        catch (Exception error) { Console.Error.WriteLine(error.ToString()); throw; }
        finally { Automation.RemoveAllEventHandlers(); }
        Test("normal Window.Close"); Pattern<WindowPattern>(root, WindowPattern.Pattern).Close();
    }
}
