To build Copy Handler from source code, you need to have:
- Visual Studio 2008 (at least standard edition, because of dependencies on MFC) + SP1 + newest Visual Studio updates
- Newest Windows SDK
- Debugging tools for Windows (needed for crash dump generation) - seems to be integrated with newer Windows SDKs

Copy Handler project depends on:
- Boost libraries (http://www.boost.org)
Common part:
    bootstrap.bat
	
1. Visual Studio 2008
     for x64 and x32 respectively
        bjam --toolset=msvc-9.0 address-model=64 define=_BIND_TO_CURRENT_VCLIBS_VERSION --build-type=complete --stagedir=lib-9.0\x64 stage
        bjam --toolset=msvc-9.0 address-model=32 define=_BIND_TO_CURRENT_VCLIBS_VERSION --build-type=complete --stagedir=lib-9.0\x32 stage

2. Visual Studio 2013
CH that is built with VS2013 uses the XP targeting to allow it to run with Windows XP.
Because of that, boost also needs to be built with this kind of targeting.

Based on MS article:
http://blogs.msdn.com/b/vcblog/archive/2012/10/08/10357555.aspx

The environment needs to be set up first, so
for x32:
    set INCLUDE=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Include;%INCLUDE%
    set PATH=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Bin;%PATH%
    set CL=/D_USING_V110_SDK71_;%CL%
    set LIB=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Lib;%LIB%
    set LINK=/SUBSYSTEM:CONSOLE,5.01 %LINK%

    bjam --toolset=msvc-12.0 address-model=32 define=_BIND_TO_CURRENT_VCLIBS_VERSION --build-type=complete --stagedir=lib-12.0\x32 stage

for x64:
    set INCLUDE=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Include;%INCLUDE%
    set PATH=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Bin;%PATH%
    set CL=/D_USING_V110_SDK71_;%CL%
    set LIB=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Lib\x64;%LIB%
    set LINK=/SUBSYSTEM:CONSOLE,5.02 %LINK%

    bjam --toolset=msvc-12.0 address-model=64 define=_BIND_TO_CURRENT_VCLIBS_VERSION --build-type=complete --stagedir=lib-12.0\x64 stage

This was tested on boost 1.57 with VS2013 Update 4 Community Edition.
There are also batch scripts prepared in tools/ to help with building boost for VS2013.