rem @echo off
echo ----

if "%1" == "" goto error
if "%2" == "" goto error
if "%3" == "" goto error

echo.

copy %1\HTMLDefines.h %1\%2\HTMLDefines.h
hhc %1\%2\help.hhp >nul
move %1\%2\*.chm %3\
goto end

:error
echo one_bld.bat - missing parameters (usage: one_bld.bat helps_base_path help_folder_name dest_folder)

:end