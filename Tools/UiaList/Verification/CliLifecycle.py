"""Preview geometry, discovery lifetime and unavailable capture through CLI JSON."""
import argparse
import base64
import ctypes
from ctypes import wintypes
from pathlib import Path
import struct
import time
from CliSmoke import Client


def verify(pid):
    repo = Path(__file__).resolve().parents[3]
    client = Client(repo / "Tools/UiaList", repo / ".github/Scripts/copilotExecute.ps1")
    user = ctypes.WinDLL("user32", use_last_error=True)
    user.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
    user.ShowWindow.argtypes = [wintypes.HWND, ctypes.c_int]
    user.PostMessageW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
    user.IsWindow.argtypes = [wintypes.HWND]

    def owned(window):
        handle = int(window["hwnd"], 16)
        owner = wintypes.DWORD()
        user.GetWindowThreadProcessId(handle, ctypes.byref(owner))
        assert owner.value == pid, "Only manipulate this run's fixture window"
        return handle

    def windows():
        return next(p["windows"] for p in client.send("List-Process")["result"]["processes"] if p["pid"] == pid)

    initial = windows()
    native = next(w for w in initial if w["class"] == "UiaFixture")
    tree = client.send("Print-Window", native["id"])["result"]
    for name in ("Open large/deep raw tree", "Open navigation windows"):
        node = next(n for n in tree["nodes"] if n["name"] == name)
        tree = client.send("Run-IInvokeProvider::Invoke", node["id"])["result"]["tree"]
    discovered = windows()
    large = next(w for w in discovered if "10,000 siblings" in w["title"])
    loaded = client.send("Print-Window", large["id"])
    tree = loaded["result"]
    before = {p: p.read_bytes() for p in Path(__file__).parent.glob(f"UiaFixture-{pid}.*.txt")}
    preview = client.send("Query-Preview", large["id"])
    assert preview["generation"] == loaded["generation"]
    capture = preview["result"]
    if capture["status"] == "Ready":
        bitmap = base64.b64decode(capture["base64"], validate=True)
        assert bitmap[:2] == b"BM"
        width, height = struct.unpack_from("<ii", bitmap, 18)
        assert width == capture["width"] and abs(height) == capture["height"]
        assert capture["dpi"] > 0 and capture["timestamp"]
        deepest = next(n for n in tree["nodes"] if n["name"].startswith("Stress node 11045 "))
        excluded = next(n for n in tree["nodes"] if n["name"].startswith("Stress node 11046 "))
        assert excluded["offscreen"] and deepest["bounds"] == excluded["bounds"]
        ancestor = next(n for n in tree["nodes"] if n["name"] == "1,000 levels")
        bounds = deepest["bounds"]
        assert bounds["bottom"] < ancestor["bounds"]["top"], "Deep child outside ancestor"
        point = {"x": bounds["left"] + 5 - capture["bounds"]["left"], "y": bounds["top"] + 5 - capture["bounds"]["top"]}
        hit = client.send("HitTest-Preview", large["id"], point)
        assert hit["generation"] == loaded["generation"] and hit["result"] == {"id": deepest["id"], "bounds": bounds}
        assert client.send("HitTest-Preview", large["id"], {"x": -1, "y": -1})["result"]["id"] is None
        print("PASS BMP dimensions/DPI/generation and deep/overlapping/outside-parent/offscreen hit tests", flush=True)
    else:
        assert capture["base64"] is None
        print("LIMIT capture unavailable; successful pixel/hit coverage not claimed", flush=True)
    assert all(p.read_bytes() == data for p, data in before.items()), "Preview sent target input"

    synthetic = next(w for w in discovered if w["class"] == "UiaSyntheticFixture" and "10,000" not in w["title"])
    selected = client.send("Print-Window", synthetic["id"])
    current = windows()
    assert all(next(w for w in current if w["hwnd"] == old["hwnd"])["id"] == old["id"] for old in discovered)
    assert client.send("Print-Window", synthetic["id"])["generation"] == selected["generation"]
    handle = owned(synthetic)
    user.ShowWindow(handle, 6)
    try:
        refreshed = client.send("Refresh-Window", synthetic["id"])
        unavailable = client.send("Query-Preview", synthetic["id"])["result"]
        assert unavailable["status"] == "Unavailable" and unavailable["base64"] is None
        capability = next(n for n in refreshed["result"]["nodes"] if n["name"] == "Synthetic capabilities")
        assert client.send("Query-Properties", capability["id"])["result"]["properties"]
    finally:
        user.ShowWindow(handle, 4)  # Restore without activation.

    navigation = next(w for w in current if w["class"] == "UiaNavigationFixture" and w["title"] == "Duplicate navigation title")
    loaded = client.send("Print-Window", navigation["id"])
    stale = loaded["result"]["nodes"][0]["id"]
    handle = owned(navigation)
    assert user.PostMessageW(handle, 0x10, 0, 0)
    deadline = time.monotonic() + 5
    while user.IsWindow(handle) and time.monotonic() < deadline:
        time.sleep(.02)
    assert not user.IsWindow(handle)
    assert client.send("Refresh-Window", navigation["id"], ok=False)["error"]["code"] == "ExpiredId"
    assert navigation["id"] not in {w["id"] for w in windows()}
    assert client.send("Query-Node", stale, ok=False)["error"]["code"] == "ExpiredId"
    assert client.send("Help-Provider")["result"]["currentTarget"] == []
    client.send("Print-Window", synthetic["id"])
    client.close()
    print("PASS surviving and removed identities; unavailable capture leaves inspection usable", flush=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--fixture", type=int, required=True)
    verify(parser.parse_args().fixture)
