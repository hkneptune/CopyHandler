rem Execute this script in the boost directory.
setlocal

call bootstrap.bat
call boost-build64.bat
call boost-build32.bat

pause
