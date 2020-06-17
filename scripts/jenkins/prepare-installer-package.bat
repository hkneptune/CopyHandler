@echo off

call "%WORKSPACE%\scripts\jenkins\_get-output-dir.bat" || exit /b 1

"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "%WORKSPACE%\scripts\setup.iss" /o"%OutDir%"
