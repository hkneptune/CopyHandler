@echo off

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1
SET OutDir=%WORKSPACE%\Output\%CHTextVersion%

signtool sign /t http://time.certum.pl /a "%OutDir%\*.exe"
