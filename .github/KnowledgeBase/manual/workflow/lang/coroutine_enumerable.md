# Enumerable Coroutine

This coroutine provider is **system::EnumerableCoroutine**. - It has a **Create** method, so this coroutine function declaration is **legal**: ``` using system::*; func GetNumbers(): int{} ${ /* legal */ } ``` Note that **int{}** is the generic version of **system::Enumerable^**, so it uses **system::EnumerableCoroutine** as the coroutine provider. - It does not have a **CreateAndRun** method, so this coroutine function declaration is **illegal**: ``` using system::*; func GetNumbers(): void $Enumerable { /* illegal */ } ``` - It has a **YieldAndPause** method, so this statement in the coroutine is **legal**: ``` $Yield EXPR; /* legal */ ``` It does not have a **YieldAndRead** method, so this statement in the coroutine is **illegal**: ``` var NAME = $Yield EXPR; /* illegal */ ``` - It has a **JoinAndPause** method, so this statement in the coroutine is **legal**: ``` $Join EXPR; /* legal */ ``` It does not have a **JoinAndRead** method, so this statement in the coroutine is **illegal**: ``` var NAME = $Join EXPR; /* illegal */ ``` - It has a **ReturnAndExit** method, which has no parameter, so this statement in the coroutine is **legal**: ``` $return; /* legal */ ``` but this statement in the coroutine is **illegal**: ``` $return EXPR; /* illegal */ ```

## Using return and $Yield

**return;** stops a **$Enumerable** coroutine.

**$Yield EXPR;** pauses a **$Enumerable** coroutine, and generates a new number for the returned **system::Enumerable^**.

``` func GetNumbers(): int{} ${ for (i in range [1, 10]) { $Yield i; if (i == 5) { return; } } } ``` This function returns **{1 2 3 4 5}**.

## Using $Join

**$Join EXPR;** works like **$Yield**, but it generates a series of numbers from the given **system::Enumerable^**.

``` func GetNumbers(): int{} ${ for (i in range [1, 3]) { $Join range [i, i + 2]; } } ``` This function returns **{1 2 3 2 3 4 3 4 5}**.

