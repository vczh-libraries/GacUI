"""Assert JSON preservation independently of normalization by Windows UIA."""
import json
from pathlib import Path
import subprocess

directory = Path(__file__).resolve().parent
repo = directory.parents[3]
wrapper = repo / ".github/Scripts/copilotExecute.ps1"
command = f"& '{wrapper}' -Mode CLI -Executable CliValueTests -Configuration Debug -Platform x64 -Interactive 6>$null"
result = subprocess.run(["pwsh", "-NoProfile", "-Command", command], cwd=directory,
    capture_output=True, encoding="utf-8", timeout=120, creationflags=subprocess.CREATE_NO_WINDOW)
assert result.returncode == 0, result.stderr
value = json.loads(result.stdout)
assert value["dimensions"] == [{"lower": -3, "upper": 8}]
items = {item["kind"]: item for item in value["value"]}
assert len(items) == 12
assert items["Signed"]["value"] == "-9007199254740993"
assert items["Unsigned"]["value"] == "18446744073709551615"
assert items["String"]["value"] == "A\0\x01\x0b\x1f中Z"
assert items["Boolean"]["value"] is False
assert items["Real"]["value"] == {"nonfinite": "NaN"}
assert items["Array"]["dimensions"] == [{"lower": 2, "upper": 3}]
assert [v["value"] for v in items["Array"]["value"]] == [{"nonfinite": "Infinity"}, {"nonfinite": "-Infinity"}]
assert all(items[k]["value"] is None for k in ["Null", "Mixed", "Unsupported"])
print("PASS all ValueData kinds, full control/NUL/Unicode text, precise integers, nonfinite reals and array bounds")
