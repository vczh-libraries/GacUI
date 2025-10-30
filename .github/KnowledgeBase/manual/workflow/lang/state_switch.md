# Handling Unexpected Inputs

Workflow offers built-in strategy for handling unexpected inputs. - **$switch(raise)** raises an exception on unexpected input and stops the state machine. - **$switch()** and **$switch(return)** consume the unexpected input. - **$switch(continue)** and **$switch(continue, return)** do not consume the unexpected input. - **$switch()** and **$switch(continue)** stop the current **$switch** and continue. - **$switch(return)** and **$switch(continue, return)** stop the current state and continue.

## $switch()

**$switch()** - ignores and consumes an unpected input - continues to execute code after **$switch** ``` module sampleModule; using system::*; class MySM : StateMachine { prop Output: string = "" {not observe} func Append(text: string): void { Output = $"$(Output)$(text); "; } $state_machine { $state_input A(name: string); $state_input B(name: string); $state_input C(name: string); $state WaitForName() { $switch () { case A(name): { Append($"A($(name))"); } case B(name): { Append($"B($(name))"); } } Append("Finishes WaitForName()"); } $state default() { $push_state WaitForName(); Append("Finishes default()"); } } } func main(): string { var sm = new MySM^(); try { sm.C("John"); } catch (ex) { sm.Append(ex.Message); } return sm.Output; } ``` The **main** function returns **"Finishes WaitForName(); Finishes default(); "**.

When **C("John")** is called, **$switch** found that this is an unexpected input, it continues to run code after **$switch**.

## $switch(raise)

**$switch()** - throws an exception when an unpected input happens - stops the state machine ``` module sampleModule; using system::*; class MySM : StateMachine { prop Output: string = "" {not observe} func Append(text: string): void { Output = $"$(Output)$(text); "; } $state_machine { $state_input A(name: string); $state_input B(name: string); $state_input C(name: string); $state WaitForName() { $switch (raise) { case A(name): { Append($"A($(name))"); } case B(name): { Append($"B($(name))"); } } Append("Finishes WaitForName()"); } $state default() { $push_state WaitForName(); Append("Finishes default()"); } } } func main(): string { var sm = new MySM^(); try { sm.C("John"); } catch (ex) { sm.Append(ex.Message); } try { sm.A("John"); } catch (ex) { sm.Append(ex.Message); } return sm.Output; } ``` The **main** function returns **"Method "C" of class "MySM" cannot be called at this moment.; The state machine has been stopped.; "**.

When **C("John")** is called, **$switch** found that this is an unexpected input, it throws an exception, and the state machine is stopped.

## $switch(return)

**$switch()** - consumes an unpected input - stops the current state - continues to execute code after this state ``` module sampleModule; using system::*; class MySM : StateMachine { prop Output: string = "" {not observe} func Append(text: string): void { Output = $"$(Output)$(text); "; } $state_machine { $state_input A(name: string); $state_input B(name: string); $state_input C(name: string); $state WaitForName() { $switch (return) { case A(name): { Append($"A($(name))"); } case B(name): { Append($"B($(name))"); } } Append("Finishes WaitForName()"); } $state default() { $push_state WaitForName(); Append("Finishes default()"); } } } func main(): string { var sm = new MySM^(); try { sm.C("John"); } catch (ex) { sm.Append(ex.Message); } return sm.Output; } ``` The **main** function returns **"Finishes default(); "**.

When **C("John")** is called, **$switch** found that this is an unexpected input, it stops **$state WaitForName()** and continues in **$state default()**.

## $switch(continue)

**$switch(continue)** - consumes an unpected input - continues to execute code after **$switch** - the unpected input is not consumes, it is left for the next **$switch** ``` module sampleModule; using system::*; class MySM : StateMachine { prop Output: string = "" {not observe} func Append(text: string): void { Output = $"$(Output)$(text); "; } $state_machine { $state_input A(name: string); $state_input B(name: string); $state_input C(name: string); $state WaitForName() { $switch (continue) { case A(name): { Append($"1st A($(name))"); } case B(name): { Append($"1st($(name))"); } } $switch (raise) { case A(name): { Append($"2nd A($(name))"); } case B(name): { Append($"2nd B($(name))"); } case C(name): { Append($"2nd C($(name))"); } } Append("Finishes WaitForName()"); } $state default() { $push_state WaitForName(); $switch (raise) { case A(name): { Append($"3rd A($(name))"); } case B(name): { Append($"3rd B($(name))"); } case C(name): { Append($"3rd C($(name))"); } } Append("Finishes default()"); } } } func main(): string { var sm = new MySM^(); try { sm.C("John"); sm.C("John"); } catch (ex) { sm.Append(ex.Message); } return sm.Output; } ``` The **main** function returns **"2nd C(John); Finishes WaitForName(); 3rd C(John); Finishes default(); "**.

When **C("John")** is called, the first **$switch** found that this is an unexpected input, it continues to run code after **$switch**.

And then the second **$switch** is executed, it found that **C("John")** is an expected input at this moment, and consumes it.

## $switch(continue, return)

**$switch(continue)** - consumes an unpected input - stops the current state - continues to execute code after this state - the unpected input is not consumes, it is left for the next **$switch** ``` module sampleModule; using system::*; class MySM : StateMachine { prop Output: string = "" {not observe} func Append(text: string): void { Output = $"$(Output)$(text); "; } $state_machine { $state_input A(name: string); $state_input B(name: string); $state_input C(name: string); $state WaitForName() { $switch (continue, return) { case A(name): { Append($"1st A($(name))"); } case B(name): { Append($"1st($(name))"); } } $switch (raise) { case A(name): { Append($"2nd A($(name))"); } case B(name): { Append($"2nd B($(name))"); } case C(name): { Append($"2nd C($(name))"); } } Append("Finishes WaitForName()"); } $state default() { $push_state WaitForName(); $switch (raise) { case A(name): { Append($"3rd A($(name))"); } case B(name): { Append($"3rd B($(name))"); } case C(name): { Append($"3rd C($(name))"); } } Append("Finishes default()"); } } } func main(): string { var sm = new MySM^(); try { sm.C("John"); sm.C("John"); } catch (ex) { sm.Append(ex.Message); } return sm.Output; } ``` The **main** function returns **"3rd C(John); Finishes default(); The state machine has been stopped.; "**.

When **C("John")** is called, the first **$switch** found that this is an unexpected input, it stops **$state WaitForName()** and continues in **$state default()**.

The second **$switch** is skipped.

And then the third **$switch** is executed, it found that **C("John")** is an expected input at this moment, and consumes it.

When **C("John")** is called for the second time, the state machine has stopped, it throws an exception.

