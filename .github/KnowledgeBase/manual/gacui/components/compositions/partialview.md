# \<PartialView\>

**\<PartialView/\>**sticks its location and size to a ratio of the location and size of its parent composition.

**\<PartialView/\>**is not a**\<Bounds/\>**, there is no writable**ExpectedBounds**and**AlignmentToParent**in this composition.

**Margin**of this composition and**InternalMargin**of its parent composition are ignored.

One of the scenario for**\<PartialView/\>**is the moving handle of a scroll bar.

## \<PartialView\>::WidthRatio and \<PartialView\>::WidthPageSize

The default value of**WidthRatio**is**0.0**.

The default value of**WidthPageSize**is**1.0**.


- Value for**WidthRatio**should be in**[0, 1]**
- Value for**WidthPageSize**should be in**[0, 1]**
- **WidthRatio**+**WidthPageSize**should not exceed 1

These properties define the location and size of this composition in the following way:
- The distance between this composition to the left border of its parent composition is**WidthRatio*** "width of its parent composition".
- The width of this composition is**WidthPageSize*** "width of its parent composition".

## \<PartialView\>::HeightRatio and \<PartialView\>::HeightPageSize

The default value of**HeightRatio**is**0.0**.

The default value of**HeightPageSize**is**1.0**.


- Value for**HeightRatio**should be in**[0, 1]**
- Value for**HeightPageSize**should be in**[0, 1]**
- **HeightRatio**+**HeightPageSize**should not exceed 1

These properties define the location and size of this composition in the following way:
- The distance between this composition to the left border of its parent composition is**HeightRatio*** "height of its parent composition".
- The height of this composition is**HeightPageSize*** "height of its parent composition".

## Sample

Please check out[dark skin control templates](https://github.com/vczh-libraries/GacUI/blob/master/Source/Skins/DarkSkin/Template_Scroll.xml).

