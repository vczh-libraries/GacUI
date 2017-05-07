cls
@echo off
call Build_WriteFolder.bat _32x32_72.png > LargeImages.xml
call Build_WriteFolder.bat _16x16_72.png > SmallImages.xml
call Build_WriteImageLoader.bat > ImageLoader.xml