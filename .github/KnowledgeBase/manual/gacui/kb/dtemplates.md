# Default Template Registrations

GacUI is released with a set of**DarkSkin**control templates, but it is you who is responsible to register them.

Registering**DarkSkin**is very easy, just write the following code in any cpp file, so that it will be executed before the application entry function (e.g.**WinMain**):
```
class DefaultSkinPlugin : public Object, public IGuiPlugin
{
public:
    GUI_PLUGIN_NAME(Custom_DefaultSkinPlugin)
    {
        GUI_PLUGIN_DEPEND(GacGen_DarkSkinResourceLoader);
    }

    void Load(bool, bool)override
    {
        RegisterTheme(Ptr(new darkskin::Theme));
    }

    void Unload(bool, bool)override
    {
    }
};
```
This piece of code register**darkskin::Theme**has a builder of all kinds of control templates after**GacGen_DarkSkinResourceLoader**is loaded. GacUI has a very simple and efficient plugin system, which load plugins according to dependencies between each other. If**DefaultSkinPlugin**is registered before the**GacGen_DarkSkinResourceLoader**, it will just hold it until**GacGen_DarkSkinResourceLoader**is registered.

Maybe someday you decide to create another set of control templates and want to make it the default one. It is very simple, just create an**\<Instance/\>**of**\<ThemeTemplates\>**like the one below, and modify the above plugin code to have the correct dependencies and theme types.

The**\<ThemeTemplates\>**of**DarkSkin**looks like this, which lists all control template type names for each kind of control. By specifying**ref.Class="darkskin::Theme"**, a C++ class**darkskin::Theme**is created, and now it is ready to register default control templates.

There is not default item templates in this list, because default item templates are specified in each control template for list controls.


```
<Instance ref.CodeBehind="false" ref.Class="darkskin::Theme" xmlns:x="darkskin::*">
  <ThemeTemplates Name="DarkSkin" PreferCustomFrameWindow="true">
    <att.SystemFrameWindow>x:SystemFrameWindowTemplate</att.SystemFrameWindow>
    <att.CustomFrameWindow>x:CustomFrameWindowTemplate</att.CustomFrameWindow>
    <att.CustomControl>x:CustomControlTemplate</att.CustomControl>
    <att.Tooltip>x:TooltipTemplate</att.Tooltip>
    <att.Label>x:LabelTemplate</att.Label>
    <att.ShortcutKey>x:ShortcutKeyTemplate</att.ShortcutKey>
    <att.ScrollView>x:ScrollViewTemplate</att.ScrollView>
    <att.GroupBox>x:GroupBoxTemplate</att.GroupBox>
    <att.Tab>x:TabTemplate</att.Tab>
    <att.ComboBox>x:ComboBoxTemplate</att.ComboBox>
    <att.MultilineTextBox>x:DocumentViewerTemplate</att.MultilineTextBox>
    <att.SinglelineTextBox>x:DocumentLabelTemplate</att.SinglelineTextBox>
    <att.DocumentViewer>x:DocumentViewerTemplate</att.DocumentViewer>
    <att.DocumentLabel>x:DocumentLabelTemplate</att.DocumentLabel>
    <att.DocumentTextBox>x:DocumentTextBoxTemplate</att.DocumentTextBox>
    <att.ListView>x:ListViewTemplate</att.ListView>
    <att.TreeView>x:TreeViewTemplate</att.TreeView>
    <att.TextList>x:TextListTemplate</att.TextList>
    <att.Menu>x:ToolstripMenuTemplate</att.Menu>
    <att.MenuBar>x:ToolstripTemplate</att.MenuBar>
    <att.MenuSplitter>x:MenuSplitterTemplate</att.MenuSplitter>
    <att.MenuBarButton>x:MenuBarButtonTemplate</att.MenuBarButton>
    <att.MenuItemButton>x:MenuItemButtonTemplate</att.MenuItemButton>
    <att.ToolstripToolBar>x:ToolstripTemplate</att.ToolstripToolBar>
    <att.ToolstripToolBarInMenu>x:ToolstripInMenuTemplate</att.ToolstripToolBarInMenu>
    <att.ToolstripButton>x:ToolstripButtonTemplate</att.ToolstripButton>
    <att.ToolstripDropdownButton>x:ToolstripDropdownButtonTemplate</att.ToolstripDropdownButton>
    <att.ToolstripSplitButton>x:ToolstripSplitButtonTemplate</att.ToolstripSplitButton>
    <att.ToolstripSplitter>x:ToolstripSplitterTemplate</att.ToolstripSplitter>
    <att.ToolstripSplitterInMenu>x:ToolstripSplitterInMenuTemplate</att.ToolstripSplitterInMenu>
    <att.Button>x:ButtonTemplate</att.Button>
    <att.CheckBox>x:CheckBoxTemplate</att.CheckBox>
    <att.RadioButton>x:RadioButtonTemplate</att.RadioButton>
    <att.DatePicker>x:DatePickerTemplate</att.DatePicker>
    <att.DateComboBox>x:ComboBoxTemplate</att.DateComboBox>
    <att.HScroll>x:HScrollTemplate</att.HScroll>
    <att.VScroll>x:VScrollTemplate</att.VScroll>
    <att.HTracker>x:HTrackerTemplate</att.HTracker>
    <att.VTracker>x:VTrackerTemplate</att.VTracker>
    <att.ProgressBar>x:ProgressBarTemplate</att.ProgressBar>
    <att.RibbonTab>x:TabTemplate</att.RibbonTab>
    <att.RibbonSmallButton>x:RibbonSmallButtonTemplate</att.RibbonSmallButton>
    <att.RibbonSmallDropdownButton>x:RibbonSmallDropdownButtonTemplate</att.RibbonSmallDropdownButton>
    <att.RibbonSmallSplitButton>x:RibbonSmallSplitButtonTemplate</att.RibbonSmallSplitButton>
    <att.RibbonLargeButton>x:RibbonLargeButtonTemplate</att.RibbonLargeButton>
    <att.RibbonLargeDropdownButton>x:RibbonLargeDropdownButtonTemplate</att.RibbonLargeDropdownButton>
    <att.RibbonLargeSplitButton>x:RibbonLargeSplitButtonTemplate</att.RibbonLargeSplitButton>
    <att.RibbonGroup>x:RibbonGroupTemplate</att.RibbonGroup>
    <att.RibbonGroupMenu>x:RibbonGroupMenuTemplate</att.RibbonGroupMenu>
    <att.RibbonSplitter>x:ToolstripSplitterTemplate</att.RibbonSplitter>
    <att.RibbonSmallIconLabel>x:RibbonSmallIconLabelTemplate</att.RibbonSmallIconLabel>
    <att.RibbonIconLabel>x:RibbonIconLabelTemplate</att.RibbonIconLabel>
    <att.RibbonButtons>x:RibbonButtonsTemplate</att.RibbonButtons>
    <att.RibbonToolstrips>x:RibbonToolstripsTemplate</att.RibbonToolstrips>
    <att.RibbonToolstripHeader>x:RibbonToolstripHeaderTemplate</att.RibbonToolstripHeader>
    <att.RibbonGallery>x:RibbonGalleryTemplate</att.RibbonGallery>
    <att.RibbonGalleryList>x:RibbonGalleryTemplate</att.RibbonGalleryList>
    <att.RibbonGalleryItemList>x:RibbonGalleryItemListTemplate</att.RibbonGalleryItemList>
    <att.RibbonToolstripMenu>x:RibbonToolstripMenuTemplate</att.RibbonToolstripMenu>
  </ThemeTemplates>
</Instance>
```


