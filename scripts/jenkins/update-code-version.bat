@echo off

rem Script updates the version.h file based on the jenkins build parameters
setlocal ENABLEDELAYEDEXPANSION

rem Detect current Major.Minos versions
call "%WORKSPACE%\scripts\jenkins\_get-version.bat"

rem Detect new version type
if "%GIT_TAG_NAME%" == "" (
	rem internal build
	SET _CHStrVersion=%CHMajorVersion%.%CHMinorVersion%git-%GIT_COMMIT:~-6%
) else (
	rem tagged build
	SET _CHStrVersion=%GIT_TAG_NAME%
)

echo String version: %_CHStrVersion%
echo Numeric ver: %CHMajorVersion%.%CHMinorVersion%.%BUILD_NUMBER%.0

echo Changing version
cscript //NoLogo "%WORKSPACE%\scripts\jenkins\_replace_version.vbs" "%WORKSPACE%\src\common\version.h.template" "%WORKSPACE%\src\common\version.h" %CHMajorVersion% %CHMinorVersion% %BUILD_NUMBER% 0 %_CHStrVersion%
