# File System Operations

Cross-platform file and directory manipulation with path handling and content access.

## FilePath

`FilePath` is a string representation of file path.

- Use `GetName`, `GetFolder`, `GetFullPath` and `GetRelativePathFor` for path manipulation.
- Use `IsFile`, `IsFolder` and `IsRoot` to tell the object represented by the path.

## File Class

When `FilePath::IsFile` returns true, `File` could be initialized with such path. It offers:

- Text reading by `ReadAllTextWithEncodingTesting`, `ReadAllTextByBom` and `ReadAllLinesByBom`.
- Text writing by `WriteAllText`, `WriteAllLines`.
- File operation by `Exists`, `Delete` and `Rename`.

### Text Reading Methods

Use `ReadAllTextWithEncodingTesting` for automatic encoding detection and text reading.
Use `ReadAllTextByBom` for reading text files with BOM (Byte Order Mark) support.
Use `ReadAllLinesByBom` for reading text files line by line with BOM support.

### Text Writing Methods

Use `WriteAllText` for writing complete text content to a file.
Use `WriteAllLines` for writing multiple lines of text to a file.

### File Operations

Use `Exists` to check if a file exists at the specified path.
Use `Delete` to remove an existing file.
Use `Rename` to change the name or move a file to a different location.

## Folder Class

When `FilePath::IsFolder` or `FilePath::IsRoot` return true, `Folder` could be initialized with such path. It offers:

- Content enumerations by `GetFolders` and `GetFiles` to enumerate the content.
- Folder operation by `Exists`, `Delete` and `Rename`.

### Content Enumeration

Use `GetFolders` to retrieve all subdirectories within the folder.
Use `GetFiles` to retrieve all files within the folder.

### Folder Operations

Use `Exists` to check if a folder exists at the specified path.
Use `Delete` to remove an existing folder and its contents.
Use `Rename` to change the name or move a folder to a different location.

### Creating Folders

`Folder::Create` is special, it creates a new folder, which means you have to initialize `Folder` with an unexisting `FilePath` before doing that. In such case `FilePath::IsFolder` would return false before calling `Create`.

## Root Directory Handling

Initializing a `Folder` with a file path with `IsRoot` returning true, is just calling `Folder`'s default constructors.

- On Windows, the root contains all drives as folders, therefore root and drives cannot be removed or renamed. A drive's full path and name will be for example `C:`.
- On Linux, the root means `/`.

## Extra Content

### Implementation Injection

You can replace the default file system implementation with a custom one for testing and specialized scenarios:

- Use `InjectFileSystemImpl(impl)` to set a custom `IFileSystemImpl` implementation
- Use `EjectFileSystemImpl(impl)` to remove a specific injected implementation
- Use `EjectFileSystemImpl(nullptr)` to reset to the default OS-specific implementation by ejecting all injected implementations
- Use `GetOSFileSystemImpl()` to get the OS-dependent default implementation (function not in header file, declare manually)

The injected implementation affects all `FilePath`, `File`, and `Folder` class operations that interact with the file system. This enables you to create in-memory file systems for testing, provide sandboxed file access, implement virtual file systems, or add custom file system behaviors like encryption or compression.

Implementation injection should typically be done during application startup before any multi-threaded usage begins, as it affects global state.

### File Stream Implementation

The file system implementation also provides file stream creation through `IFileSystemImpl::GetFileStreamImpl`. This method is used internally by `FileStream` constructors but can be useful when implementing custom file systems.

There is also a `CreateOSFileStreamImpl` function available that creates the OS-specific file stream implementation directly. Like `GetOSFileSystemImpl`, this function is not declared in header files and must be declared manually:

```cpp
namespace vl
{
    namespace stream
    {
        extern Ptr<IFileStreamImpl> CreateOSFileStreamImpl(const WString& fileName, FileStream::AccessRight accessRight);
    }
}
```

### Path Manipulation Best Practices

When working with file paths, always use `FilePath` for cross-platform compatibility. The class automatically handles path separators and normalization across different operating systems.

### Error Handling

File and folder operations may throw exceptions when encountering permission issues, missing files, or other I/O errors. Always consider wrapping file operations in appropriate exception handling.

### Performance Considerations

For large files, consider using stream-based operations instead of reading entire files into memory with `ReadAllText` methods. The streaming approach provides better memory efficiency for large file processing.

### Encoding Detection

The `ReadAllTextWithEncodingTesting` method attempts to automatically detect the encoding of text files, making it suitable for processing files with unknown encodings. However, for better performance and when the encoding is known, use the specific BOM-based reading methods.

### Testing Applications

Implementation injection is particularly valuable for unit testing file system operations:

- Create isolated test environments without affecting the real file system
- Simulate file system errors and edge cases
- Test file operations with predictable directory structures
- Mock file system behaviors for consistent testing across different environments