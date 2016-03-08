@echo off

call "%WORKSPACE%\scripts\jenkins\_get-output-dir.bat" || exit /b 1

signtool sign /tr http://time.certum.pl /a "%OutDir%\*.exe"
