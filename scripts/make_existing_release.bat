@echo off

rem Mark the changes as local ones
setlocal

rem Check input parameter
if [%1] == [] (
	echo Usage: make_package.bat TextVersion
	exit /b 1
)

SET TextVersion=%1

echo --- Initializing  ----------------------------------------------
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

SET VSInst=%ProgramFiles%
if NOT "%ProgramFiles(x86)%" == "" SET VSInst=%ProgramFiles(x86)%

echo --- Preparing source package ----------------------------------------
echo    * Retrieving tagged source code...
svn co "%ReposCH%/tags/%TextVersion%" "%MainProjectDir%" >"%TmpDir%\command.log" 2>"%TmpDir%\command-err.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while exporting sources from repository. See the log below:
	type "%TmpDir%\command-err.log"
	goto error
)

SET _cmd=type "%TmpDir%\command-err.log"
for /f %%a in ('%_cmd% ^|find "Error"') do SET Res=%%a
if NOT "%Res%" == "" (
	echo ERROR: Encountered some problems while checking out CH. See below for details:
	type "%TmpDir%\command-err.log"
	goto error
)

echo    * Create source package...
cd %MainProjectDir%
7z a "%OutputDir%\chsrc-%TextVersion%.zip" -tzip -x!"scripts\*.bat" -xr!".svn" . >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Preparation of the sources failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo --- Building solutions ----------------------------------------------
echo    * Building win32...
devenv ch.vc90.sln /rebuild "Release-Unicode|Win32"  >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Build process failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Building win64...
devenv ch.vc90.sln /rebuild "Release-Unicode|x64" >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Build process failed. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Embedding source server information to debug symbol files...
cd %ScriptDir%
call internal\embed_srcserver_info.bat
if errorlevel 1 (
	goto error
)

echo    * Preparing the symbols package...

cd %MainProjectDir%
if not exist bin\release (
	echo ERROR: The bin\release directory does not exist.
	goto error
)

cd %MainProjectDir%\bin\release

7z a "%OutputDir%\ch_symbols-%TextVersion%.zip" -tzip "*.pdb"  >"%TmpDir%\command.log"
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
xcopy "%VSInst%\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInst%\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.MFC\*" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x86\dbghelp.dll" "%TmpDir%\zip32\" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\help" "%TmpDir%\zip32\help" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\langs" "%TmpDir%\zip32\langs" >>"%TmpDir%\command.log" || SET Res=1

xcopy "bin\release\ch64.exe" "%TmpDir%\zip64\" >"%TmpDir%\command.log" || SET Res=1
xcopy "License.txt" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\chext64.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libicpf64u.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libchcore64u.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\libictranslate64u.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "bin\release\ictranslate64.exe" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInst%\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%VSInst%\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.MFC\*" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy "%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x64\dbghelp.dll" "%TmpDir%\zip64\" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\help" "%TmpDir%\zip64\help" >>"%TmpDir%\command.log" || SET Res=1
xcopy /E /I "bin\release\langs" "%TmpDir%\zip64\langs" >>"%TmpDir%\command.log" || SET Res=1

if %Res% NEQ 0 (
	echo ERROR: Detected a problem when copying files. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

cd "%TmpDir%\"

7z a -tzip "%OutputDir%\ch-%TextVersion%.zip" zip32 zip64 >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Could not create win32 zip archive. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

cd "%ScriptDir%"

echo    * Done...

goto cleanup

:error
call internal\clear_env.bat /skip_create
exit /b 1

:cleanup
call internal\clear_env.bat /skip_create

:end
exit /b 0
