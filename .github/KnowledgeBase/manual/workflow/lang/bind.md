# Data Bindings

A data binding expressions**bind(EXPR)**observes the changing of**EXPR**by attaching callbacks to all involved events that are associated with properties.

## Defining a data binding

When we say property**A**is associated with event**AChanged**, it means that the property must be declared in one of the following ways:
- **prop A: Type {}**
- **prop A: Type {const}**
- **prop A: Type {getter, AChanged}**
- **prop A: Type {getter, setter, AChanged}**

**bind**and**this**are not allowed in**bind**, unless they are in a function body of a lambda expression or a new interface expression inside**bind**.

Any event that is associated with properties that are used in a function body of a lambda expression or a new interface expressions will not be observed.

Properties that need to be observed must be written in**EXPR.PROP**. For example:
```
class C
{
    prop A: int = 0; {}

    new()
    {
        var x = bind(A + 1);
        var self = this;
        var y = bind(self.A + 1);
    }
}
```

- **x**doesn't observe**A**, because it is not in the**EXPR.PROP**form.
- **y**observes**A**, but since**this**is not allowed in**bind**, it needs to be stored in a variable.

## Listening to a data binding

A**bind**expression returns**system::Subscription^**.

The**Open**method needs to be called before the observation takes effect.

The**Close**method needs to be called when the observation is no longer needed.

**ValueChanged**is called when observed events are triggered.

When a subscription observes some objects, the reference of this subscription is captured by event handlers that are attached on properties, and these objects are also stored in this subscription because**detach**needs to be called later, which makes cycle referencing.

By calling**Close**, these cycle referencing are removed, so that memory leaks don't happen.


```
module sampleModule;

class Numbers
{
    prop A: int = 0 {}
    prop B: int = 0 {}
    prop C: int = 0 {}
}

func main(): string
{
    var numbers = new Numbers^();
    var sub = bind(numbers.A + numbers.B + numbers.C);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    numbers.A = 1;
    numbers.B = 2;
    numbers.C = 3;

    sub.Close();
    return r[0];
}
```
This function returns**"1; 3; 6; "**, because by assigning to properties,**AChanged**,**BChanged**and**CChanged**are all triggered, so the**bind**expression observes 3 changes, and the callback is executed 3 times.

## Observing property chains

Property chains are also observable, all related events will be attached.
```
module sampleModule;

class ClassA
{
    prop B: ClassB^ = new ClassB^() {}
}

class ClassB
{
    prop C: ClassC^ = new ClassC^() {}
}

class ClassC
{
    prop X: int = 0 {}
}

func main(): string
{
    var a = new ClassA^();
    var sub = bind(a.B.C.X);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    {
        a.B.C.X = 100;
    }
    {
        var c = new ClassC^();
        c.X = 200;
        a.B.C = c;
    }
    {
        var b = new ClassB^();
        b.C.X = 300;
        a.B = b;
    }

    sub.Close();
    return r[0];
}
```
This function returns**"100; 200; 300; "**, because assigned to**a.B**,**a.B.C**and**a.B.C.X**all trigger attached events.

## Capturing values

Values captured in**bind**expressions just like lambda expressions and new interface expressions, they are shallow copied.
```
module sampleModule;

class Numbers
{
    prop A: int = 0 {}
    prop B: int = 0 {}
    prop C: int = 0 {}
}

func main(): string
{
    var init = 0;
    var numbers = new Numbers^();
    var sub = bind(init + numbers.A + numbers.B + numbers.C);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    init = 100;
    numbers.A = 1;
    init = 200;
    numbers.B = 2;
    init = 300;
    numbers.C = 3;

    sub.Close();
    return r[0];
}
```
This function returns**"1; 3; 6; "**, because the change of**init**is not visible in**bind**.

## Using unobservable properties


```
module sampleModule;

class Numbers
{
    prop A: int = 0 {not observe}
    prop B: int = 0 {not observe}
    prop C: int = 0 {}
}

func main(): string
{
    var numbers = new Numbers^();
    var sub = bind(numbers.A + numbers.B + numbers.C);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    numbers.A = 1;
    numbers.B = 2;
    numbers.C = 3;

    sub.Close();
    return r[0];
}
```
This function returns**"6; "**, because**A**and**B**is not observable, the callback is triggered only when**C**is changed.

But we can manually trigger the callback by calling**Update**:
```
numbers.A = 1;
sub.Update();
numbers.B = 2;
sub.Update();
numbers.C = 3;
```
Not it returns**"1; 3; 6; "**.

## Be carefule about unobservable properties in property chains

Objects that own observed properties will be stored inside**bind**, because**detach**needs to be called.
```
module sampleModule;

class ClassA
{
    prop B: ClassB^ = new ClassB^() {not observe}
}

class ClassB
{
    prop C: ClassC^ = new ClassC^() {not observe}
}

class ClassC
{
    prop X: int = 0 {}
}

func main(): string
{
    var a = new ClassA^();
    var sub = bind(a.B.C.X);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    var oldC = a.B.C;
    {
        a.B.C.X = 100;
    }
    {
        var c = new ClassC^();
        c.X = 200;
        a.B.C = c;
    }
    {
        var b = new ClassB^();
        b.C.X = 300;
        a.B = b;
    }
    oldC.X = 400;

    sub.Close();
    return r[0];
}
```
This function returns**"100; 400; "**.

**a.B**and**a.B.C**is not observable, so assigning to them don't trigger the callback.

But when**oldC.X = 400;**is called, the callback is triggered with**400**, which is obviously not the actually value of**a.B.C.X**that is specified in**bind**.

Because**a.B.C**is cached.

**a.B**and**a.B.C**is not observable, so when we write**bind(a.B.C.X)**, it is actually**let cachedObject = a.B.C in (bind(cachedObject.X))**.

When**a.B**becomes observable but**a.B.C**still not,**a.B.C**will be updated when**a.B**is changed. If**a.B**is not changed when**a.B.C**is reassigned, the evaluation of the observed expression is still using the old previous**a.B.C**value.

In the previous example, remove**not observe**in**prop B**, and change some code between**sub.Open()**and**sub.Close**like this:
```
module sampleModule;

class ClassA
{
    prop B: ClassB^ = new ClassB^() {}
}

class ClassB
{
    prop C: ClassC^ = new ClassC^() {not observe}
}

class ClassC
{
    prop X: int = 0 {}
}

func main(): string
{
    var a = new ClassA^();
    var sub = bind(a.B.C.X);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    {
        var b = new ClassB^();
        b.C.X = 100;
        a.B = b;
    }
    {
        a.B.C.X = 200;
    }
    var oldC = a.B.C;
    {
        var c = new ClassC^();
        c.X = 300;
        a.B.C = c;
    }
    oldC.X = 400;

    sub.Close();
    return r[0];
}
```
This function returns**"100; 200; 400; "**.

Changing**a.B.C**doesn't trigger the event, because**C**is not observable.

Changing**oldC.X**triggers the event, because the previous**a.B.C**is cached in the subscription even after**a.B.C**is changed. So it doesn't actually evaluate**a.B.C.X**. Instead, it evaluate**oldC.X**.

But if we change**a.B**again, the cached**ClassC^**object will be refreshed, and it is not**oldC**anymore.

## A ? B : C


```
module sampleModule;

class Cond
{
    prop Value: bool = false {}
}

class Int
{
    prop Value: int = 0 {}
}

func main(): string
{
    var c = new Cond^();
    var a = new Int^();
    var b = new Int^();
    var sub = bind(c.Value ? a.Value : b.Value);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    c.Value = true;
    a.Value = 100;
    b.Value = 200;
    c.Value = false;
    a.Value = 300;
    b.Value = 400;

    sub.Close();
    return r[0];
}
```
This function returns**"0; 100; 100; 200; 200; 400; "**.

Even if**a**and**b**are in different branches of the if-expression, their events are all attached. When**a.Value**or**b.Value**is changed, even if it is not involved in evaluation, it still triggers the callback.

If there are property chains in branches of an if-expression, all**attach**and**detach**will happen when up-stream properties are changed, caching of objects that own observable properties are evaluated, related property getters will are called, regardless of what is evaluated from the condition expression.

Let's see a more complex example:
```
module sampleModule;

class Cond
{
    prop Value: bool = false {}
}

class Holder
{
    prop Value: Int^ = new Int^() {}
}

class Int
{
    prop Value: int = 0 {}
}

func main(): string
{
    var c = new Cond^();
    var a = new Holder^();
    var b = new Holder^();
    var sub = bind((c.Value ? a.Value : b.Value).Value);

    var r = {""};
    attach(sub.ValueChanged, func(newValue: object): void
    {
        r[0] = $"$(r[0])$(cast int newValue); ";
    });
    sub.Open();

    c.Value = true;
    a.Value.Value = 100;
    b.Value.Value = 200;
    c.Value = false;
    a.Value.Value = 300;
    b.Value.Value = 400;

    sub.Close();
    return r[0];
}
```
This function returns**"0; 100; 200; 400; "**.

When**c.Value = true;**is executed,**0;**is printed.**(c.Value ? a.Value : b.Value)**becomes**a.Value**, so**a.Value.ValueChanged**is attached,**b.Value.ValueChanged**is detached.

When**a.Value.Value = 100;**is executed,**100;**is printed, obviously.

When**b.Value.Value = 200;**is executed, nothing happens, because**b.Value.ValueChanged**has already been detached.

When**c.Value = false;**is executed,**200;**is printed.**(c.Value ? a.Value : b.Value)**becomes**b.Value**, so**a.Value.ValueChanged**is detached,**b.Value.ValueChanged**is attached.

When**a.Value.Value = 300;**is executed, nothing happens, because**a.Value.ValueChanged**has already been detached.

When**b.Value.Value = 400;**is executed,**400;**is printed, obviously.

