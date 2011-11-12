@echo off

rem Mark the changes as local ones
setlocal

if [%1] == [] (
	echo "Usage: symsrv.bat package_path symsrv_path"
	goto end
)

if [%2] == [] (
	echo "Usage: symsrv.bat package_path symsrv_path"
	goto end
)

echo --- Initializing -----------------------------------------------
echo    * Reading configuration...
call config.bat
if errorlevel 1 (
	goto error
)
echo    * Preparing environment...
call internal\prepare_env.bat
if errorlevel 1 (
	goto error
)

echo --- Processing files --------------------------------------------
echo    * Extracting files...

SET OutDir=%TmpDir%\symbols
if exist "%OutDir%" (
	rmdir /S /Q "%OutDir%" >nul
	if exist "%OutDir%" (
		echo ERROR: Deleting the temporary folder failed.
		exit /b 1
	)
)

mkdir "%OutDir%"
if errorlevel 1 (
	echo ERROR: Creating temporary directory failed.
	goto error
)

rem Unpack archive
7z x -o%OutDir% %1 >nul
if errorlevel 1 (
	echo ERROR: Unpacking archive failed.
	goto error
)

echo    * Adding files to symbol server directory...

symstore add /r /f "%OutDir%" /s "%~2" /t "Copy Handler" /v "%~1"
if errorlevel 1 (
	echo ERROR: Storing symbols failed.
	goto error
)

echo    * Cleaning up files...
rmdir /S /Q "%OutDir%"

echo    * Done

goto end

:error
echo    * Error processing files.

:end
