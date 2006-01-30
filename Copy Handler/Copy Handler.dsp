# Microsoft Developer Studio Project File - Name="Copy Handler" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Copy Handler - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Copy Handler.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Copy Handler.mak" CFG="Copy Handler - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Copy Handler - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Copy Handler - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Copy Handler - Win32 Final Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Copy Handler"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Copy Handler - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G5 /MD /W4 /GX /O2 /Ob2 /I "..\..\..\MODULES" /I "..\..\MODULES\\" /I "..\..\..\MODULES\App Framework" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FAs /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib imagehlp.lib version.lib htmlhelp.lib /nologo /subsystem:windows /machine:I386 /out:"../BIN/Release/ch.exe"
# SUBTRACT LINK32 /profile /debug /nodefaultlib
# Begin Special Build Tool
WkspDir=.
TargetPath=\projects\c++\working\Copy Handler\BIN\Release\ch.exe
SOURCE="$(InputPath)"
PostBuild_Cmds="BuildManager" "Release builds" "$(WkspDir)\ch_count.txt"	upx.exe -9 -v "$(TargetPath)"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Copy Handler - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /I "..\..\..\MODULES\App Framework" /I "..\..\..\MODULES" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib imagehlp.lib version.lib htmlhelp.lib /nologo /subsystem:windows /profile /debug /machine:I386 /out:"../BIN/Debug/ch.exe"
# SUBTRACT LINK32 /map
# Begin Special Build Tool
WkspDir=.
SOURCE="$(InputPath)"
PostBuild_Cmds="BuildManager" "Debug builds" "$(WkspDir)\ch_count.txt"
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Copy Handler - Win32 Final Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Copy_Handler___Win32_Final_Release"
# PROP BASE Intermediate_Dir "Copy_Handler___Win32_Final_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Copy_Handler___Win32_Final_Release"
# PROP Intermediate_Dir "Copy_Handler___Win32_Final_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MD /W4 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FAs /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /G5 /MD /W4 /GX /O2 /Ob2 /I "..\..\..\MODULES" /I "..\..\MODULES\\" /I "..\..\..\MODULES\App Framework" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FAs /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib imagehlp.lib version.lib htmlhelp.lib /nologo /subsystem:windows /machine:I386 /out:"../BIN/Release/ch.exe"
# SUBTRACT BASE LINK32 /profile /debug /nodefaultlib
# ADD LINK32 winmm.lib imagehlp.lib version.lib htmlhelp.lib /nologo /subsystem:windows /machine:I386 /out:"../BIN/Release/ch.exe"
# SUBTRACT LINK32 /profile /debug /nodefaultlib
# Begin Special Build Tool
WkspDir=.
TargetPath=\projects\c++\working\Copy Handler\BIN\Release\ch.exe
SOURCE="$(InputPath)"
PostBuild_Cmds="BuildManager" "Release builds" "$(WkspDir)\ch_count.txt"	upx.exe -9 -v "$(TargetPath)"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Copy Handler - Win32 Release"
# Name "Copy Handler - Win32 Debug"
# Name "Copy Handler - Win32 Final Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\AppHelper.cpp"
# End Source File
# Begin Source File

SOURCE=.\BufferSizeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\CfgProperties.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\ConfigManager.cpp"
# End Source File
# Begin Source File

SOURCE=".\COPY HANDLER.cpp"
# End Source File
# Begin Source File

SOURCE=".\COPY HANDLER.rc"

!IF  "$(CFG)" == "Copy Handler - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Compiling resources $(InputPath)
InputDir=.
OutDir=.\Release
WkspDir=.
TargetDir=\projects\c++\working\Copy Handler\BIN\Release
InputPath=".\COPY HANDLER.rc"
InputName=COPY HANDLER

"$(OutDir)\$(InputName).res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"BuildManager" "$(WkspDir)\ch_count.txt" $(InputPath) /rcupd 
	Exe2Lng.exe $(InputPath) "$(InputDir)\Scripts\header.lng" "$(OutDir)\chtmp.rc" "$(TargetDir)\..\..\other\Langs\English.lng" $(InputDir)\resource.h "$(MSDEVDIR)\..\..\VC98\MFC\Include\afxres.h" 
	rc.exe /l 0x409 /d "NDEBUG" /d "_AFXDLL" /fo"$(OutDir)\$(InputName).res" "$(OutDir)\chtmp.rc" 
	del "$(OutDir)\chtmp.rc" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Copy Handler - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Compiling resources $(InputPath)
InputDir=.
OutDir=.\Debug
WkspDir=.
TargetDir=\projects\c++\working\Copy Handler\BIN\Debug
InputPath=".\COPY HANDLER.rc"
InputName=COPY HANDLER

"$(OutDir)\$(InputName).res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"BuildManager" "$(WkspDir)\ch_count.txt" $(InputPath) /rcupd 
	Exe2Lng.exe $(InputPath) "$(InputDir)\Scripts\header.lng" "$(OutDir)\chtmp.rc" "$(TargetDir)\..\..\other\Langs\English.lng" $(InputDir)\resource.h "$(MSDEVDIR)\..\..\VC98\MFC\Include\afxres.h" 
	rc.exe /l 0x409 /d "_DEBUG" /d "_AFXDLL" /fo"$(OutDir)\$(InputName).res" "$(OutDir)\chtmp.rc" 
	del "$(OutDir)\chtmp.rc" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Copy Handler - Win32 Final Release"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\crc32.cpp"
# End Source File
# Begin Source File

SOURCE=.\CustomCopyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\DataBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\DestPath.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialogs.cpp
# End Source File
# Begin Source File

SOURCE=.\DirTreeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DstFileErrorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FFListCtrl.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\FileEx.cpp"
# End Source File
# Begin Source File

SOURCE=.\FileInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\FileSupport.cpp
# End Source File
# Begin Source File

SOURCE=.\FilterDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\HelpLngDialog.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\IniFile.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\LanguageDialog.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\LogFile.cpp"
# End Source File
# Begin Source File

SOURCE=.\MainWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MiniViewDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\NotEnoughRoomDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\Plugin.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\PluginContainer.cpp"
# End Source File
# Begin Source File

SOURCE=.\ProgressListBox.cpp
# End Source File
# Begin Source File

SOURCE=.\PropertyListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\RecentDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\register.cpp
# End Source File
# Begin Source File

SOURCE=.\ReplaceFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReplaceOnlyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReplacePathsDlg.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\ResourceManager.cpp"
# End Source File
# Begin Source File

SOURCE=.\shortcuts.cpp
# End Source File
# Begin Source File

SOURCE=.\ShortcutsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ShutdownDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SmallReplaceFilesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StaticEx.cpp
# End Source File
# Begin Source File

SOURCE=.\StatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StringHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\structs.cpp
# End Source File
# Begin Source File

SOURCE=".\Theme Helpers.cpp"
# End Source File
# Begin Source File

SOURCE=.\ThemedButton.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\TrayIcon.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\af_defs.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\AppHelper.h"
# End Source File
# Begin Source File

SOURCE=.\btnIDs.h
# End Source File
# Begin Source File

SOURCE=.\BufferSizeDlg.h
# End Source File
# Begin Source File

SOURCE=.\CfgProperties.h
# End Source File
# Begin Source File

SOURCE=.\charvect.h
# End Source File
# Begin Source File

SOURCE=..\Common\CHPluginCore.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\ConfigEntry.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\ConfigManager.h"
# End Source File
# Begin Source File

SOURCE=".\COPY HANDLER.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\crc32.h"
# End Source File
# Begin Source File

SOURCE=.\CustomCopyDlg.h
# End Source File
# Begin Source File

SOURCE=.\DataBuffer.h
# End Source File
# Begin Source File

SOURCE=.\DestPath.h
# End Source File
# Begin Source File

SOURCE=".\Device IO.h"
# End Source File
# Begin Source File

SOURCE=.\Dialogs.h
# End Source File
# Begin Source File

SOURCE=.\DirTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DstFileErrorDlg.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\ExceptionEx.h"
# End Source File
# Begin Source File

SOURCE=.\FFListCtrl.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\FileEx.h"
# End Source File
# Begin Source File

SOURCE=.\FileInfo.h
# End Source File
# Begin Source File

SOURCE=..\Common\FileSupport.h
# End Source File
# Begin Source File

SOURCE=.\FilterDlg.h
# End Source File
# Begin Source File

SOURCE=.\FolderDialog.h
# End Source File
# Begin Source File

SOURCE=.\HelpLngDialog.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\IniFile.h"
# End Source File
# Begin Source File

SOURCE=..\Common\ipcstructs.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\LanguageDialog.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\LogFile.h"
# End Source File
# Begin Source File

SOURCE=.\MainWnd.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\MemDC.h"
# End Source File
# Begin Source File

SOURCE=.\MiniViewDlg.h
# End Source File
# Begin Source File

SOURCE=.\NotEnoughRoomDlg.h
# End Source File
# Begin Source File

SOURCE=.\OptionsDlg.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\Plugin.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\PluginContainer.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\PluginCore.h"
# End Source File
# Begin Source File

SOURCE=.\ProgressListBox.h
# End Source File
# Begin Source File

SOURCE=.\PropertyListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\RecentDlg.h
# End Source File
# Begin Source File

SOURCE=.\register.h
# End Source File
# Begin Source File

SOURCE=.\ReplaceFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\ReplaceOnlyDlg.h
# End Source File
# Begin Source File

SOURCE=.\ReplacePathsDlg.h
# End Source File
# Begin Source File

SOURCE=.\resource.h

!IF  "$(CFG)" == "Copy Handler - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Generating html help include file...
InputPath=.\resource.h

"..\other\Help\HTMLDefines.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call makehelpmap.bat "$(InputPath)" "..\other\Help\HTMLDefines.h"

# End Custom Build

!ELSEIF  "$(CFG)" == "Copy Handler - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Generating html help include file...
InputPath=.\resource.h

"..\other\Help\HTMLDefines.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call makehelpmap.bat "$(InputPath)" "..\other\Help\HTMLDefines.h"

# End Custom Build

!ELSEIF  "$(CFG)" == "Copy Handler - Win32 Final Release"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Generating html help include file...
InputPath=.\resource.h

"..\other\Help\HTMLDefines.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	call makehelpmap.bat "$(InputPath)" "..\other\Help\HTMLDefines.h"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\ResourceManager.h"
# End Source File
# Begin Source File

SOURCE=.\shortcuts.h
# End Source File
# Begin Source File

SOURCE=.\ShortcutsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ShutdownDlg.h
# End Source File
# Begin Source File

SOURCE=.\SmallReplaceFilesDlg.h
# End Source File
# Begin Source File

SOURCE=.\StaticEx.h
# End Source File
# Begin Source File

SOURCE=.\StatusDlg.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\StringHelpers.h
# End Source File
# Begin Source File

SOURCE=.\structs.h
# End Source File
# Begin Source File

SOURCE=".\Theme Helpers.h"
# End Source File
# Begin Source File

SOURCE=.\ThemedButton.h
# End Source File
# Begin Source File

SOURCE="..\..\..\MODULES\App Framework\TrayIcon.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\addshort.ico
# End Source File
# Begin Source File

SOURCE=.\res\cancelled.ico
# End Source File
# Begin Source File

SOURCE=.\res\cd.ico
# End Source File
# Begin Source File

SOURCE=.\help\ch.gif
# End Source File
# Begin Source File

SOURCE=..\Documentation\Help\EN\ch.gif
# End Source File
# Begin Source File

SOURCE=..\Documentation\Help\PL\ch.gif
# End Source File
# Begin Source File

SOURCE=".\res\COPY HANDLER.ico"
# End Source File
# Begin Source File

SOURCE=".\res\COPY HANDLER.rc2"
# End Source File
# Begin Source File

SOURCE=.\res\delshort.ico
# End Source File
# Begin Source File

SOURCE=.\res\diskette.ico
# End Source File
# Begin Source File

SOURCE=.\res\err.ico
# End Source File
# Begin Source File

SOURCE=.\res\error.ico
# End Source File
# Begin Source File

SOURCE=.\res\finished.ico
# End Source File
# Begin Source File

SOURCE=.\res\folder.ico
# End Source File
# Begin Source File

SOURCE=.\res\hd.ico
# End Source File
# Begin Source File

SOURCE=.\res\Hd2.ico
# End Source File
# Begin Source File

SOURCE=.\res\info.ico
# End Source File
# Begin Source File

SOURCE=.\res\large.ico
# End Source File
# Begin Source File

SOURCE=.\res\list.ico
# End Source File
# Begin Source File

SOURCE=.\res\main_toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\net.ico
# End Source File
# Begin Source File

SOURCE=.\res\newdir.ico
# End Source File
# Begin Source File

SOURCE=.\res\paused.ico
# End Source File
# Begin Source File

SOURCE=.\res\question.ico
# End Source File
# Begin Source File

SOURCE=.\res\report.ico
# End Source File
# Begin Source File

SOURCE=.\res\shut.ico
# End Source File
# Begin Source File

SOURCE=.\res\small.ico
# End Source File
# Begin Source File

SOURCE=.\res\tribe.ico
# End Source File
# Begin Source File

SOURCE=.\res\waiting.ico
# End Source File
# Begin Source File

SOURCE=.\res\warning.ico
# End Source File
# Begin Source File

SOURCE=.\res\working.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\manifest.txt
# End Source File
# Begin Source File

SOURCE=.\RES\Thanks.txt
# End Source File
# Begin Source File

SOURCE=..\other\TODO.TXT
# End Source File
# End Target
# End Project
