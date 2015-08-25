rd /s /q .Output
mkdir .Output
cd .Output
mkdir Source
mkdir x86
mkdir x64
mkdir Index
cd ..

copy ..\Release\*.h .Output\Source\*.h
copy ..\Release\*.cpp .Output\Source\*.cpp
copy ..\Import\*.h .Output\Source\*.h
copy ..\Import\*.cpp .Output\Source\*.cpp

cl.exe Headers.h /I ".Output\Source" /D "WIN32" /D "_DEBUG" /D "WINDOWS" /D "_UNICODE" /D "UNICODE" /P /C
move Headers.i .Output\x86\Headers.txt
cd .Output\x86
call ..\..\ParseCppHeader.bat
cd ..\..
copy .Output\x86\Headers.ast.xml .Output\x86.ast.xml

cl.exe Headers.h /I ".Output\Source" /D "_WIN64" /D "WIN32" /D "_DEBUG" /D "WINDOWS" /D "_UNICODE" /D "UNICODE" /P /C
move Headers.i .Output\x64\Headers.txt
cd .Output\x64
call ..\..\ParseCppHeader.bat
cd ..\..
copy .Output\x64\Headers.ast.xml .Output\x64.ast.xml

cd .Output
..\..\..\Tools\Tools\DocResolver "x86, Windows" x86.ast.xml "x64, Windows" x64.ast.xml Resolved.ast.xml
..\..\..\Tools\Tools\DocIndex Resolved.ast.xml Index
cd ..