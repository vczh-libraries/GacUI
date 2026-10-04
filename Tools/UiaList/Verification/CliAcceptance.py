"""Independent synthetic-provider operation and large-tree checks through CLI JSON."""
import argparse
from pathlib import Path
from CliSmoke import Client


def verify(options):
    repo = Path(__file__).resolve().parents[3]
    client = Client(options.solution or repo / "Tools/UiaList", repo / ".github/Scripts/copilotExecute.ps1", options.configuration, options.platform)
    log = Path(__file__).parent / f"UiaFixture-{options.fixture}.synthetic.txt"
    def calls():
        return log.read_text(encoding="utf-16-le").splitlines()
    def discover():
        processes = client.send("List-Process")["result"]["processes"]
        return next(p for p in processes if p["pid"] == options.fixture)["windows"]
    windows = discover()
    synthetic = next(w for w in windows if w["class"] == "UiaSyntheticFixture" and "10,000" not in w["title"])
    tree = client.send("Print-Window", synthetic["id"])["result"]
    def node(name):
        return next(n["id"] for n in tree["nodes"] if n["name"] == name)
    capability = "Synthetic capabilities"
    document = "Synthetic document"
    current = client.send("Query-Properties", node(capability))["result"]
    special = next(p for p in current["properties"] if p["propertyId"] == 30158)
    assert special["value"]["value"] == "A", special  # Windows UIA truncates this property before client delivery.
    assert set(p["patternId"] for p in current["providers"] if p["patternId"]) | {10014, 10024, 10032} == set(range(10000, 10035))
    for malformed in ['Help-Command {broken}', 'Help-Command {"x":"\\q"}', 'Help-Command {"x":01}', 'Help-Command {"x":1e}', 'Help-Command {"x":NaN}', 'Help-Command {} {}']:
        assert client.send(malformed, ok=False)["error"]["code"] == "InvalidJson"
    before = calls()
    client.send("Run-IValueProvider::SetValue", node(capability), {"value": "invalid\0suffix"}, ok=False)
    client.send("Run-IRangeValueProvider::SetValue", node(capability), {"value": "12"}, ok=False)
    client.send("Run-IScrollProvider::Scroll", node(capability), {"horizontalAmount": True, "verticalAmount": 2}, ok=False)
    assert calls() == before, "Rejected input reached the provider"

    def run(command, args=None, method=None, target_name=capability, ok=True):
        nonlocal tree
        before = calls()
        old = node(target_name)
        response = client.send("Run-" + command, old, args, ok=ok)
        if not ok:
            return response
        result = response["result"]
        if method:
            added = [row for row in calls()[len(before):] if "\t" + method + "\t" in row]
            assert len(added) == 1, (command, method, added)
            assert added[0].startswith("target=" + ("2" if target_name == document else "1") + "\t"), added
            fields = dict(field.split("=", 1) for field in added[0].split("\t") if "=" in field)
            numeric = {
                "SetValue(R8)": ("value",), "Scroll": ("horizontalAmount", "verticalAmount"),
                "SetCurrentView": ("value",), "SetVisualState": ("value",), "SetDockPosition": ("value",),
                "Move": ("screenX", "screenY"), "Resize": ("width", "height"), "Rotate": ("degrees",),
                "Zoom": ("value",), "ZoomByUnit": ("zoomUnit",), "Legacy.Select": ("flagsSelect",),
                "StartListening": ("inputType",), "GetItem": ("row", "column"),
                "WaitForInputIdle": ("milliseconds",), "Custom.Navigate": ("direction",),
            }
            for field, name in zip(("a", "b"), numeric.get(method, ())):
                assert float(fields[field]) == args[name], (command, args, added)
            if method == "SetValue(BSTR)":
                text = args.get("value", args.get("szValue"))
                units = text.encode("utf-16-le")
                expected = " ".join(f"{int.from_bytes(units[i:i+2], 'little'):04X}" for i in range(0, len(units), 2))
                assert added[0].split("\t")[-1].strip() == expected
                assert int(fields["utf16"]) == len(units) // 2
            if method == "SetScrollPercent":
                assert float(fields["a"]) == (-1 if args["horizontalNoScroll"] else args["horizontalPercent"])
                assert float(fields["b"]) == (-1 if args["verticalNoScroll"] else args["verticalPercent"])
        if "tree" in result:
            tree = result["tree"]
            assert client.send("Query-Node", old, ok=False)["error"]["code"] == "ExpiredId"
            properties = {p["propertyId"]: p["value"]["value"] for p in result["readback"]["properties"]}
            states = {
                "SetValue(BSTR)": (30045, args.get("value", args.get("szValue"))),
                "SetValue(R8)": (30047, args.get("value")), "SetCurrentView": (30071, str(args.get("value"))),
                "SetVisualState": (30075, str(args.get("value"))), "SetDockPosition": (30069, str(args.get("value"))),
                "Zoom": (30145, args.get("value")), "Expand": (30070, "1"), "Collapse": (30070, "0"),
                "Select": (30079, True), "AddToSelection": (30079, True), "RemoveFromSelection": (30079, False),
            } if args else {"Expand": (30070, "1"), "Collapse": (30070, "0"), "Select": (30079, True), "AddToSelection": (30079, True), "RemoveFromSelection": (30079, False)}
            if method in states:
                property_id, expected = states[method]
                assert properties[property_id] == expected, (command, properties[property_id], expected)
        return result

    # Explicit independent inputs and expected provider methods, not generated from the inspector catalog.
    mutations = [
        ("IInvokeProvider::Invoke", None, "Invoke"),
        ("IValueProvider::SetValue", {"value": "Alpha 日本語\nBeta 中文"}, "SetValue(BSTR)"),
        ("IRangeValueProvider::SetValue", {"value": 38.5}, "SetValue(R8)"),
        ("IScrollProvider::Scroll", {"horizontalAmount": 4, "verticalAmount": 3}, "Scroll"),
        ("IScrollProvider::SetScrollPercent", {"horizontalNoScroll": False, "horizontalPercent": 17, "verticalNoScroll": False, "verticalPercent": 29}, "SetScrollPercent"),
        ("IExpandCollapseProvider::Expand", None, "Expand"),
        ("IExpandCollapseProvider::Collapse", None, "Collapse"),
        ("IMultipleViewProvider::SetCurrentView", {"value": 42}, "SetCurrentView"),
        ("IWindowProvider::SetWindowVisualState", {"value": 0}, "SetVisualState"),
        ("ISelectionItemProvider::RemoveFromSelection", None, "RemoveFromSelection"),
        ("ISelectionItemProvider::AddToSelection", None, "AddToSelection"),
        ("ISelectionItemProvider::Select", None, "Select"),
        ("IDockProvider::SetDockPosition", {"value": 5}, "SetDockPosition"),
        ("IToggleProvider::Toggle", None, "Toggle"),
        ("IToggleProvider::Toggle", None, "Toggle"),
        ("IToggleProvider::Toggle", None, "Toggle"),
        ("ITransformProvider::Move", {"screenX": -15, "screenY": 33}, "Move"),
        ("ITransformProvider::Resize", {"width": 310, "height": 210}, "Resize"),
        ("ITransformProvider::Rotate", {"degrees": 22.5}, "Rotate"),
        ("ITransformProvider2::Zoom", {"value": 150}, "Zoom"),
        ("ITransformProvider2::ZoomByUnit", {"zoomUnit": 4}, "ZoomByUnit"),
        ("IScrollItemProvider::ScrollIntoView", None, "ScrollIntoView"),
        ("ILegacyIAccessibleProvider::DoDefaultAction", None, "DoDefaultAction"),
        ("ILegacyIAccessibleProvider::Select", {"flagsSelect": 3}, "Legacy.Select"),
        ("ILegacyIAccessibleProvider::SetValue", {"szValue": "Alpha 日本語\nBeta 中文"}, "SetValue(BSTR)"),
        ("IVirtualizedItemProvider::Realize", None, "Realize"),
    ]
    for command, args, method in mutations:
        run(command, args, method)
    for value in range(5):
        run("IScrollProvider::Scroll", {"horizontalAmount": value, "verticalAmount": value}, "Scroll")
        run("ITransformProvider2::ZoomByUnit", {"zoomUnit": value}, "ZoomByUnit")
        run("ICustomNavigationProvider::Navigate", {"direction": value}, "Custom.Navigate")
    for value in range(6):
        run("IDockProvider::SetDockPosition", {"value": value}, "SetDockPosition")
    for value in range(3):
        run("IWindowProvider::SetWindowVisualState", {"value": value}, "SetVisualState")
    for value in (1, 2, 4, 8, 16, 3, 5, 9, 17, 12, 20, 13, 21):
        run("ILegacyIAccessibleProvider::Select", {"flagsSelect": value}, "Legacy.Select")
    for value in (1, 2, 4, 8, 16, 32, 3, 12, 48):
        run("ISynchronizedInputProvider::StartListening", {"inputType": value}, "StartListening")
        run("ISynchronizedInputProvider::Cancel", None, "Cancel")
    for horizontal in (False, True):
        for vertical in (False, True):
            run("IScrollProvider::SetScrollPercent", {"horizontalNoScroll": horizontal, "horizontalPercent": 25, "verticalNoScroll": vertical, "verticalPercent": 75}, "SetScrollPercent")
    before = calls()
    for command, arguments in [
        ("IRangeValueProvider::SetValue", {"value": 101}),
        ("ITransformProvider2::Zoom", {"value": 401}),
        ("ITransformProvider::Resize", {"width": 0, "height": 50}),
        ("IDockProvider::SetDockPosition", {"value": 6}),
        ("IWindowProvider::WaitForInputIdle", {"milliseconds": 5001}),
    ]:
        run(command, arguments, ok=False)
    assert calls() == before, "Out-of-bounds arguments reached the provider"
    run("IUIAutomationElement::SetFocus", method="SetFocus")
    run("IUIAutomationElement::GetClickablePoint")
    run("IUIAutomationElement7::GetCurrentMetadataValue", {"propertyId": 30005, "metadataId": 100000})
    run("IStylesProvider::GetCurrentExtendedPropertiesAsArray")
    run("ISynchronizedInputProvider::StartListening", {"inputType": 3}, "StartListening")
    run("ISynchronizedInputProvider::Cancel", None, "Cancel")
    assert run("IGridProvider::GetItem", {"row": 0, "column": 0}, "GetItem")["value"]["kind"] == "Element"
    assert run("IMultipleViewProvider::GetViewName", {"viewId": 42})["value"]["value"] == "View forty-two"
    assert run("IWindowProvider::WaitForInputIdle", {"milliseconds": 10}, "WaitForInputIdle")["value"]["value"] is True
    for command in ["ISelectionProvider::GetCurrentSelection", "ISelectionProvider2::GetCurrentSelection", "ITableProvider::GetCurrentRowHeaders", "ITableProvider::GetCurrentColumnHeaders", "ITableItemProvider::GetCurrentRowHeaderItems", "ITableItemProvider::GetCurrentColumnHeaderItems", "ILegacyIAccessibleProvider::GetCurrentSelection", "ISpreadsheetItemProvider::GetCurrentAnnotationObjects", "ISpreadsheetItemProvider::GetCurrentAnnotationTypes", "IDragProvider::GetCurrentGrabbedItems"]:
        assert run(command)["value"]["kind"] == "Array", command
    for command in ["ILegacyIAccessibleProvider::GetIAccessible", "IObjectModelProvider::GetUnderlyingObjectModel"]:
        assert run(command)["value"]["kind"] == "Opaque", command
    assert run("ISpreadsheetProvider::GetItemByName", {"name": "A1"}, "GetItemByName")["value"]["kind"] == "Element"
    assert run("ISpreadsheetProvider::GetItemByName", {"name": "missing"}, "GetItemByName")["value"]["kind"] == "Null"
    run("ICustomNavigationProvider::Navigate", {"direction": 0}, "Custom.Navigate")
    external = run("IItemContainerProvider::FindItemByProperty", {"startAfter": None, "propertyId": 30005, "value": "External document"}, "FindItemByProperty")
    external_id = external["value"]["value"]
    assert next(r for r in external["references"] if r["id"] == external_id)["treeNodeId"] is None
    assert client.send("Select-Node", external_id, ok=False)["error"]["code"] == "OutsideTree"
    client.send("Query-Node", external_id)
    external_range = client.send("Run-ITextProvider::DocumentRange", external_id)["result"]["value"]["value"]
    external_text = client.send("Run-IUIAutomationTextRange::GetText", external_range, {"maxLength": -1})["result"]["value"]
    assert external_text["value"] == "A", external_text  # CDB confirmed UIA returned a one-character BSTR; direct encoder tests preserve all seven units.
    for attribute, expected in [(40006, {"nonfinite": "NaN"}), (40010, {"nonfinite": "Infinity"}), (40011, {"nonfinite": "-Infinity"})]:
        actual = client.send("Run-IUIAutomationTextRange::GetAttributeValue", external_range, {"attributeId": attribute})["result"]["value"]
        assert actual["value"] == expected, (attribute, actual)
    text_range = run("ITextProvider::DocumentRange", method="Text.DocumentRange", target_name=document)["value"]["value"]
    before = calls()
    assert client.send("Run-IUIAutomationTextRange::Compare", text_range, {"range": external_range}, ok=False)["error"]["code"] == "WrongDocument"
    assert client.send("Run-IUIAutomationTextRange::Compare", text_range, {"range": node(capability)}, ok=False)["error"]["code"] == "WrongKind"
    assert calls() == before
    for command, args, method in [
        ("ITextProvider::GetSelection", None, "Text.GetSelection"),
        ("ITextProvider::GetVisibleRanges", None, "Text.GetVisibleRanges"),
        ("ITextProvider::RangeFromPoint", {"screenX": 900, "screenY": 160}, "Text.RangeFromPoint"),
        ("ITextProvider::RangeFromChild", {"child": node(capability)}, "Text.RangeFromChild"),
        ("ITextProvider2::RangeFromAnnotation", {"annotation": node(capability)}, "Text.RangeFromAnnotation"),
        ("ITextProvider2::GetCaretRange", None, "Text.GetCaretRange"),
        ("ITextEditProvider::GetActiveComposition", None, "Text.GetActiveComposition"),
        ("ITextEditProvider::GetConversionTarget", None, "Text.GetConversionTarget"),
    ]:
        run(command, args, method, document)
    run("ITextChildProvider::TextRange", method="TextChild.TextRange")
    section = client.send("Query-Range", text_range)["result"]
    values = [p["value"] for p in section["readouts"]]
    assert any(v["kind"] == "Mixed" for v in values)
    assert any(v["kind"] == "Unsupported" for v in values)
    assert any(v["kind"] == "Array" and v["dimensions"] == [{"lower": 0, "upper": 1}] and [x["value"] for x in v["value"]] == [10, 20] for v in values)
    clone = client.send("Run-IUIAutomationTextRange::Clone", text_range)["result"]["value"]["value"]
    for unit in range(7):
        client.send("Run-IUIAutomationTextRange::ExpandToEnclosingUnit", text_range, {"unit": unit})
        client.send("Run-IUIAutomationTextRange::Move", text_range, {"unit": unit, "count": 0})
        for endpoint in (0, 1):
            client.send("Run-IUIAutomationTextRange::MoveEndpointByUnit", text_range, {"endpoint": endpoint, "unit": unit, "count": 0})
    range_cases = [
        ("Compare", {"range": clone}), ("CompareEndpoints", {"srcEndPoint": 0, "range": clone, "targetEndPoint": 1}),
        ("FindText", {"text": "Beta", "backward": False, "ignoreCase": True}),
        ("FindAttribute", {"attributeId": 40005, "value": "Segoe UI", "backward": False}),
        ("GetAttributeValue", {"attributeId": 40006}), ("GetText", {"maxLength": -1}),
        ("GetBoundingRectangles", None), ("GetEnclosingElement", None), ("GetChildren", None),
        ("ExpandToEnclosingUnit", {"unit": 0}), ("Move", {"unit": 2, "count": 1}),
        ("MoveEndpointByUnit", {"endpoint": 1, "unit": 0, "count": -1}),
        ("MoveEndpointByRange", {"srcEndPoint": 0, "range": clone, "targetEndPoint": 0}),
    ]
    for method, args in range_cases:
        before = calls()
        client.send("Run-IUIAutomationTextRange::" + method, text_range, args)
        if method not in ("GetEnclosingElement", "GetChildren"):
            assert len([row for row in calls()[len(before):] if "\tRange." + method + "\t" in row]) == 1, method
    all_attributes = client.send("Run-IUIAutomationTextRange::ReadAllAttributes", text_range)["result"]["value"]
    assert all_attributes["kind"] == "Array" and len(all_attributes["value"]) == 44
    for method, args in [("Select", None), ("AddToSelection", None), ("RemoveFromSelection", None), ("ScrollIntoView", {"alignToTop": True})]:
        text_range = client.send("Run-ITextProvider::DocumentRange", node(document))["result"]["value"]["value"]
        result = client.send("Run-IUIAutomationTextRange::" + method, text_range, args)["result"]
        tree = result["tree"]
        assert client.send("Query-Range", text_range, ok=False)["error"]["code"] == "ExpiredId"
    text_range = client.send("Run-ITextProvider::DocumentRange", node(document))["result"]["value"]["value"]
    section = client.send("Query-Range", text_range)["result"]
    for action in section["commands"]:
        if "TextRange3::" in action["command"]:
            client.send(action["command"], text_range, {"attributeIds": [40005, 40006]} if action["parameters"] else None)
    print("PASS pattern and range operation coverage", flush=True)

    native = next(w for w in windows if w["class"] == "UiaFixture")
    tree = client.send("Print-Window", native["id"])["result"]
    for title in ["Open large/deep raw tree", "Open navigation windows"]:
        tree = client.send("Run-IInvokeProvider::Invoke", node(title))["result"]["tree"]
    windows = discover()
    navigation = [w for w in windows if w["class"] == "UiaNavigationFixture"]
    assert len([w for w in navigation if w["title"] == "Duplicate navigation title"]) == 2
    assert len({w["id"] for w in navigation}) == len(navigation)
    assert not any(w["title"] in ("Hidden navigation window", "Cloaked navigation window") for w in navigation)
    large = next(w for w in windows if "10,000 siblings" in w["title"])
    tree = client.send("Print-Window", large["id"])["result"]
    assert len(tree["nodes"]) == 11053, len(tree["nodes"])
    wide = node("10,000 siblings")
    assert len([n for n in tree["nodes"] if n["parentId"] == wide]) == 10000
    depths = {}
    for n in tree["nodes"]:
        depths[n["id"]] = depths[n["parentId"]] + 1 if n["parentId"] else 0
    assert max(depths.values()) >= 1000
    client.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--fixture", type=int, required=True)
    parser.add_argument("--platform", choices=["x64", "Win32"], default="x64")
    parser.add_argument("--configuration", choices=["Debug", "Release"], default="Debug")
    parser.add_argument("--solution", type=Path)
    verify(parser.parse_args())
