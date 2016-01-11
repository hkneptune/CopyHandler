@echo off

call "%WORKSPACE%\scripts\jenkins\_get-output-dir.bat" || exit /b 1

"C:\Program Files\7-Zip\7z.exe" a "%OutDir%\chsrc-%CHTextVersion%.zip" -tzip -x!".git" -x!"temp" -x!"Output" "%WORKSPACE%\*"
