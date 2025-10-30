# Jumping Between States

Like any function, **$state** is allowed to have zero, one or multiple parameters.

Jump to another state by **$push_state** is similar to calling a function. When that state finishes, the state machine continues from code after **$push_state**.

Jump to another state by **$goto_state** is similar to **goto**. When that state finishes, the state machine doesn't continue from code after **$goto_state**. If there is no stacked **$push_state**, the state machine stops.

``` module sampleModule; using system::*; class MySM : StateMachine { var result: int = 0; event ResultChanged(); func GetResult(): int { return result; } prop Result: int {GetResult : ResultChanged} $state_machine { $state_input Add(number: int); $state_input Sum(); $state ResultGenerated(number: int) { result = number; ResultChanged(); } $state NumberExpected() { var sum = 0; while (true) { $switch(raise) { case Add(number): { sum = sum + number; } case Sum(): { $push_state ResultGenerated(sum); break; } } } $goto_state NumberExpected(); } $state default() { $goto_state NumberExpected(); } } } func main(): string { var r = {""}; var sm = new MySM^(); attach(sm.ResultChanged, func():void { r[0] = $"$(r[0])$(sm.Result); "; }); sm.Add(1); sm.Add(2); sm.Add(3); sm.Add(4); sm.Add(5); sm.Add(6); sm.Sum(); sm.Add(1); sm.Add(1); sm.Add(2); sm.Add(3); sm.Add(5); sm.Add(8); sm.Sum(); sm.Add(1); sm.Add(2); sm.Add(4); sm.Add(8); sm.Add(16); sm.Add(32); sm.Sum(); return r[0]; } ``` The **main** function returns **"21; 20; 63; "**.

This state machine accumulate numbers by **Add** and output sum of them by **sum**.

When **Sum** is called, **$push_state ResultGenerated(sum);** is executed. When **$state ResultGenerated** finishes, the state machine continue from the last **$push_state**, which is **break;**.

And then **$goto_state NumberExpected();** is executed. This is a **$goto_state**, so it jumps to the state instead of calling it.

Using **$jump_state NumberExpected();** for making a loop results in memory leaks, because variables and resuming point for previous states are saved, but the state never finishes.

This is an infinite state machine, **$default** never ends.

