# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

In remote protocol, when `ControllerConnect` is called, an `CharacterEncoding` should be specified.
`ControllerConnect` could be called multiple time, the GacUI core will switch to the latest renderer sending this event.
Current the encoding value must match what `wchar_t` means in the current platform, but this task is going to change it.

This option affects how text box (Source\PlatformProviders\Remote\Protocol\Protocol_Renderer_Document.txt) generate and interpret caret values.
No matter what `wchar_t` means in GacUI core, but this value represents the expectation from a renderer.
For example, in JavaScript a string is represented by an array of UTF-16 code points. So `GacJS` would always specify `CharacterEncoding::UTF16`.
When the core application is running on Windows, everything is fine.
But for Linux and macOS where `wchar_t` means UTF32 by default, it will be rejects today.
The reason is that, text box in core reports all carat values using `wchar_t` code points.

In order to make a core application accepts all 3 values in `CharacterEncoding`, here is what needs to change:
- Allow all values in `ControllerConnect`'s handler.
- `GuiRemoteGraphicsParagraph`:
  - `NativeTextPosToRemoteTextPos` translates from core's native `wchar_t` meaning to remote side's expected interpretation.
  - `RemoteTextPosToNativeTextPos` does the reverted thing.
Here is what doesn't need to change:
- `GuiRemoteRendererSingle` will always use what `wchar_t` means in its platform:
  - Because when core could accept any value, the native renderer's best choice is to match its own platform.
  - This is just like why `GacJS` will always use `UTF-16`.
- Unit test renderer will always use the value matching what `wchar_t` means, in order not to introduce extra complexity.

To correctly implement this feature, you need to make sure:
- `NativeTextPosToRemoteTextPos` and `RemoteTextPosToNativeTextPos` is used in everything they are supposed to be called.
- Since `ControllerConnect` could be called multiple times, a `GuiRemoteGraphicsParagraph` should not assume the expected encoding will not change.
  - Currently it could access the value using `remote->GetGlobalConfig().documentCaretFromEncoding`, and clearly caching such value in `GuiRemoteGraphicsParagraph` will result in incorrect behavior.
- `GuiRemoteGraphicsParagraph`'s text can't change after it is created, which means `NativeTextPosToRemoteTextPos` and `RemoteTextPosToNativeTextPos` could just calculate the value using cached data:
  - When they are called, first find if the cached data has already been created, calculate all caret mappings at this point.
    - You can try `Dictionary<CharacterEncoding, Ptr<GuiRemoteCaretCache>>`. `GuiRemoteCaretCache` will be a struct storing two arrays.
  - The cache data consist with two arrays for these two functions.
  - Vlpp already implemented encoding translation, use it to create the cached data.
  - On Linux/macOS, `wchar_t` is UTF-32, which is easy. But on windows `wchar_t` means UTF-16, there will be surrogate pairs.
  - When multiple code points maps to one single `char32_t` no matter it is "native" or "remote", front carets of all such code points translated to the front caret of target code points. For example, native code points `11233` translates to remote code points `aabbbccc`:
    - 123abc here means grouped code points of one `char32_t`, not actual character values.
    - The `native-to-remote` array will be `[0,0,2,5,5,8]`.
    - The `remote-to-native` array will be `[0,0,2,2,2,3,3,3,5]`.
    - Remember that, the end of the string is also valid, that's why these two arrays have one extra value at the end.
- Caret and text pos are the same thing in `GuiRemoteGraphicsParagraph`, the only difference is that, a valid caret is where a caret bar could render, this information is from the remote side. These two functions are only about translating text pos, so you don't need to worry about this.

To test this feature without actually running `GacJS`:
- In order not to introduce extra complexity to the unit test renderer, the unit test renderer will only specify a value matching what `wchar_t` mean in the current platform. So test cases should not involve remote protocol.
- To make testing possible:
  - `GuiRemoteCaretCache` should be a public class.
  - It should have a constructor of `WString` and `CharacterEncoding`, and the two arrays will be built in the constructor.
  - When `wchar_t` means `UTF-16` (you must use `VCZH_WCHAR_UTF16` to test it, all other means are not allowed), and when you see invalid surrogate pairs, the utility from Vlpp will let you know, and in this case, an incomplete `wchar_t` will be view as a separate character.
  - You can then create a `TestRemote_Caret.cpp` to call the constructor and see if the arrays are correctly constructed.
  - `TestRemote_Caret.cpp` should be saved using UTF-8 with BOM so that it works on all platforms.
  - VlppOS/VlppRegex test cases has sample text involving surrogate pairs, you can use that string, which is short.
Unfortunately it is no way to test `GuiRemoteGraphicsParagraph` without involving remote renderer, you don't need to cover this in `UnitTest`.

## DETAILS

- `CharacterEncoding` defines the code-unit coordinate system used by renderer-facing text positions: UTF-8 bytes, UTF-16 code units, or UTF-32 code units. A successfully decoded Unicode scalar held in `char32_t` is the common cluster used to relate the native and renderer coordinate systems.
- The current connection path already accepts every defined value. `GuiRemoteEvents::OnControllerConnect` forwards the configuration, and `GuiRemoteController::OnControllerConnect` assigns the complete configuration to `remoteGlobalConfig` before `GuiRemoteGraphicsResourceManager::OnControllerConnect` resynchronizes paragraphs. The constructor's platform-selected value is only the pre-connection default. Preserve this behavior; no protocol schema, generated protocol code, or new handler validation is needed.
- Add the public, non-reflectable `vl::presentation::elements::GuiRemoteCaretCache` class to `Source/PlatformProviders/Remote/GuiRemoteGraphics_Document.h`, derive it from `Object`, and implement it in `GuiRemoteGraphics_Document.cpp`. Give it public `collections::Array<vint>` native-to-remote and remote-to-native mappings and a constructor taking `const WString&` and `remoteprotocol::CharacterEncoding`.
- Build both mappings by walking the immutable paragraph text as Unicode-scalar clusters:
  - Use `vl::encoding::UtfConversion<wchar_t>::To32` to obtain the native cluster width.
  - Use `vl::encoding::UtfConversion<char8_t>::From32` or `vl::encoding::UtfConversion<char16_t>::From32` to obtain the UTF-8 or UTF-16 renderer width; UTF-32 has width one.
  - Map every position inside a source cluster to the front position of the corresponding target cluster, and append the valid end-of-string entry to both mappings.
  - Scan by `WString::Length()` instead of a null-terminated UTF reader so embedded NUL code units do not truncate the mappings.
  - Do not use an identity shortcut when the native and renderer encodings match. An interior UTF-16 surrogate-pair position still maps to that cluster's front.
  - If native decoding fails, consume exactly one `wchar_t` and give that standalone opaque cluster a native and renderer width of one. If encoding a valid native scalar for the renderer fails, likewise use a renderer width of one. Fail early only for a `CharacterEncoding` value outside the three defined enum values.
- Add `collections::Dictionary<remoteprotocol::CharacterEncoding, Ptr<GuiRemoteCaretCache>>` to `GuiRemoteGraphicsParagraph`. Each conversion call must read `remote->GetGlobalConfig().documentCaretFromEncoding`, find or lazily create that encoding's cache, and index the appropriate mapping. Do not retain a separately selected "current" encoding or cache pointer. A reconnect can therefore select another cached mapping immediately, and reconnecting back to an earlier encoding can safely reuse its immutable mapping.
- Keep all persistent paragraph ranges in native coordinates. In particular, `textRuns`, `inlineObjectRuns`, `inlineObjectProperties`, `stagedRuns`, and `committedRuns` must not contain positions converted for whichever renderer happened to be connected when the range was created.
  - Change `TryBuildCaretRange` to validate and construct the native range only.
  - Let `MergeRuns` and `DiffRuns` continue operating on native ranges, then translate every `desc.runsDiff` begin/end immediately before sending the description to the current renderer.
  - In `GetInlineObjectFromPoint`, translate the renderer-returned begin/end to native positions before looking up the native-keyed `inlineObjectProperties` and calculating `start` and `length`.
  - This is required because `ResetRemoteParagraphSyncState` preserves the run/property maps while clearing synchronization state. Keeping them native lets the existing reconnect resynchronization emit every range using the latest renderer encoding, including ranges created before the first renderer connects.
- Audit all remaining protocol boundaries in `GuiRemoteGraphicsParagraph`:
  - Native-to-remote conversion is required for outgoing run endpoints, `OpenCaretRequest::caret`, `GetCaretRequest::caret`, the index into the renderer-returned caret-bounds arrays, `GetNearestCaretFromTextPosRequest::textPos`, and `IsValidCaretRequest::caret`.
  - Remote-to-native conversion is required for `GetCaretResponse::newCaret`, both endpoints returned by `GetInlineObjectFromPoint`, and the result of `GetNearestCaretFromTextPos`.
  - `GetCaretFromPoint` should continue iterating native positions and rely on the corrected `GetCaretBoundsInternal` to translate its array index. `IsValidTextPos` remains a native bounds check.
- Preserve the existing valid-position contract rather than clamping invalid indices. Both mapping arrays include their end sentinel, so all positions from zero through the corresponding encoded length are directly indexable.
- Lazy cache creation and paragraph reconnect processing remain confined to the existing UI/event execution path. The cache entries are immutable after construction, so no new synchronization primitive is needed.
- Leave `GuiRemoteRendererSingle`, the unit-test renderer configuration, and all generated/reflection files unchanged. `GuiRemoteCaretCache` is a native testable helper and requires no reflection registration or metadata/code-generation step.
- Add `Test/GacUISrc/UnitTest/TestRemote_Caret.cpp` to `UnitTest.vcxproj` and to `UnitTest.vcxproj.filters` under the existing `Source Files\Remote` filter. Do not edit `Test/Linux/UnitTest/vmake`, `vmake.txt`, or `makefile`; `vmake` already consumes `UnitTest.vcxproj` and will pick up the test automatically in later Linux/macOS work.

## VERIFICATION

- Save `TestRemote_Caret.cpp` as UTF-8 with BOM and verify that its first three bytes are `EF BB BF`. Include `GuiRemoteGraphics_Document.h` directly and test `GuiRemoteCaretCache` without starting a remote protocol or renderer.
- Use hard-coded expected arrays rather than deriving expectations through the same Vlpp conversion functions. Cover all three `CharacterEncoding` values with:
  - Empty text and ASCII-only text, including the final end position.
  - The mixed string `A¢中𦁚B`, proving UTF-8 widths `1,2,3,4,1`, UTF-16 widths `1,1,1,2,1`, UTF-32 widths `1,1,1,1,1`, interior positions mapping to cluster fronts, and both end sentinels.
  - Under `VCZH_WCHAR_UTF16`, expect UTF-8 native-to-remote `[0,1,3,6,6,10,11]` and remote-to-native `[0,1,1,2,2,2,3,3,3,3,5,6]`, UTF-16 `[0,1,2,3,3,5,6]` in both directions, and UTF-32 native-to-remote `[0,1,2,3,3,4,5]` and remote-to-native `[0,1,2,3,5,6]`.
  - Under `VCZH_WCHAR_UTF32`, expect UTF-8 native-to-remote `[0,1,3,6,10,11]` and remote-to-native `[0,1,1,2,2,2,3,3,3,3,4,5]`, UTF-16 native-to-remote `[0,1,2,3,5,6]` and remote-to-native `[0,1,2,3,3,4,5]`, and UTF-32 `[0,1,2,3,4,5]` in both directions.
- Under `#if defined VCZH_WCHAR_UTF16` only, construct an explicit-length `WString` excluding any terminator from the code-unit sequence `A,0xD800,B,0xDC00,0xD858,0xDC5A,C,0xD800`. It covers a high surrogate followed by a non-low-surrogate, a lone low surrogate, a valid pair, and a trailing high surrogate. Expect UTF-8 native-to-remote `[0,1,2,3,4,4,8,9,10]` and remote-to-native `[0,1,2,3,4,4,4,4,6,7,8]`, UTF-16 `[0,1,2,3,4,4,6,7,8]` in both directions, and UTF-32 native-to-remote `[0,1,2,3,4,4,5,6,7]` and remote-to-native `[0,1,2,3,4,6,7,8]`.
- Review the final `GuiRemoteGraphics_Document.cpp` call sites against the protocol-boundary list in `## DETAILS`. In particular, confirm that caret-bound arrays are never indexed with an untranslated native position and that no persistent run/property map stores renderer coordinates.
- From `REPO-ROOT/Test/GacUISrc`, build the default Debug|x64 configuration only through the absolute path to `REPO-ROOT/.github/Scripts/copilotBuild.ps1`. After it finishes, require `Build.log` to end with `Build succeeded.`, `0 Warning(s)`, and `0 Error(s)`.
- Run `UnitTest` only through the absolute path to `REPO-ROOT/.github/Scripts/copilotExecute.ps1 -Mode UnitTest -Executable UnitTest`. If an existing local `UnitTest.vcxproj.user` filter skips the new file, add `/F:TestRemote_Caret.cpp` following `Running-UnitTest.md` without removing unrelated relevant filters. Require `Execute.log` to show that `TestRemote_Caret.cpp` ran, all selected files and cases passed, and no memory-leak dump appeared after the summaries.
- Do not run GacJS, add a remote-renderer integration test, or perform Linux/macOS builds for this task. The cache tests cover every renderer encoding on Windows; the project registration and UTF-8 BOM prepare the same test for later Linux/macOS verification.

# UPDATES

# TEST [CONFIRMED]

Add `Test/GacUISrc/UnitTest/TestRemote_Caret.cpp` as a direct unit test of the requested public `GuiRemoteCaretCache` helper, without starting the remote protocol or a renderer.

- Verify empty and ASCII text for UTF-8, UTF-16, and UTF-32, including the final end-of-string position.
- Verify `A¢中𦁚B` against hard-coded platform-specific native-to-remote and remote-to-native mappings for all three encodings. This confirms code-unit widths, surrogate-pair cluster fronts on UTF-16 platforms, and both end sentinels.
- Under `VCZH_WCHAR_UTF16`, verify malformed and incomplete surrogate sequences built with an explicit `WString` length. Each malformed code unit must behave as an opaque width-one cluster while a valid pair still collapses its interior position to the cluster front.
- Verify `TestRemote_Caret.cpp` is UTF-8 with BOM and registered in `UnitTest.vcxproj` and `UnitTest.vcxproj.filters` under `Source Files\Remote`.
- Build Debug|x64 with `copilotBuild.ps1`. The pre-fix implementation is confirmed when the new test cannot compile because `GuiRemoteCaretCache` does not exist; after the proposal, the build must end with `Build succeeded.`, `0 Warning(s)`, and `0 Error(s)`.
- Run `UnitTest` with `copilotExecute.ps1`. `Execute.log` must show `TestRemote_Caret.cpp` ran, all selected test files and cases passed, and no memory-leak dump appeared after the summaries.
- Audit every `GuiRemoteGraphicsParagraph` protocol boundary named in the problem description, confirming persistent maps remain native and renderer-facing positions are translated only when sent or received.

The Debug|x64 build reached `TestRemote_Caret.cpp` and failed with `error C2065: 'GuiRemoteCaretCache': undeclared identifier`. This confirms the current implementation has no public cache helper capable of expressing the required UTF-8, UTF-16, and UTF-32 mappings. The test source BOM was independently verified as `EF BB BF`.

# PROPOSALS
