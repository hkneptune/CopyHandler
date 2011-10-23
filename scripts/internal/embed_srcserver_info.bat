@echo off

setlocal ENABLEDELAYEDEXPANSION

if "%1" == "" (
	echo Usage: embed_srcserver_info.bat path_to_sources
	goto end
)

SET MainProjectDir=%~1

call config.bat
if errorlevel 1 (
	exit /b 1
)

call internal\prepare_env.bat
if errorlevel 1 (
	goto error
)

SET _command=call svnindex.cmd /debug /source=%MainProjectDir%\src\ictranslate;%MainProjectDir%\src\libictranslate;%MainProjectDir%\src\libicpf;%MainProjectDir%\src /symbols=%MainProjectDir%\bin\release
!_command! >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Error encountered while embedding source server information. See the log below:
	type "%TmpDir%\command.log"
	exit /b 1
)

rem We expect all pdbs to be modified (currently 12 of them - 6/architecture)
SET IndexRes=Undefined
SET _command=type "%TmpDir%\command.log" 
for /f %%a in ('!_command! ^| find /c "wrote"') do set IndexRes=%%a
if NOT "!IndexRes!" == "12" (
	echo Some source server information has not been embedded. See the log below:
	type "%TmpDir%\command.log"
	exit /b 1
)

echo       ...embedded information in !IndexRes! files.
SET _command=type "%TmpDir%\command.log" 
for /f %%a in ('!_command! ^| find /c "ERROR"') do set IndexRes=%%a
if NOT "!IndexRes!" == "0" (
	echo Error encountered when embedding source server information. See the log below:
	type "%TmpDir%\command.log"
	exit /b 1
)

SET _command=type "%TmpDir%\command.log" 
for /f %%a in ('!_command! ^| find /c "WARNING"') do set IndexRes=%%a
if NOT "!IndexRes!" == "0" (
	echo Error encountered when embedding source server information. See the log below:
	type "%TmpDir%\command.log"
	exit /b 1
)

exit /b 0
