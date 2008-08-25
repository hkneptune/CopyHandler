@echo off

rem Mark the changes as local ones
setlocal

echo Performing a cleanup...
if exist copyhandler (
	rmdir /S /Q copyhandler
	if exist copyhandler (
		echo ERROR: Deleting the old temporary copyhandler folder failed.
		goto end
	)
)

echo Tagging projects...
svn cp -m "Tagged project to %1" https://libicpf.svn.sourceforge.net/svnroot/libicpf/trunk https://libicpf.svn.sourceforge.net/svnroot/libicpf/tags/%1
if errorlevel 1 (
	echo ERROR: encountered a problem while tagging libicpf project.
	goto cleanup
)

svn cp -m "Tagged project to %1" https://libictranslate.svn.sourceforge.net/svnroot/libictranslate/trunk https://libictranslate.svn.sourceforge.net/svnroot/libictranslate/tags/%1
if errorlevel 1 (
	echo ERROR: encountered a problem while tagging ictranslate project.
	goto cleanup
)

svn cp -m "Tagged project to %1" https://copyhandler.svn.sourceforge.net/svnroot/copyhandler/trunk https://copyhandler.svn.sourceforge.net/svnroot/copyhandler/tags/%1
if errorlevel 1 (
	echo ERROR: encountered a problem while tagging copyhandler project.
	goto cleanup
)

echo Checking out the tagged ch repository...
svn co https://copyhandler.svn.sourceforge.net/svnroot/copyhandler/tags/%1 copyhandler
if errorlevel 1 (
	echo ERROR: encountered a problem while checking out copyhandler project.
	goto cleanup
)

echo Creating new svn:externals definition...

echo src/libicpf https://libicpf.svn.sourceforge.net/svnroot/libicpf/tags/%1/src/libicpf >externals.txt
echo src/libictranslate https://libictranslate.svn.sourceforge.net/svnroot/libictranslate/tags/%1/src/libictranslate >>externals.txt
echo src/rc2lng https://libictranslate.svn.sourceforge.net/svnroot/libictranslate/tags/%1/src/rc2lng >>externals.txt
echo src/ictranslate https://libictranslate.svn.sourceforge.net/svnroot/libictranslate/tags/%1/src/ictranslate >>externals.txt

svn propedit --editor-cmd "%CD%\edit.bat" svn:externals copyhandler
if errorlevel 1 (
	echo ERROR: encountered a problem while checking out copyhandler project.
	goto cleanup
)

echo Performing commit of the updated svn:externals...
svn commit -m "Updated svn:externals definition" copyhandler

:cleanup
echo Cleaning up...
del externals.txt
rmdir /S /Q copyhandler

goto end

:end

echo Done.
