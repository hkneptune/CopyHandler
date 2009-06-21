@echo off
SET ReposCH=https://copyhandler.svn.sourceforge.net/svnroot/copyhandler
SET ReposIcpf=https://libicpf.svn.sourceforge.net/svnroot/libicpf
SET ReposIctranslate=https://libictranslate.svn.sourceforge.net/svnroot/libictranslate

SET CurrentDir=%CD%
SET ScriptDir=%CurrentDir%
SET OutputDir=%CurrentDir%\out
SET TmpDir=%CurrentDir%\tmp
SET MainProjectDir=%TmpDir%\copyhandler

if not exist "%ScriptDir%\make_package.bat" (
	echo ERROR: This script needs to be called from its directory.
	exit /b 1
)

exit /b 0
