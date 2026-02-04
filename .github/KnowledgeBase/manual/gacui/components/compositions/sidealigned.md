# \<SideAligned\>

**\<SideAligned/\>**sticks itself to a border of the parent composition.

**\<SideAligned/\>**is not a**\<Bounds/\>**, there is no writable**ExpectedBounds**and**AlignmentToParent**in this composition.

**Margin**of this composition and**InternalMargin**of its parent composition are ignored.

One of the scenario for**\<SideAligned/\>**is the arrow button of a scroll bar.

## \<SlideAlign\>::Direction

The default value for**Direction**is**Top**. The following are valid values for this property:
- **Left**: This makes the composition behaves like its**AlignmentToParent**is**left:0 top:0 right:-1 bottom:0**. Its width is decided by**MaxLength**,**MaxRatio**and the size of its parent composition.
- **Top**: This makes the composition behaves like its**AlignmentToParent**is**left:0 top:0 right:0 bottom:-1**. Its height is decided by**MaxLength**,**MaxRatio**and the size of its parent composition.
- **Right**: This makes the composition behaves like its**AlignmentToParent**is**left:-1 top:0 right:0 bottom:0**. Its width is decided by**MaxLength**,**MaxRatio**and the size of its parent composition.
- **Bottom**: This makes the composition behaves like its**AlignmentToParent**is**left:0 top:-1 right:0 bottom:0**. Its height is decided by**MaxLength**,**MaxRatio**and the size of its parent composition.

## \<SlideAlign\>::MaxLength and \<SlideAlign\>::MaxRatio

The default value for**MaxLength**is**10**.

The default value for**MaxRatio**is**1.0**. Value for this property should be in**[0, 1]**.

When**Direction**is**Left**or**Right**:
- let X be**MaxLength**.
- let Y be**MaxRatio*** "width of the parent composition".The width of this composition becomes**min(X, Y)**.

When**Direction**is**Top**or**Bottom**:
- let X be**MaxLength**.
- let Y be**MaxRatio*** "height of the parent composition".The height of this composition becomes**min(X, Y)**.

## Sample

Please check out[dark skin control templates](https://github.com/vczh-libraries/GacUI/blob/master/Source/Skins/DarkSkin/Template_Scroll.xml).

