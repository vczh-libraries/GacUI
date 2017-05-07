cls
@echo off
setlocal enabledelayedexpansion

echo.^<Folder^>> LargeImages.xml
echo.^<Folder^>> SmallImages.xml
for %%f in (*_16x16_*.png) do (
    set "name=%%f"
    set "prefix=!name:~0,4!"
    set "id=!name:~4,-13!"
    echo.  ^<Image name="!id!" content="File"^>!prefix!!id!_32x32_72.png^</Image^>>> LargeImages.xml
    echo.  ^<Image name="!id!" content="File"^>!prefix!!id!_16x16_72.png^</Image^>>> SmallImages.xml
)
echo.^</Folder^>>> LargeImages.xml
echo.^</Folder^>>> SmallImages.xml

echo.^<Workflow^>> ImageLoader.xml
echo.  ^<^^![CDATA[>> ImageLoader.xml
echo.    module imageloader;>> ImageLoader.xml
echo.>> ImageLoader.xml
echo.    using presentation::*;>> ImageLoader.xml
echo.    using presentation::controls::*;>> ImageLoader.xml
echo.    using presentation::controls::list::*;>> ImageLoader.xml
echo.>> ImageLoader.xml
echo.    func LoadListView^(listView : GuiListView*^) : void>> ImageLoader.xml
echo.    {>> ImageLoader.xml
echo.      var host = listView.RelatedControlHost;>> ImageLoader.xml
echo.>> ImageLoader.xml
for %%f in (*_16x16_*.png) do (
    set "name=%%f"
    set "id=!name:~4,-13!"
    echo.      {>> ImageLoader.xml
    echo.        var item = new ListViewItem^^^(^);>> ImageLoader.xml
    echo.        item.Text = "!id!";>> ImageLoader.xml
    echo.        item.LargeImage = cast ^(GuiImageData^^^^^) host.ResolveResource^("res", "LargeImages/!id!"^);>> ImageLoader.xml
    echo.        item.SmallImage = cast ^(GuiImageData^^^^^) host.ResolveResource^("res", "SmallImages/!id!"^);>> ImageLoader.xml
    echo.        item.SubItems.Add^("!name:~0,4!"^);>> ImageLoader.xml
    echo.        item.SubItems.Add^("!name:~-12,-7!"^);>> ImageLoader.xml
    echo.        item.SubItems.Add^("!name!"^);>> ImageLoader.xml
    echo.        listView.Items.Add^(item^);>> ImageLoader.xml
    echo.      }>> ImageLoader.xml
)
echo.    }>> ImageLoader.xml
echo.>> ImageLoader.xml
echo.  ]]^>>> ImageLoader.xml
echo.^</Workflow^>>> ImageLoader.xml