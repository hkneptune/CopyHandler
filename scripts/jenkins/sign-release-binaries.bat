@echo off

rem signtool sign /tr http://time.certum.pl /a "%WORKSPACE%\bin\release\*.dll" || exit /b 1
signtool sign /tr http://time.certum.pl /a /fd sha256 /td sha256 "%WORKSPACE%\bin\release\*.dll" || exit /b 1
rem signtool sign /tr http://time.certum.pl /a "%WORKSPACE%\bin\release\*.exe" || exit /b 1
signtool sign /tr http://time.certum.pl /a /fd sha256 /td sha256 "%WORKSPACE%\bin\release\*.exe"
