@echo off

rem Mark the changes as local ones
setlocal

if [%1] == [] (
	echo Usage: symsrv_add_release.bat ^<symbol_server_dir^> [dir_with_packages]
	goto end
)

echo --- Initializing -----------------------------------------------
echo    * Reading configuration...
call config.bat
if errorlevel 1 (
	goto error
)

SET PackagesDir=%OutputDir%
if NOT [%2] == [] (
	SET PackagesDir=%2
)

echo --- Preparing files --------------------------------------------
echo    * Scanning directory %PackagesDir% for packages...
for /R %PackagesDir% %%f in (*.zip) do (
	echo    * Processing package %%f...
	call symsrv_add_single_package.bat "%%f" "%1"
	if errorlevel 1 (
		goto error
	)
)

echo    * Done

goto end

:error
	echo ERROR: encountered an error while processing packages.
	
:end
