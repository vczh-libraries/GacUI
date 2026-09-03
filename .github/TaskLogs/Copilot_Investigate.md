# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Previous [TODO\_Task\_MouseEvent.md](TODO_Task_MouseEvent.md) has been executed but there is a few bugs:
```markdown
  - "Click here with any mouse button" has wrong color and font like other labels. The most simplist way is to put a label and wire event handlers to its `<BoundsComposition-set/>` property, so you don't worry about the color and font.
  - Under remote protocol, both `FullControlTest` and `RemoteProtocolTest` displays "Ctrl+osSuper+Q" and "{Ctrl+Shift+Alt+osSuper+Q}" in native renderer or GacJS, which is incorrect. Meanwhile non-remote protocol native app (e.g. `CppTest`) displays "Ctrl+Win+Q" and "{Ctrl+Shift+Alt+Win+Q}" which is the expected result.
    - This localization feature is important, add it to .github/Jobs/DebugRemoteProtocolSop.md to verify the text of these lables if there is no such step.
  - Ctrl+Win+Q seems to be conflict a windows predefined shortcut key, change it to Ctrl+Alt+Win+Q for testing.
```

fix it, follow the verification steps in the origin task [TODO\_Task\_MouseEvent.md](TODO_Task_MouseEvent.md) to make sure your change is good.

# UPDATES

# TEST [CONFIRMED]

- Reproduce initial remote connection with shortcuts created before the renderer connects. Verify actual bound label text, including a command installed in a nested control, and verify label refresh after renderer replacement.
- In FullControlTest Window Manager and RemoteProtocolTest Home, require themed mouse labels and exact Windows shortcut labels: `Ctrl+Q`, `Ctrl+Alt+Win+Q`, and `{Ctrl+Shift+Alt+Win+Q}`.
- Follow the original task's generation, complete unit-test, native input, remote transport/replacement, and release verification requirements. Check GacJS against the updated core as well.

The existing Debug x64 native renderer over named pipe reproduces RemoteProtocolTest's `Ctrl+osSuper+Q` and `{Ctrl+Shift+Alt+osSuper+Q}`. Both applications report the click target as black Lucida Console 12, while adjacent labels use light Segoe UI 15. FullControlTest's current executable already refreshes its direct-window shortcuts to `Win`. The new regression compiles with zero warnings/errors and exercises window, nested-control, and template-owned commands before initial connection and across `Win`, `Command`, and `Super` renderer configurations.

The focused regression was also run under CDB against the pre-fix binary. It failed exactly at the nested-control label assertion (`labels[1]->GetText()` still lacked the renderer's `Win` name). The preliminary unfiltered pre-fix run was stopped in favor of this focused reproduction; the complete Debug x64 suite subsequently passed against the fix.

# PROPOSALS

- No.1 Notify components throughout the window composition tree and use themed labels

## No.1 Notify components throughout the window composition tree and use themed labels

The renderer sends the correct canonical name, and `GuiRemoteController::OnControllerConnect` publishes the environment change. `GuiWindow::EnvironmentChanged` currently visits only its own components. RemoteProtocolTest installs its commands on the nested Home tab, so their `DescriptionChanged` notifications never reach the bound labels. Extend the existing ownership-based notification to every instance root in the window's composition tree, including associated controls and templates. Detached commands remain unnotified.

### CODE CHANGE

- Move component notification into `GuiInstanceRootObject::InvokeEnvironmentChanged`, and traverse the composition tree from `GuiWindow::EnvironmentChanged` after updating the display font.
- Replace both raw mouse `SolidLabel` elements with themed `Label` controls and attach the unified handlers to `att.BoundsComposition-set`.
- Change the local Super shortcut and dialog text to `Ctrl+Alt+Win+Q` in both resources. Strengthen SOP checks for exact renderer-localized labels, themed mouse labels, and renderer replacement.
- Retain protocol schema and reflected API signatures; regenerate application resources and required metadata using the repository tools.
- The required browser five-button check also exposed default history navigation: Chromium emitted Mouse4 down/up and then navigated to `about:blank`. Update GacJS's consumed extended-button events to prevent browser Back/Forward navigation, document the behavior, and add a Playwright regression using real CDP Back/Forward input.

### VERIFICATION

- The fixed focused shortcut file passed all 14 cases under CDB. The complete Debug x64 suite passed 89/89 files and 1720/1720 cases through the required wrapper, with no memory-leak dump after the summary.
- Debug Win32 and x64 builds and reflection metadata generation passed before XML generation. `GacUI_Compiler` regenerated both architectures of both samples without any UI error files. The required post-generation Win32/x64 metadata workflow and x64 metadata test passed; the final Debug x64 build reported zero warnings and zero errors.
- The local Win32 `GacUI_Host` displayed all three exact Windows shortcut names. The Debug x64 host also passed all five native Windows mouse-message down/up pairs, including TRUE X-button return values, and displayed the expected native Ctrl+Q dialog. Label styling remained `Label:#F1F1F1FF,Segoe UI,15` before and after input. It closed through its Exit tab and the execution wrapper returned zero.
- Both samples passed focused GacJS operations over HTTP and MiniHTTP: exact labels and matching theme style, all three distinct shortcut dialogs, all five mouse down/up pairs, renderer reconnection/takeover, repeated shortcuts and extended buttons, and application-controlled shutdown. See `TestMatrix_GacJS.md` for the browser-history bug and the expected diagnostic after intentional MiniHTTP shutdown.
- GacJS import and code generation completed. The final `yarn build` and `yarn test` passed all 10 packages, including 11 protocol files and 55 cases. The two new Playwright cases cover exact localized labels and real CDP Back/Forward input on initial and replacement renderers. The GacJS fix and regression were pushed as `bb69e68`.
- Both samples passed the available native checks over Pipe, HTTP, and MiniHTTP: exact visible renderer DOM labels and theme style, all five Windows mouse-message pairs, Ctrl+Q through renderer automation, reconnection/takeover, repeated labels/style/Ctrl+Q/Mouse4/Mouse5, and application-controlled shutdown. The native matrix records the harness corrections and exact scope.
- Real Windows Super-key activation is blocked by the noninteractive desktop: no foreground window is available and `SendInput` returns Access denied (5). All six native matrix rows remain marked X for this outstanding requirement. Window-message mouse checks are not represented as a successful `RegisterHotKey` activation.
- `Tools/Build.ps1 -Project GacUI` completed successfully at 2026-09-02 19:47: both full Release rebuilds, both metadata generators, both metadata tests, both complete unit suites, the GacGen rebuild, both DarkSkin architectures, and both CodePack passes passed. No failure markers or generation `Errors.txt` files remained. The packaged diff contains only the intended `GacUI.cpp`/`GacUI.h` change; skin, metadata, and protocol outputs are unchanged. The release runs changed only frame IDs in five unrelated `ScrollResetOnNavigation` JSON snapshots and the corresponding render traces; all other JSON content was identical, and those recordings were restored.
- `Tools/Build.ps1 -Project Release` completed successfully at 2026-09-02 20:56. Its tools build, Workflow builds before and after code generation, all 87 XML resources for both architectures, all six Debug tutorial solutions, the Debug executable audit, and all six Release tutorial solutions passed. There were no failure markers or XML generation error files. Its final repository audit reported only the intended uncommitted GacUI and Release changes.
- The sibling Release import also synchronizes already-committed GacUI changes missing from its previous snapshot: component environment notification, mouse-handler consolidation/names, the snapshot-viewer handler name, and the current mouse protocol metadata. All six changed imports were verified byte-for-byte against the current GacUI outputs. FullControlTest regenerated its intended Label/chord changes; four independent horizontal/vertical setter pairs were also reordered by GacGen with identical values. The final source/generated-file review and `git diff --check` passed.

The implementation passed its regression and all available verification gates. Proposal confirmation remains pending only the original task's requirement to activate native local/global Windows-key shortcuts through real keyboard input. The desktop still reported no foreground window when rechecked at 2026-09-02 20:07; the successful mouse-message and browser checks do not substitute for that requirement.
