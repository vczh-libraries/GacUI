# Classes

Classes can be defined in**modules**,**namespaces**,**classes**and**interfaces**.

A class is a type that contains data and behaviors. It can be allocated directly, without having to be implemented like interfaces.

A class can inherit from other classes, but it cannot override virtual members in base classes. A class cannot define virtual members in Workflow script. So**override**is not allowed for all members in classes.

Unlike C++, all members in classes are public. But you can change access controls for members in[generated C++ code](../.././workflow/codegen.md).

Variables, methods, properties, events, constructors and destructors are allowed in classes.

Static methods, classes, interfaces, enums are allowed in classes, they are accessed by**TYPE::MEMBER**outside of the class.

Unlike interfaces, a class does not control whether it should be created and held by a shared pointer or a raw pointer, this is controlled by individual constructors. When there is no contructor,**new(){}**will be added to the class automatically, which allocate the class in a shared pointer.

A class could also be a[state machine](../.././workflow/lang/state.md).


```
module sampleModule;

class Counter
{
    var counter: int = 0;

    func Increase(): int
    {
        counter = counter + 1;
        return counter;
    }
}

func main(): string
{
    var counter = new Counter^();
    var r = "";
    for (i in range [0, 10))
    {
        r = r & $"$(counter.Increase()); ";
    }
    return r;
}
```


The default contructor is not defined, so**new(){}**is added to the class when compiling. This makes**new Counter^()**a legal expression to allocate this class. Meanwhile,**new Counter*()**is not allowed, unless**new*(){}**is explicitly declared in the class.

The type of**counter**becomes**Counter^**. You don't need to call**delete counter;**, the object will be deleted automatically when the last shared pointer is out of scope.

Now we call**counter.Increase()**10 times, which generates numbers from 1 to 10. Function**main**returns**"1; 2; 3; 4; 5; 6; 7; 8; 9; 10; "**.

