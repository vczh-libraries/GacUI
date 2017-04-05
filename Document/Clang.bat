cls
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\ClangC2\bin\x86\clang.exe" -I.Output\Source -x c++-header -D "_DEBUG" -D "_WINDOWS" -D "_UNICODE" -D "UNICODE" -D "abstract"= -w -E -o .Output\clang.i
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\ClangC2\bin\x86\clang.exe" -I.Output\Source -x c++-header -D "_DEBUG" -D "_WINDOWS" -D "_UNICODE" -D "UNICODE" -D "abstract"= -w -Xclang -ast-dump Headers.h > .Output\clang.txt
"Clang\Preprocessor\bin\Debug\Preprocessor.exe" .Output\Source .Output\clang.txt .Output\clang.clean.txt