@echo off

rem Script retrieves version information from version.h file

setlocal ENABLEDELAYEDEXPANSION

rem Detect current Major.Minos versions
set _ver_cmd=type "%WORKSPACE%\src\common\version.h"

for /f "tokens=3 delims= " %%a in ('!_ver_cmd! ^|find "define PRODUCT_VERSION1 "') do set _MajorVersion=%%a
if errorlevel 1 (
	echo Problem with retrieving _MajorVersion.
	exit /b 1
)

if "!_MajorVersion!" == "" (
	echo Can't get the major version.
	exit /b 1
)

set _ver_cmd=type "%WORKSPACE%\src\common\version.h"
for /f "tokens=3 delims= " %%a in ('!_ver_cmd! ^|find "define PRODUCT_VERSION2 "') do set _MinorVersion=%%a
if errorlevel 1 (
	echo Problem with retrieving _MinorVersion.
	exit /b 1
)
if "!_MinorVersion!" == "" (
	echo Can't get the minor version.
	exit /b 1
)

set _ver_cmd=type "%WORKSPACE%\src\common\version.h"
for /f "tokens=3 delims= " %%a in ('!_ver_cmd! ^|find "define PRODUCT_VERSION3 "') do set _BuildVersion=%%a
if errorlevel 1 (
	echo Problem with retrieving _BuildVersion.
	exit /b 1
)
if "!_BuildVersion!" == "" (
	echo Can't get the build version.
	exit /b 1
)

set _ver_cmd=type "%WORKSPACE%\src\common\version.h"
for /f "tokens=3 delims= " %%a in ('!_ver_cmd! ^|find "define PRODUCT_VERSION4 "') do set _DetailVersion=%%a
if errorlevel 1 (
	echo Problem with retrieving _DetailVersion.
	exit /b 1
)
if "!_DetailVersion!" == "" (
	echo Can't get the detail version.
	exit /b 1
)

set _ver_cmd=type "%WORKSPACE%\src\common\version.h"
for /f "tokens=3 delims= " %%a in ('!_ver_cmd! ^|find "define PRODUCT_VERSION "') do set _TextVersion=%%a
if errorlevel 1 (
	echo Problem with retrieving _TextVersion.
	exit /b 1
)
if "!_TextVersion!" == "" (
	echo Can't get the detail version.
	exit /b 1
)

endlocal & call SET CHMajorVersion=%_MajorVersion%& SET CHMinorVersion=%_MinorVersion%& SET CHBuildVersion=%_BuildVersion%& SET CHDetailVersion=%_DetailVersion%& SET CHTextVersion=%_TextVersion:"=%

echo Detected version %CHMajorVersion%.%CHMinorVersion%.%CHBuildVersion%.%CHDetailVersion% (%CHTextVersion%)
