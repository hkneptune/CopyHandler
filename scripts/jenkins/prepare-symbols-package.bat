@echo off

call "%WORKSPACE%\scripts\jenkins\_get-output-dir.bat" || exit /b 1

"C:\Program Files\7-Zip\7z.exe" a "%OutDir%\chsymbols-%CHTextVersion%.zip" -tzip "%WORKSPACE%\bin\release\*.pdb"
