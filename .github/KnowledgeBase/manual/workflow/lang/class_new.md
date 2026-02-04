# Allocating (class)

New class expressions are the only way to allocate classes.

A new class expression could allocate a class by calling the a class constructor, the constructor decides to return a shared pointer or a raw pointer.
```
class MyClass
{
    new(x:int){}
    new*(x:string){}
    delete{}
}

func main(): void
{
    var a = new MyClass^(0);
    var b = new MyClass*("");
    delete b;
}
```

- The first constructor accepts an integer and returns a shared pointer,**new MyClass^(0)**allocate**MyClass**and execute this constructor,**new MyClass*(0)**is illegal.
- The second constructor accepts an string and returns a raw pointer,**new MyClass*("")**allocate**MyClass**and execute this constructor,**new MyClass^("")**is illegal.
- **a**is held by a shared pointer,**delete (cast MyClass* a);**will throw an exception.
- **b**is not held by any shared pointer, so**delete b;**is necessary to prevent from memory leaks.
- The destructor**delete{}**will be executed for both**a**, when**a**is out of scope.
- The destructor**delete{}**will be executed for both**b**, when**delete b;**.

It crashes if you call a member on a deleted object.

It crashes if you**delete**an object that hold by shared pointers.

Where there is no constructor in a class,**new(){}**will be automatically added to this class, which takes no argument and returns a shared pointer.

There cannot be more than one destructor in a class.

