@echo off

rem Mark the changes as local ones
setlocal

rem Check input parameter
if [%1] == [] (
	echo Usage: make_package.bat beta^|final
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

echo --- Detecting version numbers ----------------------------------
call internal\detect_version.bat %1
if errorlevel 1 (
	goto error
)

echo --- Checking if current trunk is already tagged ----------------
svn --depth empty co "%ReposCH%/tags/%TrunkTextVersion%" "%MainProjectDir%" >nul 2>nul
if errorlevel 1 (
	echo    * Tag not found. Will tag.
) else (
	echo    * Tag %TrunkTextVersion% already found. Skipping tagging.
	SET TextVersion=%TrunkTextVersion%
	SET SVNVersion=%TrunkSVNVersion%
	goto make_existing
)

echo --- Tagging the release ----------------------------------------
call internal\svntag.bat %TextVersion%
if errorlevel 1 (
	goto error
)

:make_existing
call make_existing_release.bat %TextVersion%
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
