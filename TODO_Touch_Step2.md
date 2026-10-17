Details are in [TODO_Touch.md](./TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests.

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

- Port to remote protocol, update `DebugRemoteProtocolSop.md` to have the new test page, and make sure it works with native renderers and GacJS.
  - Unlike automation service, remote protocol would require point sequence start/end, and any point event shoot between start/end will need to have an index to maintain the order in core, and any point event outside of start/end will be ignored. Subsequence starts before end and subsequence ends before start will also be ignored. This item is talking about remote protocol schema and how core is processing them.
  - Prepare remote protocol based test cases.
  - Port to GacJS.
  - GacJS and remote native renderer should always report to the browser the whole GacUI div or the OS the whole native window supports point events.
    - So that point events will be passed to core, and core translates unhandled point events to mouse events.
    - That means fallbacks should be done in core instead of renderer side, it would be similar to automation service, shared code is available to simplify the code.
    - It also make touching experience on phone browser works, otherwise fingers will just drag the whole webpage away instead of sending proper remote protocol events.
- Update KB pages on `Tools` repo, KB pages should not mention `TODO_Touch*.md`, they should be self-contained, including:
  - How to implement `INativeWindow` to respond for native OS point events.
  - How to respond to `INativeWindowListener` and composition point events.
  - How a remote protocol renderer interact with core by point events.
  - The format of point events in automation service.
  - Mention that point events only available on Windows and browser, including WASM (even not implemented at this moment but just treat it works, doesn't need honesty because the next task is going to do this).
- Run `job:copilotInitAll` but skip learning, details could be obtainerd from `../AGENTS.md`.
