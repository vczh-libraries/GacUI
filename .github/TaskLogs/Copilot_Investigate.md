# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Replace the standalone Windows clock tutorial with RawClock, using the common raw-element contract and native Direct2D/GDI painters. Support default Direct2D and optional `/Gdi`, and run the complete Release build. Update canonical knowledge-base and XML authoring guidance, browser/Wayland/macOS documents, and the public website to describe `<Raw/>` as the recommended ownerdraw API in native, hosted and remote GUI applications, with TUI unsupported. Publish the website, commit and push all changes, then remove the five completed task specifications in a separate GacUI commit. Documentation must stand on its own without referring to the removed specifications.

# TEST

- Build `<Release repo>` with `<Tools repo>/Tools/Build.ps1 -Project Release`; verify RawClock generation, both drawing backends, ticking and layout.
- Check documentation against current source contracts, including platform factory selection, native callback lifetimes, ID/data updates and remote/browser lifecycle APIs.
- Build and test `<WebsiteSource repo>`, export static pages and the manual, publish through the existing GitHub Pages workflow, and verify the live changed pages.
- Synchronize canonical `<Tools repo>/Copilot` guidance into consuming repositories and verify the second synchronization is stable.
- Check all affected diffs and confirm no documentation references the removed task specifications.

# PROPOSALS

- No.1 [CONFIRMED] Publish a common raw-ownerdraw tutorial and documentation contract across the existing native/browser implementations.

## No.1 [CONFIRMED] Publish RawClock and current raw-element guidance

### CODE CHANGE

The tutorial keeps XML/Workflow ownership of clock state and uses standalone native attachment helpers derived from the FullControlTest example. The Windows startup path selects GDI with `/Gdi` and defaults to Direct2D. The refreshed BlackSkin showcase also retains a Windows clock attachment; `<Tools repo>/Tools/BuildRelease.ps1` synchronizes its helper from the owning GacUI source. Canonical guidance explains the four native element types, the portable remote Core element, frame-batched state and painter lifetime. Platform documents correct stale factory and hosted-renderer descriptions. Website pages describe the XML shape, native callbacks and remote state, and link the renamed tutorial.

### CONFIRMED

- Website build and all 57 tests passed. Main/document static downloads and Markdown export passed. WebsiteSource commit `104ddfe` and published-site commit `7ed1780b1` were pushed; deployment run `38846804098` succeeded. Live RawClock introduction and raw-element documentation were inspected.
- The required Wasm preparation attempt built GacJS successfully, then stopped because this Windows host has no `make` or WSL. The existing published Wasm demo remained byte-for-byte unchanged; failed-build makefile churn was restored. No new Wasm runtime result is claimed.
- Canonical guidance and the exported manual were synchronized into eight consuming repositories. All canonical guidance files matched their copies, 32 updated manual copies matched the four exported pages, and introduced document links/source paths were checked. Existing side-specific ALT guidance was preserved during synchronization.
- The final Markdown/HTML/XML audit across all fourteen affected repositories found no references to the task specifications outside those specifications themselves. The two remaining repository documentation inventories and workspace-root Markdown files were also clear.
- Platform documentation was checked against native and TypeScript implementations. Wayland and Cocoa builds were not rerun on this Windows host; those repositories contain documentation-only changes.
- BuildRelease PowerShell syntax passed. Independent review checked native resource lifetimes, GDI state restoration, backend parsing, resource locations, XML ownership and project inventories.
- Focused RawClock Debug/Release x64 builds passed with zero warnings or errors. Debug x64 Direct2D and GDI runs showed ticking data, resized through UI Automation, completed native drawing callbacks with valid render targets/device contexts, and exited cleanly. A Debug Win32 resource-location smoke test and a Release x64 GDI callback/shutdown check also passed. One UI Automation CLI readback timed out after its target had already closed successfully.
- BlackSkin Debug Win32 selected Misc / Raw Clock through automation and UI Automation, showed advancing `FullControlTest.Clock` data, completed its native Direct2D callback with valid target/bounds, and exited cleanly without a runtime-error dialog. The locked desktop produced black captures, so visible pixel appearance remains unverified.
- The required `<Tools repo>/Tools/Build.ps1 -Project Release` completed with exit code zero. Tool deployment, Workflow build/code generation/rebuild, document and tutorial XML generation, all six Debug Win32 tutorial solutions with binary checks, and all six Release Win32 tutorial solutions passed. Together with the focused x64 builds, RawClock compiled in all four Debug/Release and Win32/x64 configurations.
- Release changes were committed separately as authored tutorial/documentation changes (`fd95978f`) and generated resources/imports (`9586bb13`), then pushed. Documentation and website changes in the other affected repositories were also committed and pushed. The five completed task specifications are removed in the following dedicated GacUI cleanup commit.
