# \<DatePicker\>

- **\<DatePicker/\>** - **C++/Workflow**: (vl::)presentation::controls::GuiDatePicker* - **Template Tag**: \<DatePickerTemplate/\> - **Template Name**: DatePicker

**\<DatePicker/\>** is a control that looks like a calendar. In the default control templates, year and month can be chosen in two combo box controls. When a specific month is selected, a calender will display days in their correct positions.

A calendar-like control template is not easy to implement in GacUI XML Resource for **\<DatePicker/\>**, so **\<CommonDatePickerLook/\>** is provided for writing such control templates. Please check out [ the default control templates ](https://github.com/vczh-libraries/GacUI/blob/master/Source/Skins/DarkSkin/Template_Misc.xml) for details.

## \<DatePicker\> Properties

### Date (DateChanged)

The default value is the today provided by the OS.

### DateFormat (DateFormatChanged)

The default value is first value in ** Locale::UserDefault() `missing document: /vlppos/ref/VL__LOCALE__USERDEFAULT@__VL__LOCALE().html`. GetLongDateFormats() `missing document: /vlppos/ref/VL__LOCALE__GETLONGDATEFORMATS@VOID(__VL__COLLECTIONS__LIST___VL.html`**.

A valid value for this property must satisfy FormatDate`missing document: /vlppos/ref/VL__LOCALE__FORMATDATE@__VL__WSTRING(__VL__WSTRING_CONST_&,___VL.html` of the current **DateLocale** value.

### DateLocale (DateLocaleChanged)

The default value is first value in Locale::UserDefault()`missing document: /vlppos/ref/VL__LOCALE__USERDEFAULT@__VL__LOCALE().html`.

A valid value for this property must be either: - Locale::Locale()`missing document: /vlppos/ref/VL__LOCALE__$__CTOR@(__VL__WSTRING_CONST_&).html` - Locale::UserDefault()`missing document: /vlppos/ref/VL__LOCALE__USERDEFAULT@__VL__LOCALE().html` - Locale::SystemDefault()`missing document: /vlppos/ref/VL__LOCALE__SYSTEMDEFAULT@__VL__LOCALE().html` - Any values in Locale::Enumerate()`missing document: /vlppos/ref/VL__LOCALE__ENUMERATE@VOID(__VL__COLLECTIONS__LIST___VL__LOCALE_.html`

When **DateLocale** is changed, **DateFormat** will also be changed.

#### Sample

- Source code: [control_basic_datepicker](https://github.com/vczh-libraries/Release/blob/master/SampleForDoc/GacUI/XmlRes/control_basic_datepicker/Resource.xml) - ![](https://gaclib.net/doc/gacui/control_basic_datepicker.gif)

### Text

This is a read-only property, calling **SetText** does nothing.

The value is **DateLocale.FormatDate(Date, DateFormat)**.

## \<DatePicker\> Events

### DateChanged

This event raises when **Date** is changed.

In the default control templates, **Date** will also be changed when **DateNavigated** or **DateSelected** raises.

### DateNavigated

In the default control template, this event raises when users choose a different value in the year or month combo box.

### DateSelected

In the default control template, this event raises when users choose a specific day from the calendar.

