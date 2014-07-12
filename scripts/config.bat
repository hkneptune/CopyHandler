@echo off
SET ReposCH=https://copyhandler.svn.sourceforge.net/svnroot/copyhandler
SET ReposIcpf=https://libicpf.svn.sourceforge.net/svnroot/libicpf
SET ReposIctranslate=https://libictranslate.svn.sourceforge.net/svnroot/libictranslate

SET CurrentDir=%CD%
SET ScriptDir=%CurrentDir%
SET CHRootDir=%CurrentDir%\..
SET OutputDir=%CurrentDir%\out
SET TmpDir=%CurrentDir%\tmp

SET VSInstallDirX64=%ProgramFiles%\Microsoft Visual Studio 9.0
SET ProgramFilesX86=%ProgramFiles%

rem NOTE: Strange construct here because of strange behaviour of %ProgramFiles(x86)% which loses last parenthesis otherwise
if "%ProgramFiles(x86)%" == "" SET VSInstallDirX86=%ProgramFiles%\Microsoft Visual Studio 9.0
if NOT "%ProgramFiles(x86)%" == "" SET VSInstallDirX86=%ProgramFiles(x86)%\Microsoft Visual Studio 9.0
if NOT "%ProgramFiles(x86)%" == "" SET ProgramFilesX86=%ProgramFiles(x86)%

if not exist "%ScriptDir%\config.bat" (
	echo ERROR: This script needs to be called from its directory.
	exit /b 1
)

rem Detect 7-zip location
SET SEVENZIPEXE=
for %%X in (7z.exe) do (set SEVENZIPEXE=%%~$PATH:X)
if "%SEVENZIPEXE%" == "" set SEVENZIPEXE=%ProgramFiles%\7-Zip\7z.exe
if not exist "%SEVENZIPEXE%" (
	echo 7-zip executable is not in the PATH nor at its default location. Please install 7-zip.
	exit /b 1
)

rem Detect inno setup compiler
SET ISCCEXE=
for %%X in (iscc.exe) do (set ISCCEXE=%%~$PATH:X)
if "%ISCCEXE%" == "" set ISCCEXE=%ProgramFilesX86%\Inno Setup 5\iscc.exe
if not exist "%ISCCEXE%" (
	echo Inno setup compiler executable is not in the PATH nor at its default location. Please install Inno Setup.
	exit /b 1
)

exit /b 0
