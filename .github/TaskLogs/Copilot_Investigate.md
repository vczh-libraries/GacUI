# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

In [RemotingHelpers](Test/RemotingHelpers/) there are some test only files. They will be code-packed in `Release`, and when calling `../Tools/Tools/Build.ps1 -Project UpdateRelease` they will be copied to `../Release/Import` and then removed. I would like you to do the following work:

- Create [RemotingHelpers](Test/RemotingHelpers/) /README.md to describe how to use them, they are test only helpers to improve the coding performance of setting up remote protocol for GacUI app. Although people are expected to later implement INetworkProtocol(Server|Client) by their own for security reason, but existing implementations are helpful when starting a new project. You need to:
  - List what is available with a short description
  - For each class or function, describe how to use them, what is the mechanism inside, where to find test apps in GacUI that uses them for reference.
- Fix `Build.ps1 -Project UpdateRelease` to not delete code-packed version of them, instead move them to `../Release/Import-Test`, also copy the new created README.md there.
- Note that since the README.md is for describing code before and after codepack. so avoid mentioning any actual file name in `Test/RemotingHelpers` and `../Release/Import`, add this to the very top of the README.md in your own words.
- Avoid building anything, since there is a job be executed parallelly. To verify, you can only call CodePack manually on GacUI, and run `Build.ps1 -Project UpdateRelease`. The script will modify the Release repo, you must check if Import-Test folder is properly prepared and ignore other Release repo side effect, as at the end you must revert all Release side effects (including not to keep Import-Test folder).
- commit and push all local changes (I would expect only Tools and GacUI repos) once finishing.

# UPDATES

# TEST [CONFIRMED]

Do not build any solution or project. Verification is limited to these repository workflows:

1. Run GacUI CodePack manually and inspect the dedicated test-only packed outputs.
2. Run `Build.ps1 -Project UpdateRelease` and confirm `Release/Import-Test` contains the packed helper outputs plus the helper README, while `Release/Import` does not receive those test-only artifacts.
3. Restore the Release repository completely, including removal of `Import-Test`, and confirm only the intended GacUI and Tools repository changes remain.

Success requires the README to cover every helper API and reference its consuming test applications without naming representation-specific source or packed files. It also requires both commands to succeed, the temporary `Import-Test` snapshot to have the expected contents, and the Release repository to finish clean.

Static inspection confirmed the old `UpdateRelease` implementation removed legacy helper names from the production import and did not prepare `Import-Test`. GacUI CodePack already emits one neutral root pair and one IncludeOnly mirror, so the release update must copy only the root pair to the new test-only snapshot.

# PROPOSALS

- No.1 Preserve a documented test-helper snapshot outside production imports [CONFIRMED]

## No.1 Preserve a documented test-helper snapshot outside production imports

Document the complete generic helper surface in a representation-independent README. Change `UpdateRelease` so the dedicated packed helper pair is transferred to `Release/Import-Test` instead of being removed after the ordinary production imports are refreshed, and copy the README into the same folder. Keep ordinary GacUI release pairs and `Release/Import` independent from this test-only layer.

### CODE CHANGE

Created the requested symbol-based README with the complete public helper inventory, method sequencing, internal mechanisms, transport/security boundary, and links to every consuming GacUI test application. It contains no representation-specific helper or import file name.

Updated the owning Tools release script to recreate `Release/Import-Test` as a deterministic snapshot, copy the neutral root CodePack pair there, and copy the README beside it while retaining cleanup of stale helper artifacts from production `Release/Import`.

Updated GacUI repository guidance to distinguish the production import from the test-only snapshot. No C++ helper implementation or production release artifact was edited.

### CONFIRMED

The permitted manual GacUI CodePack command completed successfully. It reproduced the dedicated root helper pair and its IncludeOnly mirror without changing any tracked GacUI release output, and a symbol scan confirmed that the helper APIs do not leak into ordinary `GacUI*` amalgamations.

The permitted `Build.ps1 -Project UpdateRelease` command then completed successfully. The generated `Release/Import-Test` snapshot contained exactly three files: the README and the neutral declaration and implementation outputs. SHA-256 comparisons proved that all three were byte-identical to their GacUI sources. No `Test.RemotingHelpers*` artifact existed in production `Release/Import`.

After inspection, all tracked changes made by the release update were restored and the generated `Release/Import-Test` directory was removed. The Release repository finished clean. No verification command other than the two explicitly permitted workflows and read-only repository/file inspections was run.
