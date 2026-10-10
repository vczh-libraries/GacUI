# File System Operations

Cross-platform file and directory manipulation with path handling and content access.

## FilePath

`FilePath` is a string representation of file path.

- Use `GetPathDelimiter` to get the platform path delimiter.
- Use `operator/`, `GetName`, `GetFolder`, `GetFullPath` and `GetRelativePathFor` for path manipulation. On POSIX, joining an absolute right-hand path replaces the base folder; relative paths still resolve from the base.
- Use `IsFile`, `IsFolder` and `IsRoot` to tell the object represented by the path.
- Use `FilePath::IsAbsolutePath(path)` to compare a string with `FilePath(path).GetFullPath()`. This tests normalized absolute form, without requiring existence or resolving symbolic links. An absolute path containing alternate separators, trailing delimiters or dot segments can fail this comparison. The platform's virtual root representation follows the same equality rule.

## File Class

`File` can be initialized with an existing file path or a path where a file will be written; `FilePath::IsFile` need not already return true. It offers:

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

### Metadata and Copies

`File::GetFileInfo()` and `Folder::GetFileInfo()` return `filesystem::FileInfo`. Both use `CHECK_ERROR` when the corresponding file or folder does not exist; a failed required native metadata query also raises an error. The declarations and shared operations are owned by `<VlppOS repo>/Source/FileSystem.h` and its sibling implementation files.

- `size` and `hardLinkCount` describe file length and native link count; an unavailable link count is zero. Directory size has platform-specific meaning.
- `creationTime`, `lastAccessTime`, `lastModifiedTime` and `lastChangeTime` are `Nullable<DateTime>` in UTC. The last field is metadata-change time, not creation time. Unsupported or unrepresentable times remain empty. Windows retains FILETIME comparison precision; POSIX uses DateTime's millisecond representation and cannot represent pre-epoch timestamps here.
- `canRead`, `canWrite` and `canExecute` describe access for the current process. Windows probes access through native handles; POSIX uses effective access checks. A sharing restriction can prevent a Windows probe. Executable permission does not validate executable file format, and permissions need not agree when another operating system mounts the same filesystem.
- `isReadOnly` is the Windows attribute or the absence of POSIX write mode bits, independently of privileged-process access. `isDirectory`, `isSymbolicLink` and `isReparsePoint` describe entry kinds. Metadata follows symbolic links when target metadata is accessible, while the two link flags describe the original path.
- Native flags include hidden, system, archive, compressed, encrypted, sparse, temporary, offline, not-content-indexed, immutable and append-only. Unsupported flags are false. Linux obtains optional birth time and supported extra attributes through `statx`; macOS exposes birth time and native file flags.

`File::CopyToFile(destination, recursively)` overwrites a destination file while preserving supported native metadata. `CopyToFolder` appends the source name and calls `CopyToFile`. Passing `true` permits creation of missing destination parents; passing `false` requires them to exist. Missing sources, identical paths or hard-link aliases, invalid destinations, failed I/O and metadata-copy failures return false. Symbolic links are followed; a failure can leave partial destination content.

Windows uses `CopyFileW` and restores all four settable timestamps. macOS uses `fcopyfile` with `COPYFILE_ALL`. Linux copies data, mode bits, exposed extended attributes and access/modification times; it cannot preserve inode identity, birth/change times or ownership through this operation. Sparse allocation is not guaranteed. The API does not promise metadata that the destination filesystem cannot represent. Check the return value instead of replacing this operation with text or stream rewriting.

## Folder Class

When `FilePath::IsFolder` or `FilePath::IsRoot` return true, `Folder` could be initialized with such path. It offers:

- Content enumerations by `GetFolders` and `GetFiles` to enumerate the content.
- Folder operation by `Exists`, `Create`, `Delete` and `Rename`.

### Content Enumeration

Use `GetFolders` to retrieve all subdirectories within the folder.
Use `GetFiles` to retrieve all files within the folder.

### Folder Operations

Use `Exists` to check if a folder exists at the specified path.
Use `Create(false)` to create the folder directly, and `Create(true)` to create missing containing folders first.
Use `Delete(false)` to remove an existing folder directly, and `Delete(true)` to remove its contents recursively.
Use `Rename` to change the name or move a folder to a different location.

### Creating Folders

`Folder::Create` is special, it creates a new folder, which means you have to initialize `Folder` with an unexisting `FilePath` before doing that. In such case `FilePath::IsFolder` would return false before calling `Create`.

Pass `false` to `Create` when only the final folder should be created. Pass `true` when missing containing folders should be created recursively.

Creating a root returns false, terminating recursive creation when a requested Windows drive is unavailable.

### Deleting Folders

Pass `false` to `Delete` when only the specified folder should be removed. Pass `true` when the folder tree should be removed recursively.

## Root Directory Handling

Initializing a `Folder` with a file path with `IsRoot` returning true, is just calling `Folder`'s default constructors.

- On Windows, the root contains all drives as folders, therefore root and drives cannot be removed or renamed. A drive's full path and name will be for example `C:`.
- On Linux, the root means `/`.

## Extra Content

### Implementation Injection

You can replace the default file system implementation with a custom one for testing and specialized scenarios:

- Use `InjectFileSystemImpl(impl)` to set a custom `IFileSystemImpl` implementation
- Use `EjectFileSystemImpl(impl)` to remove that implementation and all implementations injected after it
- Use `EjectFileSystemImpl(nullptr)` to reset to the default OS-specific implementation by ejecting all injected implementations
- Use `GetOSFileSystemImpl()` to get the OS-dependent default implementation (function not in header file, declare manually)

The injected implementation affects all `FilePath`, `File`, and `Folder` class operations that interact with the file system. This enables you to create in-memory file systems for testing, provide sandboxed file access, implement virtual file systems, or add custom file system behaviors like encryption or compression.

Custom implementations also provide `IFileSystemImpl::GetFileInfo` and `FileCopy`; report unavailable metadata with empty times/default flags and unsupported copying with false.

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

File and folder operations such as `Delete`, `Rename`, `Create` and the text I/O overloads returning `bool` report ordinary I/O failures through their return value. Check that value; exception handling alone does not detect these failures.

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


## WebAssembly OPFS backend

`OpfsFileSystemImpl` in `<VlppOS repo>/Source/FileSystem.Wasm.cpp` is the default `IFileSystemImpl` under `VCZH_WASM`, selected lazily by the ordinary injection chain. It calls JavaScript OPFS APIs directly through `EM_ASYNC_JS`; applications link with Asyncify and await suspending Embind exports. It does not use an Emscripten filesystem backend or require app-specific JavaScript callbacks.

Pthread callers complete each asynchronous OPFS operation through an Emscripten-managed callback using `emscripten_sleep(0)`. This keeps returning workers joinable on Emscripten 3.1.6, whose plain promise resumption misses thread-exit handling. The continuation is confined to the OPFS boundary; ordinary thread sleeps retain their blocking semantics.

`/` is the OPFS root and the fixed working directory. Paths use `/`, normalize `.` and `..`, replace the base when joined with an absolute path, and reject traversal above root. Names cross the JavaScript boundary as UTF-16 while C++ retains `WString`.

OPFS metadata reports file size and last-modified time, directory kind and access to application-owned storage. Other times and native attributes are unavailable. Metadata-preserving `FileCopy` returns false without modifying the destination because OPFS cannot set the source timestamps on a copy.

`stream::FileStream` uses an internal `stream::MemoryStream`. ReadOnly and ReadWrite snapshot the entire file on open; ReadWrite creates missing files and preserves existing bytes. WriteOnly begins empty. Writable close replaces the complete OPFS content and may truncate it to zero; read-only close never writes. This differs from native ReadWrite's existing truncate-on-open behavior. Changes become visible to newly opened readers after close; already open readers retain their snapshots. Large files therefore require whole-file memory capacity.

Ordinary I/O failures return false or make a stream unavailable; failed writeback raises a C++ error. Root deletion and rename are rejected. File and folder rename copy the contents then remove the source because portable OPFS directory handles do not support rename. This is not atomic, and existing destinations and moves into descendants are rejected.

The unit-test launcher clears origin storage, creates configured empty folders and downloads only selected fixtures before starting Wasm. `vbuild` is JSON with a `"WASM=YES"` object. Its optional `rootFolder` maps to `/`, `includes` unions file globs, `excludes` subtracts globs and `folders` lists literal empty directories. Without `rootFolder` the other fields are ignored; without `includes` no files load. The Node server is read-only: test modifications never propagate to host files.
