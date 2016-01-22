@echo off
SETLOCAL

SET PATH=C:\Program Files\Cppcheck;%PATH%

mkdir "%WORKSPACE%\CPPCheck"

cppcheck --enable=warning,style,performance,portability,information,unusedFunction --suppress=cstyleCast --library=microsoft_sal.cfg --library=windows.cfg --std=c++11 --xml-version=2 --xml --platform=win32W --force src 2>"%WORKSPACE%\CPPCheck\cppcheck.xml"
