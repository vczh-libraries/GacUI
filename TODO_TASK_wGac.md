Mirror `TODO_TASK.md` to `wGac`, including to update `FullControlTest` and `GuiWaylandElementRenderer`. Additionally:
- `FullControlTestRawClockInterpretor.(h|cpp)` is added, `RawClockInterpretorBase` could be used to implement the raw clock rendering.
- A `IGuiPlugin` should be created to make the interpretor automatically loaded by both `FullControlTest` and `RemotingTest_Rendering_Wayland`, they can share the same code now.
Make sure `import.sh` + `syncProj.sh` + `build.sh` is working.
To verify, you should test `FullControlTest` and `RemotingTest_Rendering_Wayland`, but limit the target to only `RawClock` and `Touching` test pages.
