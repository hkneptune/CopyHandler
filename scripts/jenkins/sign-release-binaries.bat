@echo off

signtool sign /tr http://time.certum.pl /a "%WORKSPACE%\bin\release\*.dll" || exit /b 1
signtool sign /tr http://time.certum.pl /a "%WORKSPACE%\bin\release\*.exe"
