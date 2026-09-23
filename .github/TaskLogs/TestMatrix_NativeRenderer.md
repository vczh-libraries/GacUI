# Test Matrix Card 2026-09-22 22:38:15 -07:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] |  |
| [Windows][CppTest_Rvm][`/Http`] |  |
| [Windows][CppTest_Rvm][`/MiniHttp`] |  |
| [Windows][CppTest_Rvm][`/Cli:<path>`] |  |
| [Windows][`/RPT`][`/Pipe`] |  |
| [Windows][`/RPT`][`/Http`] |  |
| [Windows][`/RPT`][`/MiniHttp`] |  |
| [Windows][`/FCT`][`/Pipe`] |  |
| [Windows][`/FCT`][`/Http`] |  |
| [Windows][`/FCT`][`/MiniHttp`] |  |
| [Windows][`/RVMT`][`/Pipe`] |  |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] |  |
| [Windows][`/RVMT`][`/Http`] |  |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] |  |
| [Windows][`/RVMT`][`/MiniHttp`] |  |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] |  |

## Issues Found and Fix

## Verification scope

Requested: GacJS on Windows and local CppTest only. All other native targets are excluded; their result cells remain blank.

## Local GUI Baselines

| Test App | 1st |
| --- | --- |
| CppTest | 22:39:51 -07:00 |
| CppTest_Metaonly |  |
| CppTest_Tui |  |


CppTest completed: exact two-list add/clear, bracket input, distinct Search/document markers retained through tab changes, 80 mouse payload operations with shared label styling, and all three real native shortcut dialogs including Windows global-hot-key dispatch. Exit tab → self.Close() completed with wrapper exit 0; no owned native windows remain. Native remote renderers, CppTest_Metaonly, CppTest_Rvm and CppTest_Tui were excluded.
