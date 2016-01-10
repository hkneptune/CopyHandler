@echo off

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1

SET OutDir=%WORKSPACE%\Output\%CHTextVersion%
mkdir "%OutDir%"

"C:\Program Files\7-Zip\7z.exe" a "%OutDir%\chsrc-%CHTextVersion%.zip" -tzip -x!".git" -x!"temp" -x!"Output" "%WORKSPACE%\*"
