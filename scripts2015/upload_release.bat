@echo off

rem Mark the changes as local ones
setlocal

echo --- Initializing -----------------------------------------------
echo    * Reading configuration...
call config.bat
if errorlevel 1 (
	goto error
)

echo --- Uploading files --------------------------------------------
echo cd uploads >"%TmpDir%\filelist.txt"
for %%f in (out\*.*) do echo put %%f >>"%TmpDir%\filelist.txt"


psftp -v -b "%TmpDir%\filelist.txt" ixen@frs.sourceforge.net
if errorlevel 1 (
	goto error
)

echo    * Done

goto end

:error
echo    * Error uploading files.
del "%TmpDir%\filelist.txt"

:end
