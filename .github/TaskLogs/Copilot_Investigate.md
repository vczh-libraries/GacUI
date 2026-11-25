# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

`RemotingTest_Rendering_Win32`'s `RawClock` test page is not rendering, make sure it works like `CppTest`. If any changes is applied to `FullControlTest` make sure it is updated to `Release/GacUI_ControlTemplates/BlackSkin` tutorial, but I would except `FullControlTest` is fine and guess the issue happens only in the remote protocol native renderer test app side. Also update `GacJS` to make sure the same new rendering is ported to `GacJS` about `RawClock` and `Touching` test page. The `wasm_fct/index.html` test page might also need to update, but you are not able to test it on Windows, the good thing is the code structure is almost the same with `GacJS`'s main test page.

# UPDATES

# TEST [CONFIRMED]

- Open Misc > Raw Clock in Debug x64 `CppTest` and in `RemotingTest_Core /FCT` with `RemotingTest_Rendering_Win32`. Compare the received time with the painted face and hands on successive ticks. Exercise Direct2D and GDI. Inspect native pixels as well as Core and renderer automation data so a ticking but empty surface fails the check.
- Exercise the built GacJS HTTP page with fixed protocol data. Sample the white clock face and ticks, the three hand colors and their black outlines. Check the Touching rectangle fill, its border and the orange contact ring, then update its transform and release the contact. Run the existing live clock and touch tests against Core to check ticking, page changes and contact updates.
- Check that the HTTP and Wasm FCT entries load the same two painters and that the packaged files match. Run the controlled Wasm page lifecycle tests on Windows. Running the Wasm Core itself requires the Linux build.

Before the fix, both Win32 remote backends received ticking `FullControlTest::Clock` data over MiniHTTP while their captured clock surfaces stayed empty. The local Direct2D `CppTest` capture showed the rim, hour marks and colored hands, and the local RawClock UIA scenario passed 186 assertions. The remote GDI Touching capture showed its sky-blue rectangle.

Two controlled Chromium cases failed on the original GacJS painters. The white rim had no black outline. The Touching border sample was sky blue and its contact sample was opaque yellow.
