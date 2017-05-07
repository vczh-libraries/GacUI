setlocal enabledelayedexpansion

echo.^<Workflow^>
echo.  ^<^^![CDATA[
echo.    module imageloader;
echo.
echo.    using presentation::*;
echo.    using presentation::controls::*;
echo.    using presentation::controls::list::*;
echo.
echo.    func LoadListView^(listView : GuiListView*^) : void
echo.    {
echo.      var host = listView.RelatedControlHost;
echo.
for %%f in (*_16x16_*.png) do (
    set "name=%%f"
    set "id=!name:~4,-13!"
    echo.      {
    echo.        var item = new ListViewItem^^^(^);
    echo.        item.Text = "!id!";
    echo.        item.LargeImage = cast ^(GuiImageData^^^^^) host.ResolveResource^("res", "LargeImages/!id!"^);
    echo.        item.SmallImage = cast ^(GuiImageData^^^^^) host.ResolveResource^("res", "SmallImages/!id!"^);
    echo.        item.SubItems.Add^("!name:~0,3!"^);
    echo.        item.SubItems.Add^("!name:~-12,-7!"^);
    echo.        item.SubItems.Add^("!name!"^);
    echo.        listView.Items.Add^(item^);
    echo.      }
)
echo.    }
echo.
echo.  ]]^>
echo.^</Workflow^>