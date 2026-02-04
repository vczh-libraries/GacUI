# Runtime Instructions

This is for reference only. You are not expected to generate instructions by yourself, but it could be useful when programming against the Workflow script debugger interface.

All instructions are defined in**vl::workflow::runtime::WfInsCode**.

An instruction could have two types of arguments:
- **Embedded arguments**: They are generated at compile time and are saved in fields of**vl::workflow::runtime::WfInstruction**.
- **Stack arguments**: They are stored in the stack at runtime, most of them will be consumed and removed from the stack after executing the instruction.Argument schema of each instructions are defined by static methods of**vl::workflow::runtime::WfInstruction**.

When an instruction is named after, for example,**ConvertToType(flag, type)**, it means the name of the instruction is**WfInsCode::ConvertToType**, and embedded arguments are stored in field**flagParameter**and**typeParameter**.

## Stack Operations

### Nop

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
```


This instruction does nothing.

### LoadValue(value)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
value
```


Push a value to the stack.

### LoadFunction(index)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
index
```


Push the function index to the stack. This instruction load functions for creating closures. To push a function value to the stack, use**CreateClosure**instead.

### LoadException

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
value of the exception
```


Push the exception object of the current thread context to the stack.

### LoadLocalVar(index)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
(int) value of the variable
```


Push the value of the local variable of the specified index to the stack. Local variables are stored in the current stack frame.

### LoadCapturedVar(index)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
value of the variable
```


Push the value of the captured variable of the specified index to the stack. Captured variables are stored in the closure context that is assigned to the current stack frame.

### LoadGlobalVar(index)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
value of the variable
```


Push the value of the global variable of the specified index to the stack.

### LoadMethodInfo(method)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
(IMethodInfo*)
```


Push the**IMethodInfo***object to the stack.

### LoadMethodClosure(method)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
closure of the IMethodInfo* object
```


Push the closure of the**IMethodInfo***object to the stack. It is created by calling**IMethodInfo::CreateFunctionProxy**, this function returns a function object that can be invoked.

### LoadClosureContext

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
(Ptr<WfRuntimeVariableContext>)
```


Push the closure context of the current stack frame to the stack. Local variables are stored in the current stack frame.

### StoreLocalVar(index)

Stack pattern before:
```
--> top
value
```


Stack pattern after:
```
--> top
```


Pop a value from the stack to update a specified local variable.

### StoreCapturedVar(index)

Stack pattern before:
```
--> top
value
```


Stack pattern after:
```
--> top
```


Pop a value from the stack to update a specified captured variable. Captured variables are stored in the closure context that is assigned to the current stack frame.

### StoreGlobalVar(index)

Stack pattern before:
```
--> top
value
```


Stack pattern after:
```
--> top
```


Pop a value from the stack to update a specified global variable.

### Duplicate

Stack pattern before:
```
--> top
value
```


Stack pattern after:
```
--> top
value
value
```


Duplicate the top value in the stack.

### Pop

Stack pattern before:
```
--> top
value
```


Stack pattern after:
```
--> top
```


Pop a value from the stack.

## Jumping Operations

### Return

Stack pattern before:
```
--> stack frame 1 (top)
value
... others
--> stack frame 2
... others
```


Stack pattern after:
```
--> stack frame 2 (top)
value
... others
```


Pop a value as a function return value, pop the current stack frame so that all remaining values of this stack frame is removed, push the return value to the next stack frame, and the function ends, jumping to the next instruction from one that calling into the current function. If there is not stack frame after running this instruction, the return value will be the only value in the stack, and the script ends.

### Jump(index)

Jump to the specified instruction.

### JumpIf(index)

Stack pattern before:
```
--> top
value
```


Stack pattern after:
```
--> top
```


Pop a value from the stack. If it is**true**, jump to the specified instruction. If it is**false**, continue to execute the next instruction. If it is not a boolean value, exception occurs.

### Invoke(index, count)

Stack pattern before:
```
--> stack frame 2 (top)
arg-n
...
arg-1
... others
```


Stack pattern after:
```
--> stack frame 1 (top)
... local variables
arg-n
...
arg-1
--> stack frame 2 (top)
... others
```


Jump to the specified function, taking a specified number of stack values as arguments. Arguments are transferred to the new stack frame, so after the function ends, they are all gone.

### InvokeWithContext(index, count)

Works like**Invoke(index, count)**, but the target function shares captured values of the current function. This happens when methods of the same interface implementation are calling each other, because all of these methods share the same captured values.

### InstallTry(index)

Push a new trap frame with the entry instruction to the**catch**handler.**UninstallTry(count)**must be called pairing with**InstallTry(index)**.

### UninstallTry(count)

Pop a new trap frame. Keep the specified number of stack values right after this trap frame. This happens when no exception occurs while evaluating the left operand of the**??**operator, the value of the left operand becomes the value of the whole expression, which should be kept in the stack.

### RaiseException

Raise an exception.

When an exception occurs, all stack frames and stack values that are pushed after the top trap frame will be removed, and jump to the specified instruction of the top trap frame. If there is no trap frame, the script ends with this exception.

## Object Creating Operations

### CreateArray(count)

Stack pattern before:
```
--> top
item-1
...
item-n
```


Stack pattern after:
```
--> top
(Ptr<IValueList>) value of created array
```


Pop the specified number of values from the stack, push the created array containing these values in popping order.

### CreateObservableList(count)

Stack pattern before:
```
--> top
item-1
...
item-n
```


Stack pattern after:
```
--> top
(Ptr<IValueObservableList>) value of created observable list
```


Pop the specified number of values from the stack, push the created observable list containing these values in popping order.

### CreateMap(count)

Stack pattern before:
```
--> top
value-1
key-1
...
value-n
key-n
```


Stack pattern after:
```
--> top
(Ptr<IValueDictionary>) value of created map
```


Pop the specified number of value-key-pair from the stack, push the created observable map. A pair is two values on the stack instead of being a pair object. The order of keys in the map depends on value of keys.

### CreateClosureContext(count)

Stack pattern before:
```
--> top
item-n
...
item-1
```


Stack pattern after:
```
--> top
(Ptr<WfRuntimeVariableContext>) value of created closure context
```


Pop the specified number of values from the stack, push the created closure context containing these values in reverse popping order. If there is no value to be stored, the created context will be**null**.

### CreateClosure

Stack pattern before:
```
--> top
(int) function
(Ptr<WfRuntimeVariableContext>) closure context
```


Stack pattern after:
```
--> top
(Ptr<IValueFunctionProxy>) value of created closure
```


Pop the index of global function and a closure context, push the created closure to the stack.

### CreateInterface(method, count)

Stack pattern before:
```
--> top
(int) function-1
(IMethodInfo*) method-1
...
function-n
method-n
(Ptr<WfRuntimeVariableContext>) closure context
```


Stack pattern after:
```
--> top
value of created interface
```


Pop the specified number of function-method-pair from the stack. Push the created interface to the stack. A pair is two values on the stack instead of being a pair object, declaring that which method is implemented by which global function.

The**methodParamter**of the instruction is the constructor of the interface. Such interface must be registered using one of the**BEGIN_INTERFACE_PROXY_***macro. Please check outclass Description`missing document: /vlppreflection/ref/VL__REFLECTION__DESCRIPTION.html`for more details.

### CreateRange

Stack pattern before:
```
--> top
value of ending
value of beginning
```


Stack pattern after:
```
--> top
(Ptr<IValueEnumerable>) value of created range
```


Pop the value of ending and beginning in order, push the created range, which is a enumerable collection, including the value of beginning but excluding the value of ending. Each value is one greater than the previous one in this collection.

### CreateStruct(flag, type)

Stack pattern before:
```
--> top
```


Stack pattern after:
```
--> top
value of created struct
```


Create a struct of the specified type and push to the stack. All fields are filled with default values according to their types.

### DeleteRawPtr

Stack pattern before:
```
--> top
value to delete
```


Stack pattern after:
```
--> top
```


Pop a value from the stack and delete it. If the value is not a raw pointer, then it is just popped but not deleted. Otherwise,Value::DeleteRawPtr`missing document: /vlppreflection/ref/VL__REFLECTION__DESCRIPTION__VALUE__DELETERAWPTR@BOOL().html`is called.

## Type Operations

### ConvertToType(flag, type)

Stack pattern before:
```
--> top
value to convert
```


Stack pattern after:
```
--> top
value of converted
```


Pop a value from the stack and push the converted value to the stack. This instruction performs a[strong casting](.././workflow/lang/expr.md)on a value. Exception occurs if the conversion fails.

### TryConvertToType(flag, type)

Stack pattern before:
```
--> top
value to convert
```


Stack pattern after:
```
--> top
value of converted
```


Pop a value from the stack and push the converted value to the stack. This instruction performs a[strong casting](.././workflow/lang/expr.md)on a value.**null**will be pushed to the stack if the conversion fails.

### TestType(flag, type)

Stack pattern before:
```
--> top
value to test
```


Stack pattern after:
```
--> top
(bool) value of testing result
```


Pop a value from the stack and perform a[type testing](.././workflow/lang/expr.md).**true**will be pushed to the stack if the testing succeeds. Otherwise**false**.

### GetType

Stack pattern before:
```
--> top
value to test
```


Stack pattern after:
```
--> top
(ITypeDescriptor*) type of the value
```


Pop a value from the stack and get its type descriptor. For**null**, its type descriptor is also**null**.

## Reflection Operations

### GetProperty(property)

Stack pattern before:
```
--> top
value of the object
```


Stack pattern after:
```
--> top
value of the property
```


Pop a value from the stack as the object, push the value of the specified property of this object.

### SetProperty(property)

Stack pattern before:
```
--> top
value of the object
value of the property
```


Stack pattern after:
```
--> top
```


Pop a value from the stack as the object, pop a value as the new value of the property update the value of the specified property of this object.

### UpdateProperty(property)

Stack pattern before:
```
--> top
value of the object
value of the property
```


Stack pattern after:
```
--> top
value of the object
```


Pop a value from the stack as the object, pop a value as the new value of the property update the value of the specified property of this object. Unlike**SetProperty(property)**, the value of the object will be pushed back to the stack after executing this instruction.

### InvokeProxy(count)

Stack pattern before:
```
--> top
(Ptr<IValueFunctionProxy>) value of the function
arg-n
...
arg-1
```


Stack pattern after:
```
--> top
value of the return value
```


Pop the function along with a specified number of stack values as arguments from the stack, run the function with arguments, push the return value to the stack.

If the function is a global function or a lambda expression in the same assembly, instead of creating a new thread context, the current stack will be use to store the new stack frame for this function.

### InvokeMethod(method, count)

Stack pattern before:
```
--> top
value of the object
arg-n
...
arg-1
```


Stack pattern after:
```
--> top
value of the return value
```


Pop the object along with a specified number of stack values as arguments from the stack, run the specified method on that object with arguments, push the return value to the stack.

If the method is implemented by another piece of Workflow script in the same assembly, instead of creating a new thread context, the current stack will be use to store the new stack frame for this function. For non-static method, the object will becomes the only captured value.

### InvokeEvent(event, count)

Stack pattern before:
```
--> top
value of the object
arg-n
...
arg-1
```


Stack pattern after:
```
--> top
value of the return value
```


Pop the object along with a specified number of stack values as arguments from the stack, run the specified event on that object with arguments, push**null**as the return value to the stack.

### InvokeBaseCtor(method, count)

Stack pattern before:
```
--> top
value of the object
arg-n
...
arg-1
```


Stack pattern after:
```
--> top
value of the return value
```


Pop the object along with a specified number of stack values as arguments from the stack, run the specified method on that object with arguments, push**null**as the return value to the stack.

This instruction is used in classes that defined using Workflow script.

If the base constructor is implemented by another piece of Workflow script in the same assembly, instead of creating a new thread context, the current stack will be use to store the new stack frame for this function.

If the base constructor is implemented by another piece of Workflow script in another assembly, the base constructor will be applied to the object directly.

If the base constructor is implemented by C++, aggregation will be used to bind the object and the created C++ instance.

### AttachEvent(event)

Stack pattern before:
```
--> top
value of the function
value of the object
```


Stack pattern after:
```
--> top
(Ptr<IEventHandler>) value of the event handler
```


Pop the callback function and the object from the stack, attach the callback function to the specified event of the object, push the event handler to the stack.

### DetachEvent(event)

Stack pattern before:
```
--> top
(Ptr<IEventHandler>) value of the event handler
value of the object
```


Stack pattern after:
```
--> top
(bool) value of detaching result
```


Pop the event handler and the object from the stack, detach the event handler from the specified event of the object.**true**will be pushed to the stack if the detaching succeeds.

## Set Operations

### TestElementInSet

Stack pattern before:
```
--> top
value of the set
value of the element
```


Stack pattern after:
```
--> top
(bool) value of testing result
```


Pop the set and the value from the stack, test if the element is in the set.**true**will be pushed to the stack if the testing succeeds.

## Comparison Operations

Stack pattern before:
```
--> top
right-operand
left-operand
```


Stack pattern after:
```
--> top
(int or bool) value of the result
```


### CompareLiteral(type)

Compare booleans, numbers or strings.

Push**-1**to the stack if the left operand is smaller. Push**1**to the stack if the left operand is larger. Push**0**to the stack if operands are equal to each other.

### CompareReference

Compare two pointers.

Push**true**to the stack if operands are equal to each other. Push**false**to the stack if operands are not equal to each other, or any of them is not a pointer.

### CompareValue

Compare two values.

Push**true**to the stack if operands are equal to each other. Push**true**to the stack if operands are not equal to each other.

### OpLT

This instruction executes**left \< right**.

### OpGT

This instruction executes**left \> right**.

### OpLE

This instruction executes**left \<= right**.

### OpGE

This instruction executes**left \>= right**.

### OpEQ

This instruction executes**left == right**.

### OpNE

This instruction executes**left != right**.

## Unary Operations

Stack pattern before:
```
--> top
operand
```


Stack pattern after:
```
--> top
value of the result
```


Pop the operand, push the result of a specified unary operator. Instructions in this category apply[unary operators](.././workflow/lang/expr_unary.md)on the operand.

### OpNot(type)

This instruction executes**not operand**.

### OpPositive(type)

This instruction executes**+ operand**.

### OpNegative(type)

This instruction executes**- operand**.

## Binary Operations

Stack pattern before:
```
--> top
right-operand
left-operand
```


Stack pattern after:
```
--> top
value of the result
```


Pop the right and left operands in order, push the result of a specified binary operator. Instructions in this category apply[binary operators](.././workflow/lang/expr_binary.md)on operands.

### OpConcat

This instruction executes**left & right**on strings.

### OpExp(type)

This instruction executes**left ^ right**.

### OpAdd(type)

This instruction executes**left + right**.

### OpSub(type)

This instruction executes**left - right**.

### OpMul(type)

This instruction executes**left * right**.

### OpMod(type)

This instruction executes**left % right**.

### OpDiv(type)

This instruction executes**left / right**.

### OpShl(type)

This instruction executes**left shl right**.

### OpShr(type)

This instruction executes**left shr right**.

### OpXor(type)

This instruction executes**left xor right**.

### OpAnd(type)

This instruction executes**left and right**.

For flags, values are converted to**UInt64**, and then converted back after running this instruction.

### OpOr(type)

This instruction executes**left or right**.

For flags, values are converted to**UInt64**, and then converted back after running this instruction.

