# Implementing (interface)

New interface expressions are the only way to implement interfaces.

An new interface expression implements one interface, but this interface can inherit from multiple other interfaces.

If the interface is**interface IThatInterface { ... }**, the new interface expressions should use**new IThatInterface^ { ... }**, to create an object that hold by a shared pointer.

If the interface is**interface IThatInterface* { ... }**, the new interface expressions should use**new IThatInterface* { ... }**, to create an object that hold by a raw pointer. If such object is not deleted, it cause memory leaks.

Besides of members in the interface, the new interface expressions could also contain:
- variables
- functions not starting with**override**All of them are only accessible by name inside the new interface expressions.

## New and delete objects implementing shared interfaces


```
interface IMyInterface
{
    func Do():void;
}

var my = new IMyInterface^
{
    override func Do():void {}
};
```


You are not allowed to write**new IMyInterface***because the interface is not**interface IMyInterface***.

The type of**my**is**IMyInterface^**, but you can still get a**IMyInterface***by doing an explicit cast:**var my2 = cast IMyInterface* my;**.

When the object that**my2**points to is still alive,**cast IMyInterface^ my2**is safe.

When all**IMyInterface^**values end their life cycles, the object will be deleted, even if there are still**IMyInterface***values.

It crashes if you call a member on a deleted object.

It crashes if you**delete**an object that hold by shared pointers.

## New and delete objects implementing raw interfaces


```
interface IMyInterface*
{
    func Do():void;
}

var my = new IMyInterface*
{
    override func Do():void {}
};
```


You are not allowed to write**new IMyInterface^**because the interface is**interface IMyInterface***.

The type of**my**is**IMyInterface***, but you can still get a**IMyInterface^**by doing an explicit cast:**var my2 = cast IMyInterface^ my;**.

When the object that**my**points to is still alive,**cast IMyInterface^ my**is safe.

When all**IMyInterface^**values end their life cycles, the object will be deleted, even if there are still**IMyInterface***values.

It crashes if you call a member on a deleted object.

It crashes if you**delete**an object that hold by shared pointers.

In order to delete**my**, you either call**delete my;**, or**cast IMyInterface^ my**.

## Memory leaks

If two objects hold each other by shared points, it causes memory leaks:
```
interface IMyInterface
{
    prop Next: IMyInterface^ {not observe}
}

func main(): void
{
    var a = new IMyInterface^
    {
        override prop Next: IMyInterface^ = null {not observe}
    };

    var b = new IMyInterface^
    {
        override prop Next: IMyInterface^ = null {not observe}
    };
    
    a.Next = b;
    b.Next = a;
}
```
Until you break the connection manually.

