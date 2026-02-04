# Animations

The GacUI XML[<Animation/>](../.././gacui/xmlres/tag_animation.md)resource creates helper classes for animations. It creates a class like this:
```
class MyAnimation
{
    prop Current: STATE_CLASS^{}
    new (current: STATE_CLASS^){}
    func CreateAnimation(target: STATE_CLASS^, time: UInt64);
}
```
**STATE_CLASS**is a class with fields of number types or**Color**. By creating an**MyAnimation^**in a window like this:
```
<Instance ...>
    <ref.Members><![CDATA[
        var myAnimation : MyAnimation^ = new MyAnimation(initial_state);
    ]]></ref.Members>
    <Window ref.Name="self">
        <Something PropertyToAnimate-bind="self.myAnimation.Current.FIELD"/>
    </Window>
</Instance>
```
You are able to bind the expression**self.myAnimation.Current.FIELD**to any compatible property. When the animation is running, fields in**self.myAnimation.Current**will keep updating, so that to animate the UI.

## Interpolation Functions

The first step to animate a field in**self.myAnimation.Current**is to set a proper interpolation function, which is described in[<Animation/>](../.././gacui/xmlres/tag_animation.md). Only fields that mentioned by a**\<Target/\>**tag could be animated. All animated fields must associate an interpolation function. There is also a place for a default interpolation function, which will apply on all animated fields unless one is specified in**\<Target/\>**.

An interpolation function is an Workflow expression in a function type, which takes a**double**and returns a**double**.

The function parameter is the progress of the animation from**0**to**1**. If an animation is 10 seconds long, then**0**is the beginning,**1**is the ending, and**0.4**means**10 seconds * 0.4**which is at the end of the 4th second.

The return value is the interpolation of the animation from**0**to**1**. If the begin state is**5**and the end state is**15**, then returning**0**means**5**, returning**1**means**15**, and returning**0.4**means the**5 + (15 - 5) * 0.4**which is**9**.

The interpolation calculate that, where a field should go given the progress of the animation. If a linear interpolation function is expected, then just return the parameter. Such a function would typically be:
- for[ordered lambda expression](../.././workflow/lang/expr.md):**[$1]**
- for[lambda expression](../.././workflow/lang/expr.md): for example:**func (progress: double) : double { return progress; }**Sometimes it is better to make the field accelerate at the first half and decelerate at the second half, such a function could be easily created using:
```
func (x: double): double
{
    if (x < 0.5) { return x * x * 2; } else { return 1 - (1 - x) * (1 - x) * 2; }
}
```


## Running an Animation

In the**\<Animation/\>**generated class, there is a**CreateAnimation(state, time)**function, which means the animation begins from the current state, and run towards**state**in**time**milliseconds.

Calling this function doesn't make the animation run, instead it returns an**(vl::)presentation::controls::IGuiAnimation^**object.**AddAnimation**and**KillAnimation**of**(vl::)presentation::controls::GuiInstanceRootObject**controls how the animation run. This class is the base class for all UI[root instances](../.././gacui/xmlres/instance/root_instance.md), so**AddAnimation**and**KillAnimation**, or**self.AddAnimation**and**self.KillAnimation**, are accessible in the XML.

**AddAnimation**adds an**IGuiAnimation**object to the UI object, and start the animation immediately.

**KillAnimation**stops an**IGuiAnimation**object that has been added to the UI object, and stop the animation immediately.

For animations that need to switch from state to state for multiple times, like a button when the mouse is moving in and out, there is a pattern to control the animation:
```
<ref.Members><![CDATA[
    var myAnimation : MyAnimation^ = new MyAnimation(initial_state);
    var lastAnimation : IGuiAnimation^ = null;
]]></ref.Members>
...
var newAnimation = myAnimation.CreateAnimation(newState, animationLengthInMilliseconds);
KillAnimation(lastAnimation);
lastAnimation = newAnimation;
AddAnimation(lastAnimation);
```
When the state needs to move to a new state when the animation is running, this piece of code stops the current running animation, and replace it with a new one. The new animation moves the current state from where it is to a new state, and it should look smooth.

**KillAnimation**could accept**null**, and it does nothing.

## Managing Multiple Animations

It is easy to copy the above pattern multiple times for each animation.

Please remember that, the created**lastAnimation**will change**myAnimation.Current.FIELD**when it is executing, so if multiple animation needs to run at the same time, multiple**lastAnimation**and**myAnimation**should be created at the same time. Please give them good names to make the code looks clear.

When a UI object is disposing, e.g. when the close button on a window is clicked, all animations will be shut down, and calling**AddAnimation**will result in an exception.

It is function idential to split all fields to multiple animation objects. But if multiple fields represent different parts of the same animation, do your best to make one animation for all these fields to increase the performance.

In[this tutorial project](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_animation/Resource.xml), a button is controlled by 4 colors, which changes in one animation because they represent different parts of the same animation. So only one**\<Animation/\>**and only one instance of it is needed for one button.

Here is how it looks like:


- Source code:[kb_animation](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_animation/Resource.xml)
- ![](https://gaclib.net/doc/gacui/kb_animation.gif)

