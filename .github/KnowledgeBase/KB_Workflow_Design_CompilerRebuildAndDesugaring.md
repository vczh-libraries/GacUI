# Workflow Compiler Rebuild And Desugaring

## How the Workflow analyzer turns parsed modules and high-level Workflow constructs into ordinary AST for bytecode and C++ generation.

This document describes the maintenance-facing design of the Workflow compiler frontend. It focuses on `WfLexicalScopeManager::Rebuild` in `Source/Analyzer/WfAnalyzer.cpp`, the analysis passes it coordinates, and how high-level syntax such as `bind`, co-provider operators, raw coroutines, and state machines is lowered before the emitters run.

The central invariant is that special syntax is a frontend concern. Bytecode generation in `Source/Emitter` and C++ generation in `Source/Cpp` consume ordinary Workflow AST, or virtual nodes whose `expanded*` field points to ordinary Workflow AST. Raw high-level nodes should not be treated as backend concepts.

## Source Map

The compiler frontend is spread across these files:

- `Source/Analyzer/WfAnalyzer.cpp`: owns `WfLexicalScopeManager`, `Clear`, `Rebuild`, and shared manager state.
- `Source/Analyzer/WfAnalyzer_ContextFreeDesugar.cpp`: performs rewrites that do not need type or scope information.
- `Source/Analyzer/WfAnalyzer_ValidateStructure_*.cpp`: checks syntax placement and structural legality before name/type analysis.
- `Source/Analyzer/WfAnalyzer_BuildGlobalNameFromModules.cpp`: creates global names, reflected custom types, and class/type member placeholders.
- `Source/Analyzer/WfAnalyzer_BuildScope.cpp`: creates lexical scopes, symbols, lambda captures, observe scopes, coroutine scopes, and state-machine scopes.
- `Source/Analyzer/WfAnalyzer_CompleteScope.cpp`: fills symbol and member type information after all global names and scopes exist.
- `Source/Analyzer/WfAnalyzer_CheckScopes_*.cpp`: checks duplicate symbols, missing symbol types, and declaration dependency cycles.
- `Source/Analyzer/WfAnalyzer_ValidateSemantic_*.cpp`: resolves expressions, overloads, captures, interface implementations, and context-sensitive expansion.
- `Source/Analyzer/WfAnalyzer_ExpandBindExpression.cpp`: lowers `bind`.
- `Source/Analyzer/WfAnalyzer_ExpandStatement.cpp`: lowers `switch`, `foreach`, co-provider statements, and co-operator statements.
- `Source/Analyzer/WfAnalyzer_ExpandNewCoroutineExpression.cpp`: lowers `$coroutine{}` to `ICoroutine`.
- `Source/Analyzer/WfAnalyzer_ExpandStateMachine.cpp`: lowers state-machine declarations and state-machine-only statements.
- `Source/Emitter/*` and `Source/Cpp/*`: consume expanded AST for bytecode and C++ output.

Parser syntax and generated AST definitions are in:

- `Source/Parser/Syntax/Syntax/Expressions.txt`.
- `Source/Parser/Syntax/Syntax/Statements.txt`.
- `Source/Parser/Syntax/Syntax/Decls.txt`.
- `Source/Parser/Syntax/Lexer.txt`.
- `Source/Parser/Generated/WorkflowAst.h`.

Generated sample outputs under `Test/Generated/Workflow32` and `Test/Generated/Workflow64` are useful for checking the printed shape of expansions. They are generated files and should not be edited directly.

## Rebuild Pipeline

`WfLexicalScopeManager::Rebuild` is a barriered compiler pipeline. It captures `errorCount = errors.Count()` after initialization and uses `EXIT_IF_ERRORS_EXIST` after major phases. Because `errorCount` is not reset between phases, any error added by a phase prevents later phases from running. A validation added to an early phase must not depend on state that is only built by a later phase.

### Environment Load And Reset

The first phase:

- calls `IWfCompilerCallback::OnLoadEnvironment`;
- calls `Clear(keepTypeDescriptorNames, false)`;
- creates `globalName` if needed;
- loads reflected type names with `BuildGlobalNameFromTypeDescriptors`;
- calls `IWfCompilerCallback::OnInitialize`.

`Clear` resets more than modules and errors. It clears semantic side tables such as `expressionResolvings`, coroutine resolution maps, `lambdaCaptures`, `interfaceMethodImpls`, declaration metadata maps, and state-machine maps. Any new analysis cache maintained by `WfLexicalScopeManager` should be cleared here and considered when generated AST is revalidated.

### Context-Free Desugaring And Structure Validation

For each module, `Rebuild` calls:

- `ContextFreeModuleDesugar(this, module)`;
- `ValidateModuleStructure(this, module)`.

Context-free desugaring rewrites syntax that can be transformed without resolved types or scopes. Existing examples include format expressions, auto properties, and generated helper interfaces used by coroutine result conversion.

Structure validation checks placement rules that do not require semantic information. Examples include:

- no nested `bind`;
- no `observe` outside `bind`;
- no explicit `attach` or `detach` inside `bind`;
- no `co-pause` outside `$coroutine{}`;
- no co-operator outside a co-provider statement;
- no state-machine switch or invoke outside a state body;
- state-machine declarations only as class members.

### Global Names And Initial Scopes

The next phase creates the name and initial scope model:

- `BuildGlobalNameFromModules(this)` adds namespaces, Workflow-defined type descriptors, and class/type member placeholders.
- `BuildScopeForModule(this, module)` builds lexical scopes and symbols.
- `ValidateScopeName(this, globalName)` validates the global-name tree.
- `CheckScopes_DuplicatedSymbol(this)` reports duplicate symbols after scopes exist.

This phase is intentionally before type completion. It gives the compiler a complete declaration graph before signatures and member types are fully filled.

### Scope Completion And Scope Checks

The next phase calls:

- `CompleteScopeForModule(this, module)`;
- `CheckScopes_SymbolType(this)`;
- `CheckScopes_CycleDependency(this)`.

`CompleteScopeForModule` fills type information that could not be known until all names and scopes were available. State machines rely heavily on this phase because generated input methods, generated cache fields, and the generated `<state>CreateCoroutine` method all need completed signatures and field types.

### Semantic Validation And Type-Dependent Expansion

For each module, `Rebuild` then calls:

- `IWfCompilerCallback::OnValidateModule(module)`;
- `ValidateModuleSemantic(this, module)`.

This phase resolves expression types into `expressionResolvings`, selects overloads, records lambda captures, records interface method implementations, validates type-dependent rules, and expands context-sensitive virtual AST nodes.

High-level constructs such as `bind`, `$coroutine{}`, co-provider statements, switch/foreach statement virtual nodes, and state machines are deliberately expanded here because they depend on semantic side tables produced by earlier validation.

### Post-Semantic Metadata

After semantic validation:

- `PopulateAttributesOnTypeDescriptors(this)` transfers validated Workflow attributes onto generated type descriptors.
- `ValidateModuleRPC(this, module)` runs when `validateRpc` is true and can populate `rpcMetadata`.

The compiler callback is a progress hook, not part of compiler correctness. Logging and external diagnostics can be implemented differently by different callers.

## Virtual AST Expansion Model

Workflow has two important virtual-node families:

- `WfVirtualCfe*`: context-free virtual nodes expanded before scope construction.
- `WfVirtualCse*`: context-sensitive virtual nodes expanded during semantic validation.

The common context-sensitive recheck pattern is:

1. Validate the original virtual node enough to collect semantic data.
2. If no new errors were added, create `expandedExpression`, `expandedStatement`, or `expandedDeclarations`.
3. Attach source code ranges from the original node to generated nodes for diagnostics.
4. Run context-free desugaring on the generated subtree.
5. Build scopes for the generated subtree under the original parent scope.
6. Remove stale duplicate-symbol and symbol-type check records for the affected parent scope.
7. Re-run `CheckScopes_DuplicatedSymbol` and `CheckScopes_SymbolType`.
8. Semantically validate the generated subtree.

This pattern is visible in `ValidateSemantic_Expression.cpp`, `ValidateSemantic_Statement.cpp`, and `ValidateSemantic_Declaration.cpp`. It is the main reason generated code can use the same language features as user-written Workflow code: generated AST goes back through the ordinary compiler checks before backend emission.

The emitters preserve the invariant:

- `WfEmitter_Expression.cpp` and `WfCpp_Expression.cpp` dispatch virtual expressions through `expandedExpression`.
- Statement and declaration emitters similarly use expanded statements and declarations.
- Raw coroutine and state-machine-only statements reaching backend generation are internal errors.

## Bind Lowering

`bind` is parsed as `WfBindExpression`, a `WfVirtualCseExpression` with an original `expression` and generated `expandedExpression`. `observe` syntax is parsed as `WfObserveExpression`, with `WfObserveType::SimpleObserve` for `parent.observe(...)` and `WfObserveType::ExtendedObserve` for `parent.observe as name(...)`.

### Validation And Scope Preparation

Before expansion:

- `ValidateStructureExpressionVisitor::Visit(WfBindExpression*)` rejects nested `bind`.
- `ValidateStructureExpressionVisitor::Visit(WfObserveExpression*)` requires `observe` to appear under `bind`, rejects nested observe in observe-event expressions, checks simple observe shapes, and requires extended observe to have at least one event.
- `ValidateStructureExpressionVisitor::Visit(WfAttachEventExpression*)` and `Visit(WfDetachEventExpression*)` reject manual attach/detach inside `bind`.
- `BuildScopeForExpression::Visit(WfObserveExpression*)` creates a local scope for extended observe and adds the alias symbol.
- `ValidateSemanticExpressionVisitor::Visit(WfBindExpression*)` validates the bound expression and returns readonly `Ptr<IValueSubscription>`.

The runtime interface is `IValueSubscription` in `Import/VlppReflection.h`. It exposes `ValueChanged`, `Open`, `Update`, and `Close`.

### BindContext

`ExpandBindExpression` in `WfAnalyzer_ExpandBindExpression.cpp` begins by running `CreateBindContextVisitor` over the already-resolved original expression. The visitor builds `BindContext`, which contains:

- `observeParents`: observed expression to parent expression that must be cached.
- `observeEvents`: observed expression to reflected events that must be attached.
- `orderedObserves`: observes in discovery order.
- `cachedExprs`: parent expressions that become generated `<bind-cache>N` fields.
- `renames`: reference expression to expression substituted during dependency discovery and expression rewrite.
- `exprAffects` and `exprCauses`: expression-level dependency graph.
- `observeAffects` and `observeCauses`: observe-to-observe dependency graph.

`CreateBindContextVisitor::Visit(WfMemberExpression*)` detects implicit observable property access by checking `manager->expressionResolvings[node].propertyInfo`. If the property has `GetValueChangedEvent()` or an event named `<PropertyName>Changed`, the member expression becomes an observed expression and its parent is cached.

`CreateBindContextVisitor::Visit(WfObserveExpression*)` records explicit observes, caches their parent, visits the observed value expression, and records each resolved event expression.

`renames` is an expression substitution mechanism, not closure capture. It resolves references to `let` variables and extended-observe aliases back to their defining expressions, so dependency discovery can see through source-local names.

### Dependency Propagation

`CreateBindContextVisitor::DirectDepend(expr, depended)` propagates all observes that can change `depended` into the causes of `expr`.

`CreateBindContextVisitor::ObservableDepend(expr, parent)` records the observe, records its parent, ensures the parent is cached, and builds observe-to-observe edges when the parent itself depends on earlier observes.

The resulting graph drives efficient callback generation. When one event fires, the generated callback can compute the transitive downstream observes that depend on the changed observe, detach only those affected handlers, recompute only the affected cached parents, reattach handlers, and then call the activator.

### Generated Subscription

`ExpandBindExpression` replaces the bind expression with a generated `WfNewInterfaceExpression` typed as `Ptr<IValueSubscription>`.

The generated object contains:

- `var <bind-cache>N : ParentType = default;` for each cached parent expression.
- `var <bind-handler>I_J : EventHandler^ = null;` for each observed event.
- `var <bind-opened> : bool = false;`.
- `var <bind-closed> : bool = false;`.
- `func <bind-activator>()`.
- `func <bind-callback>I_J(...)` for each observed event.
- overrides for `Open`, `Update`, and `Close`.

`CreateWritableVariable` and `CreateDefaultValue` construct generated fields and initial values. `CreateDefaultValue` is also used by coroutine and state-machine lowering, so generated reset behavior is shared.

### Expression And Control-Flow Rewrite

`ExpandObserveExpressionVisitor` rewrites the original expression:

- Cached expressions become references to `<bind-cache>N`.
- `let` variables whose values are cached are removed from the generated `let`.
- Simple observe becomes member access on the rewritten parent.
- Extended observe becomes the rewritten inner expression.

The activator evaluates the rewritten original expression into `<bind-activator-result>` and raises `ValueChanged(<bind-activator-result>)`.

`CreateBindCacheAssignStatement` emits cache assignment using `WfBinaryOperator::FailedThen`, conceptually:

```workflow
<bind-cache>N = expanded-parent-expression ?? default(parent-type);
```

This keeps subscription maintenance resilient to failed parent evaluation by resetting the cache to the default type value.

`CreateBindOpenFunction` generates:

1. set `<bind-opened>` if it was false;
2. assign caches in dependency order;
3. attach all observed event handlers;
4. return true, or return false if already opened.

`CreateBindUpdateFunction` calls `<bind-activator>()` only when opened and not closed.

`CreateBindCloseFunction` sets `<bind-closed>`, detaches non-null handlers, resets caches and handler fields to defaults, and returns whether it actually closed.

Each generated callback:

1. starts from the observe whose event fired;
2. walks `observeAffects` to find affected downstream observes;
3. detaches affected downstream handlers in reverse order;
4. recomputes each affected parent cache once;
5. reattaches affected handlers;
6. calls `<bind-activator>()`.

`Test/Generated/Workflow64/Parsing.Codegen.BindLet.txt` demonstrates this printed shape: callbacks detach handlers under old cached child objects, recompute child caches such as `<bind-cache>5`, reattach, and invoke the activator.

### Variables And Captures

Bind has three variable mechanisms:

- observed parent expressions become generated cache fields;
- `let` variables and extended-observe aliases are handled through `BindContext::renames` and may disappear from the generated expression if their value is cached;
- outer function variables are captured by ordinary anonymous-interface capture machinery.

`BuildScopeForExpression::Visit(WfNewInterfaceExpression*)` creates a `WfLexicalCapture` for the generated interface implementation. During semantic resolution, references from generated subscription methods to outer function locals are recorded in `manager->lambdaCaptures`. Bytecode metadata names those as `<captured>x`, while bind-owned state uses names such as `<bind-cache>0` and `<bind-handler>0_0`.

`ExpandBindExpression` therefore does not convert surrounding locals into bind-specific fields. It relies on the same closure model used by user-written `new interface` expressions.

## Co-Provider To Raw Coroutine To Trivial Workflow

Coroutine-related syntax is lowered in two distinct layers:

1. Co-provider syntax is lowered to raw coroutine syntax. `WfCoProviderStatement` is rewritten to a provider `Create` or `CreateAndRun` call that receives an anonymous function returning `WfNewCoroutineExpression`.
2. Raw coroutine syntax is lowered to trivial Workflow code. `WfNewCoroutineExpression` is compiled into a generated `WfNewInterfaceExpression` implementing `ICoroutine`, with explicit fields, state integers, dispatch loops, and ordinary statements.

This layering is important for maintenance. Co-provider lowering does not directly build a coroutine flow chart. It generates a raw coroutine body containing `WfCoPauseStatement`, provider method calls, result checks, and normal statements. Then the raw coroutine expander owns all control-flow splitting, variable persistence, pause/resume handling, exception routing, and final `ICoroutine` object generation.

### Parsed Forms

The compiler does not have a hardcoded `$yield` node. `Source/Parser/Syntax/Lexer.txt` defines `COROUTINE_OPERATOR` as `$` followed by an uppercase name. Operators such as `$Yield`, `$Await`, and `$Join` all use the same internal shape.

The parser represents coroutine-related syntax as:

- `WfCoProviderStatement` for provider bodies such as `$ { ... }` or `$Async { ... }`.
- `WfCoOperatorStatement` for statement operators such as `$Yield i;`.
- `WfCoOperatorStatement` with `varName` for result-producing operators such as `var x = $Await async;`.
- `WfCoOperatorExpression` for context expressions such as `$.Context`.
- `WfNewCoroutineExpression` for raw `$coroutine{}`.

`WfCoProviderStatement`, `WfCoOperatorStatement`, and `WfCoOperatorExpression` are the high-level provider layer. `WfNewCoroutineExpression` is the raw coroutine layer.

### Semantic Resolution Before Lowering

`BuildScopeForStatement::Visit(WfCoProviderStatement*)` creates provider-scope symbols:

- `$PROVIDER`: selected provider type.
- `$IMPL`: provider implementation type passed to the generated coroutine creator.

`ValidateSemanticStatementVisitor::Visit(WfCoProviderStatement*)` resolves the provider:

- unnamed `$ { ... }` starts from the containing function return type and searches for `<ReturnTypeDescriptorName>Coroutine`, including base type descriptors;
- named `$Async { ... }` strips `$`, resolves `Async`, then searches for `AsyncCoroutine` or accepts `AsyncCoroutine` directly;
- void-like provider functions use static `CreateAndRun`;
- value-returning provider functions use static `Create`;
- the selected creator must accept `Ptr<IValueFunctionProxy<Ptr<ICoroutine>, ImplType>>`;
- the selected method is saved in `manager->coProviderResolvings`.

`ValidateSemanticStatementVisitor::Visit(WfCoOperatorStatement*)` resolves each operator against `$PROVIDER` and `$IMPL`:

- `$Yield` becomes operator name `Yield`;
- the provider is searched for static `YieldAndRead`;
- if there is no result variable, static `YieldAndPause` is also considered;
- candidates must take `$IMPL` as the first parameter;
- overload selection runs with a dummy `$IMPL` first argument followed by source arguments;
- the selected provider method is saved in `manager->coOperatorResolvings`.

For result-producing operators such as `var x = $Await async;`, semantic validation also finds a static `CastResult(Value)` helper from argument types. The helper's return type becomes the lexical type of `x`, and the helper is saved in `manager->coCastResultResolvings`.

Return statements inside provider bodies are provider-specific. `ValidateSemanticStatementVisitor::Visit(WfReturnStatement*)` resolves provider static `ReturnAndExit`, requiring `$IMPL` as the first parameter, and saves the method in `manager->coOperatorResolvings`.

`WfCoOperatorExpression`, the expression form used for `$.Context`, cannot be expanded at the outer provider level because it needs the generated `<co-impl>` variable. Its expansion is delayed until the generated raw coroutine body is being copied and validated under the scope containing `<co-impl>`.

### Layer 1: Co-Provider To Raw Coroutine

`ExpandCoProviderStatement` in `WfAnalyzer_ExpandStatement.cpp` rewrites `WfCoProviderStatement` into ordinary AST that calls the provider creator with a generated function. Conceptually, the generated shape is:

```workflow
Provider::Create-or-CreateAndRun(
    func(<co-impl> : ImplType) : Coroutine^
    {
        return $coroutine(<co-result>) { rewritten provider body };
    }
);
```

Internally this first layer is represented by:

- a `WfFunctionExpression`;
- an anonymous `WfFunctionDeclaration`;
- a generated `WfFunctionArgument` named `<co-impl>`, typed as the resolved `$IMPL`;
- a `WfNewCoroutineExpression` named `<co-result>`;
- a `WfCallExpression` to the selected provider `Create` or `CreateAndRun` method;
- a `WfBlockStatement` stored in `WfCoProviderStatement::expandedStatement`.

The important result is that the high-level provider statement has become a normal provider method call whose callback returns a raw coroutine. From this point on, the provider body is treated as a coroutine body.

`ExpandCoProviderStatementVisitor` copies the source provider body into the generated `WfNewCoroutineExpression` and rewrites provider-only statements during the copy.

For `$Yield i;`, the rewritten raw coroutine body contains:

- a `WfCoPauseStatement`;
- an inner pause block containing a `WfCallExpression` to the resolved provider method, such as `EnumerableCoroutine::YieldAndPause(<co-impl>, i)`;
- ordinary post-pause checks against `<co-result>`.

Conceptually:

```workflow
$pause
{
    EnumerableCoroutine::YieldAndPause(<co-impl>, i);
}
if (<co-result> is not null)
{
    if (<co-result>.Failure is not null) raise <co-result>.Failure;
}
```

For `var x = $Await async;`, the raw coroutine body is also a pause around the resolved provider call. After resume, it checks `<co-result>.Failure`, reads `<co-result>.Result`, converts it through the resolved `CastResult(Value)` method, and emits a normal `WfVariableStatement` for `x`.

For `return expr;`, the provider body is rewritten to call `Provider::ReturnAndExit(<co-impl>, ...)` and then emit a final `return`. If the provider result type exposes `StoreResult`, the source return expression is wrapped by that helper before being passed to `ReturnAndExit`.

After this first layer, the remaining provider-specific concepts are expressed through ordinary calls and through `WfCoPauseStatement`. The compiler no longer needs a special co-provider control-flow model.

### Layer 2: Raw Coroutine To Flow Chart

`ExpandNewCoroutineExpression` in `WfAnalyzer_ExpandNewCoroutineExpression.cpp` handles raw `WfNewCoroutineExpression`, regardless of whether it came from user-written `$coroutine{}` or from `ExpandCoProviderStatement`.

The expander builds a flow-chart model:

- `FlowChart` owns all nodes, `headNode`, `lastNode`, and temporary exception variables.
- `FlowChartNode` holds copied statements, conditional branches, default destination, exception destination, pause destination, action, exception variable, and an `embedInBranch` optimization flag.
- `FlowChartBranch` holds an optional condition and a destination.
- `GenerateFlowChartStatementVisitor::ScopeContext` models structured exits for functions, loops, labeled blocks, and try/finally.

The flow-chart pipeline is:

1. `FindCoroutineAwaredStatements` marks statements that affect suspension or control-flow splitting: virtual statements, coroutine statements, `break`, `continue`, `return`, `goto`, `if`, `while`, `try`, and blocks containing aware statements.
2. `FindCoroutineAwaredVariables` finds local variables inside aware regions.
3. `FindCoroutineReferenceRenaming` assigns generated field names for locals and temporary control variables that must survive.
4. `GenerateFlowChart` copies statements into flow-chart nodes and rewrites references to renamed symbols.
5. `RemoveUnnecessaryNodes` merges trivial nodes, removes the default destination after pause sentinels, and marks simple branch targets for embedding.
6. `ExpandNewCoroutineExpression` emits a generated `WfNewInterfaceExpression` implementing `ICoroutine`.

This second layer is where raw coroutine code stops being structured source code and becomes explicit resumable control flow.

### Variable Conversion In Raw Coroutine Lowering

Coroutine variable conversion is symbol-based, not text-based. The renaming map is keyed by `WfLexicalSymbol*`, avoiding collisions when generated or source variables reuse textual names in nested scopes.

Naming rules include:

- source local `x` can become `<coN>x`;
- generated temporaries such as `<for-begin>i` can become `<coN-for-begin>i`;
- catch variables, `if` expression variables, and try/finally temporary exception variables can also be renamed.

When a renamed `WfVariableStatement` appears in an aware region, `GenerateFlowChartStatementVisitor::Visit(WfVariableStatement*)` replaces the declaration with an assignment to the generated field. `ExpandNewCoroutineExpression` then emits one generated field declaration for each renamed symbol, using the original symbol type and `CreateDefaultValue`.

Variables not in the renaming map remain ordinary locals. Outer function variables are handled by normal function/lambda capture analysis around the generated function expression, not by coroutine field conversion.

For a co-provider body, this means variables introduced by source code and variables introduced by co-operator rewriting both participate in the same raw coroutine persistence algorithm. For example, a variable produced by `var x = $Await ...` is initially emitted as a normal `WfVariableStatement` after the pause. If later coroutine states need `x`, `FindCoroutineReferenceRenaming` can promote it to a generated coroutine field.

### Pause Conversion In Raw Coroutine Lowering

`GenerateFlowChartStatementVisitor::Visit(WfCoPauseStatement*)` turns a raw coroutine pause into three flow-chart nodes:

1. a node with `action = SetPause`;
2. a sentinel node containing an empty `WfCoPauseStatement`;
3. a resume-continuation node stored in `pauseDestination`.

If the pause has an inner statement, such as `EnumerableCoroutine::YieldAndPause(<co-impl>, i)`, the inner statement is copied into the `SetPause` node before the sentinel pause.

`ExpandFlowChartNode` converts a `SetPause` node into ordinary code that:

1. sets coroutine status to waiting;
2. records `<co-state-before-pause>`;
3. sets `<co-state>` to the state after the pause;
4. runs the provider operation or other inner pause statements;
5. returns from `Resume`.

On the next `Resume`, the generated coroutine:

- verifies it is waiting;
- sets status to executing;
- examines the resume input when `<co-state-before-pause> != -1`;
- clears the pre-pause state if there is no failure;
- routes failure to a catch node or raises it;
- dispatches through a generated loop using `<co-state>`.

In a co-provider-generated coroutine, the resume input is named `<co-result>`. That is why the first-layer co-provider rewrite emits checks against `<co-result>` after every operator pause: once the second layer has split the coroutine, those checks execute only after the provider runtime resumes the coroutine with a `CoroutineResult`.

### Layer 3: Flow Chart To Trivial Workflow Code

The final output of raw coroutine lowering is generated ordinary Workflow AST, not a special backend coroutine instruction.

`ExpandNewCoroutineExpression` emits a generated `WfNewInterfaceExpression` implementing `ICoroutine`. The generated object contains:

- generated fields for renamed locals and temporaries;
- `<co-state>`;
- `<co-state-before-pause>`;
- auto properties for `Failure` and `Status`;
- override `Resume(<raise-exception>, <coroutine-output>)`.

`Resume` is trivial Workflow control flow:

- it uses `if` checks and loops to dispatch on `<co-state>`;
- normal flow-chart edges assign `<co-state>` and continue;
- branch edges emit ordinary `if` statements, sometimes embedding small destination nodes;
- `break`, `continue`, `goto`, and `return` are represented as jumps to generated flow-chart destinations;
- `InlineScopeExitCode` inserts pending `finally` code while leaving scopes;
- nodes with `exceptionDestination` are wrapped in generated try/catch code that assigns the caught exception to a renamed exception field, sets `<co-state>` to the catch node, and continues.

This is the point where both user-written raw `$coroutine{}` and co-provider-generated raw coroutine bodies become the same kind of generated trivial Workflow code. Bytecode and C++ emitters then see ordinary interface implementation, fields, methods, properties, statements, and expressions through `expandedExpression` or `expandedStatement`.

## State Machine Lowering

State machines are context-sensitive declarations. The parser represents them as `WfStateMachineDeclaration`, with:

- `WfStateInput` for `$state_input`;
- `WfStateDeclaration` for `$state`;
- `WfStateSwitchStatement` for `$switch`;
- `WfStateInvokeStatement` for `$goto_state` and `$push_state`.

### Preparation Before Expansion

State machines participate in several `Rebuild` phases before `ExpandStateMachine` runs:

- `BuildGlobalNameFromModules::BuildClassMemberVisitor::Visit(WfStateMachineDeclaration*)` synthesizes class member placeholders for input methods, input argument fields, state argument fields, and `<state>CreateCoroutine`.
- `BuildScopeForDeclarationVisitor::Visit(WfStateMachineDeclaration*)` creates lexical symbols for state inputs, states, state arguments, switch-case arguments, and state bodies.
- `CompleteScopeForClassMemberVisitor::Visit(WfStateMachineDeclaration*)` fills input method signatures, generated field types, and `<state>CreateCoroutine(<state>startState:int):void`.
- `ValidateSemanticDeclarationVisitor::Visit(WfClassDeclaration*)` requires a class containing a state machine to inherit from `system::StateMachine`.
- `ValidateSemanticDeclarationVisitor::Visit(WfStateMachineDeclaration*)` requires a default state and validates each state body.
- `ValidateSemanticStatementVisitor::Visit(WfStateSwitchStatement*)` resolves switch cases to generated input methods and assigns case argument symbol types from input method parameters.
- `ValidateSemanticStatementVisitor::Visit(WfStateInvokeStatement*)` resolves target states and validates invocation arguments against target state argument types.

`WfStateMachineInfo` stores `createCoroutineMethod`, `inputIds`, and `stateIds`. `inputIds` and `stateIds` are filled by `ExpandStateMachine`.

### Generated Class Members

`ExpandStateMachine` turns a state-machine declaration into ordinary class members:

- private fields named like `<stateip-INPUT>ARG` for input arguments;
- private fields named like `<statesp-STATE>ARG` for state arguments;
- generated input methods;
- generated `<state>CreateCoroutine`.

Generated fields are added to `WfStateMachineDeclaration::expandedDeclarations` as `WfVariableDeclaration` nodes with `@cpp:Private`, and connected back to reflected field metadata through `manager->declarationMemberInfos`.

This is different from coroutine local conversion. Raw coroutine lowering hoists locals only when the flow chart proves they must survive a pause. State-machine lowering creates class fields up front for semantic state because input arguments and state invocation arguments must survive across input method calls, state transitions, and pushed child states.

### Generated Input Methods

For each `$state_input`, `ExpandStateMachine` emits a normal class method using the existing `WfClassMethod` from `stateInputMethods`.

Each input method:

1. initializes the state machine on first call by setting `stateMachineInitialized`, creating the default-state coroutine with `<state>CreateCoroutine(0)`, and calling `ResumeStateMachine`;
2. stores the input id in `stateMachineInput`;
3. copies method arguments into `<stateip-INPUT>ARG` fields;
4. calls `ResumeStateMachine` again to deliver the input.

The generated method is ordinary AST: `WfFunctionDeclaration`, `WfIfStatement`, `WfBinaryExpression`, `WfMemberExpression`, `WfCallExpression`, and `WfExpressionStatement`.

### Generated State Coroutine

`ExpandStateMachine` emits:

```workflow
func <state>CreateCoroutine(<state>startState : int) : void
```

The method creates a `WfNewCoroutineExpression` and assigns it to `stateMachineCoroutine`. It saves the previous coroutine in `<state>previousCoroutine`, creates a coroutine using `<state>stateMachineObject = this`, and restores the previous coroutine in a `finally` block.

The generated coroutine body contains:

- a protected block labeled `<state-label>OUT_OF_STATE_MACHINE`;
- a `finally` block restoring `stateMachineCoroutine`;
- local `<state>state = <state>startState`;
- `while(true)` labeled with `<state-label>OUT_OF_CURRENT_STATE`;
- local `<state>currentState = <state>state`;
- assignment `<state>state = -1`;
- a `WfSwitchStatement` over `<state>currentState`;
- one switch case for each declared state.

Each state case copies `<statesp-STATE>ARG` fields into local state arguments, copies the original state body through `ExpandStateMachineStatementVisitor`, and emits a goto to leave the state machine as default fall-through.

This generated code intentionally uses the same ordinary control-flow constructs understood by coroutine lowering: labels, gotos, loops, switches, try/finally, and `WfCoPauseStatement`.

### State Switch Lowering

`ExpandStateMachineStatementVisitor::Visit(WfStateSwitchStatement*)` rewrites `$switch` into a block containing:

- an `if` that pauses when `stateMachineInput == -1`;
- a `WfSwitchStatement` over `stateMachineInput`.

For each explicit case:

- the case expression is the numeric id from `smInfo->inputIds`;
- the generated case clears `stateMachineInput` to `-1`;
- input arguments are copied from `<stateip-INPUT>ARG` fields into local case variables;
- the original case body is copied.

If the switch type is `WfStateSwitchType::Default`, the expander also creates raising cases for every input not explicitly listed.

Default branch behavior depends on `WfStateSwitchType`:

- `Default`: no default branch; missing input cases were generated as raising cases.
- `Pass`: no default branch; unhandled input remains available.
- `PassAndReturn`: `goto <state-label>OUT_OF_STATE_MACHINE`.
- `Ignore`: clear `stateMachineInput`.
- `IgnoreAndReturn`: clear `stateMachineInput`, then leave the state machine.

The generated `WfSwitchStatement` is itself another context-sensitive virtual statement and is later expanded by the normal switch expander.

### State Invoke Lowering

`ExpandStateMachineStatementVisitor::Visit(WfStateInvokeStatement*)` handles `Goto` and `Push`.

Both forms first copy invocation arguments into generated state argument fields:

```workflow
<state>stateMachineObject.<statesp-STATE>ARG = copiedArgument;
```

`WfStateInvokeType::Goto` then assigns the target state id to `<state>state` and jumps to `<state-label>OUT_OF_CURRENT_STATE`. This is a domain-state transition inside the same generated coroutine.

`WfStateInvokeType::Push` calls `<state>CreateCoroutine(TargetStateId)` and emits `WfCoPauseStatement`. This replaces `stateMachineCoroutine` with a child coroutine and pauses the current coroutine. The stack-like behavior is encoded by the generated `previousCoroutine` local plus the `finally` block that restores `stateMachineCoroutine`.

`system::StateMachine::ResumeStateMachine` in `Source/Library/WfLibraryPredefined.cpp` drives this protocol. It detects whether a resume pushed a child coroutine, whether a child finished and restored the parent coroutine, and whether a child failure should be delivered back to the parent as a `CoroutineResult`.

### Shared And Different From Coroutine Lowering

State-machine lowering shares these constructions with raw coroutine lowering:

- it generates `WfNewCoroutineExpression`;
- it generates `WfCoPauseStatement`;
- it uses ordinary Workflow control-flow AST;
- it uses `CreateDefaultValue`;
- it relies on the same virtual-node recheck pipeline;
- it ultimately produces an `ICoroutine` implementation with `<co-state>`, `<co-state-before-pause>`, `Failure`, `Status`, and `Resume`;
- locals in the generated coroutine body can be renamed into coroutine fields by `FindCoroutineReferenceRenaming`.

The important differences are:

- raw coroutine lowering starts from a statement body and directly builds a `FlowChart`; state-machine lowering starts from a class declaration and first generates fields, methods, and a generated coroutine body;
- raw coroutine state ids are implementation states in `FlowChartNode` order; state-machine ids in `smInfo->stateIds` are domain states, and coroutine lowering later adds another implementation-state layer;
- raw coroutine persistence is discovered by `FindCoroutineAwaredVariables`; state-machine input and state arguments are stored in generated class fields before coroutine lowering;
- raw coroutine resumption receives `CoroutineResult` directly from a `Resume` caller; state-machine resumption goes through generated input methods and `StateMachine::ResumeStateMachine`;
- state-machine switch policies have no raw coroutine equivalent.

## Backend Contract And Maintenance Rules

The backend contract is simple: generated code must be ordinary Workflow AST by the time bytecode or C++ generation needs it.

For maintenance work:

- decide whether new syntax is context-free or context-sensitive;
- context-free syntax can expand before scopes;
- context-sensitive syntax must validate original nodes first, generate `expanded*`, and re-run desugar, scope building, scope checks, and semantic validation on generated AST;
- generated AST must be compatible with `BuildScope`, `CompleteScope`, semantic validation, bytecode emission, and C++ emission;
- new semantic side tables must be cleared by `WfLexicalScopeManager::Clear`;
- generated nodes should keep source code ranges from the construct that caused them;
- logging and callbacks should not be required for compiler correctness;
- raw coroutine and state-machine statements should remain frontend-only.

The generated samples under `Test/Generated/Workflow32` and `Test/Generated/Workflow64` are useful for auditing the printed desugared shape. For example, bind samples show generated subscriptions, co-provider samples show generated coroutine state fields, and state-machine samples show generated input methods plus coroutine-backed state loops.
