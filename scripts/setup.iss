; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!
#define SETUP_COMPILER 1
#include "../src/common/version.h"
#define MyAppName PRODUCT_NAME
#define MyAppVerName PRODUCT_NAME + " " + PRODUCT_VERSION
#define MyAppPublisher "J�zef Starosczyk"
#define MyAppURL "http://www.copyhandler.com"

#ifndef X86_64
	#define X86_64 0
#endif

#if X86_64
	#define InstallerFilename "chsetup64_" + PRODUCT_VERSION

	#define ExeFilename "ch64.exe"
	#define ShellExtFilename "chext64.dll"
	#define LibicpfFilename "libicpf64u.dll"
	#define LibCHCoreFilename "libchcore64u.dll"
	#define LibictranslateFilename "libictranslate64u.dll"
	#define ICTranslateFilename "ictranslate64.exe"
	#define MSRedistDir "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\amd64"
	#define DbgHelp "C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x64"
#else
	#define InstallerFilename "chsetup32_" + PRODUCT_VERSION

	#define ExeFilename "ch.exe"
	#define ShellExtFilename "chext.dll"
	#define LibicpfFilename "libicpf32u.dll"
	#define LibCHCoreFilename "libchcore32u.dll"
	#define LibictranslateFilename "libictranslate32u.dll"
	#define ICTranslateFilename "ictranslate.exe"
	#define MSRedistDir "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86"
	#define DbgHelp "C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE\Remote Debugger\x86"
#endif

[Setup]
AppName={#MyAppName}
AppVerName={#MyAppVerName}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
#if X86_64
DefaultDirName={pf64}\{#MyAppName}
#else
DefaultDirName={pf}\{#MyAppName}
#endif
DefaultGroupName={#MyAppName}
AllowNoIcons=true
LicenseFile=..\License.txt
OutputDir=.\
OutputBaseFilename={#InstallerFilename}
Compression=lzma/ultra
SolidCompression=true
AppMutex=_Copy handler_ instance
ShowLanguageDialog=auto
AppID={{9CF6A157-F0E8-4216-B229-C0CA8204BE2C}
InternalCompressLevel=ultra
AppCopyright={#COPYRIGHT_INFO}
AppVersion={#PRODUCT_VERSION}
UninstallDisplayIcon={app}\{#ExeFilename}
AppContact=ixen(at)copyhandler(dot)com
VersionInfoVersion=
VersionInfoTextVersion={#PRODUCT_VERSION}
VersionInfoCopyright={#COPYRIGHT_INFO}

[Languages]
Name: english; MessagesFile: compiler:Default.isl
Name: brazilianportuguese; MessagesFile: compiler:Languages\BrazilianPortuguese.isl
Name: catalan; MessagesFile: compiler:Languages\Catalan.isl
Name: czech; MessagesFile: compiler:Languages\Czech.isl
Name: danish; MessagesFile: compiler:Languages\Danish.isl
Name: dutch; MessagesFile: compiler:Languages\Dutch.isl
Name: finnish; MessagesFile: compiler:Languages\Finnish.isl
Name: french; MessagesFile: compiler:Languages\French.isl
Name: german; MessagesFile: compiler:Languages\German.isl
Name: hungarian; MessagesFile: compiler:Languages\Hungarian.isl
Name: italian; MessagesFile: compiler:Languages\Italian.isl
Name: norwegian; MessagesFile: compiler:Languages\Norwegian.isl
Name: polish; MessagesFile: compiler:Languages\Polish.isl
Name: portuguese; MessagesFile: compiler:Languages\Portuguese.isl
Name: russian; MessagesFile: compiler:Languages\Russian.isl
Name: slovak; MessagesFile: compiler:Languages\Slovak.isl
Name: slovenian; MessagesFile: compiler:Languages\Slovenian.isl
Name: basque; MessagesFile: compiler:Languages\Basque.isl
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: startatboot; Description: Run program at system startup; Flags: unchecked; Languages: " spanish basque slovenian slovak russian portuguese norwegian italian hungarian german french finnish dutch danish czech catalan brazilianportuguese english"
Name: startatboot; Description: Uruchom program przy starcie systemu; Flags: unchecked; Languages: polish

[Files]
Source: ..\bin\release\{#ExeFilename}; DestDir: {app}; Flags: ignoreversion
Source: ..\License.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\bin\release\{#ShellExtFilename}; DestDir: {app}; Flags: restartreplace uninsrestartdelete regserver replacesameversion; Tasks: ; Languages: 
Source: ..\bin\release\{#LibicpfFilename}; DestDir: {app}; Flags: ignoreversion
Source: ..\bin\release\{#LibCHCoreFilename}; DestDir: {app}; Flags: ignoreversion
Source: ..\bin\release\{#LibictranslateFilename}; DestDir: {app}; Flags: ignoreversion
Source: ..\bin\release\{#ICTranslateFilename}; DestDir: {app}; Flags: ignoreversion
Source: ..\bin\release\help\*; DestDir: {app}\help; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\bin\release\langs\*; DestDir: {app}\langs; Flags: ignoreversion recursesubdirs createallsubdirs
Source: {#MSRedistDir}\Microsoft.VC90.CRT\*; DestDir: {app}; Flags: ignoreversion
Source: {#MSRedistDir}\Microsoft.VC90.MFC\*; DestDir: {app}; Flags: ignoreversion
Source: {#DbgHelp}\dbghelp.dll; DestDir: {app}; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: {group}\{#MyAppName}; Filename: {app}\{#ExeFilename}; WorkingDir: {app}
Name: {group}\Translate Copy Handler; Filename: {app}\{#ICTranslateFilename}; Parameters: """{app}\langs\english.lng"""; WorkingDir: {app}\lang; Languages: " spanish basque slovenian slovak russian portuguese norwegian italian hungarian german french finnish dutch danish czech catalan brazilianportuguese english"
Name: {userdesktop}\{#MyAppName}; Filename: {app}\{#ExeFilename}; Tasks: desktopicon; WorkingDir: {app}
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}; Filename: {app}\{#ExeFilename}; Tasks: quicklaunchicon; WorkingDir: {app}
Name: {group}\Uninstall Copy Handler; Filename: {app}\unins000.exe; Tasks: ; Languages: " spanish basque slovenian slovak russian portuguese norwegian italian hungarian german french finnish dutch danish czech catalan brazilianportuguese english"; WorkingDir: {app}
Name: {group}\Przet�umacz program Copy Handler; Filename: {app}\{#ICTranslateFilename}; Parameters: """{app}\langs\english.lng"""; WorkingDir: {app}\lang; Languages: polish
Name: {group}\Odinstaluj program Copy Handler; Filename: {app}\unins000.exe; Tasks: ; Languages: polish; WorkingDir: {app}

[Run]
Filename: {app}\{#ExeFilename}; Description: {cm:LaunchProgram,{#MyAppName}}; Flags: nowait postinstall skipifsilent

[Registry]
Root: HKLM; Subkey: SOFTWARE\Microsoft\Windows\CurrentVersion\Run; ValueType: string; ValueName: Copy Handler; Flags: dontcreatekey deletevalue
Root: HKCU; Subkey: SOFTWARE\Microsoft\Windows\CurrentVersion\Run; ValueType: string; ValueName: Copy Handler; Tasks: " startatboot"; ValueData: {app}\{#ExeFilename}; Flags: uninsdeletevalue

[Dirs]
Name: {app}\help; Flags: uninsalwaysuninstall
Name: {app}\langs; Flags: uninsalwaysuninstall

[_ISTool]
UseAbsolutePaths=false
