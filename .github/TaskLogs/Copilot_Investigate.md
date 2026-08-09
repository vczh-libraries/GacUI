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

- No.1 Make each renderer guide own its complete Cartesian-product matrix [CONFIRMED]

## No.1 Make each renderer guide own its complete Cartesian-product matrix [CONFIRMED]

Move the normative matrix into an early `Test Matrix` section in each guide,
remove reliance on the deleted verification job, state the platform-specific
target counts, and update later startup text so it no longer narrows the listed
applications to `/RPT` and `/FCT`. Keep operational details and shared UI
expectations delegated to the existing SOP. Review the SOP separately for gaps
between its declared behavior and executable steps, adding feature or error
handling procedures only there rather than duplicating them in either setup
guide.

### CODE CHANGE

- Added an early `Test Matrix` to both renderer setup guides. The native guide
  now lists the Cartesian product for standalone RVM requesters and all three
  Core applications on every platform, including the platform-specific target
  count. The GacJS guide lists `/RPT`, `/FCT`, and `/RVMT` across its supported
  HTTP transports and explicitly excludes only the local requester and named
  pipes.
- Added the missing `/RVMT` host startup and cleanup commands to the Windows,
  Linux, and macOS setup sections and removed the stale dependency on the
  deleted Tools verification job.
- Kept feature actions and pass/fail behavior in `DebugRemoteProtocolSop.md`.
  Added executable checks for rejecting a second view-model host and for the
  exact GacJS error-mask result of Core-authored RPT and RVM-host-loss errors.
  The final SOP scan also found and added the missing normal close procedure for
  `/FCT`, including the required renderer terminal state and no-hang check.
  Reduced duplicated result wording in the setup guides to references to the
  shared SOP.

### CONFIRMED

Static document checks confirmed that both `Test Matrix` headings immediately
follow their background introductions, all requested application and transport
tokens are present, the Windows GacJS row expands to six targets, the obsolete
verification-job link is absent, and the SOP contains both the second-host
admission test and GacJS-specific fatal-error observations. `git diff --check`
also passes. The full live GacJS verification is intentionally performed only
after this documentation proposal is committed, as requested.

The post-commit Windows GacJS verification rebuilt GacUI successfully with zero
warnings and zero errors, then passed `yarn build` and the complete `yarn test`
suite. All six matrix targets passed the SOP: `/RPT`, `/FCT`, and `/RVMT` over
both `/Http` and `/MiniHttp`. This included renderer replacement and state
continuity for `/RPT`, list/editor state and force-exit for `/FCT`, and second
host rejection plus continued translation through the accepted host for
`/RVMT`. The `/RPT` fatal operation produced one exact
`This is a fatel error!` mask and Core exit code 3 on both transports.

The `/RVMT` fatal addendum also passed both host-loss timings on both transports.
For each idle-next-call run, terminating the accepted host and typing again
produced one exact `RemotingTest_RvmHost disconnected.` mask, one matching page
error, and Core exit code 3 without a UI hang. For each delivery-
acknowledgement-loss run, the host was terminated under CDB at the transport's
exact pre-replacement-poll function (`HttpClient::BeginReadingLoopUnsafe` for
`/Http` and `SocketHttpClient::Impl::SubmitReceivePoll` for `/MiniHttp`). The
server deadline released the blocked caller, produced the same single exact
error, and terminated Core with exit code 3. No product-code correction was
needed after the rebuilt binaries were exercised.
