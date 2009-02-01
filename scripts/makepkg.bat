@echo off

rem Mark the changes as local ones
setlocal

rem ---------------------------------------------------
rem Remember the current path
set OutputDir=%CD%

rem ---------------------------------------------------
echo Performing a cleanup...
rem cd %tmp%
if exist copyhandler (
	rmdir /S /Q copyhandler
	if exist copyhandler (
		echo ERROR: Deleting the old temporary copyhandler folder failed.
		goto end
	)
)

rem ---------------------------------------------------
echo Exporting copyhandler from the repository...

svn export https://copyhandler.svn.sourceforge.net/svnroot/copyhandler/trunk copyhandler
if errorlevel 1 (
	echo ERROR: encountered a problem while exporting sources from repository.
	goto cleanup
)

cd copyhandler

rem ----------------------------------------------------
echo Preparing the source package
zip -r %OutputDir%\chsrc.zip * -x scripts\*.bat
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
iscc setup32.iss /o%OutputDir%
if errorlevel 1 (
	echo Preparation of the installer version failed.
	goto cleanup
)

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

iscc setup64.iss /o%OutputDir%
if errorlevel 1 (
	echo Preparation of the installer version failed.
	goto cleanup
)

rem -------------
rem We are in scripts/
cd ..

rem 1st phase - loose files

zip -r -j -9 %OutputDir%\ch32.zip "bin\release\ch.exe" "License.txt" "bin\release\chext.dll" "bin\release\libicpf32u.dll" "bin\release\libchcore32u.dll" "bin\release\libictranslate32u.dll" "bin\release\ictranslate.exe" "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*" "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\*" "C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x86\dbghelp.dll"

if errorlevel 1 (
	echo Could not create win32 zip archive.
	goto cleanup
)

rem 2nd phase - help and langs
cd bin\release

zip -r -9 %OutputDir%\ch32.zip "help\*" "langs\*"
if errorlevel 1 (
	echo Could not create win32 zip archive.
	goto cleanup
)


rem ----------------------------------------------
cd ..\..

rem 1st phase - loose files

zip -r -j -9 %OutputDir%\ch64.zip "bin\release\ch64.exe" "License.txt" "bin\release\chext64.dll" "bin\release\libicpf64u.dll" "bin\release\libchcore64u.dll" "bin\release\libictranslate64u.dll" "bin\release\ictranslate64.exe" "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*" "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.MFC\*" "C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x64\dbghelp.dll"

if errorlevel 1 (
	echo Could not create win64 zip archive.
	goto cleanup
)

rem 2nd phase - help and langs
cd bin\release

zip -r -9 %OutputDir%\ch64.zip "help\*" "langs\*"

if errorlevel 1 (
	echo Could not create win64 zip archive.
	goto cleanup
)

cd ..\..

:cleanup
echo Cleaning up the temporary files...
cd %tmp%
rmdir /S /Q copyhandler

goto end

:end
