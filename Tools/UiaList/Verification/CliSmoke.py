"""Exercise the real UiaListCli process through the repository execution wrapper."""
import argparse
import json
from pathlib import Path
import queue
import subprocess
import tempfile
import threading


class Client:
    def __init__(self, solution, wrapper, configuration="Debug", platform="x64"):
        command = f"& '{wrapper}' -Mode CLI -Executable UiaListCli -Configuration {configuration} -Platform {platform} -Interactive 6>$null"
        self.process = subprocess.Popen(
            ["pwsh", "-NoProfile", "-Command", command], cwd=solution,
            stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True, encoding="utf-8", bufsize=1,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
        self.responses = queue.Queue()
        self.errors = []
        self.transcript = []
        self.output = Path(tempfile.gettempdir()) / f"UiaListCli-{self.process.pid}.jsonl"
        self.output.write_text("", encoding="utf-8")
        def read():
            for line in self.process.stdout:
                self.responses.put(line)
            self.responses.put(None)
        def errors():
            self.errors.extend(self.process.stderr.readlines())
        threading.Thread(target=read, daemon=True).start()
        threading.Thread(target=errors, daemon=True).start()

    def send(self, command, target=None, args=None, ok=True):
        line = command + (" " + target if target else "")
        if args is not None:
            line += " " + json.dumps(args, ensure_ascii=False, separators=(",", ":"))
        self.process.stdin.write(line + "\n")
        self.process.stdin.flush()
        raw = self.responses.get(timeout=120)
        assert raw is not None, "CLI exited: " + "".join(self.errors)
        result = json.loads(raw)
        assert set(result) == {"command", "ok", "generation", "result", "error"}, result
        assert result["command"] == command.split()[0], result
        assert result["ok"] == ok, {"command": command, "ok": result["ok"], "error": result["error"]}
        assert (result["error"] is None) == ok, result
        self.transcript.append({"input": line, "output": result})
        with self.output.open("a", encoding="utf-8") as output:
            output.write(json.dumps(self.transcript[-1], ensure_ascii=False) + "\n")
        return result

    def close(self, eof=False):
        if not eof:
            self.send("Exit-Application")
        self.process.stdin.close()
        assert self.process.wait(timeout=30) == 0, "".join(self.errors)
        assert self.responses.get(timeout=5) is None, "Extra stdout response"
        print(f"PASS {len(self.transcript)} commands; {self.output}", flush=True)


def smoke(options):
    repo = Path(__file__).resolve().parents[3]
    solution = options.solution or repo / "Tools/UiaList"
    client = Client(solution, repo / ".github/Scripts/copilotExecute.ps1", options.configuration, options.platform)
    client.process.stdin.write(" \t\n\n")
    client.process.stdin.flush()
    client.send("Help-Command")
    catalog = client.send("Help-Provider")["result"]["catalog"]
    assert len(catalog) == 37, len(catalog)
    client.send("Help-Command {broken}", ok=False)
    client.send("Bogus", ok=False)
    client.send("List-Process", args={"unexpected": 0}, ok=False)
    client.send("Query-Node", "missing", ok=False)
    processes = client.send("List-Process")["result"]["processes"]
    fixture = next(p for p in processes if p["pid"] == options.fixture)
    window = next(w for w in fixture["windows"] if w["class"] == "UiaSyntheticFixture" and "10,000" not in w["title"])
    tree = client.send("Print-Window", window["id"])
    nodes = tree["result"]["nodes"]
    assert len(nodes) >= 45, len(nodes)
    capability = next(n for n in nodes if n["name"] == "Synthetic capabilities")
    document = next(n for n in nodes if n["name"] == "Synthetic document")
    inspected = client.send("Query-Node", capability["id"])["result"]
    assert len(inspected["providers"]) >= 30
    assert any(p["value"]["kind"] == "Boolean" and p["value"]["value"] is False for p in inspected["properties"])
    assert any(p["value"]["kind"] == "Signed" and p["value"]["value"] == "0" for p in inspected["properties"])
    assert any(p["value"]["kind"] == "String" and p["value"]["value"] == "" for p in inspected["properties"])
    invalid = client.send("Set-Property", capability["id"], {"propertyId": 30045, "value": 1}, ok=False)
    assert invalid["error"]["code"] == "InvalidType"
    old = capability["id"]
    edited = client.send("Set-Property", old, {"propertyId": 30045, "value": "CLI 日本語 中文\nsecond line"})
    assert edited["generation"] != tree["generation"]
    assert any(p["propertyId"] == 30045 and p["value"]["value"] == "CLI 日本語 中文\nsecond line" for p in edited["result"]["readback"]["properties"])
    client.send("Query-Node", old, ok=False)
    nodes = edited["result"]["tree"]["nodes"]
    capability = next(n for n in nodes if n["name"] == "Synthetic capabilities")
    document = next(n for n in nodes if n["name"] == "Synthetic document")
    client.send("Select-Node", capability["id"])
    client.send("Query-Providers", document["id"])
    text_range = client.send("Run-ITextProvider::DocumentRange", document["id"])["result"]["value"]["value"]
    client.send("Query-Range", text_range)
    cloned = client.send("Run-IUIAutomationTextRange::Clone", text_range)["result"]["value"]["value"]
    same = client.send("Run-IUIAutomationTextRange::Compare", text_range, {"range": cloned})
    assert same["result"]["value"]["value"] is True
    text = client.send("Run-IUIAutomationTextRange::GetText", text_range, {"maxLength": -1})["result"]["value"]
    assert text["kind"] == "String" and text["value"]
    preview = client.send("Query-Preview", window["id"])["result"]
    if preview["status"] == "Ready":
        import base64
        bitmap = base64.b64decode(preview["base64"], validate=True)
        assert bitmap[:2] == b"BM"
    client.send("HitTest-Preview", window["id"], {"x": -1, "y": -1})
    client.send("Refresh-Window", window["id"])
    expired = client.send("Query-Range", text_range, ok=False)
    assert expired["error"]["code"] == "ExpiredId"
    client.close()
    eof = Client(solution, repo / ".github/Scripts/copilotExecute.ps1", options.configuration, options.platform)
    eof.send("Help-Command")
    eof.close(eof=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--fixture", type=int, required=True)
    parser.add_argument("--platform", choices=["x64", "Win32"], default="x64")
    parser.add_argument("--configuration", choices=["Debug", "Release"], default="Debug")
    parser.add_argument("--solution", type=Path)
    smoke(parser.parse_args())
