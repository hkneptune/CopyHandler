@echo off
echo Copying program, languages and help systems to output directory.
md ..\..\out\prog
md ..\..\out\prog\langs
md ..\..\out\prog\help

del ..\..\out\prog\*.* /F /Q
del ..\..\out\prog\langs\*.* /F /Q
del ..\..\out\prog\help\*.* /F /Q

rem copy the program
copy ..\..\bin\release\* ..\..\out\prog\

rem copy langs
copy ..\langs\* ..\..\out\prog\langs\

rem copy help systems
copy ..\help\compiled\* ..\..\out\prog\help\

pause