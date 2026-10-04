"""Separate expected, readback, fatal, capability and framing checks."""
import argparse
import json
from pathlib import Path
from CliSmoke import Client


def verify(pid):
    repo = Path(__file__).resolve().parents[3]
    def client():
        return Client(repo / "Tools/UiaList", repo / ".github/Scripts/copilotExecute.ps1")
    def setup(c):
        processes = c.send("List-Process")["result"]["processes"]
        windows = next(p for p in processes if p["pid"] == pid)["windows"]
        window = next(w for w in windows if w["class"] == "UiaSyntheticFixture" and "10,000" not in w["title"])
        tree = c.send("Print-Window", window["id"])["result"]
        return window, next(n["id"] for n in tree["nodes"] if n["name"] == "Synthetic capabilities")
    log = Path(__file__).parent / f"UiaFixture-{pid}.synthetic.txt"
    def setters():
        return sum("\tSetValue(BSTR)\t" in x for x in log.read_text(encoding="utf-16-le").splitlines())

    c = client()
    c.process.stdin.buffer.write(b"Help-Command \xff\n")
    c.process.stdin.buffer.flush()
    invalid = json.loads(c.responses.get(timeout=120))
    assert invalid["error"]["code"] == "InvalidEncoding", invalid
    assert c.send('Help-Command {"x":"\0"}', ok=False)["error"]["code"] == "InvalidJson"
    c.process.stdin.write("Help-Command\nHelp-Provider\n")
    c.process.stdin.flush()
    for expected in ("Help-Command", "Help-Provider"):
        response = json.loads(c.responses.get(timeout=120))
        assert response["ok"] and response["command"] == expected
    window, element = setup(c)
    unavailable = c.send("Run-ISpreadsheetProvider::GetItemByName", element, {"name": "__uia_unavailable__"}, ok=False)["error"]
    assert unavailable["code"] == "Unavailable" and unavailable["phase"] == "invocation" and unavailable["hresult"], unavailable
    c.send("Help-Command")
    tree = c.send("Print-Window", window["id"])["result"]
    document = next(n["id"] for n in tree["nodes"] if n["name"] == "Synthetic document")
    text_range = c.send("Run-ITextProvider::DocumentRange", document)["result"]["value"]["value"]
    before = log.read_text(encoding="utf-16-le").count("\tRange.Move\t")
    readback = c.send("Run-IUIAutomationTextRange::Move", text_range, {"unit": 0, "count": 12345}, ok=False)["error"]
    assert readback["code"] == "Unavailable" and readback["phase"] == "readback", readback
    assert log.read_text(encoding="utf-16-le").count("\tRange.Move\t") == before + 1, "Range edit was retried after readback failure"
    tree = c.send("Refresh-Window", window["id"])["result"]
    element = next(n["id"] for n in tree["nodes"] if n["name"] == "Synthetic capabilities")
    c.send("Query-Properties", element)
    other = client()
    other_window, other_element = setup(other)
    result = other.send("Run-IToggleProvider::Toggle", other_element)["result"]
    before = setters()
    rejected = c.send("Set-Property", element, {"propertyId": 30045, "value": "must not commit"}, ok=False)
    assert rejected["error"]["code"] == "CapabilityChanged", rejected
    assert setters() == before
    for _ in range(2):
        other_element = next(n["id"] for n in result["tree"]["nodes"] if n["name"] == "Synthetic capabilities")
        result = other.send("Run-IToggleProvider::Toggle", other_element)["result"]
    other.close()
    c.close()

    fatal = client()
    _, element = setup(fatal)
    result = fatal.send("Run-ISpreadsheetProvider::GetItemByName", element, {"name": "__uia_fatal__"}, ok=False)
    assert result["error"]["code"] == "Fatal" and result["error"]["phase"] == "invocation", result
    fatal.process.stdin.close()
    assert fatal.process.wait(timeout=30) != 0
    assert fatal.responses.get(timeout=5) is None
    print("PASS expected invocation, committed readback failure, changed capability, fatal exit and pipeline framing", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--fixture", type=int, required=True)
    verify(parser.parse_args().fixture)
