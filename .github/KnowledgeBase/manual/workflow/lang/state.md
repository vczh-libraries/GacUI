# State Machines

A state machine works like a coroutine, but unlike coroutine which only has**Resume**, a state machine could have multiple strong-typed inputs.

A state machine is a class. It must inherit from**system::StateMachine**The last member must be**$state_machine { ... }**.

A state machine defines multiple states with arguments. The entry state is called**default**.**$goto_state**and**$push_state**could be use for jump between states.

A state machine defines multiple strong-typed inputs. The state machine is driven by input. When a**$switch**state is executed, it waits for selected inputs. A required input method must be called to resume the state machine.

Let's start with an example:
```
module sampleModule;

using system::*;

class MySM : StateMachine
{
    prop Output: string = "" {not observe}

    func Append(text: string): void
    {
        Output = $"$(Output)$(text); ";
    }

    $state_machine
    {
        $state_input A(name: string);
        $state_input B(name: string);

        $state WaitForName()
        {
            $switch ()
            {
            case A(name):
                {
                    Append($"A($(name))");
                }
            case B(name):
                {
                    Append($"B($(name))");
                }
            }
        }

        $state default()
        {
            Append("First line");
            $push_state WaitForName();
            Append("Last line");
        }
    }
}

func main(): string
{
    var sm = new MySM^();
    try
    {
        sm.A("John");
        sm.B("Smith");
    }
    catch (ex)
    {
        sm.Append(ex.Message);
    }
    return sm.Output;
}
```
The**main**function returns**"First line; A(John); Last line; The state machine has been stopped.; "**.


- **new MySM^()**is executed, the state machine is not started.
- **sm.A("John");**is executed. Because this is the first time a**$state_input**is called, the state machine starts.
- The entry state of a state machine is**default**, it immediately enters**$state default()**.**"First line; "**is printed.
- **$push_state WaitForName()**is executed, it jumps to**$state WaitForName()**.
- It is supposed to stop at**$switch**. But because**A**has not been consumed, the state machine continues and choose**case A(name):**.**"A(John)"**is printed.
- Now**$state WaitForName()**finishes. Because the state machine jumps to this state via**$push_state**, so it still needs to go back to**$state default**.**"Last line;"**is printed.
- The state machine stops.
- **sm.B("Smith");**is executed, because the state machines has stopped, it throws an exception.

**$state**defines states.**$state_input**defines inputs from outside. State machines have more freedom in exchanging data comparing to coroutines.

