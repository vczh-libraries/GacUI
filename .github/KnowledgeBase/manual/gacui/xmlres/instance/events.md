# Events

There are two ways to handle an event:
```
<Object ev.EVENT1="FUNCTION_NAME">
    <ev.EVENT2>FUNCTION_NAME</ev.EVENT2>
</Object>
```
Or using data binding:
```
<Object ev.EVENT1-eval="{ ... }">
    <ev.EVENT2-eval><![CDATA[
        { ... }
    ]]<![CDATA[></ev.EVENT2-eval>
</Object>
```


## By Function

If a function name is assigned to an event, an empty C++ function will be generated.

You are recommended to use**ref.CodeBehind="true"**on the**\<Instance/\>**. By doing this, you will get a separated pair of C++ files for this**\<Instance/\>**. In the cpp file you will see an very obvious place holder for**FUNCTION_NAME**. You can insert anything you want inside this function.

When you change the XML later,**GacBuild.ps1**will recognize your change and merge new generated code with your change.

**IMPORTANT**:
- Only changing code in expected places, which will be marked using obvious comments in generated C++ code. Changing to all other places will be lost after running**GacBuild.ps1**.
- **GacBuild.ps1**expects "{" and "}" to take the whole line.**DO NOT**put "{" after the previous line.

Please check out[C++ Code Generation](../../.././workflow/codegen.md)for details.

## By Workflow Statement

**-eval**must be used if a[Workflow statement](../../.././workflow/lang/stat.md)is assigned to an event.

If the event is assigned using a tag,**\<![CDATA[ ]]\>**is required to contain the code.

Use a block statement if multiple statements need to be executed for this event.

