@echo off

SET OutDir=%WORKSPACE%\Output
mkdir "%OutDir%"

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1

"C:\Program Files\7-Zip\7z.exe" a "%WORKSPACE%\Output\chsymbols-%CHTextVersion%.zip" -tzip "%WORKSPACE%\bin\release\*.pdb"
