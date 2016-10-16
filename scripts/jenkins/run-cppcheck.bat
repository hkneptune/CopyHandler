@echo off
SETLOCAL

SET PATH=C:\Program Files\Cppcheck;%PATH%

mkdir "CPPCheck"

cppcheck --enable=warning,style,performance,portability,information --suppressions-list=scripts\jenkins\cppcheck-suppress.txt --inline-suppr --library=microsoft_sal.cfg --library=windows.cfg --std=c++11 --xml-version=2 --xml --platform=win32W -i ext -i tests --project=ch.vc140.sln 2>"CPPCheck\cppcheck.xml"
