pushd %~dp0
del *.xml
copy ..\..\..\Test\GacUISrc\Host\Resources\DarkSkin\*.xml .
..\..\..\..\Tools\Tools\GacGen Resource.xml
popd