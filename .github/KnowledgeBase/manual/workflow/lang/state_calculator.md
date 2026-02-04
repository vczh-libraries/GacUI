# Example: Calculator

The following example shows how to make a calculator using state machine.


```
module sampleModule;
        
using system::*;

class Calculator : StateMachine
{
    var valueFirst : string = "";
    var op : string = "";
    prop Value : string = "0" {}

    func Update(value : string) : void
    {
        SetValue(value);
        valueFirst = value;
    }

    func Calculate() : void
    {
        if (valueFirst == "")
        {
            valueFirst = Value;
        }
        else if (op == "+")
        {
            Update((cast double valueFirst) + (cast double Value));
        }
        else if (op == "*")
        {
            Update((cast double valueFirst) * (cast double Value));
        }
        else
        {
            raise $"Unrecognized operator: $(op)";
        }
    }

    $state_machine
    {
        $state_input Digit(i : int);
        $state_input Dot();
        $state_input Add();
        $state_input Mul();
        $state_input Equal();
        $state_input Clear();

        $state Digits()
        {
            $switch (continue)
            {
            case Digit(i):
                {
                    Value = Value & i;
                    $goto_state Digits();
                }
            }
        }

        $state Integer(newNumber: bool)
        {
            $switch (continue)
            {
            case Digit(i):
                {
                    if (newNumber)
                    {
                        Value = i;
                    }
                    else
                    {
                        Value = Value & i;
                    }
                    $goto_state Digits();
                }
            }
        }

        $state Number()
        {
            $push_state Integer(true);
            $switch (continue, return)
            {
            case Dot():
                {
                    Value = Value & ".";
                }
            }
            $push_state Integer(false);
        }

        $state Operator()
        {
            $push_state Number();
            $switch (raise)
            {
            case Add():    {Calculate(); op = "+";}
            case Mul():    {Calculate(); op = "*";}
            case Equal():  {Calculate();}
            case Clear():
                {
                    valueFirst = "";
                    op = "";
                    Value = "0";
                }
            }
            $goto_state Operator();
        }
    
        $state default()
        {
            $goto_state Operator();
        }
    }
}

func main(): string
{
    var r = {""};
    var c = new Calculator^();
    attach(c.ValueChanged, func():void
    {
        r[0] = $"$(r[0])$(c.Value)\r\n";
    });

    c.Digit(1); c.Digit(2); c.Add(); c.Digit(3); c.Digit(4); c.Mul(); c.Digit(5); c.Digit(6); c.Equal(); c.Clear();
    c.Digit(2); c.Mul(); c.Equal(); c.Equal(); c.Equal(); c.Equal(); c.Clear();
    return r[0];
}
```


The state machine simulates how numbers on a calculator is changed while pressing buttons.

The**main**function returns:
```
1
12
3
34
46
5
56
2576
0
2
4
16
256
65536
0
```
It calculates:
- 12 + 34 = 46
- 46 * 56 = 2576
- 2 * 2 = 4
- 4 * 4 = 16
- 16 * 16 = 256
- 256 * 256 = 65536

