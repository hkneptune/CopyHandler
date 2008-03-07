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
echo Building the solution
devenv ch.vc90.sln /rebuild "Release-Unicode|Win32"
if errorlevel 1 (
	echo Build process failed.
	goto cleanup
)

echo Preparing the zip packages

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

rem zip -r %OutputDir%\ch.zip *.exe *.dll help\ langs\ && cd ..\.. && zip %OutputDir%\ch.zip License.txt
rem if errorlevel 1 (
rem 	echo Preparation of the zipped version failed.
rem 	goto cleanup
rem )

rem --------------------------------------------------------
echo Preparing the installer package
cd ..\..
if not exist scripts (
	echo The scripts directory does not exist.
	goto cleanup
)

cd scripts

compil32 /cc setup.iss
if errorlevel 1 (
	echo Preparation of the installer version failed.
	goto cleanup
)

if not exist ..\bin\ch.exe (
	echo Cannot find the created setup file.
	goto cleanup
)

copy ..\bin\ch.exe %OutputDir%\chsetup.exe

:cleanup
echo Cleaning up the temporary files...
cd %tmp%
rmdir /S /Q copyhandler

goto end

:end
