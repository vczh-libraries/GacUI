# Test Matrix Card 2026-11-03T02:35:29Z

Scope: document press selection and momentum in rebuilt Wasm FCT for the current investigation. RPT/RVMT are excluded by the request.

| Test Item | Result |
| --- | --- |
| Rebuilt Wasm FCT / document touch and momentum | Not run: no Linux build environment is available on this Windows host. |

## Observations and limitations

The required Linux wrapper cannot run here: WSL reports that it is not installed, Docker and Emscripten are not installed, and no SSH host configuration exists. Existing deployed Wasm files have not been treated as current-build evidence. Native-Core HTTP/MiniHTTP browser verification is recorded separately in the investigation and does not verify Wasm FCT.

## Issues and fixes

No Wasm implementation conclusion can be drawn from an unavailable rebuild. The current Core, authored demo resources and shared browser fixture must be rebuilt/deployed through `<GacUI repo>/.github/Jobs/job.rpWasm.prompt.md` in a Linux environment before this row can pass.
