# General Instruction

## Solution to Work On

You are working on the solution `REPO-ROOT\Test\GacUISrc\GacUISrc.sln`,
therefore `SOLUTION-ROOT` is `REPO-ROOT\Test\GacUISrc`.

## Projects for Verification

The `REPO-ROOT\Test\GacUISrc\UnitTest\UnitTest.vcxproj` is the unit test project.
When any *.h or *.cpp file is changed, unit test is required to run.

The unit test project is required to run when any source code or test cases are changed.
Except for the `GuiRemoteRendererSingle` class which is not covered in the unit test.

For any GacUI specific unit test that running with the GacUI unit test framework,
when it calls `GacUIUnitTest_StartFast_WithResourceAsText` with path, for example, `Application/Windows/Order`,
running it ends up creating log files in `REPO-ROOT\Test\Resources\UnitTestSnapshots\Application\Windows`:
- Order.json: an entry of snapshots for this test case
- Order[*].*: log files about remote protocol recordings and compiler output
- Order\Frame_*.json: snapshot of the UI DOM tree for each frame.

Each Frame_*.json is captured at each `OnNextIdleFrame` call, recording what the UI look like before running the code in this frame.
This is the reason why the name of the frame should say what the previous frame was done,
so that frame names in snapshot files make sense.

### Code Generation Projects

Code generation projects are CLI projects.
They are required to run when a certain set of files are changed,
in order to generate code paring with them.
Here are a list of projects to run and files that should trigger them:

#### Metadata_Generate and Metadata_Test

These two projects need to run if any reflection code is touched:
- `GuiReflection*.cpp` is updated.
- `GacUI_Compiler` project is executed.

To execute these projects, you should:
- Build the solution with Debug|Win32.
- Run `Metadata_Generate` with Debug|Win32.
- Build the solution with Debug|x64.
- Run `Metadata_Generate` with Debug|x64.
- Run `Metadata_Test` with Debug|x64.

#### Metadata_UpdateProtocol

This project need to run if `REPO-ROOT\Source\PlatformProviders\Remote\Protocol\*.txt` is updated.

It generates `REPO-ROOT\Source\PlatformProviders\Remote\Generated\*`.

#### GacUI_Compiler

This project need to run if any of the following XML file is updated:
- `REPO-ROOT\Source\Utilities\FakeServices\Dialog\*.xml` -> generates `REPO-ROOT\Source\Utilities\FakeServices\Dialog\Source\*`.
- `REPO-ROOT\Source\UnitTestUtilities\SnapshotViewer\*.xml` -> generates `REPO-ROOT\Source\UnitTestUtilities\SnapshotViewer\Source\*`.
- `REPO-ROOT\Test\Resources\App\DarkSkin\*.xml` -> generates `REPO-ROOT\Test\GacUISrc\Generated_DarkSkin\Source_(x86|x64)\*`.
  - **IMPORTANT**: `REPO-ROOT\Source\Skins\DarkSkin` has another copy, this is used by the CI and is not involved in this solution, ignore it.
- `REPO-ROOT\Test\Resources\App\FullControlTest\*.xml` -> generates `REPO-ROOT\Test\GacUISrc\Generated_FullControlTest\Source_(x86|x64)\*`.
- `REPO-ROOT\Test\Resources\App\RemoteProtocolTest\*.xml` -> generates `REPO-ROOT\Test\GacUISrc\Generated_RemoteProtocolTest\Source_(x86|x64)\*`.

After running `GacUI_Compiler`, you should always `git status` to find if there is any untracked `*.UI.errors.txt`.
- Such file means there are compile errors in some xml files, read it to find the detail.
- You don't need to delete the file, if `GacUI_Compiler` succeeds the next time, they will be gone.
