using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Automation;
using System.Windows.Automation.Text;

public static class GacUIShowcaseTests
{
    delegate bool EnumProc(IntPtr hwnd, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumWindows(EnumProc callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr hwnd, out uint process);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetClassName(IntPtr hwnd, StringBuilder text, int count);
    [DllImport("user32.dll")] static extern bool IsWindowVisible(IntPtr hwnd);
    [StructLayout(LayoutKind.Sequential)] struct NativeRect { public int Left, Top, Right, Bottom; }
    [DllImport("user32.dll")] static extern bool GetWindowRect(IntPtr hwnd, out NativeRect rectangle);
    [DllImport("user32.dll", SetLastError=true)] static extern bool PostMessage(IntPtr hwnd, uint message, IntPtr wParam, IntPtr lParam);
    static string inputEndpoint;
    static void Press(string key) { Input("!KeyPress:" + key); }
    static void Input(string command)
    {
        using (var client = new System.Net.WebClient())
        {
            client.Headers[System.Net.HttpRequestHeader.ContentType] = "application/json; charset=utf8";
            var response = Encoding.UTF8.GetString(client.UploadData(inputEndpoint, Encoding.UTF8.GetBytes(command)));
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
    static bool isHosted;
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
        var windows = isHosted ? Find(root, ControlType.Window, TreeScope.Children) : OwnedWindows();
        if (isHosted) windows.AddRange(Find(root, ControlType.Menu, TreeScope.Children));
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
    static void Wait(Func<bool> predicate, string message)
    {
        var timer = Stopwatch.StartNew();
        while (timer.ElapsedMilliseconds < 10000) { if (predicate()) { assertions++; return; } Thread.Sleep(30); }
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
    static T Pattern<T>(AutomationElement element, AutomationPattern pattern) { object value; Check(element.TryGetCurrentPattern(pattern, out value), "Missing " + pattern.ProgrammaticName + " on " + element.Current.Name); return (T)value; }
    static void Absent(AutomationElement element, AutomationPattern pattern) { object value; Check(!element.TryGetCurrentPattern(pattern, out value), "Unexpected " + pattern.ProgrammaticName + " on " + element.Current.Name); }
    static void Invoke(AutomationElement parent, string name) { Pattern<InvokePattern>(Named(parent, name, ControlType.Button), InvokePattern.Pattern).Invoke(); }
    static AutomationElement Page(AutomationElement parent, string name)
    {
        var page = Named(parent, name, ControlType.TabItem);
        var selection = Pattern<SelectionItemPattern>(page, SelectionItemPattern.Pattern);
        selection.Select(); Wait(() => selection.Current.IsSelected, name + " selection");
        return page;
    }
    static List<AutomationElement> Items(AutomationElement list) { return Find(list, ControlType.ListItem, TreeScope.Children); }
    static void Same(AutomationElement actual, AutomationElement expected, string message) { Check(Automation.Compare(actual, expected), message); }
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
        Wait(() => { foreach (var window in OwnedWindows()) { item = window.FindFirst(TreeScope.Descendants, new AndCondition(new PropertyCondition(AutomationElement.NameProperty, name), new PropertyCondition(AutomationElement.ControlTypeProperty, ControlType.ListItem))); if (item != null) return true; } return false; }, "popup item " + name);
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
                if (views.GetViewName(view) == "Detail") Pattern<GridPattern>(list, GridPattern.Pattern); else Absent(list, GridPattern.Pattern);
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
        }
    }
    static void WalkTabs(AutomationElement container, int depth)
    {
        if (depth > 5) return;
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
                Check(seen.Add(current.AutomationId), "duplicate logical node: " + current.Name);
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

        Test("review / combo ownership and atomic rejection");
        var listPage = Page(Page(root, "List"), "TextList");
        var combo = Find(listPage, ControlType.ComboBox)[0];
        var expansion = Pattern<ExpandCollapsePattern>(combo, ExpandCollapsePattern.Pattern);
        expansion.Expand();
        Wait(() => expansion.Current.ExpandCollapseState == ExpandCollapseState.Expanded, "review combo opens");
        verify(Find(combo, ControlType.ListItem).Count > 0, "T1 combo dropdown items descend from combo");
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
        cell.SetFocus();
        int focused = grid.Current.HasKeyboardFocus ? 1 : 0;
        foreach (AutomationElement item in grid.FindAll(TreeScope.Descendants, new PropertyCondition(AutomationElement.HasKeyboardFocusProperty, true))) focused++;
        verify(focused == 1, "P2 one focused semantic element in grid");
        var headers = Find(grid, ControlType.Header);
        verify(headers.Count == 1 && headers[0].Current.Orientation == OrientationType.Horizontal, "P2 header orientation is horizontal");

        var calendarPage = Page(Page(root, "Misc"), "DatePicker");
        var calendar = Find(calendarPage, ControlType.Calendar)[0];
        var weekdayHeaders = Pattern<TablePattern>(calendar, TablePattern.Pattern).Current.GetColumnHeaders();
        bool visibleHeaders = weekdayHeaders.Length == 7;
        foreach (var header in weekdayHeaders) { var bounds = header.Current.BoundingRectangle; visibleHeaders &= !bounds.IsEmpty && bounds.Width > 0 && bounds.Height > 0; }
        verify(visibleHeaders, "T6 calendar weekday headers have visible geometry");
        var tabs = Find(root, ControlType.Tab, TreeScope.Children);
        verify(tabs.Count > 0 && tabs[0].Current.Orientation == OrientationType.Horizontal, "P2 tab orientation is horizontal");

        Check(failures.Count == 0, "UIA review contract failures: " + string.Join("; ", failures.ToArray()));
    }
    public static void Run(int processId, bool hosted, string scenario, int port)
    {
        Check(Thread.CurrentThread.GetApartmentState() == ApartmentState.MTA, "client must be MTA");
        IntPtr hwnd = IntPtr.Zero;
        Wait(() => { EnumWindows((h,p) => { uint id; GetWindowThreadProcessId(h,out id); var text=new StringBuilder(256); GetClassName(h,text,256); if (id==processId && text.ToString()=="VczhWindow" && IsWindowVisible(h)) hwnd=h; return true; },IntPtr.Zero); return hwnd!=IntPtr.Zero; }, "owned HWND");
        Wait(() => { root = AutomationElement.FromHandle(hwnd); return root.Current.ProcessId == processId && root.Current.FrameworkId == "GacUI"; }, "owned GacUI provider ready");
        inputEndpoint = "http://localhost:" + port + "/Automation/" + (hosted ? "CppTest" : "CppTest_Metaonly") + "/IO";
        ownedProcess = processId; isHosted = hosted;
        Check(root.Current.ProcessId == processId && root.Current.FrameworkId == "GacUI", "owned native GacUI root; HWND=" + hwnd + "; PID=" + root.Current.ProcessId + "; FrameworkId=" + root.Current.FrameworkId + "; Name=" + root.Current.Name);
        Check(root.Current.Name == "Complete Control Showcase", "showcase title");
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
        Automation.AddStructureChangedEventHandler(root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref structureEvents));
        Automation.AddAutomationPropertyChangedEventHandler(root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref propertyEvents), AutomationElement.NameProperty, SelectionItemPattern.IsSelectedProperty, TogglePattern.ToggleStateProperty);
        Automation.AddAutomationEventHandler(TextPattern.TextChangedEvent, root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref textEvents));
        Automation.AddAutomationEventHandler(TextPattern.TextSelectionChangedEvent, root, TreeScope.Subtree, (s,e) => Interlocked.Increment(ref selectionEvents));
        try
        {
            if (scenario == "Review") ReviewContracts();
            if (scenario == "All" || scenario == "List") { TextLists(); ListViews(); Trees(); }
            if (scenario == "All" || scenario == "Grid") Grids();
            if (scenario == "All" || scenario == "Text") TextControls();
            if (scenario == "All" || scenario == "Refresh") RefreshLists();
            if (scenario == "All" || scenario == "Calendar") Calendars();
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
            var timer = Stopwatch.StartNew(); Invoke(messagePage, "Show Dialog"); Check(timer.ElapsedMilliseconds < 3000, "Invoke must return before modal dismissal");
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
            Check(Pattern<SelectionItemPattern>(palette, SelectionItemPattern.Pattern).Current.IsSelected, "palette selection");
            }
            Console.WriteLine("PASS " + assertions + " assertions; structure=" + structureEvents + "; property=" + propertyEvents + "; text=" + textEvents);
        }
        catch (Exception error) { Console.Error.WriteLine(error.ToString()); throw; }
        finally { Automation.RemoveAllEventHandlers(); }
        Test("normal Window.Close"); Pattern<WindowPattern>(root, WindowPattern.Pattern).Close();
    }
}
