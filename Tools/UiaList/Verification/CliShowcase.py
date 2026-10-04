"""Exercise FullControlTest using UiaListCli parsing, dispatch and JSON only.

The independent expectations follow Test/UIA_CppTest_Shared.cs. No inspector or
showcase HTTP endpoint is used by this driver.
"""
import argparse
from pathlib import Path
from CliSmoke import Client


class Showcase:
    def __init__(self, pid, hosted):
        repo = Path(__file__).resolve().parents[3]
        self.c = Client(repo / "Tools/UiaList", repo / ".github/Scripts/copilotExecute.ps1")
        self.pid, self.hosted = pid, hosted
        self.window = next(w for p in self.c.send("List-Process")["result"]["processes"] if p["pid"] == pid for w in p["windows"] if w["title"] == "Complete Control Showcase")
        self.tree = self.c.send("Print-Window", self.window["id"])["result"]
        self.checks = 0

    def check(self, condition, detail):
        assert condition, detail
        self.checks += 1

    def refresh(self):
        self.tree = self.c.send("Refresh-Window", self.window["id"])["result"]

    def current(self, node):
        return next(n for n in self.tree["nodes"] if n["runtimeId"] == node["runtimeId"])

    def nodes(self, role=None, name=None, parent=None, direct=False):
        lookup = {n["id"]: n for n in self.tree["nodes"]}
        parent = self.current(parent)["id"] if parent else None
        result = []
        for n in self.tree["nodes"]:
            if role is not None and n["controlType"] != role: continue
            if name is not None and n["name"] != name: continue
            p = n["parentId"]
            if parent:
                while p and p != parent and not direct: p = lookup[p]["parentId"]
                if p != parent: continue
            result.append(n)
        return result

    def node(self, role=None, name=None, parent=None):
        found = self.nodes(role, name, parent)
        assert found, (role, name, [(n["controlType"], n["name"]) for n in self.tree["nodes"]])
        return found[0]

    def query(self, node):
        return self.c.send("Query-Node", self.current(node)["id"])["result"]

    def property(self, node, property_id):
        return next(p["value"]["value"] for p in self.query(node)["properties"] if p["propertyId"] == property_id)

    def run(self, operation, node, args=None):
        result = self.c.send("Run-" + operation, self.current(node)["id"], args)["result"]
        if "tree" in result: self.tree = result["tree"]
        return result

    def page(self, *names):
        parent = None
        for name in names:
            page = self.node(50019, name, parent)
            result = self.run("ISelectionItemProvider::Select", page)
            self.check(any(p["propertyId"] == 30079 and p["value"]["value"] is True for p in result["readback"]["properties"]), "Selected tab " + name)
            parent = self.current(page)
        return parent

    def invoke(self, name, parent=None):
        return self.run("IInvokeProvider::Invoke", self.node(50000, name, parent))

    def value(self, node, text):
        result = self.c.send("Set-Property", self.current(node)["id"], {"propertyId": 30045, "value": text})["result"]
        self.tree = result["tree"]
        self.check(any(p["propertyId"] == 30045 and p["value"]["value"] == text for p in result["readback"]["properties"]), "Actual text readback")

    def range(self, node):
        return self.run("ITextProvider::DocumentRange", node)["value"]["value"]

    def text(self, text_range):
        return self.c.send("Run-IUIAutomationTextRange::GetText", text_range, {"maxLength": -1})["result"]["value"]["value"]

    def text_lists(self):
        page = self.page("List", "TextList")
        self.invoke("Clear", page)
        self.invoke("Add 10 items", page)
        lists = self.nodes(50008, parent=page)
        self.check(len(lists) == 2, "Two semantic text lists")
        for container in lists:
            self.check([n["name"] for n in self.nodes(50007, parent=container, direct=True)] == [str(i) for i in range(10)], "Ten named list items")
        item = self.nodes(50007, parent=lists[0])[1]
        self.run("ISelectionItemProvider::Select", item)
        self.check(self.property(item, 30079) is True, "List selection")
        combo = self.node(50003, parent=page)
        for mode in ("Check", "Radio"):
            self.run("IExpandCollapseProvider::Expand", combo)
            choice = self.node(50007, mode)
            self.run("ISelectionItemProvider::Select", choice)
            if self.property(combo, 30070) == "1": self.run("IExpandCollapseProvider::Collapse", combo)
            old = self.property(item, 30086)
            self.run("IToggleProvider::Toggle", item)
            self.check(self.property(item, 30086) != old, mode + " toggle")
        self.invoke("Remove odd items", page)
        self.check([n["name"] for n in self.nodes(50007, parent=lists[0], direct=True)] == ["1", "3", "5", "7", "9"], "List deletion")
        checkbox = self.node(50002, parent=page)
        old = self.property(checkbox, 30086)
        self.run("IToggleProvider::Toggle", checkbox)
        self.check(self.property(checkbox, 30086) != old, "Checkbox state")
        radio = self.node(50013, parent=page)
        self.run("ISelectionItemProvider::Select", radio)
        self.check(self.property(radio, 30079) is True, "Radio selection")
        print("PASS showcase text lists and buttons", flush=True)

    def grids(self):
        page = self.page("List", "ListView")
        grids = self.nodes(50028, parent=page)
        self.check(len(grids) == 2, "Two detail list views")
        for grid in grids:
            self.check(self.property(grid, 30063) == "4", "Four list-view columns")
            headers = self.run("ITableProvider::GetCurrentColumnHeaders", grid)["value"]["value"]
            names = []
            for ref in headers:
                props = self.c.send("Query-Node", ref["value"])["result"]["properties"]
                names.append(next(p["value"]["value"] for p in props if p["propertyId"] == 30005))
            self.check(names == ["Id", "Category", "Size", "File"], "Header names")
            cell = self.run("IGridProvider::GetItem", grid, {"row": 0, "column": 2})["value"]["value"]
            props = self.c.send("Query-Node", cell)["result"]["properties"]
            self.check(any(p["propertyId"] == 30065 and p["value"]["value"] == "2" for p in props), "Returned cell column")
            rows = self.nodes(50029, parent=grid, direct=True)
            self.run("ISelectionItemProvider::Select", rows[0])
            self.run("IScrollItemProvider::ScrollIntoView", rows[-1])
            for view in range(6):
                self.run("IMultipleViewProvider::SetCurrentView", grid, {"value": view})
                self.check(self.property(grid, 30071) == str(view), "View " + str(view))
        page = self.page("List", "TreeView")
        for tree in self.nodes(50023, parent=page):
            blue = self.node(50024, "Blue+", tree)
            self.run("IExpandCollapseProvider::Expand", blue)
            children = self.nodes(50024, parent=blue, direct=True)
            self.check(bool(children), "Expanded tree children")
            self.run("ISelectionItemProvider::Select", children[0])
            self.check(self.property(children[0], 30079) is True, "Nested tree selection")
            self.run("IExpandCollapseProvider::Collapse", blue)
            self.check(not self.nodes(50024, parent=blue, direct=True), "Collapsed tree excludes descendants")
        page = self.page("List", "BindableDataGrid")
        grid = self.node(50028, parent=page)
        self.check(self.property(grid, 30062) == "5" and self.property(grid, 30063) == "5", "Five by five grid")
        cell = self.run("IGridProvider::GetItem", grid, {"row": 0, "column": 0})["value"]["value"]
        result = self.c.send("Run-IInvokeProvider::Invoke", cell)["result"]
        self.tree = result["tree"]
        editor = self.node(50004, parent=page)
        self.value(editor, "CLI edited name")
        self.check(bool(self.nodes(name="CLI edited name", parent=grid)), "Grid edit saved immediately")
        print("PASS showcase lists, six views, tree and editable grid", flush=True)

    def texts(self):
        for name in ("TextBox", "TextBox (No Tab)", "Document", "Document (No Tab)"):
            page = self.page("Control", "TextBox", name)
            inputs = [n for n in self.nodes(parent=page) if "IValueProvider" in n["providers"] and "ITextProvider" in n["providers"]]
            for edit in inputs:
                if self.property(edit, 30046): continue
                self.value(edit, "CLI text 日本語 中文")
                text_range = self.range(edit)
                self.check(self.text(text_range) == "CLI text 日本語 中文", "Text range matches Value")
                found = self.c.send("Run-IUIAutomationTextRange::FindText", text_range, {"text": "text", "backward": False, "ignoreCase": False})["result"]["value"]["value"]
                self.check(self.text(found) == "text", "FindText exact result")
                clone = self.c.send("Run-IUIAutomationTextRange::Clone", found)["result"]["value"]["value"]
                self.check(self.c.send("Run-IUIAutomationTextRange::Compare", found, {"range": clone})["result"]["value"]["value"], "Clone identity")
                result = self.c.send("Run-IUIAutomationTextRange::Select", found)["result"]
                self.tree = result["tree"]
                selected = self.run("ITextProvider::GetSelection", edit)["value"]["value"]
                self.check(len(selected) == 1 and self.text(selected[0]["value"]) == "text", "Selected text readback")
                self.value(edit, "")
                self.check(self.text(self.range(edit)) == "", "Empty document")
            for document in self.nodes(50030, parent=page):
                if "ITextProvider" not in document["providers"]: continue
                section = self.c.send("Query-Range", self.range(document))["result"]
                self.check(bool(section["readouts"]), "Document attributes and geometry")
        page = self.page("Control", "Embedded Controls")
        document = self.node(50030, parent=page)
        refs = self.c.send("Run-IUIAutomationTextRange::GetChildren", self.range(document))["result"]["value"]["value"]
        self.check(len(refs) == 17, "Seventeen embedded children")
        print("PASS showcase editable text, document ranges and embedded controls", flush=True)

    def calendar_layout(self):
        page = self.page("Misc", "DatePicker")
        calendars = self.nodes(50001, parent=page)
        self.check(len(calendars) == 2, "Two calendars")
        for calendar in calendars:
            self.check(self.property(calendar, 30062) == "6" and self.property(calendar, 30063) == "7", "Calendar grid dimensions")
            day = self.run("IGridProvider::GetItem", calendar, {"row": 2, "column": 2})["value"]["value"]
            result = self.c.send("Run-ISelectionItemProvider::Select", day)["result"]
            self.tree = result["tree"]
            self.check(any(p["propertyId"] == 30079 and p["value"]["value"] is True for p in result["readback"]["properties"]), "Calendar day selected")
        for name in ("Easy Layout", "Eazy Layout (Table)"):
            page = self.page("Layout", name)
            labels = ["Left one", "Left two", "Left three", "Right check"] if name == "Easy Layout" else ["Row A", "Row B", "Shared 120", "Column 1x", "Column 2x"]
            for label in labels:
                node = self.node(name=label, parent=page)
                b = node["bounds"]
                self.check(not node["offscreen"] and b["right"] > b["left"] and b["bottom"] > b["top"], "Layout bounds " + label)
            edit = self.node(50004, parent=page)
            self.value(edit, "CLI layout binding")
            self.check(bool(self.nodes(50020, "CLI layout binding", page)), "Layout binding readback")
            self.invoke("Rebuild" if name == "Easy Layout" else "Rebuild tables", page)
            self.page("Layout", name)
            self.check(bool(self.nodes(50004)), "Layout rebuilt providers reacquired")
        print("PASS showcase calendars and both Easy Layout pages", flush=True)

    def walk(self, parent=None, depth=0):
        assert depth < 8
        tabs = self.nodes(50018, parent=parent)
        # Only the outermost tab under this page; nested tabs are visited recursively.
        if not tabs: return
        pages = self.nodes(50019, parent=tabs[0], direct=True)
        for old in pages:
            if old["name"] == "Exit": continue
            self.run("ISelectionItemProvider::Select", self.current(old))
            current = self.current(old)
            for node in self.nodes(parent=current):
                self.check(node["controlType"] >= 50000 and bool(node["client"]) and bool(node["runtimeId"]), "Role and identity " + node["name"])
            self.walk(current, depth + 1)

    def window_operations(self):
        root = self.tree["nodes"][0]
        b = root["bounds"]
        self.run("ITransformProvider::Move", root, {"screenX": b["left"] + 7, "screenY": b["top"] + 9})
        moved = self.current(root)["bounds"]
        self.check(moved["left"] == b["left"] + 7 and moved["top"] == b["top"] + 9, "Physical window move")
        self.run("ITransformProvider::Move", root, {"screenX": b["left"], "screenY": b["top"]})
        self.run("ITransformProvider::Resize", root, {"width": b["right"]-b["left"]+13, "height": b["bottom"]-b["top"]+17})
        resized = self.current(root)["bounds"]
        self.check(resized["right"]-resized["left"] == b["right"]-b["left"]+13, "Window resize")
        self.run("ITransformProvider::Resize", root, {"width": b["right"]-b["left"], "height": b["bottom"]-b["top"]})
        page = self.page("Control", "Document Editor (Toolstrip)")
        self.check(bool(self.nodes(50021, parent=page)), "Toolbar role")
        file = self.node(50011, "File", page)
        self.run("IExpandCollapseProvider::Expand", file)
        self.check(self.property(file, 30070) == "1", "File menu expanded")
        self.run("IExpandCollapseProvider::Collapse", file)
        self.check(self.property(file, 30070) == "0", "File menu collapsed")
        self.refresh()
        page = self.node(50019, "Document Editor (Toolstrip)")
        document = self.node(50030, parent=page)
        self.value(document, "CLI toolbar command test")
        result = self.c.send("Run-IUIAutomationTextRange::Select", self.range(document))["result"]
        self.tree = result["tree"]
        before_weight = self.c.send("Run-IUIAutomationTextRange::GetAttributeValue", self.range(document), {"attributeId": 40007})["result"]["value"]["value"]
        self.invoke("Bold", page)
        after_weight = self.c.send("Run-IUIAutomationTextRange::GetAttributeValue", self.range(document), {"attributeId": 40007})["result"]["value"]["value"]
        self.check(before_weight != after_weight, "Toolbar changes document formatting")
        page = self.page("Misc", "Dialogs", "MessageDialog")
        self.invoke("Show Dialog", page)
        if self.hosted:
            modal = self.node(50032, "The Title")
        else:
            modal_window = next(w for p in self.c.send("List-Process")["result"]["processes"] if p["pid"] == self.pid for w in p["windows"] if w["title"] == "The Title")
            self.tree = self.c.send("Print-Window", modal_window["id"])["result"]
            modal = self.tree["nodes"][0]
        self.check(self.property(modal, 30077) is True, "Modal window state")
        self.check(bool(self.nodes(50000, parent=modal)), "Modal dismissal buttons")
        self.run("IWindowProvider::Close", modal)
        self.tree = self.c.send("Print-Window", self.window["id"])["result"]
        self.check(not self.nodes(50032, "The Title"), "Modal closed")
        page = self.page("Window Manager")
        self.invoke("Open New Window", page)
        if self.hosted:
            windows = [n for n in self.nodes(50032) if n["parentId"]]
            self.check(bool(windows), "Hosted logical window in same HWND tree")
            self.run("IWindowProvider::Close", windows[0])
        else:
            windows = [w for p in self.c.send("List-Process")["result"]["processes"] if p["pid"] == self.pid for w in p["windows"] if w["id"] != self.window["id"]]
            self.check(bool(windows), "Ordinary subwindow with own HWND")
            other = windows[-1]
            self.tree = self.c.send("Print-Window", other["id"])["result"]
            self.run("IWindowProvider::Close", self.tree["nodes"][0])
            self.tree = self.c.send("Print-Window", self.window["id"])["result"]
        page = self.page("Window Manager")
        palette = self.node(50013, "Aurora", page)
        self.run("ISelectionItemProvider::Select", palette)
        self.refresh()
        palette = self.node(50013, "Aurora")
        self.check(self.property(palette, 30079) is True, "Palette replacement retains selection")
        print("PASS showcase menus, toolbar, window lifecycle, transform and palette refresh", flush=True)

    def verify(self):
        self.text_lists()
        self.grids()
        self.texts()
        self.calendar_layout()
        self.walk()
        print("PASS showcase recursive tab inventory", flush=True)
        self.window_operations()
        self.run("IWindowProvider::Close", self.tree["nodes"][0])
        self.c.close()
        print(f"PASS showcase {self.checks} assertions; hosted={self.hosted}", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--pid", type=int, required=True)
    parser.add_argument("--hosted", action="store_true")
    options = parser.parse_args()
    Showcase(options.pid, options.hosted).verify()
