@echo off

SET OutDir=%WORKSPACE%\Output
mkdir "%OutDir%"

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1

"C:\Program Files\7-Zip\7z.exe" a "%WORKSPACE%\Output\chsrc-%CHTextVersion%.zip" -tzip -x!".git" -x!"temp" -x!"Output" "%WORKSPACE%\*"
