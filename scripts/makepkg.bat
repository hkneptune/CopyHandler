@echo off

rem Mark the changes as local ones
setlocal

rem ---------------------------------------------------
rem Remember the current path
set OutputDir=%CD%

rem ---------------------------------------------------
echo Performing a cleanup...
cd %tmp%
if exist copyhandler (
	rmdir /S /Q copyhandler
	if exist copyhandler (
		echo ERROR: Deleting the old temporary copyhandler folder failed.
		goto end
	)
)

rem ---------------------------------------------------
echo Exporting copyhandler from the repository...

svn export http://gforge.draknet.sytes.net/svn/copyhandler/trunk copyhandler
if errorlevel 1 (
	echo ERROR: encountered a problem while exporting sources from repository.
	goto cleanup
)

cd copyhandler

rem ----------------------------------------------------
echo Preparing the source package
zip -r %OutputDir%\chsrc.zip * -x scripts\makepkg.bat
if errorlevel 1 (
	echo Preparation of the sources failed.
	goto cleanup
)

rem ----------------------------------------------------
echo Initializing Visual Studio environment variables...
if not exist "%VS90COMNTOOLS%\vsvars32.bat" (
	echo Can't find the vsvars32.bat file.
	goto cleanup
) else (
	call "%VS90COMNTOOLS%\vsvars32.bat"
)

rem ---------------------------------------------------
echo Building the solution (Win32)
devenv ch.vc90.sln /rebuild "Release-Unicode|Win32"
if errorlevel 1 (
	echo Build process failed.
	goto cleanup
)

echo Preparing the zip packages (Win32)

if not exist bin\release (
	echo The bin\release directory does not exist.
	goto cleanup
)

cd bin\release

zip -r %OutputDir%\ch_symbols.zip *.pdb
if errorlevel 1 (
	echo Could not create symbols archive.
	goto cleanup
)

echo Preparing the installer package
cd ..\..
if not exist scripts (
	echo The scripts directory does not exist.
	goto cleanup
)

cd scripts

echo %CD%
dir
compil32 /cc setup32.iss
if errorlevel 1 (
	echo Preparation of the installer version failed.
	goto cleanup
)

if not exist ..\bin\chsetup32.exe (
	echo Cannot find the created setup file.
	goto cleanup
)

copy ..\bin\chsetup32.exe %OutputDir%\chsetup32.exe
cd ..

rem ---------------------------------------------------
echo Building the solution (x64)
devenv ch.vc90.sln /rebuild "Release-Unicode|x64"
if errorlevel 1 (
	echo Build process failed.
	goto cleanup
)

echo Preparing the zip packages (x64)

if not exist bin\release (
	echo The bin\release directory does not exist.
	goto cleanup
)

cd bin\release

zip -r %OutputDir%\ch_symbols64.zip *64*.pdb
if errorlevel 1 (
	echo Could not create symbols archive.
	goto cleanup
)

echo Preparing the installer package
cd ..\..
if not exist scripts (
	echo The scripts directory does not exist.
	goto cleanup
)

cd scripts

compil32 /cc setup64.iss
if errorlevel 1 (
	echo Preparation of the installer version failed.
	goto cleanup
)

if not exist ..\bin\chsetup64.exe (
	echo Cannot find the created setup file.
	goto cleanup
)

copy ..\bin\chsetup64.exe %OutputDir%\chsetup64.exe

:cleanup
echo Cleaning up the temporary files...
cd %tmp%
rmdir /S /Q copyhandler

goto end

:end
