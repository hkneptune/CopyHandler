@echo off

signtool sign /t http://time.certum.pl /a "%WORKSPACE%\bin\release\*.dll" || exit /b 1
signtool sign /t http://time.certum.pl /a "%WORKSPACE%\bin\release\*.exe"
