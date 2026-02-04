# Using Input Methods

Like any function,**$state_input**is allowed to have zero, one or multiple parameters. But since it always returns**void**, so you don't need to specify the return value.

When**$switch()**happens, it waits for selected inputs. If unexpected inputs are called, you can specify what should happen[in the parentheses](../.././workflow/lang/state_switch.md).

You don't have to keep the same parameter names in**case**. For example:
```
module sampleModule;

using system::*;

class MySM : StateMachine
{
    prop Output: string = "" {not observe}

    $state_machine
    {
        $state_input A(name: string);
        $state_input B(name: string);

        $state default()
        {
            $switch ()
            {
            case A(foo):
                {
                    Output = $"A($(foo))";
                }
            case B(bar):
                {
                    Output = $"B($(bar))";
                }
            }
        }
    }
}

func main(): string
{
    var sm = new MySM^();
    sm.A("John");
    return sm.Output;
}
```
The**main**function returns**"A(John)"**.

Although it defines**A**using**$state_input A(name: string);**, but you can still use**case A(foo):**.**name**is not accessible here, because the parameter name is**foo**.

You don't need to keep the parameter name consistent in all**case**statements:
```
module sampleModule;

using system::*;

class MySM : StateMachine
{
    prop Output: string = "" {not observe}

    $state_machine
    {
        $state_input A(name: string);
        $state_input B(name: string);

        $state default()
        {
            $switch ()
            {
            case A(foo):
                {
                    Output = $"A($(foo))";
                }
            case B(bar):
                {
                    Output = $"B($(bar))";
                }
            }
            $switch ()
            {
            case A(cat):
                {
                    Output = $"$(Output) A($(cat))";
                }
            case B(dog):
                {
                    Output = $"$(Output) B($(dog))";
                }
            }
        }
    }
}

func main(): string
{
    var sm = new MySM^();
    sm.A("John");
    sm.B("Smith");
    return sm.Output;
}
```
The**main**function returns**"A(John) B(Smith) "**.

