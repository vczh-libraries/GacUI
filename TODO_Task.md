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
