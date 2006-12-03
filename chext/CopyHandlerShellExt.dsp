# Microsoft Developer Studio Project File - Name="CopyHandlerShellExt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CopyHandlerShellExt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CopyHandlerShellExt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CopyHandlerShellExt.mak" CFG="CopyHandlerShellExt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CopyHandlerShellExt - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CopyHandlerShellExt - Win32 Release MinDependency" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CopyHandlerShellExt - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib shell32.lib gdi32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../BIN/Debug/chext.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\PROJECTS\c++\working\Copy Handler\BIN\Debug\chext.dll
InputPath=\PROJECTS\c++\working\Copy Handler\BIN\Debug\chext.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	start regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build
# Begin Special Build Tool
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Cmds="BuildManager" "Debug builds" "$(WkspDir)\chext_count.txt"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "CopyHandlerShellExt - Win32 Release MinDependency"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseMinDependency"
# PROP BASE Intermediate_Dir "ReleaseMinDependency"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseMinDependency"
# PROP Intermediate_Dir "ReleaseMinDependency"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G5 /MT /W4 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib shell32.lib gdi32.lib comctl32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../BIN/Release/chext.dll"
# SUBTRACT LINK32 /debug /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\ReleaseMinDependency
TargetPath=\PROJECTS\c++\working\Copy Handler\BIN\Release\chext.dll
InputPath=\PROJECTS\c++\working\Copy Handler\BIN\Release\chext.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build
# Begin Special Build Tool
WkspDir=.
TargetPath=\PROJECTS\c++\working\Copy Handler\BIN\Release\chext.dll
SOURCE="$(InputPath)"
PostBuild_Cmds="BuildManager" "Release builds" "$(WkspDir)\chext_count.txt"	upx.exe -9 -v "$(TargetPath)"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "CopyHandlerShellExt - Win32 Debug"
# Name "CopyHandlerShellExt - Win32 Release MinDependency"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\clipboard.cpp
# End Source File
# Begin Source File

SOURCE=.\CopyHandlerShellExt.cpp
# End Source File
# Begin Source File

SOURCE=.\CopyHandlerShellExt.def
# End Source File
# Begin Source File

SOURCE=.\CopyHandlerShellExt.idl
# ADD MTL /tlb ".\CopyHandlerShellExt.tlb" /h "CopyHandlerShellExt.h" /iid "CopyHandlerShellExt_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\CopyHandlerShellExt.rc
# End Source File
# Begin Source File

SOURCE=.\DropMenuExt.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\FileSupport.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuExt.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StringHelpers.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\clipboard.h
# End Source File
# Begin Source File

SOURCE=.\DropMenuExt.h
# End Source File
# Begin Source File

SOURCE=..\Common\FileSupport.h
# End Source File
# Begin Source File

SOURCE=.\IContextMenuImpl.h
# End Source File
# Begin Source File

SOURCE=..\Common\ipcstructs.h
# End Source File
# Begin Source File

SOURCE=.\IShellExtInitImpl.h
# End Source File
# Begin Source File

SOURCE=.\MenuExt.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StringHelpers.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DropMenuExt.rgs
# End Source File
# Begin Source File

SOURCE=.\MenuExt.rgs
# End Source File
# End Group
# End Target
# End Project
