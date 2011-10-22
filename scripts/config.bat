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

rem NOTE: Strange construct here because of strange behaviour of %ProgramFiles(x86)% which loses last parenthesis otherwise
if "%ProgramFiles(x86)%" == "" SET VSInstallDirX86=%ProgramFiles%\Microsoft Visual Studio 9.0
if NOT "%ProgramFiles(x86)%" == "" SET VSInstallDirX86=%ProgramFiles(x86)%\Microsoft Visual Studio 9.0

if not exist "%ScriptDir%\config.bat" (
	echo ERROR: This script needs to be called from its directory.
	exit /b 1
)

exit /b 0
