# \<Animation\>

After a **GacUI XML Resource** is converted to a resource binary: - This resource does not exist in the resource binary. - The equivalent Workflow script will be included in the Workflow assembly part in the resource binary. - If you use the one specified in **GacGenConfig/Cpp**, the Workflow assembly is also not included in the resource binary, because you are supposed to use the generated C++ code.

\<Animation/\> create a helper to create instances of **presentation::controls::IGuiAnimation^**. The created animation performs customizable transformations for selected fields in a selected class.

Typically a \<Animation/\> looks like this: ``` <Animation name="RESOURCE_NAME"> <Gradient ref.Class="ANIMATION_CLASS" Type="STATE_CLASS"> <Interpolation> <![CDATA[ DEFAULT_INTERPOLATION_FUNCTION ]]> </Interpolation> <Targets> <Target Name="FIELD_1"/> <Target Name="FIELD_2"> <Interpolation> <![CDATA[ INTERPOLATION_FUNCTION_FOR_THIS_FIELD ]]> </Interpolation> </Target> </Targets> </Gradient> </Animation> ``` It generates a class named **ANIMATION_CLASS**: ``` class ANIMATION_CLASS { prop Begin: STATE_CLASS^ {} prop End: STATE_CLASS^{} prop Current: STATE_CLASS^{} new (current: STATE_CLASS^) { ... } func CreateAnimation(target: STATE_CLASS^, time: UInt64): IGuiAnimation^ { ... } } ``` While **STATE_CLASS** must be an existing class, which could be created using \<Script/\>: ``` class STATE_CLASS { prop FIELD_1: TYPE{} prop FIELD_2: TYPE{} } ``` **TYPE** must be an integer type, a floating point number type, or a color type.

**IMPORTANT**: In order to perform data binding on **ANIMATION_CLASS::Current**, all fields in **STATE_CLASS** are recommended to be properties with events.

When creating **ANIMATION_CLASS**, an instance of **STATE_CLASS** is required to be the initial state.

When calling **CreateAnimation**, the current state becomes the begin state, the first argument becomes the end state, the second argument is the length of the animation in milliseconds. The returned animation object could be started or killed using **AddAnimation** or **KillAnimation** of the root object.

An interpolation function takes a **double** between 0 and 1, returns a **double** between 0 and 1. It could be a function name, a lambda expression, or any [Workflow expression](../.././workflow/lang/expr.md) that returns a function.

When the animation is playing, the argument is the current play time divided by the animation length.

The return value of the interpolation function is used to determine fields in the **Current** property.

[Here](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_animation/Resource.xml) is an example of \<Animation\>. It creates animations on the control template of radio buttons.

Here is the state class: ``` class RadioButtonTemplateState { prop Length : double = 0 {} prop Text : Color = {} {} prop Border : Color = {} {} prop Background : Color = {} {} ... } ``` In this class, six static functions are defined to return configuration for different button states.

Here is the animation: ``` <Animation name="RadioButtonTemplateAnimation"> <Gradient ref.Class="sample::RadioButtonTemplateAnimation" Type="sample::RadioButtonTemplateState"> <Interpolation><![CDATA[ func (x: double): double { if (x < 0.5) { return x * x * 2; } else { return 1 - (1 - x) * (1 - x) * 2; } } ]]></Interpolation> <Targets> <Target Name="Length"/> <Target Name="Text"/> <Target Name="Border"/> <Target Name="Background"/> </Targets> </Gradient> </Animation> ``` The interpolation function makes the animation run faster and faster in the first half, and run slower and slower in the second half. It does this by transfer the animation progress **x**, which is from 0 to 1, to another value that is 0 to 1. The image of its derivative looks like a mountain.

When the button state is changed, the custom member function **OnStateChanged** is called, which kills the ongoing animation and starts a new one.

When the animation is running, properties of **self.animationBuilder.Current** is changed. **self.animationBuilder** is a custom member field of type **sample::RadioButtonTemplateAnimation^**. Now we only need to bind sizes and colors to properties of **self.animationBuilder.Current**, and the UI will move when the animation is running.

Here is how it looks like:

- Source code: [kb_animation](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/kb_animation/Resource.xml) - ![](https://gaclib.net/doc/gacui/kb_animation.gif)

