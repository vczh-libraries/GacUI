This is a continuation of previous works:
- TODO_Injection.md
- TODO_Injection2.md

1. Fix `RemotingTest_Core`. When starting it with `/Http`, it crashes without admin permission because it listens to +:8888.
I would like you to do a fallback, if listening to +:8888 does not work, fallback to localhost:8888 instead. And the `HttpServer` class with expose a "GetLocalhostOnly()" property telling you the choise, and `RemotingTest_Core` needs to say it listens to `localhost` only when returning true.

2. In `wGac` and `iGac`, `.test.sh` accepts `-remoteFS` and other arguments, it should use double -. Fix them and update their markdown files accordingly. Do not update the document if the argument is not mentioned. If C++ apps accept single - you should also change it to use double -. No verification is needed.
Fix `TODO_Injection3.md` accordingly as this task is about to submit to `iGac` execution, in this case `iGac` knows that double - is the correct one.

3. Update `GacJS` to implement all 3 injections. File system implementation will follow WASM's core side implementation.
- In `Gaclib/website/entry`, currently we have `?rvmhost`. We need to extend it to:
  - ?rvmhost&remoteDT&remoteLocale&remoteFS, case insensitive, all optional.
  - `inject*` decides how `ControllerGlobalConfig::inject*` is set.
  - Add 4 check boxes and a "REFRESH" button to the first row, when "REFRESH" is clicked, each check box decides if options after "?" is used, and jumps to the current URL with new options appended. All default to unchecked.
  - Additionally, support `corehost=localhost`, with a check box and a text box, text box should be disabled when the check box is not checked.
- Before running `GacJS` to verify, local changes are recommended to push first, in order to verify:
  - You can change `GacJS` to offer fake implementation, for example, a different locale and format list, a different time zone, a fake file system, and see if they correctly affects dedicated pages in `FullControlTest`. Since local changes are already pushed, you can easily identify code that offers fake implementations and revert them once they are no longer needed. DO NOT check in fake implementations, they are meant to be discarded after finishing this task.
  - You need to turn options in core/renderer on and off and make sure core picks the right implementation.
  - When a renderer disconnects, you can still use core side automation service to read the UI and see if fallback works.
- Update `GacJS`'s document to imply introduce these new features.
