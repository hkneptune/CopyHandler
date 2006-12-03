@echo off
md ..\..\out\LDK
md ..\..\out\LDK\res
md ..\..\out\LDK\res\help
del ..\..\out\LDK\*.* /F /Q
del ..\..\out\LDK\res\*.* /F /Q
del ..\..\out\LDK\res\Help\*.* /F /Q

rem copy the current language
copy ..\Langs\English.lng ..\..\out\LDK\res\program.lng

rem help files
copy ..\Help\English\* ..\..\out\LDK\res\help\

rem build the documentation and copy
hhc "..\SDK\CH SDK.hhp" >nul
copy "..\SDK\CH SDK.chm" ..\..\out\LDK\

pause