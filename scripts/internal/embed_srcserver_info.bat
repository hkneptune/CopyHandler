@echo off

call config.bat
if errorlevel 1 (
	exit /b 1
)

call internal\prepare_env.bat
if errorlevel 1 (
	goto error
)

for %%f in (%MainProjectDir%\src %MainProjectDir%\src\ictranslate %MainProjectDir%\src\libictranslate %MainProjectDir%\src\rc2lng %MainProjectDir%\src\libicpf) do (
	SET _cmd=call svnindex.cmd /debug /source=%%f /symbols=%MainProjectDir%\bin\release
	echo    * Processing source path %%f...

	%_cmd% >"%TmpDir%\command.log"
	if errorlevel 1 (
		echo ERROR: Error encountered while embedding source server information.
		exit /b 1
	)

	rem We expect at least one embedding to succeed per source path
	SET IndexRes=Undefined
	SET _cmd=type "%TmpDir%\command.log" 
	for /f %%a in ('%_cmd% ^| find /c "wrote"') do set IndexRes=%%a
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
)

exit /b 0
