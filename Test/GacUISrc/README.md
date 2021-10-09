# GacUI Developer's Projects

- **Metadata_Generate**: Run both Win32 and X64 to generate metaonly reflection binary files
- **Metadata_Test**: Test the generated binary files
- **GacUI_Compiler**: Compile `Host/Resources/FullControlTest/Resource.xml` and get both binary and C++ files, with `VCZH_DEBUG_METAONLY_REFLECTION`
- **GacUI_Host**: Load the compiled binary file and run
- **GacUI_Playground**: Compile and load XML with generated DarkSkin
- **CppTest**: Compile and run generated C++ files with `VCZH_DEBUG_NO_REFLECTION`
- **CppTest_Metaonly**: Compile and run generated C++ files with `VCZH_DEBUG_METAONLY_REFLECTION`
- **CppTest_Reflection**: Compile and run generated C++ files
- **UnitTest**: Test cases for GacUI compiler

- **Lib_**: Lib files under different configurations
  - **Use Library Dependencies Input** must be turned on for all exe projects

## Notice

Run **Metadata_Generate** and **Metadata_Test** to update types.

Run **GacUI_Compiler** before executing GacUI in other projects.
