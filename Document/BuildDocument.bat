rd /s /q .Output
mkdir .Output
cd .Output
mkdir Source
mkdir Tools
mkdir x86
mkdir x64
cd ..

MSBuild DocTools\DocTools.sln /p:Configuration=Release;OutputPath=..\..\.Output\Tools\
copy ..\Release\*.h .Output\Source\*.h
copy ..\Release\*.cpp .Output\Source\*.cpp
copy ..\Import\*.h .Output\Source\*.h
copy ..\Import\*.cpp .Output\Source\*.cpp

cl.exe Headers.h /I ".Output\Source" /D "WIN32" /D "_DEBUG" /D "WINDOWS" /D "_UNICODE" /D "UNICODE" /P /C
move Headers.i .Output\x86\Headers.txt
cd .Output\x86
call ..\..\ParseCppHeader.bat
cd ..\..

cl.exe Headers.h /I ".Output\Source" /D "_WIN64" /D "WIN32" /D "_DEBUG" /D "WINDOWS" /D "_UNICODE" /D "UNICODE" /P /C
move Headers.i .Output\x64\Headers.txt
cd .Output\x64
call ..\..\ParseCppHeader.bat
cd ..\..