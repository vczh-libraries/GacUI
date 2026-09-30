The task is about starting GacUI in GacJS with wasm.

The first step is to implement a remote protocol communication layer for wasm.
Files should be put in `Test/RemotingHelpers/RemotingServer/Wasm`.
All files there should be in `#if defined VCZH_WASM`.
Currently only WASM platform is using it, so no need to add them to any vcxproj.
In this folder you are going to maintain one or more pair of C++ source files, exposing some text exchanging functions to JavaScript, their signatures look like this in TypeScript
```TypeScript
function SendDataToWasmCore(data: string):void;
function StartApplication(receiver: function(data:string):void):void;
```
Renderer starts core in a worker thread using `StartApplication` and install a receiver function.
Renderer talks to core using `SendDataToWasmCore`.
Core talks to renderer using the receiver.
All functions should be in `EMSCRIPTEN_BINDINGS(GacUIWasmApplication)`

Although WASM communication layer is an `INetworkProtocolServer` but it works like having a remote client already installed because the receiver and the `SendDataToWasmCore` is working like a singleton. Global variables will be needed but keep them minimum, e.g., to store the receiver and the singleton `INetworkProtocolServer`. `StartApplication` can just check if the `INetworkProtocolServer` pointer is already there, does nothing and return if yes.

You can change the signature of exposed functions if anything more is needed to implement `INetworkProtocolServer`.

The second step is to implement `Test/Linux/WasmFCT` and `Test/Linux/WasmRPT`:
- The first one launches `FullControlTest`.
- The second one launches `RemoteProtocolTest`.
- The second one launches `RemoteViewModelTest`.
They should be built with web assembly. Although many files will be generated to their `Bin` folder, but everything will be ignored except `app.wasm` and `app.mjs`.

The third step is to use them in `GacJS` repo. In `GacJS/Gaclib/website` we already have an `index.html` talking to a core application live. Now we do:
- `/wasm-fct/index.html`
- `/wasm-rpt/index.html`
- `/wasm-rvmt/index.html`
  - In this web page you need to addicionally implement a view model in TypeScript, you can check out how the original `index.html` is doing.
They will be put to assets, and after building `website` you could find them in `.../entry/lib/dist`.
You need to manually copy those wasm files to this folder say `.../dist/wasm-(fct|rpt|rvmt)/app.(wasm|mjs)`, sitting in the same folder having `index.html`.
**IMPORTANT**: DO NOT copy those files during `npm run build`, instead you should just add a `GacJS/copy-wasm.sh`, assuming `website` and `GacUI` already built, and it just copy those app files to `dist`. If `dist` or `app.*` is missing it warns and stops.
You are going to create a `remote-protocol-wasm` package just like `remote-protocol-http` but they calls exposed functions.
Now after building the project, you should start `entry` and test if those UI is working, you can follow `GacUI/Jobs/DebugRemoteProtocolSop.md` to try every feature.

Currently it only works on Linux, add `GacUI/.github/Jobs/job.rpWasm.prompt.md` to describe how to build and run the test, follow the language in `job.rp(Windows|XPlat).prompt.md`, short, informative, precise.

commit and push all local changes.
