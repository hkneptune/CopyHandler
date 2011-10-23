@echo off

setlocal ENABLEDELAYEDEXPANSION

rem Validate input parameters
if [%1] == [] (
	echo Usage: detect_incorrect_manifest.bat path_to_exe_dll_file
	goto end
)

rem initialize
call config.bat
if errorlevel 1 (
	exit /b 1
)

call internal\prepare_env.bat
if errorlevel 1 (
	goto error
)

if not exist "%VS90COMNTOOLS%\vsvars32.bat" (
	echo ERROR: Can't find the vsvars32.bat file.
	goto error
) else (
	call "%VS90COMNTOOLS%\vsvars32.bat" >nul
)

rem process input path
SET InputFile=%~1
SET TmpManifestLocation=%TmpDir%\extracted.manifest

rem dll or exe?
if NOT "%InputFile%" == "!InputFile:.dll=dummy!" (
	rem this is a dll
	SET ResID=2
) else if NOT "%InputFile%" == "!InputFile:.exe=dummy!" (
	rem exe file
	SET ResID=1
) else (
	echo ERROR: Unknown file type encountered.
	goto error
)

mt.exe -nologo -inputresource:"%InputFile%";#%ResID% -out:"%TmpManifestLocation%"
if errorlevel 1 (
	echo ERROR: cannot extract manifest from file %InputFile%.
	goto error
)

SET _command=type "%TmpManifestLocation%"

rem Currently we expect to have only 9.0.30729.5570 referenced in the manifest; if there is any older version, then some library linked with the project uses outdated version (boost?)
for %%v in ("9.0.21022.8" "9.0.30411" "9.0.30729.1" "9.0.30729.4148") do (
	for /f %%a in ('%_command% ^| find %%v') do (
		if NOT "%%a" == "0" (
			echo       ERROR: %~Nx1 references an old version of MFC ^(%%v^)
			goto error
		)
	)
)

goto end

:error
exit /b 1

:end
exit /b 0
