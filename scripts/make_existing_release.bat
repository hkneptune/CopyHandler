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

echo    * Initializing Visual Studio environment
if not exist "%VS90COMNTOOLS%\vsvars32.bat" (
	echo ERROR: Can't find the vsvars32.bat file.
	goto error
) else (
	call "%VS90COMNTOOLS%\vsvars32.bat" >nul
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
cd "%MainProjectDir%"
echo    * Building win32 release solution...
devenv ch.vc90.sln /rebuild "Release-Unicode|Win32"  >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Build process failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Building win64 release solution...
devenv ch.vc90.sln /rebuild "Release-Unicode|x64" >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Build process failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
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

7z a "%OutputDir%\chsrc-%CHTextVersion%.zip" -tzip -x^^!"scripts\*.bat" -xr^^!".svn" . >"%TmpDir%\command.log"
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

cd %MainProjectDir%
if not exist bin\release (
	echo ERROR: The bin\release directory does not exist.
	goto error
)

cd %MainProjectDir%\bin\release

7z a "%OutputDir%\ch_symbols-%CHTextVersion%.zip" -tzip "*.pdb"  >"%TmpDir%\command.log"
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

iscc setup.iss /o%OutputDir%  >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Preparation of the installer version failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Preparing zip package...
cd %MainProjectDir%

rem Prepare files
SET Res=0
xcopy "bin\release\ch.exe" "%TmpDir%\zip32\" >"%TmpDir%\command.log" || SET Res=1
xcopy "License.txt" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\chext.dll" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libicpf32u.dll" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libchcore32u.dll" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libictranslate32u.dll" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\ictranslate.exe" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\x86\Microsoft.VC90.CRT\*" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\x86\Microsoft.VC90.MFC\*" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX64%\Common7\IDE\Remote Debugger\x86\dbghelp.dll" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\help" "%TmpDir%\zip32\help" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\langs" "%TmpDir%\zip32\langs" >>"%TmpDir%\command.log" || SET Res=1

xcopy "bin\release\ch64.exe" "%TmpDir%\zip64\" >"%TmpDir%\command.log" || SET Res=1
xcopy "License.txt" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\chext64.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libicpf64u.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libchcore64u.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libictranslate64u.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\ictranslate64.exe" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\amd64\Microsoft.VC90.CRT\*" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX86%\VC\redist\amd64\Microsoft.VC90.MFC\*" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInstallDirX64%\Common7\IDE\Remote Debugger\x64\dbghelp.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\help" "%TmpDir%\zip64\help" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\langs" "%TmpDir%\zip64\langs" >>"%TmpDir%\command.log" || SET Res=1

if %Res% NEQ 0 (
	echo ERROR: Detected a problem when copying files. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

cd "%TmpDir%\"

7z a -tzip "%OutputDir%\ch-%CHTextVersion%.zip" zip32 zip64 >"%TmpDir%\command.log"
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
