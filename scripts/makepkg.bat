@echo off

rem Mark the changes as local ones
setlocal

rem ---------------------------------------------------
rem Remember the current path
set CurrentDir=%CD%
set OutputDir=%CurrentDir%\out
set TmpDir=%OutputDir%\tmp
set MainProjectDir=%OutputDir%\tmp\copyhandler

rem ---------------------------------------------------
rem Setup environment
echo Initializing...

if exist %TmpDir% (
	rmdir /S /Q %TmpDir%
	if exist %TmpDir% (
		echo ERROR: Deleting the old temporary folder failed.
		goto end
	)
)

mkdir %TmpDir%
if not exist %TmpDir% (
	echo ERROR: Creating temporary folder failed.
	goto end
)

if exist %OutputDir% (
	rmdir /S /Q %OutputDir%
	if exist %OutputDir% (
		echo ERROR: Deleting the old output folder failed.
		goto end
	)
)

mkdir %OutputDir%
if not exist %OutputDir% (
	echo ERROR: Creating temporary folder failed.
	goto end
)

mkdir %TmpDir%\zip32
if not exist %TmpDir% (
	echo ERROR: Creating temporary zip32 folder failed.
	goto end
)

mkdir %TmpDir%\zip64
if not exist %TmpDir% (
	echo ERROR: Creating temporary zip64 folder failed.
	goto end
)

rem enter the temporary directory
cd %TmpDir%

rem ---------------------------------------------------
echo Exporting copyhandler from the repository...

svn export https://copyhandler.svn.sourceforge.net/svnroot/copyhandler/trunk copyhandler
if errorlevel 1 (
	echo ERROR: encountered a problem while exporting sources from repository.
	goto cleanup
)

cd %MainProjectDir%

rem ----------------------------------------------------
echo Preparing the source package
7z a -tzip %OutputDir%\chsrc.zip -x!scripts\*.bat .
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

cd %MainProjectDir%\bin\release

7z a -tzip %OutputDir%\ch_symbols.zip *.pdb
if errorlevel 1 (
	echo Could not create symbols archive.
	goto cleanup
)

echo Preparing the installer package
cd %MainProjectDir%
if not exist scripts (
	echo The scripts directory does not exist.
	goto cleanup
)

cd %MainProjectDir%\scripts

iscc setup32.iss /o%OutputDir%
if errorlevel 1 (
	echo Preparation of the installer version failed.
	goto cleanup
)

cd %MainProjectDir%

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

cd %MainProjectDir%\bin\release

7z a -tzip %OutputDir%\ch_symbols64.zip *64*.pdb
if errorlevel 1 (
	echo Could not create symbols archive.
	goto cleanup
)

echo Preparing the installer package
cd %MainProjectDir%
if not exist scripts (
	echo The scripts directory does not exist.
	goto cleanup
)

cd %MainProjectDir%\scripts

iscc setup64.iss /o%OutputDir%
if errorlevel 1 (
	echo Preparation of the installer version failed.
	goto cleanup
)

rem -------------
rem We are in scripts/ and going up
cd %MainProjectDir%

rem Prepare files
xcopy "bin\release\ch.exe" "%TmpDir%\zip32\"
xcopy "License.txt" "%TmpDir%\zip32\"
xcopy "bin\release\chext.dll" "%TmpDir%\zip32\"
xcopy "bin\release\libicpf32u.dll" "%TmpDir%\zip32\"
xcopy "bin\release\libchcore32u.dll" "%TmpDir%\zip32\"
xcopy "bin\release\libictranslate32u.dll" "%TmpDir%\zip32\"
xcopy "bin\release\ictranslate.exe" "%TmpDir%\zip32\"
xcopy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*" "%TmpDir%\zip32\"
xcopy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\*" "%TmpDir%\zip32\"
xcopy "C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x86\dbghelp.dll" "%TmpDir%\zip32\"
xcopy /E /I "bin\release\help" "%TmpDir%\zip32\help"
xcopy /E /I "bin\release\langs" "%TmpDir%\zip32\langs"

cd "%TmpDir%\zip32\"

7z a -tzip %OutputDir%\ch32.zip .
if errorlevel 1 (
	echo Could not create win32 zip archive.
	goto cleanup
)

rem ----------------------------------------------
cd %MainProjectDir%

xcopy "bin\release\ch64.exe" "%TmpDir%\zip64\"
xcopy "License.txt" "%TmpDir%\zip64\"
xcopy "bin\release\chext64.dll" "%TmpDir%\zip64\"
xcopy "bin\release\libicpf64u.dll" "%TmpDir%\zip64\"
xcopy "bin\release\libchcore64u.dll" "%TmpDir%\zip64\"
xcopy "bin\release\libictranslate64u.dll" "%TmpDir%\zip64\"
xcopy "bin\release\ictranslate64.exe" "%TmpDir%\zip64\"
xcopy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*" "%TmpDir%\zip64\"
xcopy "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.MFC\*" "%TmpDir%\zip64\"
xcopy "C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x64\dbghelp.dll" "%TmpDir%\zip64\"
xcopy /E /I "bin\release\help" "%TmpDir%\zip64\help"
xcopy /E /I "bin\release\langs" "%TmpDir%\zip64\langs"
 
cd "%TmpDir%\zip64\"

7z a -tzip %OutputDir%\ch64.zip .
if errorlevel 1 (
	echo Could not create win64 zip archive.
	goto cleanup
)

:cleanup
echo Cleaning up the temporary files...
cd "%TmpDir%"
cd ..
rmdir /S /Q "%TmpDir%"

goto end

:end
