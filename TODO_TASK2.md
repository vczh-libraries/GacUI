You are required to build `GacJS` to take some esbuilt javascript files directly to `WebsiteSource`.
There are critical js files:
- `index.js`: currently have everything, you are going to refactor a little bit:
  - Rename it to `gacui.js`, meanwhile exlude every `website/*` packages from it.
- `wasm.js`: esbuilt from `remote-protocol-wasm`.
- `http.js`: esbuilt from `remote-protocol-http`.
- `rvm.js`: esbuilt from `rvm`.
- `rvmhost` is a cli application, no esbuilt is needed.

Do the refactoring first, make sure `GacJS` website's entry and 3 wasm files are working, following `GacUI/.github/Jobs/DebugRemoteProtocolWithGacJS.md`, check out details but no need to follow `job.rp(Wasm|XPlat).prompt.md` just to know how to setup.
commit and push all local changes before continuing.

Now the refactoring is ready, `gacui.js` and `wasm.js` could just be brought to `WebsiteSource` directly.
In the `website` package, the `index.html` is manually written. When switching to the `Web` page, there are 3 vertical tabs, add a new `Start WASM Now!` tab open `/wasm-fct/index.html`.
There should be a `WebsiteSource/packages/website/scripts/Copy-WASM.sh` doing:
- Exit when `GacJS` and `GacUI` does not exist.
- Build `GacJS`.
- Verify if `GacUI/Test/Linux/WasmFCT/Bin/app.(wasm|mjs|worker.js)` exists:
  - If yes, do `vbuild -bw`
  - If no, do `vbuild -fbw`.
- esbuilt scripts above are copied to `website/lib/dist/wasm-fct` folder.
- `GacUI/Test/Linux/WasmFCT/Bin/app.(wasm|mjs|worker.js)` are copied to `website/lib/dist/wasm-fct`
And verify if `wasm-fct/index.html` is working correctly:
- Such index.html should be part of `WebsiteSource`, you can copy it from `GacJS` but it needs modifications, and keep this version in `WebsiteSource` separatedly from `GacJS` forever:
  - No need the thirt restart button.
commit and push all local changes before continuing.

And the publish to `vczh-libraries.github.io`.
- Update `WebsiteSource/job.publish.prompt.md`:
  - `Copy-WASM.sh` must be called before publishing.
  - Unless explicitly requested, existing `wasm-fct` folder `vczh-libraries.github.io` should not be replaced, because most of the time binary changing doesn't affect anything, keep them unchanged.
    - But this time there is no `wasm-fct`, so copying is needed.
commit and push all local changes before continuing.
