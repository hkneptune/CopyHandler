@echo off

call config.bat
if errorlevel 1 (
	exit /b 1
)

rem Setup environment
if not exist "%TmpDir%" (
	mkdir "%TmpDir%"
	if not exist "%TmpDir%" (
		echo ERROR: Creating temporary folder failed.
		exit /b 1
	)
)

if not exist "%OutputDir%" (
	mkdir "%OutputDir%"
	if not exist "%OutputDir%" (
		echo ERROR: Creating temporary folder failed. See the log below:
		type "%TmpDir%\command.log"
		exit /b 1
	)
)

rem Prepare directories
if not exist "%TmpDir%\32bit" (
	mkdir "%TmpDir%\32bit"
	if not exist "%TmpDir%\32bit" (
		echo ERROR: Creating temporary 32bit folder failed.
		goto error
	)
)

if not exist "%TmpDir%\64bit" (
	mkdir "%TmpDir%\64bit"
	if not exist "%TmpDir%\64bit" (
		echo ERROR: Creating temporary 64bit folder failed.
		goto error
	)
)

exit /b 0
