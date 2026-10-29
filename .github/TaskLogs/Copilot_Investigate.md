# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Execute GacUI/TODO_Injection_XPlat2.md , commit and push all local changes once finishing

# TEST [CONFIRMED]

Execute the native Firefox provider matrix and the Chromium/Firefox WASM lifecycle and persistence checks from <GacUI repo>/TODO_Injection_XPlat2.md. Record actual results in the platform and WASM test cards. Require deterministic import/code generation, official platform builds, and root GacJS tests.

# PROPOSALS

- No.1 Synchronize current upstream sources and explicitly disable WASM injection. [CONFIRMED]

## No.1 Synchronize current upstream sources and explicitly disable WASM injection

The existing platform import/synchronization inventories and double-dash forwarding already implement the requested contracts. Verify them against current inputs. Make the shared WASM Core configuration and renderer capabilities explicitly false, preserving the configurable library and HTTP query opt-in. Extend live browser regression coverage to inspect the actual handshake and requests.

### CODE CHANGE

- Explicitly set all three injection fields to false in <GacUI repo>/Test/Linux/Wasm/WasmApplication.cpp and <GacJS repo>/Gaclib/website/remote-protocol-wasm/src/index.ts.
- Extend the existing Chromium/Firefox worker-message observer to require all three false ControllerConnect fields and reject injection requests during the exercised operations and Reload cycles.
- Wait for TouchingList's measured row position after deletion, preserving the identity assertions and half-pixel tolerance. The first full Firefox pass exposed the distinction between DOM removal and completed layout; 25 consecutive Firefox FCT lifecycle runs and the full Chromium/Firefox suite passed.
- Regenerate the WASM makefiles/source inventories through the official build wrapper. RPT/RVMT now include the current injection sources; FCT's generated formatting is retained.
- Synchronize wGac imports and update the affected GacUI, GacJS and bilingual wGac usage documentation. Preserve the existing HTTP opt-in and library configuration behavior.
- Record native integration evidence in <wGac repo>/TestMatrix_Injection_XPlat2.md and browser WASM evidence in <GacUI repo>/.github/TaskLogs/TestMatrix_Wasm.md. Remove temporary Core instrumentation before the final production rebuild and smoke test.

### CONFIRMED

Both complete wGac import/synchronization/build passes succeeded, with no additional source differences on repetition. GacJS dependency setup, import, codegen, build and 211 root test cases passed; repeated codegen was stable. The native Firefox matrix passed all 12 individual opt-in combinations, all-enabled and mixed selections using fresh Core calls and distinguishable fixtures/request traces. It also passed timezone/locale/OPFS contracts, both document editors, loss/replacement, supported same-machine host forms, RPT and Unicode browser-host RVMT with the required restart after observed host loss. Foreground and unblock launcher forwarding was observed in real process arguments. Native Wayland FCT standard/hosted, RPT and RVMT smoke checks passed.

Temporary Core instrumentation was removed and the official launcher rebuilt production Core. Fresh Firefox localization, actual input, both document editors, embedded-image export and native-storage isolation passed again without the probe. No framework injection behavior or transport recovery was changed.

All three optimized WASM application builds succeeded. Explicit packaging preserved all nine generated files and their original module factories; negative packaging cases failed before changing any destination. The final local browser suite passed 10/10 cases, and the corrected Firefox FCT test passed 25 consecutive complete lifecycle runs. Both browsers passed the extra OPFS document/persistence checks with false capabilities, no injection requests and working ImageSave. The pthread-enabled VlppOS unit launcher passed 107/107 cases in each browser on its separate origin.

Detailed commands, observations, harness corrections and coverage limits are retained in <wGac repo>/TestMatrix_Injection_XPlat2.md and <GacUI repo>/.github/TaskLogs/TestMatrix_Wasm.md. This evidence covers Linux and same-machine networking; it does not claim macOS, Safari/WebKit, Windows HTTP.sys, public deployment, another physical computer or physical-device input. The Windows-only and public-deployment test gates are recorded separately from the successful live runs.
