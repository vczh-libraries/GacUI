# \<SharedSizeRoot\> and \<SharedSizeItem\>

**\<SharedSizeRoot/\>**synchronizes**\<SharedSizeItem/\>**in it to have the same widths or heights.

**\<SharedSizeItem/\>**doesn't have to be a direct child of**\<SharedSizeRoot/\>**.

One of a useful secenario is to create control templates for menu items.**\<SharedSizeRoot/\>**is put in the container for menu items,**\<SharedSizeItem/\>**is put in the control template for each menu item. When menu items are added in the container, All**\<SharedSizeItem/\>**becomes (indirect) child compositions of the**\<SharedSizeRoot/\>**, and then widths for names and shortcut keys are synchronized.

This is why menu items are aligned like a table even when each pair of name and shortcut key are in separate controls.

## \<SharedSizeItem\>::Group

The default value is**an empty string**.

Only**\<SharedSizeItem/\>**in the same**Group**synchronizes size between each other. Two**\<SharedSizeItem/\>**in different**Group**do not affect each other.

If this**\<SharedSizeItem/\>**is not a child composition of a**\<SharedSizeRoot/\>**, sizes are not synchronized with any others.

If this**\<SharedSizeItem/\>**is a child composition of multiple**\<SharedSizeRoot/\>**and**\<SharedSizeItem/\>**,
- If the nearest parent composition amnong them is a**\<SharedSizeItem/\>**, sizes are not synchronized with any others.
- If the nearest parent composition amnong them is a**\<SharedSizeRoot/\>**, sizes are only synchronized with other valid**\<SharedSizeItem/\>**in this**\<SharedSizeRoot/\>**.

## \<SharedSizeItem\>::SharedWidth

The default value is**false**.

If**SharedWidth**is set to true, this**\<SharedSizeItem/\>**synchronizes its width with other**\<SharedSizeItem/\>**in the same**Group**.

## \<SharedSizeItem\>::SharedHeight

The default value is**false**.

If**SharedWidth**is set to true, this**\<SharedSizeItem/\>**synchronizes its height with other**\<SharedSizeItem/\>**in the same**Group**.

## Sample

Please check out the demo for[<SharedSizeRoot/>](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/RepeatTabPage.xml)and[<SharedSizeItem/>](https://github.com/vczh-libraries/Release/blob/master/Tutorial/GacUI_ControlTemplate/BlackSkin/UI/FullControlTest/RepeatComponents.xml).

