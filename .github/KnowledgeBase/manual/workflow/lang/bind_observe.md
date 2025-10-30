# Observe events explicitly

Sometimes an expression may not have proper observable properties. In this case, you can explicitly specify which events are needed in order to track an expression. Here comes the **observe** expression.

An **observe** expression must be in a **bind** expression. A **bind** expression must not be in an **observe** expression.

## O.observe

**O.observe(Prop)** means **O.observe as o(o.Prop on o.PropChanged)**.

**O.observe(Prop on EventA, EventB, ...)** means **O.observe as o(o.Prop on o.EventA, o.EventB ...)**.

## O.observe as o

For **O.observe as o(EXPR on E1, E2, ...)**: - **O** could be any expression. - **o** is an expression alias of **O**. - **EXPR** is the expression being tracked, you can use **o** in **EXPR**. - **E1**, **E2** ... can be any expression that are referencing events. - There must be at least one event after **on**. - Involved events in **EXPR** are all ignored, only **E1**, **E2** ... will be subscribed.

``` module sampleModule; class Numbers { prop A: int = 0 {} prop B: int = 0 {} func GetC(): int { return A + B; } prop C: int {GetC} } func main(): string { var numbers = new Numbers^(); var sub = bind(numbers.observe as x(x.C on x.AChanged, x.BChanged)); var r = {""}; attach(sub.ValueChanged, func(newValue: object): void { r[0] = $"$(r[0])$(cast int newValue); "; }); sub.Open(); numbers.A = 1; numbers.B = 2; numbers.A = 3; numbers.B = 4; sub.Close(); return r[0]; } ``` The **main** function returns **"1; 3; 5; 7; "**.

In this example, we want to observe **numbers.C**. Unfortunately this property is not associated with an event. But we know that, either **AChanged** or **BChanged** happens means **C** is changed. So we use the **observe** expression to specify it.

## Nested observe expressions

For any **O.observe as o(EXPR on ...)** expression, since **EXPR** could be any expression except **bind**, **EXPR** could contain another **observe** expression. ``` module sampleModule; class Numbers { prop A: int = 0 {} prop B: int = 0 {} func GetC(): int { return A + B; } prop C: int {GetC} } func main(): string { var n1 = new Numbers^(); var n2 = new Numbers^(); var sub = bind( n1.observe as x( x.C + n2.observe as y( y.C on y.AChanged, y.BChanged ) on x.AChanged, x.BChanged ) ); var r = {""}; attach(sub.ValueChanged, func(newValue: object): void { r[0] = $"$(r[0])$(cast int newValue); "; }); sub.Open(); n1.A = 1; n1.B = 2; n2.A = 3; n2.B = 4; sub.Close(); return r[0]; } ``` The **main** function returns **"1; 3; 6; 10 "**.

This example track the expression **n1.C + n2.C**. But the **C** property is not observable, so **n1.observe as x(x.C on x.AChanged, x.BChanged)** and **n2.observe as y(y.C on y.AChanged, y.BChanged)** is necessary.

In order to express **n1.C + n2.C**, one of the following could be used: - **n1.observe as x(x.C on x.AChanged, x.BChanged) + n2.observe as y(y.C on y.AChanged, y.BChanged)** - **n1.observe as x(x.C + n2.observe as y(y.C on y.AChanged, y.BChanged) on x.AChanged, x.BChanged)**

In a **bind** expression, the difference between **O.observe as o(EXPR on ...)** and **let o = O in (EXPR)** is which events are going to be subscribed. Obviously the above two choices are equivalent.

## Cached objects

For any **O.observe as o(EXPR on E1, E2, ...)** expression, **E1**, **E2** ... are not tracked. Changing related properties do not trigger anything, and it may cause **bind** to work improperly. ``` module sampleModule; class Pair { prop A: int = 0 {} prop B: int = 0 {} } class Sum { prop AB: Pair^ = new Pair^() {} func GetC(): int { return AB.A + AB.B; } prop C: int {GetC} } func main(): string { var s = new Sum^(); var sub = bind(s.observe as x(x.C on x.AB.AChanged, x.AB.BChanged)); var r = {""}; attach(sub.ValueChanged, func(newValue: object): void { r[0] = $"$(r[0])$(cast int newValue); "; }); sub.Open(); s.AB = new Pair^(); s.AB.A = 1; s.AB.B = 2; s.AB.A = 3; s.AB.B = 4; sub.Close(); return r[0]; } ``` The **main** function returns **""**.

This example tracks **s.C** by subscribing **s.AB.AChanged** and **s.AB.BChanged**.

After **sub.Open()** is called, **s.AB** is changed. But this subscription does not track **s.AB**, so it is still waiting for **AChanged** and **BChanged** from the old **s.AB**, which causes the callback not being triggered.

In order to track **s.AB**, we could change the **bind** expression to: - **bind(s.AB.observe as x(s.C on x.AChanged, x.BChanged))** Now the **main** function returns **"0; 0; 0; 1; 3; 5; 7; "**.

There are three **"0; "** being printed, which are caused by the changing of **s.AB** and resubscription on events.

Because **s.AB** is not in **observe**, **s.ABChanged** is used to track **s.AB**. Once it is changed, **s.AB.AChanged** and **s.AB.BChanged** will be resubscribed.

And we see that **s.C** does not contain **x**, which is fine.

