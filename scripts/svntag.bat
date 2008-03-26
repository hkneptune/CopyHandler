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
svn cp -m "Tagged project to %1" http://gforge.draknet.sytes.net/svn/libicpf/trunk http://gforge.draknet.sytes.net/svn/libicpf/tags/%1
if errorlevel 1 (
	echo ERROR: encountered a problem while tagging libicpf project.
	goto cleanup
)

svn cp -m "Tagged project to %1" http://gforge.draknet.sytes.net/svn/ictranslate/trunk http://gforge.draknet.sytes.net/svn/ictranslate/tags/%1
if errorlevel 1 (
	echo ERROR: encountered a problem while tagging ictranslate project.
	goto cleanup
)

svn cp -m "Tagged project to %1" http://gforge.draknet.sytes.net/svn/copyhandler/trunk http://gforge.draknet.sytes.net/svn/copyhandler/tags/%1
if errorlevel 1 (
	echo ERROR: encountered a problem while tagging copyhandler project.
	goto cleanup
)

echo Checking out the tagged ch repository...
svn co http://gforge.draknet.sytes.net/svn/copyhandler/tags/%1 copyhandler
if errorlevel 1 (
	echo ERROR: encountered a problem while checking out copyhandler project.
	goto cleanup
)

echo Creating new svn:externals definition...

echo src/libicpf http://gforge.draknet.sytes.net/svn/libicpf/tags/%1/src/libicpf >externals.txt
echo src/libictranslate http://gforge.draknet.sytes.net/svn/ictranslate/tags/%1/src/libictranslate >>externals.txt
echo src/rc2lng http://gforge.draknet.sytes.net/svn/ictranslate/tags/%1/src/rc2lng >>externals.txt
echo src/ictranslate http://gforge.draknet.sytes.net/svn/ictranslate/tags/%1/src/ictranslate >>externals.txt

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
