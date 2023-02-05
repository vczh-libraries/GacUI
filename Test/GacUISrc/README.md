# GacUI Developer's Projects

- **Metadata_Generate**: Run both Win32 and X64 to generate metaonly reflection binary files.
- **Metadata_Test**: Test the generated binary files, both Win32 and X64 at the same time.
- **GacUI_Compiler**: Compile `DarkSkin` and `FullControlTest`, both Win32 and X64 at the same time.
  - **GacUI_Host**: Load the compiled binary file and run.
  - **CppTest**: Compile and run generated C++ files in **hosted mode** with `VCZH_DEBUG_NO_REFLECTION`.
  - **CppTest_Metaonly**: Compile and run generated C++ files with `VCZH_DEBUG_METAONLY_REFLECTION`.
  - **CppTest_Reflection**: Compile and run generated C++ files.
  - **Playground**: Compile and load XML with generated DarkSkin.
- **UnitTest**: Test cases for GacUI.

- **Lib_**: Lib files under different configurations
  - **Use Library Dependencies Input** must be turned on for all exe projects

## Source File Dependencies

- **Core**: Minimum source code that opens and renders a window in the OS.
- **CoreApplication** -> **Core**: Minimum source code that runs `GuiApplication`.
- **GacUI** -> **CoreApplication**: All controls.
- **Utilities** -> **CoreApplication**: Utilities.
- **Utilities_Controls** -> **Utilities** + **GacUI**: Utilities that brings UI.
- **Reflection** -> **Utilities_Controls**: All reflection definition.
- **Compiler** -> **Reflection**: GacUI XML Compiler.
- **Windows** -> **Utilities**: Windows platform provider.

## Notice

- Run **Metadata_Generate** in both Win32 and X64 to generate binary files.
- Run **Metadata_Test** to update types.
- Run **GacUI_Compiler** if DarkSkin or FullControlTest is changed.
