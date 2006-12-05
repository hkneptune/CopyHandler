@echo off
echo ------------------------------------------------
echo Building html help files
echo.

rem %1 is a string with a destination path
if "%1" == "" goto default
if "%2" == "" goto default
set srcpath=%1
set dstpath=%2

goto do

:default
set srcpath=.
set dstpath=.\compiled

:do
call %srcpath%\one_bld.bat %srcpath% English %dstpath%
call %srcpath%\one_bld.bat %srcpath% Polish %dstpath%
goto end

:error
echo.
echo Missing parameter.
echo Usage: build.bat help_base_path destination_path
echo.
echo.

:end
pause