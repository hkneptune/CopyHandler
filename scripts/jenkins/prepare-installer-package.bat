@echo off

SET OutDir=%WORKSPACE%\Output
mkdir "%OutDir%"

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1

"C:\Program Files (x86)\Inno Setup 5\ISCC.exe" "%WORKSPACE%\scripts\setup.iss" /o"%OutDir%"
