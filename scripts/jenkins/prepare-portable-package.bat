@echo off

rem Script prepares the portable CH package after build
call "%WORKSPACE%\scripts\jenkins\_get-output-dir.bat" || exit /b 1

SET TmpDir=%WORKSPACE%\temp\chzip-%BUILD_NUMBER%

SET VSInstallDirX86=C:\Program Files (x86)\Microsoft Visual Studio 14.0

mkdir "%TmpDir%"

rem Prepare 32bit build
xcopy "%WORKSPACE%\bin\release\ch.exe" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\License.txt" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\chext.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\regchext.exe" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libchcore32u.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libchengine32u.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libserializer32u.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libstring32u.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\liblogger32u.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libictranslate32u.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\ictranslate.exe" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\sqlite3_32.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "%WORKSPACE%\scripts\portable_config\ch.xml" "%TmpDir%\32bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.CRT\*" "%TmpDir%\32bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.MFC\*" "%TmpDir%\32bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x86\dbghelp.dll" "%TmpDir%\32bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\*" "%TmpDir%\32bit\" || exit /B 1
xcopy /E /I "%WORKSPACE%\bin\release\help" "%TmpDir%\32bit\help" || exit /B 1
xcopy /E /I "%WORKSPACE%\bin\release\langs" "%TmpDir%\32bit\langs" || exit /B 1

rem Prepare 64bit build
xcopy "%WORKSPACE%\bin\release\ch64.exe" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\License.txt" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\chext64.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\regchext64.exe" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libchcore64u.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\liblogger64u.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libchengine64u.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libserializer64u.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libstring64u.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libictranslate64u.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\ictranslate64.exe" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\sqlite3_64.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "%WORKSPACE%\scripts\portable_config\ch.xml" "%TmpDir%\64bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x64\Microsoft.VC140.CRT\*" "%TmpDir%\64bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x64\Microsoft.VC140.MFC\*" "%TmpDir%\64bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x64\dbghelp.dll" "%TmpDir%\64bit\" || exit /B 1
xcopy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x64\*" "%TmpDir%\64bit\" || exit /B 1
xcopy /E /I "%WORKSPACE%\bin\release\help" "%TmpDir%\64bit\help" || exit /B 1
xcopy /E /I "%WORKSPACE%\bin\release\langs" "%TmpDir%\64bit\langs" || exit /B 1

rem Prepare 32bit shell extension for 64bit build
xcopy "%WORKSPACE%\bin\release\chext.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libchcore32u.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libchengine32u.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libserializer32u.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\libstring32u.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\liblogger32u.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "%WORKSPACE%\bin\release\sqlite3_32.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1

xcopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.CRT\*" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x86\Microsoft.VC140.MFC\*" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "C:\Program Files (x86)\Windows Kits\8.1\Debuggers\x86\dbghelp.dll" "%TmpDir%\64bit\ShellExt32\" || exit /B 1
xcopy "C:\Program Files (x86)\Windows Kits\10\Redist\ucrt\DLLs\x86\*" "%TmpDir%\64bit\ShellExt32\" || exit /B 1

cd %TmpDir%
"C:\Program Files\7-Zip\7z.exe" a -tzip "%OutDir%\ch-portable-%CHTextVersion%.zip" 32bit 64bit
