@echo off

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1

SET OutDir=%WORKSPACE%\Output\%CHTextVersion%
mkdir "%OutDir%"

"C:\Program Files\7-Zip\7z.exe" a "%OutDir%\chsymbols-%CHTextVersion%.zip" -tzip "%WORKSPACE%\bin\release\*.pdb"
