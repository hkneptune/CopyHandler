@echo off

rem Script prepares all packages based on the source code in which this script is placed.

rem Mark the changes as local ones
setlocal ENABLEDELAYEDEXPANSION

echo --- Initializing ----------------------------------------------------
call config.bat
if errorlevel 1 (
	exit /b 1
)

echo    * Clearing environment...
call internal\clear_env.bat
if errorlevel 1 (
	goto error
)

echo    * Initializing MSBuild environment
if not exist "%VS140COMNTOOLS%\VsMSBuildCmd.bat" (
	echo ERROR: Can't find the vsvars32.bat file.
	goto error
) else (
	call "%VS140COMNTOOLS%\VsMSBuildCmd.bat" >nul
)

set MainProjectDir=%CHRootDir%

rem Update the version string in the version.h for trunk
echo    * Detecting current version information...
call internal\detect_internal_version.bat "%MainProjectDir%"
if errorlevel 1 (
	goto error
)

if "%CHReleaseType%" == "internal" (
	echo    * Updating version information in version.h for the internal release...
	cscript //NoLogo internal\replace_version.vbs "%MainProjectDir%\src\common\version.h.template" "%MainProjectDir%\src\common\version.h" !CHMajorVersion! !CHMinorVersion! !CHSVNVersion! !CHCustomVersion! !CHTextVersion! >"%TmpDir%\command.log"
	if errorlevel 1 (
		echo ERROR: encountered a problem while updating version information. See the log below:
		type "%TmpDir%\command.log"
		goto error
	)
)

echo --- Building solutions ----------------------------------------------
rem cd "%MainProjectDir%"
echo    * Building release...
msbuild "..\msbuild.proj" /t:Release /m:4 >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Build process failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo --- Verifying build -------------------------------------------------
cd %MainProjectDir%
if not exist bin\release (
	echo ERROR: The bin\release directory does not exist.
	goto error
)

cd %MainProjectDir%\scripts

SET FoundWrongManifest=0
for %%v in (%MainProjectDir%\bin\release\*.dll %MainProjectDir%\bin\release\*.exe) do (
	echo    * Verifying %%~nv%%~xv...
 	call internal\detect_incorrect_manifest.bat "%%v"
	if errorlevel 1 (
    SET FoundWrongManifest=1
    )
)

if "!FoundWrongManifest!" == "1" (
	echo ERROR: Incorrect manifest detected in one or more executables.
	goto error
)

if NOT "%SKIPCHSIGNING%" == "1" (
	echo --- Signing executables ---------------------------------------------
	signtool sign /t http://time.certum.pl /a "%MainProjectDir%\bin\release\*.dll" "%MainProjectDir%\bin\release\*.exe" 2>"%TmpDir%\command-err.log"
	if errorlevel 1 (
		echo ERROR: Cannot sign executables! See the log below:
		type "%TmpDir%\command-err.log"
		goto error
	)
) else (
	echo WARNING: Signing executables was disabled.
)

echo --- Preparing packages ----------------------------------------------
echo    * Create source package for version %CHTextVersion%...

rem Export the current working copy to a separate directory to avoid including unnecessary files in the source archive
SET CHSrcDir="%TmpDir%\ch-src"
svn export "%MainProjectDir%" "%CHSrcDir%" >"%TmpDir%\command.log" 2>"%TmpDir%\command-err.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while exporting local sources to a separate location. See the log below:
	type "%TmpDir%\command-err.log"
	goto error
)

cd %CHSrcDir%

"%SEVENZIPEXE%" a "%OutputDir%\chsrc-%CHTextVersion%.zip" -tzip -x^^!"scripts\*.bat" -xr^^!".svn" . >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Preparation of the sources failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Preparing the symbols package...

cd %MainProjectDir%\scripts
if "%CHCustomVersion%" == "1" (
	if "%CHReleaseType%" == "internal" (
		echo    * WARNING: Skipping embedding source server info in PDB files due to local sources modifications...
	) else (
		echo    * ERROR: Tagged sources contains local modifications - cannot embed source server info in PDB files...
		goto error
	)
) else (
	echo    * Embedding source server info in PDB files...
	call internal\embed_srcserver_info.bat "%MainProjectDir%"

	if errorlevel 1 (
		echo ERROR: encountered a problem while embedding source server information in debug symbols.
		goto error
	)
)

cd %MainProjectDir%\bin\release

"%SEVENZIPEXE%" a "%OutputDir%\ch_symbols-%CHTextVersion%.zip" -tzip "*.pdb"  >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Could not create symbols archive. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Preparing the installer package...
cd %MainProjectDir%
if not exist scripts (
	echo ERROR: The scripts directory does not exist.
	goto error
)

cd %MainProjectDir%\scripts

"%ISCCEXE%" setup.iss /o%OutputDir%  >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Preparation of the installer version failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

if NOT "%SKIPCHSIGNING%" == "1" (
	echo    * Signing installer package...
	signtool sign /t http://time.certum.pl /a "%OutputDir%\*.exe" 2>"%TmpDir%\command-err.log"
	if errorlevel 1 (
		echo ERROR: Cannot sign executables! See the log below:
		type "%TmpDir%\command-err.log"
		goto error
	)
) else (
	echo WARNING: Signing executables was disabled.
)

echo    * Preparing zip package...
cd %MainProjectDir%

rem Prepare files
SET Res=0
xcopy "bin\release\ch.exe" "%TmpDir%\32bit\" >"%TmpDir%\command.log" || SET Res=1
xcopy "License.txt" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\chext.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libicpf32u.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libchcore32u.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libictranslate32u.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\ictranslate.exe" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\sqlite3_32.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "scripts\portable_config\ch.xml" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\x86\Microsoft.VC140.CRT\*" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\x86\Microsoft.VC140.MFC\*" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\Common7\IDE\Remote Debugger\x86\dbghelp.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\Common7\IDE\Remote Debugger\x86\dbgcore.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%UniversalCRTSdkDir%\Redist\ucrt\DLLs\x86\*.dll" "%TmpDir%\32bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\help" "%TmpDir%\32bit\help" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\langs" "%TmpDir%\32bit\langs" >>"%TmpDir%\command.log" || SET Res=1

xcopy "bin\release\ch64.exe" "%TmpDir%\64bit\" >"%TmpDir%\command.log" || SET Res=1
xcopy "License.txt" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\chext64.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libicpf64u.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libchcore64u.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libictranslate64u.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\ictranslate64.exe" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\sqlite3_64.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "scripts\portable_config\ch.xml" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\x64\Microsoft.VC140.CRT\*" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\x64\Microsoft.VC140.MFC\*" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\Common7\IDE\Remote Debugger\x64\dbghelp.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\Common7\IDE\Remote Debugger\x64\dbgcore.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%UniversalCRTSdkDir%\Redist\ucrt\DLLs\x64\*.dll" "%TmpDir%\64bit\" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\help" "%TmpDir%\64bit\help" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\langs" "%TmpDir%\64bit\langs" >>"%TmpDir%\command.log" || SET Res=1

if %Res% NEQ 0 (
	echo ERROR: Detected a problem when copying files. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

cd "%TmpDir%\"

"%SEVENZIPEXE%" a -tzip "%OutputDir%\ch-%CHTextVersion%.zip" 32bit 64bit >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Could not create win32 zip archive. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

cd "%ScriptDir%"

echo    * Done...

goto cleanup

:error
echo    * Clearing environment...
call internal\clear_env.bat /skip_create
exit /b 1

:cleanup
echo    * Clearing environment...
call internal\clear_env.bat /skip_create

:end
exit /b 0
