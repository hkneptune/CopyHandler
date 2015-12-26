@echo off

rem Script creates a tag in svn for the alpha/beta/final release (uses the version.h and svn info as the source of information)

rem Mark the changes as local ones
setlocal

if [%1] == [] (
	echo Usage: svntag.bat ^<Alpha^|Beta^|rcX^|Final^>
	exit /b 1
)

rem This script wasn't tested yet (written 2011-10-22 23:04)
echo --- Initializing ----------------------------------------------------

rem Include config
call config.bat
if errorlevel 1 (
	exit /b 1
)

call internal\clear_env.bat
if errorlevel 1 (
	echo ERROR: Problem with preparing environment.
	goto error
)

rem checkout ch project (trunk)
echo    * Retrieving trunk for CH...
SET CHTmpTrunkDir=%TmpDir%\ch-trunk
svn --ignore-externals co "%ReposCH%/trunk" "%CHTmpTrunkDir%" >"%TmpDir%\command.log"
if errorlevel 1 (
	echo    ERROR: Cannot checkout trunk. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

rem detect version (for trunk it should get major.minor from version.h, svn rev from svn revision id)
echo    * Detecting trunk version information...
call internal\detect_internal_version.bat "%CHTmpTrunkDir%"
if errorlevel 1 (
	goto error
)

rem generate the tag to be created for sources
SET /a TagSVNRev=%CHSVNVersion%+2
SET /a NewTrunkMinor=%CHMinorVersion%
SET /a NewTrunkSVNRev=%TagSVNRev%+1
SET NewTrunkTextTag=%CHMajorVersion%.%NewTrunkMinor%internal-svn%NewTrunkSVNRev%

if "%1" == "Final" (
	SET TextTag=%CHMajorVersion%.%CHMinorVersion%Final
	SET /a NewTrunkMinor=%CHMinorVersion%+1
) else if "%1" == "Alpha" (
	SET TextTag=%CHMajorVersion%.%CHMinorVersion%Alpha-svn%TagSVNRev%
) else if "%1" == "Beta" (
	SET TextTag=%CHMajorVersion%.%CHMinorVersion%Beta-svn%TagSVNRev%
) else (
	SET TextTag=%CHMajorVersion%.%CHMinorVersion%%1
)

echo    * Tagging projects with %TextTag%...
call internal\svntag_single.bat "%ReposIcpf%" %TextTag%
if errorlevel 1 (
	goto error
)

call internal\svntag_single.bat "%ReposIctranslate%" %TextTag%
if errorlevel 1 (
	goto error
)

call internal\svntag_single.bat "%ReposCH%" %TextTag%
if errorlevel 1 (
	goto error
)

echo    * Checking out the tagged ch repository...
SET CHTmpTagDir=%TmpDir%\ch-tagged
svn --ignore-externals co "%ReposCH%/tags/%TextTag%" "%CHTmpTagDir%" >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while checking out copyhandler project. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Creating new svn:externals definition...

echo src/libicpf %ReposIcpf%/tags/%TextTag%/src/libicpf >"%TmpDir%\externals.txt"
echo src/libictranslate %ReposIctranslate%/tags/%TextTag%/src/libictranslate >>"%TmpDir%\externals.txt"
echo src/rc2lng %ReposIctranslate%/tags/%TextTag%/src/rc2lng >>"%TmpDir%\externals.txt"
echo src/ictranslate %ReposIctranslate%/tags/%TextTag%/src/ictranslate >>"%TmpDir%\externals.txt"

svn propedit --editor-cmd "type %TmpDir%\externals.txt >" svn:externals "%CHTmpTagDir%"  >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while checking out copyhandler project. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Updating version information for tagged sources...
cscript //NoLogo internal\replace_version.vbs "%CHTmpTagDir%\src\common\version.h.template" "%CHTmpTagDir%\src\common\version.h" %CHMajorVersion% %CHMinorVersion% %TagSVNRev% 0 %TextTag% >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while checking out copyhandler project. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Performing commit of the updated version and svn:externals...
svn commit -m "Updated svn:externals definition" "%CHTmpTagDir%" >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while committing changes to repository. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Updating version information for trunk sources (%NewTrunkTextTag%)...
cscript //NoLogo internal\replace_version.vbs "%CHTmpTrunkDir%\src\common\version.h.template" "%CHTmpTrunkDir%\src\common\version.h" %CHMajorVersion% %NewTrunkMinor% %NewTrunkSVNRev% 0 %NewTrunkTextTag% >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while checking out copyhandler project. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

echo    * Performing commit of the updated version...
svn commit -m "Automatic version bump after tagging %TextTag%" "%CHTmpTrunkDir%" >"%TmpDir%\command.log"
if errorlevel 1 (
	echo ERROR: encountered a problem while committing changes to repository. See the log below:
	type "%TmpDir%\command.log"
	goto error
)

goto cleanup

:error
call internal\clear_env.bat /skip_create

exit /b 1

:cleanup
echo    * Cleaning up the temporary files...
call internal\clear_env.bat /skip_create

:end
exit /b 0
