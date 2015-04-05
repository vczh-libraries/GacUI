rd /s /q .Output
mkdir .Output
cd .Output
mkdir Source
mkdir Tools
mkdir x86
mkdir x64
cd ..
MSBuild RemoveSystemInclude\RemoveSystemInclude\RemoveSystemInclude.csproj /p:Configuration=Release;OutputPath=..\..\.Output\Tools\
MSBuild RemoveEmptyLines\RemoveEmptyLines\RemoveEmptyLines.csproj /p:Configuration=Release;OutputPath=..\..\.Output\Tools\
MSBuild Tokenizer\Tokenizer\Tokenizer.csproj /p:Configuration=Release;OutputPath=..\..\.Output\Tools\
MSBuild Parser\Parser\Parser.csproj /p:Configuration=Release;OutputPath=..\..\.Output\Tools\
copy ..\Release\*.h .Output\Source\*.h
copy ..\Release\*.cpp .Output\Source\*.cpp
copy ..\Import\*.h .Output\Source\*.h
copy ..\Import\*.cpp .Output\Source\*.cpp
.Output\Tools\RemoveSystemInclude .Output\Source\Vlpp.h
.Output\Tools\RemoveSystemInclude .Output\Source\Vlpp.cpp
.Output\Tools\RemoveSystemInclude .Output\Source\VlppWorkflow.h
.Output\Tools\RemoveSystemInclude .Output\Source\VlppWorkflow.cpp
.Output\Tools\RemoveSystemInclude .Output\Source\GacUI.h
.Output\Tools\RemoveSystemInclude .Output\Source\GacUI.cpp
.Output\Tools\RemoveSystemInclude .Output\Source\GacUIReflection.h
.Output\Tools\RemoveSystemInclude .Output\Source\GacUIReflection.cpp
.Output\Tools\RemoveSystemInclude .Output\Source\GacUIWindows.h
.Output\Tools\RemoveSystemInclude .Output\Source\GacUIWindows.cpp

cl.exe Headers.h /I ".Output\Source" /D "WIN32" /D "_DEBUG" /D "WINDOWS" /D "_UNICODE" /D "UNICODE" /P /EP /C
move Headers.i .Output\x86\Headers.txt
cd .Output\x86
call ..\..\BuildDocumentForSinglePlatform.bat
cd ..\..

cl.exe Headers.h /I ".Output\Source" /D "_WIN64" /D "WIN32" /D "_DEBUG" /D "WINDOWS" /D "_UNICODE" /D "UNICODE" /P /EP /C
move Headers.i .Output\x64\Headers.txt
cd .Output\x64
call ..\..\BuildDocumentForSinglePlatform.bat
cd ..\..