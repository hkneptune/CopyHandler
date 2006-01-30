
CopyHandlerShellExtps.dll: dlldata.obj CopyHandlerShellExt_p.obj CopyHandlerShellExt_i.obj
	link /dll /out:CopyHandlerShellExtps.dll /def:CopyHandlerShellExtps.def /entry:DllMain dlldata.obj CopyHandlerShellExt_p.obj CopyHandlerShellExt_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del CopyHandlerShellExtps.dll
	@del CopyHandlerShellExtps.lib
	@del CopyHandlerShellExtps.exp
	@del dlldata.obj
	@del CopyHandlerShellExt_p.obj
	@del CopyHandlerShellExt_i.obj
