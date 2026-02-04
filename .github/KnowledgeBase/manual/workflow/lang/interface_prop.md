# Properties and Events (interface)

Properties and events are allowed in interfaces.

## Defining properties and events

When property**Name**is associated with event**NameChanged**, magic doesn't happen automatically, the implementation of**SetName**should trigger**NameChanged**approppriately.

The property could also be readonly, or not associated with an event.

### Full-featured auto-property


```
interface IWithProp
{
    prop Name: string {}
}
```
which means:
```
interface IWithProp
{
    event NameChanged();
    func GetName(): string;
    func SetName(value: string): void;
    prop Name: string {GetName, SetName : NameChanged}
}
```


### Readonly auto-property


```
interface IWithProp
{
    prop Name: string {const}
}
```
which means:
```
interface IWithProp
{
    event NameChanged();
    func GetName(): string;
    prop Name: string {GetName : NameChanged}
}
```


### Auto-property Without event


```
interface IWithProp
{
    prop Name: string {not observe}
}
```
which means:
```
interface IWithProp
{
    func GetName(): string;
    func SetName(value: string): void;
    prop Name: string {GetName, SetName}
}
```


### Readonly auto-property without event


```
interface IWithProp
{
    prop Name: string {const, not observe}
}
```
which means:
```
interface IWithProp
{
    func GetName(): string;
    prop Name: string {GetName}
}
```


## Implementing properties

Auto-properties are also allowed in new interface expressions to create default implementation for properties:
```
var withProp = new IWithProp^
{
    override prop Name: string = "an-initialized-value" {}
};
```

- The**override**keyword is required.
- Auto-properties in new interface expressions must be initialized.
- It doesn't matter whether the property in the interface is auto-property or not.It is almost equivalent to:
```
var withProp = new IWithProp^
{
    var name: string = "an-initialized-value";

    override func GetName(): string
    {
        return name;
    }

    override func SetName(value: string): void
    {
        if (name != value)
        {
            name = value;
            NameChanged();
        }
    }
};
```
but the**name**variable is not accessible.

You don't need to**override**an event in an interface.

When an auto-property is used in an new interface expression, the configuration of**const**and**not observe**must be exactly the same to the property that is overrided in the interface, semantically (because it could be declared using a normal property, not using an auto-property).

Even if you use**const**for an auto-property in an new interface expression, the**SetName**function is always there. In this case, the only way to change the value of the property is to call**SetName**.

## Using events

Events don't have to be associated with properties. You can define events in interfaces and trigger them in new interface expressions.

Events are allowed to have arguments, but just like a function type, arguments do not have names.


```
module sampleModule;

interface IWithEvent
{
    event SomethingHappened(string);
    func MakeItHappen(something: string): void;
}

func main(): string
{
    var withEvent = new IWithEvent^
    {
        override func MakeItHappen(something: string): void
        {
            SomethingHappened(something);
        }
    };

    var r = {""};
    var handler = attach(
        withEvent.SomethingHappened,
        func (something:string): void
        {
            r[0] = r[0] & $"$(something); ";
        }
    );

    withEvent.MakeItHappen("Good");
    withEvent.MakeItHappen("Cool");
    withEvent.SomethingHappened("Wonderful");

    detach(withEvent.SomethingHappened, handler);
    return r[0];
}
```

- Triggering an event is like calling a function.
- Events can be triggered anywhere, not just in the new interface expression.
- The only way to access an event is to trigger it, you are not allowed to copy the event object.
- **attach**is used to subscribe an event with a value of a function type that is compatible with the event (exactly same argument types and returning**void**).
- The result from**attach**is used to unsubscribe an event in**detach**.
- **detach**expression returns a**bool**value, it returns null when this event handler is not a subscription for this event, or this event handler has been unsubscribed.

**IMPORTANT**: Variables are captured by value in ordered lambda expressions, lambda expressions and new interface expressions. This is the reason why we define**r**as**string []**. Because**r**is not allowed to be changed in the event callback, but we can change elements in**r**.

Function**main**returns**"Good; Cool; Wonderful; "**.

