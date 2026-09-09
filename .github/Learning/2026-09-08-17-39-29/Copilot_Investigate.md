# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

I think it is fine to add the `*Element` to the default XML namespace list, please do that, rebuild, use \<TuiBorder/> directly in XML files instead of adding the xmlns:elements thing, and commit+push the code. No need to do the release process.

# UPDATES

## UPDATE

btw you need to run UnitTest to regenerate those snapshots, anyway please continue your work

# TEST

Regenerate all resource projects with GacUI_Compiler, including TuiSkin and TuiControlTest using unqualified TuiBorder tags. Build Debug Win32/x64 and run the required metadata generators/validation. Regenerate both architecture compiler snapshots through the existing unit-test executable, accepting the added default import while checking that UI frame/recording snapshots remain unchanged. No release process or existing-app manual tests.

# PROPOSALS

- No.1 Add the default element suffix mapping and simplify TUI XML. [CONFIRMED]

## No.1 Add the default element suffix mapping and simplify TUI XML

Add presentation::elements::*Element immediately after presentation::elements::Gui*Element in GuiInstanceContext's default namespace list. Remove the explicit elements namespace and prefix from TUI XML, including the checked-in skin XML copy. Update namespace documentation and regenerate all affected outputs with the existing tools.

### CODE CHANGE

Added the default suffix mapping and removed elements namespace declarations/prefixes from nine authored TUI resource XML files and seven mirrored skin XML files. Updated the XML mapping guideline, namespace manual and TUI architecture documentation. GacUI_Compiler regenerated all nine resources for both architectures successfully with no resource errors. The resulting 36 compiler text files contain only 1,464 added default-import lines, alongside 36 updated binary resources; generated C++ is unchanged. Final Debug Win32/x64 solution builds passed with zero warnings/errors. Both metadata generators and Metadata_Test x64 passed, leaving metadata unchanged. Debug Win32 UnitTest passed 90/90 files and 1,737/1,737 cases, with no leak dump. Its 736 changed compiler snapshots contain only 1,758 added import lines, and no UI frame/recording changes. Release x64 was also built for the complete UnitTest snapshot pass; the release/publishing process was not invoked.

The Release x64 build passed with zero warnings/errors, and its first complete UnitTest run passed 90/90 files and 1,737/1,737 cases. Snapshot review found four additional recording changes in ScrollResetOnNavigation (element ordering in frame 4), while the same recordings had matched baseline after Debug Win32. Repeat the x64 suite without source changes to investigate whether these differences reproduce; do not manually restore the recording files.

### CONFIRMED

The repeated Release x64 UnitTest run passed all 90 files and 1,737 cases without a source change. The four ScrollResetOnNavigation recording differences did not reproduce: regeneration returned them to baseline, with no manual snapshot editing or restoration. This records an observed recording variation, not a fix for its underlying cause.

The final snapshot diff consists of 1,472 compiler text files (736 per architecture), containing only 3,516 added `using presentation::elements::*Element;` lines. All UI frame and recording files match baseline. Both architectures compile the authored TUI resources with unqualified TuiBorder tags. Generated C++ remains unchanged, and the 36 generated resource text files contain only the new import alongside 36 regenerated binary resources. All requested builds and metadata checks passed; Debug Win32 UnitTest additionally passed its leak check. The exact XML replacement audit and git diff --check passed. Release, Import and reflected metadata are unchanged, and no release/publishing process or existing-app manual testing was performed.
