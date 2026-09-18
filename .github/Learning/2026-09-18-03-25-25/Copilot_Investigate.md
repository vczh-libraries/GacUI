# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Build the app defined by [AGENTS.md](Tools/UiaList/AGENTS.md)according to [Planning.md](Tools/UiaList/Planning.md), commit and push all local changes once finishing

# UPDATES

## UPDATE

FYI it would be easier to copy vcxproj configurations from other existing project files

# TEST

The initial Tools/UiaList directory contains requirements and planning documents only: no solution, product source, or fixture exists. Use the implementation plan's B01-S07 acceptance matrix. Generate XML resources with GacBuild, build all four product configurations with copilotBuild, exercise the GUI using its Debug automation service and an independent native fixture, inspect normal shutdown under CDB, and run the existing GacUI UnitTest. Record concrete evidence and any environment-dependent verification gaps under Tools/UiaList/Verification.

# PROPOSALS

- No.1 [CONFIRMED] Implement the planned native MVVM inspector.

## No.1 [CONFIRMED] Implement the planned native MVVM inspector

Follow Planning.md milestones M1-M7 in dependency order. XML defines interfaces, localization, controls, bindings, and modal presentation; handwritten C++ implements model services and view models. Use the existing read-only release/import libraries, compressed generated resources, a dedicated UIA MTA queue, independent one-frame Windows Graphics Capture, typed SDK catalogs and pattern dispatch, and native verification fixtures. Keep generated code exclusively owned by GacBuild. Commit and push all local changes after implementation and verification as requested.

### CODE CHANGE

Implemented the three-project solution using the existing CppTest/Lib_GacUI settings, explicit inventories, all four configurations, compressed generated resources, and English/Chinese/Japanese localization. XML owns main tabs, property/action templates, parameter editors, nested modal windows, focus, and events; handwritten C++ owns process discovery, raw-tree identity, typed values, property drafts, all 35 pattern adapters, returned references, and text-range workspaces. Dedicated MTA queues own UIA and one-frame Windows Graphics Capture. Explicit refresh and generation/serial guards preserve the accepted operation's target and reject stale presentation results.

Added an independent Win32/provider fixture, catalog verification, HTTP GUI driver, repeated modal/mutation smoke test, and a 10,000-sibling/1,000-level traversal-and-replacement test. Debug startup now reports an occupied automation URL and terminates. Native tree teardown is iterative. Documentation and observed results, including unfinished acceptance coverage, are in Tools/UiaList/Verification/Results.md.

The existing WindowsAutomationService had a confirmed x64 modal-ID truncation bug: it emitted pointer-sized IDs but parsed them as Windows unsigned long. Fixed the source conversion and regenerated Release/GacUI.Windows.cpp with CodePack. This upstream fix is committed separately from the new tool.

### CONFIRMED

Both GacBuild architecture paths reproduced generated output, including an authored-string perturbation and restoration. All four product configurations built without warnings/errors. Catalogs match the selected SDK, and all three locales contain the same 43 keys. The existing GacUI regression suite passed 90/90 files and 1,768/1,768 cases twice. Native and synthetic GUI runs exercised properties, patterns, text ranges, both modal levels, capture, exactly-once Invoke, refresh, and normal shutdown. Release executable-only startup and zero exit were observed on both architectures. The full Planning.md acceptance matrix is not yet established; Results.md records the limits instead of treating unexecuted scenarios as passes.

Sixty native smoke cycles passed across all four inspector/target architecture combinations: 25 x64/x64, 25 Win32/x64, five x64/Win32, and five Win32/Win32. The last three combinations used the final implementation. Twenty-five deep-tree traversal/replacement cycles passed after iterative ownership teardown; a subsequent fix moved final detachment into RequestClose while bound observers remain alive. The final x64 Debug executable closed with all 11,053 nodes loaded, exit zero, and no CRT leak report. The final Win32 Debug executable also opened both modal levels from an executable-only directory and exited zero without a CRT leak report. An occupied Debug automation URL displayed its operation and URL, then exited with code one. This confirms the implemented approach and observed behavior, while leaving the explicitly listed acceptance gaps unfinished.
