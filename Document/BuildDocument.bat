mkdir .Output
cd .Output
del *.* /Q
cd ..
cl.exe Headers.h /I "..\Import" /I "..\Release" /D "WIN32" /D "_DEBUG" /D "WINDOWS" /D "_UNICODE" /D "UNICODE" /P /C
move Headers.i .Output\Headers.i