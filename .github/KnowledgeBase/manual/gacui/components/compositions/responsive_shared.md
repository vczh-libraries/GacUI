# \<ResponsiveShared\>

In **\<ResponsiveView/\>**, sometimes when it switch to a different view, the content is totaly changed by switching from one **GuiResponsiveCompositionBase** to another **GuiResponsiveCompositionBase** in **Views**. If a control with important status needs to be shared between different views, **\<ResponsiveShared/\>** could help.

This kind of scenarios are very common. For example, different views may share a text box. If different text box controls are used in different view to represent the same input data, users will be confused like the selected area is gone just because of changing the window size.

## \<ResponsiveView\>::SharedControls

All controls need to share between views must be created in the **SharedControls** collection property.

In order to make these controls being able to use in **\<ResponsiveShared/\>**, usually a control name needs to be specified in the [ref.Name](../../.././gacui/xmlres/tag_instance.md) attribute for each control.

When the **\<ResponsiveView/\>** is deleted, all controls in **SharedControls** will also be deleted.

## \<ResponsiveShared\>::Shared

For any control that needs to appear in a specific location in a view, a **\<ResponsiveShared/\>** is used as a place holder. It is a **\<Bounds/\>** so all necessary properties are available just like a control's **BoundsComposition**.

In this composition, the control to appear is specified using the **Shared** property, usually using a [-ref](../../.././gacui/xmlres/instance/properties.md) binding.

When a **GuiResponsiveCompositionBase** in **Views** is selected, all containing **\<ResponsiveShared/\>** in this composition will get a notice to make **Shared.BoundsComposition** become a child composition, with **Shared.BoundsComposition.AlignmentToParent** set to **left:0 top:0 right:0 bottom:0**. After that, all shared controls are moved to their place holders.

