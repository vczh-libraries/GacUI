# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

I would like you to carefully review [DebugRemoteProtocolWithGacJS.md](DebugRemoteProtocolWithGacJS.md) and [DebugRemoteProtocolWithNativeRenderer.md](DebugRemoteProtocolWithNativeRenderer.md) , and fix the matrix part in the document, make sure they list complete test targets, use your own word, emphasis the test matrix at the beginning of documents, after background introduction, with its own section called "Test Matrix".

For native renderer, we should test:
CppTest_Rvm + RemotingTest_RvmHost with all network protocols (but for Linux/macOS only /MiniHttp is available)
RemotingTest_Core:
- Dimension 1: /RPT, /FCT, /RVMT, when doing /RVMT we also need to run RemotingTest_RvmHost
- Dimension 2: /Pipe, /Http, /MiniHttp (but for Linux/macOS only /MiniHttp is available)
- By saying dimension I mean

For GacJS, we should follow the exact matrix as native renderer but exclude:
- CppTest_Rvm, as this test app does not need a remote renderer
- /Pipe, because GacJS only supports /Http and /MiniHttp (but for Linux/macOS only /MiniHttp is available)

commit and push all local changes, rebase if conflict.

After committing, you are going to (only) do full GacJS verification. commit and push all local changes if anything need to fix, rebase if conflict. GacUI has been updated so you need to rebuild it.

# UPDATES

## UPDATE

by the way, also scan [DebugRemoteProtocolSop.md](DebugRemoteProtocolSop.md) and see if it miss something, by the way, this sop document records actual test steps against feature and error handling, other two documents records the setup, keep them in this way.

# TEST [CONFIRMED]

Compare both operation guides with the requested Cartesian products and the
shared SOP. The problem is confirmed because the native guide presents only a
transport table, omits `CppTest_Rvm`, and describes Core startup using only
`/RPT` and `/FCT`; the GacJS guide likewise omits `/RVMT` and delegates its
required matrix to a deleted Tools job. The resulting documents do not enumerate
the required targets themselves.

The documentation change succeeds when each guide contains a prominent
`Test Matrix` section immediately after its background introduction, describes
dimensions as Cartesian products rather than alternatives, lists every
platform-supported target, and consistently identifies
`RemotingTest_RvmHost` as required for every `/RVMT` run. After the documentation
commit, the Windows GacJS matrix succeeds only if a fresh rebuilt GacUI Core and
the GacJS browser renderer pass the shared SOP for all six application/transport
combinations: `/RPT`, `/FCT`, and `/RVMT`, each over `/Http` and `/MiniHttp`.

# PROPOSALS

- No.1 Make each renderer guide own its complete Cartesian-product matrix

## No.1 Make each renderer guide own its complete Cartesian-product matrix

Move the normative matrix into an early `Test Matrix` section in each guide,
remove reliance on the deleted verification job, state the platform-specific
target counts, and update later startup text so it no longer narrows the listed
applications to `/RPT` and `/FCT`. Keep operational details and shared UI
expectations delegated to the existing SOP. Review the SOP separately for gaps
between its declared behavior and executable steps, adding feature or error
handling procedures only there rather than duplicating them in either setup
guide.

### CODE CHANGE
