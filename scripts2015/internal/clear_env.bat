@echo off

call config.bat
if errorlevel 1 (
	exit /b 1
)

rem Setup environment
if exist "%TmpDir%" (
	echo       * Removing temporary directory...

	cd "%TmpDir%\.."

	rmdir /S /Q "%TmpDir%" >nul
	if exist "%TmpDir%" (
		rem Wait for a while for system to delete the files
		timeout 2 /NOBREAK >nul
	)
	if exist "%TmpDir%" (
		echo ERROR: Deleting the old temporary folder failed.
		exit /b 1
	)
)

if NOT "%1" == "/skip_create" (
	if exist "%OutputDir%" (
		echo       * Removing OutputDirectory...
		rmdir /S /Q "%OutputDir%" >nul
		if exist "%OutputDir%" (
			echo ERROR: Deleting the old output folder failed.
			exit /b 1
		)
	)

	mkdir "%TmpDir%"
	if not exist "%TmpDir%" (
		echo ERROR: Creating temporary folder failed.
		exit /b 1
	)

	mkdir "%OutputDir%"
	if not exist "%OutputDir%" (
		echo ERROR: Creating temporary folder failed.
		exit /b 1
	)

	mkdir "%TmpDir%\32bit"
	if not exist "%TmpDir%\32bit" (
		echo ERROR: Creating temporary 32bit folder failed.
		goto error
	)

	mkdir "%TmpDir%\64bit"
	if not exist "%TmpDir%\64bit" (
		echo ERROR: Creating temporary 64bit folder failed.
		goto error
	)
)

exit /b 0
