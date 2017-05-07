setlocal enabledelayedexpansion

echo.^<Folder^>
for %%f in (*_16x16_*.png) do (
    set "name=%%f"
    set "prefix=!name:~0,4!"
    set "id=!name:~4,-13!"
    echo.  ^<Image name="!id!" content="File"^>!prefix!!id!%1^</Image^>
)
echo.^</Folder^>