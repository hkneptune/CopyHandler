rem Execute this script in the boost directory.
setlocal

b2 -j 8 --toolset=msvc-14.2 --link=static --threading=multi --runtime-link=shared address-model=32 define=_BIND_TO_CURRENT_VCLIBS_VERSION define=BOOST_USE_WINAPI_VERSION=0x0601 --build-type=complete --stagedir=lib-14.2\x32 stage

rmdir /S /Q bin.v2
