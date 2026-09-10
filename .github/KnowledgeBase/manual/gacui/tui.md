# Build a terminal application

GacUI's text user interface (TUI) runs locally in a terminal on Windows, Linux and macOS. It uses the same controls, compositions, data binding and XML resource compiler as a graphical application, with `TuiSkin` templates sized in terminal cells. The [TuiSkin tutorial](https://github.com/vczh-libraries/Release/tree/master/Tutorial/GacUI_ControlTemplate/TuiSkin) contains a complete control showcase, localized dialogs, rich text and window-closing examples.

## Choose the platform entry point

Define `void GuiMain()` as for a graphical GacUI application, and call the platform's terminal setup function from your console entry point:
- Windows: `SetupTuiWindowsRenderer()`, declared in GacUI's `GacUI.h`. Use Windows Terminal.
- Linux: `vl::presentation::wayland::SetupTuiWaylandRenderer()`, declared in wGac's `WGac/TUI/TuiWGacController.h`.
- macOS: `vl::presentation::osx::SetupTuiCocoaRenderer()`, declared in iGac's `Mac/TUI/TuiCocoaController.h`.

All three return `int` and accept an optional `const vl::presentation::TuiConfiguration& configuration = {}`. Its `tabInterval` field defaults to 4 and must be positive; it controls tab stops in terminal text layout.

The setup function initializes GacUI and calls `GuiMain()` inside the terminal session. It combines the shared terminal renderer and hosted-window support with the platform's native services. For example, wGac supplies native image services and a terminal clipboard service using XWayland when available, while iGac uses Cocoa services without creating an AppKit window or taking focus away from the terminal. Linux and macOS startup initialize `LC_CTYPE` from the environment before creating services; use a UTF-8 locale. The [wGac showcase](https://github.com/vczh-libraries/wGac/tree/master/WGacTuiControlTest) and [iGac showcase](https://github.com/vczh-libraries/iGac/tree/master/MacTuiControlTest) provide platform entry points and projects.

GacUI uses [VlppOS TUI](.././vlppos/using-tui.md) for terminal input, cell buffers and output. The setup functions select its automatic color mode. `TuiConfiguration` configures GacUI text layout; it does not contain the lower-level `TuiStartOptions::colorMode` option.

## Build and run the Windows showcase

- [Download the Release repository](https://github.com/vczh-libraries/Release), including its Import, Tools and Tutorial folders.
- Open `Tutorial/GacUI_ControlTemplate/GacUI_ControlTemplate.sln` with the Visual Studio C++ toolset configured by its projects. Build the TuiSkin project in Debug or Release, for Win32 or x64. The solution also builds its GacUILite dependency.
- Open Windows Terminal and change directory to `Tutorial/GacUI_ControlTemplate/TuiSkin`. Keep this as the working directory: the application loads `../UIRes/TuiSkin.bin`.
- Run the executable from the solution's output folder. Debug Win32 uses `../Debug/TuiSkin.exe`; Debug x64 uses `../x64/Debug/TuiSkin.exe`. Substitute Release for a Release build.

Run interactively with the terminal's input and output handles; do not redirect application output into a file or pipe. The first frame fits the current viewport, even when it differs from the showcase's authored 120 by 40 size. Resize the terminal to check layout and scrolling. The Exit page demonstrates direct and queued Hide/Close requests, a close veto and explicit Stop calls. An accepted close returns to the shell and restores terminal state.

## Console startup and TuiSkin registration

The tutorial is a Console application. Its reusable startup source, [GacUILite/TuiMain.cpp](https://github.com/vczh-libraries/Release/blob/master/Tutorial/Lib/GacUILite/TuiMain.cpp), calls the local terminal setup function:
```C++
int wmain()
{
    return SetupTuiWindowsRenderer();
}
```

Define `GACUILITE_TUI_APPLICATION` for every configuration of the console project. Compile both GacUILite startup sources directly in that project: `WinMain.cpp` is wholly guarded by `#ifndef GACUILITE_TUI_APPLICATION`, and `TuiMain.cpp` by `#ifdef GACUILITE_TUI_APPLICATION`. The shared static library is compiled separately, so an executable's macro cannot change its compilation. Existing graphical projects use the library's GUI startup without defining this macro.

GacUILite compiles `Import/Skins/TuiSkin/TuiSkin.cpp` with `/bigobj`. The startup plugin depends on `GacGen_TuiSkinResourceLoader` and initializes the palette before registering the theme:
```C++
tuiskin::SetColorPackage(tuiskin::CreateDefaultColorPackage());
RegisterTheme(Ptr(new tuiskin::TuiTheme));
```
 These calls run before controls are constructed in `GuiMain()`. Keep the plugin registration and dependency from the supplied startup source when reusing it.

## Compile and load resources

Authored resources live in `TuiSkin/UI/TuiControlTest`. After editing XML, localized text or document resources, run the supplied resource builder and rebuild the C++ project. For a checkout at `C:/Code/Release`, use:
```PowerShell
& 'C:\Code\Release\Tools\GacBuild.ps1' -FileName 'C:\Code\Release\Tutorial\GacUI.xml'
```
 The builder generates and merges C++ for both architectures. The tutorial compiles `Source/TuiControlTestPartialClasses.cpp` and includes `Source/TuiControlTest.h`. Generated ordinary includes name `GacUI.h` and `Skins/TuiSkin/TuiSkin.h`.

The resource's `GacGenConfig/Cpp/Resource` setting is `../../../UIRes/TuiSkin.bin`, relative to its resource directory. Generated C++ supplies the UI classes; the binary supplies non-code resources such as rich text documents. The tutorial loads the binary before constructing its main window:
```C++
void GuiMain()
{
    {
        FileStream fileStream(L"../UIRes/TuiSkin.bin", FileStream::ReadOnly);
        GetResourceManager()->LoadResourceOrPending(fileStream);
    }
    tuidemo::TuiMainWindow window;
    window.ForceCalculateSizeImmediately();
    GetApplication()->Run(&window);
}
```
 Build from the packaged project to retain its required imports and preprocessor definitions. It uses `VCZH_DEBUG_NO_REFLECTION`; the separate `TuiSkinReflection` pair is unnecessary for this generated-C++ application.

## Change the theme while running

After changing a TuiSkin color package or replacing a registered theme, call `GuiApplication::RefreshThemes()` on the UI thread to update existing windows, including hidden menus and popups. For a single control subtree, call `GuiControl::RefreshThemes()`. Both methods rebuild templates supplied by the registered themes and preserve explicitly assigned `ControlTemplate` factories. An explicit template does not prevent its child controls from being refreshed.

For example, queue a palette change from a button's event handler so that the current event finishes before its template is replaced:
```C++
GetApplication()->InvokeInMainThread(GetApplication()->GetMainWindow(), []()
{
    tuiskin::SetColorPackage(tuiskin::CreatePurplePackage());
    GetApplication()->RefreshThemes();
});
```
 Updating the palette alone does not rebuild existing templates. See [Refreshing existing controls](.././gacui/kb/dtemplates.md) for refresh scope and state preservation.

## Layout, controls and terminal limits

One layout unit is one terminal cell. Use `TuiFont` at size 1; the application does not select the terminal's physical font or cell size. CJK text can occupy two columns. Text styles and colors depend on the terminal's rendering capabilities. Size templates for cell geometry instead of reusing pixel-sized graphical templates.

Keyboard shortcuts can only use events the terminal delivers. Window-manager and terminal shortcuts may consume combinations such as Alt+F4, Super/Windows keys or Command keys before the application receives them. POSIX terminals often do not report physical key releases, and modifier state comes from received terminal events. Test shortcuts in the target terminal and provide a visible command for essential actions.

The main showcase uses `SystemFrameWindow` and `MinSizeLimitation="NoLimit"` on its bounds so a large hidden page cannot enlarge the physical viewport. Child windows, menus and fake Message/Color/Font/File dialogs are hosted inside that viewport. Normal main-window Hide/Close is cancellable through the existing closing events; dismissing a child dialog keeps the application running, and an open modal retains normal close interception.

TuiSkin includes buttons, text boxes, menus, tabs, lists, trees, detail/grid views, scroll controls and text-based document controls. The showcase demonstrates their supported templates. ImageFrame, polygons, gradients, 3D elements and other unregistered graphical elements have no TUI renderer. The document image demonstrations use textual placeholders. Ribbon and toolbar templates are not supplied. Use supported menu/button controls when adapting such an interface.

