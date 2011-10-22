@echo off

rem Scripts checks out trunk or specific tag of CH and launches preparation of the packages for the retrieved version.

rem Mark the changes as local ones
setlocal

rem Check input parameter
if [%1] == [] (
	echo Usage: make_release.bat ^<trunk^|tag_name^>
	exit /b 1
)

echo --- Initializing -----------------------------------------------
echo    * Reading configuration...
call config.bat
if errorlevel 1 (
	goto error
)

echo    * Preparing environment...
call internal\prepare_env.bat
if errorlevel 1 (
	goto error
)

echo --- Retrieving code ----------------------------------
if "%1" == "trunk" (
	SET CHRepositoryAddress=%ReposCH%/trunk
) else (
	SET CHRepositoryAddress=%ReposCH%/tags/%1
)

echo    * Checking out %CHRepositoryAddress%...
SET CHTmpDir=%TmpDir%\ch-svn
svn co "%CHRepositoryAddress%" "%CHTmpDir%" >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: Could not check out source code. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

rem call the original version of the script that was used to prepare the version (might not work for versions prior to 1.40)
ch %CHTmpDir%\scripts
call make_existing_release.bat
if errorlevel 1 (
	goto error
)

goto cleanup

:error
call internal\clear_env.bat /skip_create
exit /b 1

:cleanup
call internal\clear_env.bat /skip_create

:end
exit /b 0
