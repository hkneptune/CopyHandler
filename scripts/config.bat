@echo off
SET ReposCH=https://svn.copyhandler.com/svn/copyhandler
SET ReposIcpf=https://svn.copyhandler.com/svn/libicpf
SET ReposIctranslate=https://svn.copyhandler.com/svn/libictranslate

SET CurrentDir=%CD%
SET ScriptDir=%CurrentDir%
SET CHRootDir=%CurrentDir%\..
SET OutputDir=%CurrentDir%\out
SET TmpDir=%CurrentDir%\tmp

SET VSInstallDirX64=%ProgramFiles%\Microsoft Visual Studio 12.0
SET ProgramFilesX86=%ProgramFiles%

SET LC_MESSAGES=en_EN

rem NOTE: Strange construct here because of strange behaviour of %ProgramFiles(x86)% which loses last parenthesis otherwise
if "%ProgramFiles(x86)%" == "" SET VSInstallDirX86=%ProgramFiles%\Microsoft Visual Studio 12.0
if NOT "%ProgramFiles(x86)%" == "" SET VSInstallDirX86=%ProgramFiles(x86)%\Microsoft Visual Studio 12.0
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

SET SVNINDEXCMD=
for %%X in (svnindex.cmd) do (set SVNINDEXCMD=%%~$PATH:X)
if "%SVNINDEXCMD%" == "" set SVNINDEXCMD=%ProgramFiles%\Debugging Tools for Windows (x64)\srcsrv\svnindex.cmd
if not exist "%SVNINDEXCMD%" (
	echo svnindex.cmd not found in PATH environment variable nor in its default location. Please install Debugging tools for Windows.
	exit /b 1
)

exit /b 0
