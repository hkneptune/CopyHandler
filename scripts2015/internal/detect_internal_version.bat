@echo off

setlocal ENABLEDELAYEDEXPANSION

rem Validate input parameters
if [%1] == [] (
	echo Usage: detect_version_internal.bat path
	goto end
)

echo       * Detecting type of the working copy
set _svn_info=svn info "%1"
for /f "delims=" %%a in ('!_svn_info! ^|find "URL"') do set _SVNUrl=%%a
if errorlevel 1 (
	echo Problem with retrieving working copy URL.
	goto error
)

if NOT "!_SVNUrl!" == "!_SVNUrl:trunk=dummy!" (
	SET _ReleaseType=internal
) else if NOT "!_SVNUrl!" == "!_SVNUrl:tags=dummy!" (
	SET _ReleaseType=tag
) else (
	echo Cannot detect type of the working copy. Directory not under version control system?
	exit /b 1
)

rem In case of tagged releases we scan working copy for revision number only to get the modification flag
echo       * Scanning svn working copy for revision number
rem Get SVN version
SET _svn_version=svnversion -n "%1"
for /f %%a in ('!_svn_version!') do set _SVNCalculatedVersion=%%a
if errorlevel 1 (
	echo Problem with scanning svn WC for current version.
	goto error
)
if "!_SVNCalculatedVersion!" == "" (
	echo Can't get the major version.
	goto error
)

rem Which format does svn version have?
rem When version contain colon, then take only the newer revision from it
if NOT "!_SVNCalculatedVersion!" == "!_SVNCalculatedVersion::=dummy!" (
	SET _SVNCalculatedVersion=!_SVNCalculatedVersion:*:=!
)

rem If svn version contains S (switched), then just ignore this char
if NOT "!_SVNCalculatedVersion!" == "!_SVNCalculatedVersion:S=dummy!" (
	SET _SVNCalculatedVersion=!_SVNCalculatedVersion:S=!
)

rem If we have only partial working copy (P), then fail
if NOT "!_SVNCalculatedVersion!" == "!_SVNCalculatedVersion:P=dummy!" (
	echo Only partial working copy detected - cannot detect version.
	goto error
)

rem If we have local modifications, then remember it, so we can modify the version number/string later
if NOT "!_SVNCalculatedVersion!" == "!_SVNCalculatedVersion:M=dummy!" (
	SET _SVNCalculatedVersion=!_SVNCalculatedVersion:M=!
	SET _SVNIsModified=1
) else (
	SET _SVNIsModified=0
)

echo       * Detecting version numbers contained in version.h
set _ver_cmd=type "%1\src\common\version.h"

for /f "tokens=3 delims= " %%a in ('!_ver_cmd! ^|find "define PRODUCT_VERSION1 "') do set _MajorVersion=%%a
if errorlevel 1 (
	echo Problem with retrieving _MajorVersion.
	goto error
)
if "!_MajorVersion!" == "" (
	echo Can't get the major version.
	goto error
)

set _ver_cmd=type "%1\src\common\version.h"
for /f "tokens=3 delims= " %%a in ('!_ver_cmd! ^|find "define PRODUCT_VERSION2 "') do set _MinorVersion=%%a
if errorlevel 1 (
	echo Problem with retrieving _MinorVersion.
	goto error
)
if "!_MinorVersion!" == "" (
	echo Can't get the major version.
	goto error
)

if "!_ReleaseType!" == "tag" (
	rem If handling the tagged revision, overwrite !_SVNVersion!, %_CustomVersion% and !_TextVersion! with the values contained in version.h
	set _ver_cmd2=type "%1\src\common\version.h"
	for /f "tokens=3 delims= " %%a in ('!_ver_cmd2! ^|find "define PRODUCT_VERSION3 "') do set _SVNVersion=%%a
	if errorlevel 1 (
		echo Problem with retrieving svn version.
		goto error
	)
	if "!_SVNVersion!" == "" (
		echo Can't get the svn version.
		goto error
	)
	
	set _ver_cmd2=type "%1\src\common\version.h"
	for /f "tokens=3 delims= " %%a in ('!_ver_cmd2! ^|find "define PRODUCT_VERSION4 "') do set _CustomVersion=%%a
	if errorlevel 1 (
		echo Problem with retrieving custom version.
		goto error
	)
	if "!_CustomVersion!" == "" (
		echo Can't get the custom version.
		goto error
	)

	set _ver_cmd2=type "%1\src\common\version.h"
	for /f "tokens=3 delims= " %%a in ('!_ver_cmd2! ^|find "define PRODUCT_VERSION "') do set _TextVersion=%%a
	if errorlevel 1 (
		echo Problem with retrieving text version.
		goto error
	)
	if "!_TextVersion!" == "" (
		echo Can't get the text version.
		goto error
	)
	
	rem Get rid of the '"' characters from the _TextVersion
	if "!_SVNIsModified!" == "0" (
		SET _TextVersion=!_TextVersion:"=!
	) else (
		SET _TextVersion=!_TextVersion:"=!_M
	)
) else (
	SET _SVNVersion=!_SVNCalculatedVersion!
	rem For internal version we format the text version on our own
	if "!_SVNIsModified!" == "0" (
		SET _TextVersion=!_MajorVersion!.!_MinorVersion!internal-svn!_SVNVersion!
	) else (
		SET _TextVersion=!_MajorVersion!.!_MinorVersion!internal-svn!_SVNVersion!_M
	)

	if "!_TextVersion!" == "" (
		echo Cannot calculate the text version.
		goto error
	)
)

rem Overwrite %_CustomVersion% with !_SVNIsModified!
SET _CustomVersion=!_SVNIsModified!

endlocal & call SET CHMajorVersion=%_MajorVersion%& SET CHMinorVersion=%_MinorVersion%& SET CHSVNVersion=%_SVNVersion%& SET CHCustomVersion=%_CustomVersion%& SET CHTextVersion=%_TextVersion%& SET CHReleaseType=%_ReleaseType%

echo       * Detected current working copy (%CHReleaseType%) at %CHMajorVersion%.%CHMinorVersion%.%CHSVNVersion%.%CHCustomVersion% (%CHTextVersion%)

goto end

:error
rem Get outside of the temp directory to be able to delete it
exit /b 1

:end
exit /b 0
