@echo off

call config.bat
if errorlevel 1 (
	exit /b 1
)

call internal\prepare_env.bat
if errorlevel 1 (
	goto error
)

SET _cmd=call svnindex.cmd /debug /source=%MainProjectDir%\src /source=%MainProjectDir%\src\ictranslate /source=%MainProjectDir%\src\libictranslate /source=%MainProjectDir%\src\rc2lng /source=%MainProjectDir%\src\libicpf /symbols=%MainProjectDir%\bin\release
rem SET _cmd=call svnindex.cmd /debug /source=%MainProjectDir%\src  /symbols=%MainProjectDir%\bin\release

%_cmd% >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Error encountered while embedding source server information.
	exit /b 1
)

SET IndexRes=Undefined
SET _cmd=type "%TmpDir%\command.log" 
for /f %%a in ('%_cmd% ^| find /c "zero source files found"') do set IndexRes=%%a
if NOT "%IndexRes%" == "0" (
	echo Some source server information has not been embedded. See the log below:
	type "%TmpDir%\command.log"
	exit /b 1
)

for /f %%a in ('%_cmd% ^| find /c "ERROR"') do set IndexRes=%%a
if NOT "%IndexRes%" == "0" (
	echo Error encountered when embedding source server information. See the log below:
	type "%TmpDir%\command.log"
	exit /b 1
)

for /f %%a in ('%_cmd% ^| find /c "WARNING"') do set IndexRes=%%a
if NOT "%IndexRes%" == "0" (
	echo Error encountered when embedding source server information. See the log below:
	type "%TmpDir%\command.log"
	exit /b 1
)

exit /b 0
