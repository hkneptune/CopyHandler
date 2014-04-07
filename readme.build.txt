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
        bjam --toolset=msvc-12.0 address-model=64 define=_BIND_TO_CURRENT_VCLIBS_VERSION --build-type=complete --stagedir=lib-12.0\x64 stage
        bjam --toolset=msvc-12.0 address-model=32 define=_BIND_TO_CURRENT_VCLIBS_VERSION --build-type=complete --stagedir=lib-12.0\x32 stage
