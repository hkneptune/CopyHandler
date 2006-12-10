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

rem ---------------------------------------------------
echo Removing the libicpf template project from the solution
if not exist copyhandler\src\libicpf (
	echo The directory copyhandler\src does not exist in the project - too many changes in the project?
	goto cleanup
)

cd copyhandler\src
rmdir /S /Q libicpf
if exist libicpf (
	echo ERROR: Encountered some problems with removing libicpf directory.
	goto cleanup
)

rem ----------------------------------------------------
echo Exporting libicpf from the repository...

svn export http://gforge.draknet.sytes.net/svn/libicpf/trunk libicpf
if errorlevel 1 (
	echo ERROR: encountered a problem while exporting sources from repository.
	goto cleanup
)

cd ..

rem ----------------------------------------------------
echo Preparing the source package
zip -r %OutputDir%\chsrc.zip *
if errorlevel 1 (
	echo Preparation of the sources failed.
	goto cleanup
)

rem ----------------------------------------------------
echo Initializing Visual Studio 7.1 environment variables...
if not exist "%VS71COMNTOOLS%\vsvars32.bat" (
	echo Can't find the vsvars32.bat file.
	goto cleanup
) else (
	call "%VS71COMNTOOLS%\vsvars32.bat"
)

rem ---------------------------------------------------
echo Building the solution

devenv ch.sln /rebuild Release
if errorlevel 1 (
	echo Build process failed.
	goto cleanup
)

echo Preparing the zip package

if not exist bin\release (
	echo The bin\release directory does not exist.
	goto cleanup
)

cd bin\release

zip -r %OutputDir%\ch.zip *.exe *.dll help\ langs\ && cd ..\.. && zip %OutputDir%\ch.zip License.txt
if errorlevel 1 (
	echo Preparation of the zipped version failed.
	goto cleanup
)

rem --------------------------------------------------------
echo Preparing the installer package
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

copy ..\bin\ch.exe %OutputDir%\ch.exe

:cleanup
echo Cleaning up the temporary files...
cd %tmp%
rem rmdir /S /Q copyhandler

goto end

:end
