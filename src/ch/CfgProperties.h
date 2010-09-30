/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef __PROPERTYTYPES_H__
#define __PROPERTYTYPES_H__

#pragma once

class TConfig;

// properties definitions
enum ECHProperties
{
	PP_PCLIPBOARDMONITORING = 0,
	PP_PMONITORSCANINTERVAL,
	PP_PRELOADAFTERRESTART,
	PP_PCHECK_FOR_UPDATES_FREQUENCY,
	PP_PUPDATE_CHECK_FOR_BETA,
	PP_PSHUTDOWNAFTREFINISHED,
	PP_PTIMEBEFORESHUTDOWN,
	PP_PFORCESHUTDOWN,
	PP_PAUTOSAVEINTERVAL,
	PP_PPROCESSPRIORITYCLASS,
	PP_PLANGUAGE,

	PP_STATUSREFRESHINTERVAL,
	PP_STATUSSHOWDETAILS,
	PP_STATUSAUTOREMOVEFINISHED,

	PP_FDWIDTH,
	PP_FDHEIGHT,
	PP_FDSHORTCUTLISTSTYLE,
	PP_FDEXTENDEDVIEW,
	PP_FDIGNORESHELLDIALOGS,

	PP_MVSHOWFILENAMES,
	PP_MVSHOWSINGLETASKS,
	PP_MVREFRESHINTERVAL,
	PP_MVAUTOSHOWWHENRUN,
	PP_MVAUTOHIDEWHENEMPTY,
	PP_MVUSESMOOTHPROGRESS,

	PP_CMSETDESTATTRIBUTES,
	PP_CMSETDESTDATE,
	PP_CMPROTECTROFILES,
	PP_CMLIMITMAXOPERATIONS,
	PP_CMREADSIZEBEFOREBLOCKING,

	PP_CMDEFAULTPRIORITY,
	PP_CMDISABLEPRIORITYBOOST,
	PP_CMDELETEAFTERFINISHED,

	PP_SHSHOWCOPY,
	PP_SHSHOWMOVE,
	PP_SHSHOWCOPYMOVE,
	PP_SHSHOWPASTE,
	PP_SHSHOWPASTESPECIAL,
	PP_SHSHOWCOPYTO,
	PP_SHSHOWMOVETO,
	PP_SHSHOWCOPYMOVETO,
	PP_SHSHOWFREESPACE,
	PP_SHSHOWSHELLICONS,
	PP_SHINTERCEPTDRAGDROP,
	PP_SHINTERCEPTKEYACTIONS,
	PP_SHINTERCEPTCTXMENUACTIONS,

	PP_BFUSEONLYDEFAULT,
	PP_BFDEFAULT,
	PP_BFONEDISK,
	PP_BFTWODISKS,
	PP_BFCD,
	PP_BFLAN,
	PP_BFUSENOBUFFERING,
	PP_BFBOUNDARYLIMIT,

	PP_LOGENABLELOGGING,
	PP_LOGMAXSIZE,
	PP_LOGLEVEL,

	PP_SNDPLAYSOUNDS,
	PP_SNDERRORSOUNDPATH,
	PP_SNDFINISHEDSOUNDPATH,

	PP_SHORTCUTS,
	PP_RECENTPATHS,

	// invisible options
	PP_LAST_UPDATE_TIMESTAMP
};

enum EUpdatesFrequency
{
	eFreq_Never,
	eFreq_EveryStartup,
	eFreq_Daily,
	eFreq_Weekly,
	eFreq_OnceEvery2Weeks,
	eFreq_Monthly,
	eFreq_Quarterly,
	eFreq_Max
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Properties definitions

template<ECHProperties PropID> struct PropData;

#define PROPERTY(enum_id, val_type, val_name, def_value)\
	template<> struct PropData<enum_id>\
	{\
		typedef val_type value_type;\
		static value_type GetDefaultValue() { return def_value; }\
		static const wchar_t* GetPropertyName() { return val_name; }\
	}

#define PROPERTY_MINMAX(enum_id, val_type, val_name, def_value, min_val, max_val)\
	template<> struct PropData<enum_id>\
	{\
		typedef val_type value_type;\
		static value_type GetDefaultValue() { return def_value; }\
		static const wchar_t* GetPropertyName() { return val_name; }\
	}

const long long Hour = 3600UL*1000UL;
const long long Minute = 60UL*1000UL;
const long long Second = 1000UL;

typedef std::vector<CString> CStringVector;

// General settings
PROPERTY(PP_PCLIPBOARDMONITORING, bool, _T("CHConfig.General.Program.EnableClipboardMonitoring"), false);
PROPERTY_MINMAX(PP_PMONITORSCANINTERVAL, unsigned int, _T("CHConfig.General.Program.ClipboardMonitorScanInterval"), 1000, 0, 3600UL*1000UL);
PROPERTY(PP_PRELOADAFTERRESTART, bool, _T("CHConfig.General.Program.RunWithSystem"), false);

PROPERTY_MINMAX(PP_PCHECK_FOR_UPDATES_FREQUENCY, unsigned int, _T("CHConfig.General.Program.Updates.Frequency"), eFreq_Weekly, eFreq_Never, eFreq_Max - 1);
PROPERTY(PP_PUPDATE_CHECK_FOR_BETA, bool, _T("CHConfig.General.Program.Updates.CheckForBetaVersions"), true);

PROPERTY(PP_PPROCESSPRIORITYCLASS, int, _T("CHConfig.General.Program.ProcessPriority"), NORMAL_PRIORITY_CLASS);
PROPERTY(PP_PLANGUAGE, CString, _T("CHConfig.General.Program.Language"), _T("<PROGRAM>\\Langs\\English.lng"));

PROPERTY(PP_SHORTCUTS, CStringVector, _T("CHConfig.General.Program.Shortcuts.Shortcut"), (CStringVector()));
PROPERTY(PP_RECENTPATHS, CStringVector, _T("CHConfig.General.Program.RecentPaths.Path"), (CStringVector()));

PROPERTY(PP_LOGENABLELOGGING, bool, _T("CHConfig.General.Logging.Enable"), true);
PROPERTY_MINMAX(PP_LOGMAXSIZE, int, _T("CHConfig.General.Logging.SizeLimit"), 512384, 1024, 0xffffffff);
PROPERTY_MINMAX(PP_LOGLEVEL, unsigned int, _T("CHConfig.General.Logging.LoggingLevel"), 1, 0, 3);

// GUI
PROPERTY_MINMAX(PP_STATUSREFRESHINTERVAL, unsigned int, _T("CHConfig.GUI.StatusDialog.RefreshInterval"), 1000, 0, 24*Hour);
PROPERTY(PP_STATUSSHOWDETAILS, bool, _T("CHConfig.GUI.StatusDialog.ShowDetails"), true);
PROPERTY(PP_STATUSAUTOREMOVEFINISHED, bool, _T("CHConfig.GUI.StatusDialog.AutoRemoveFinishedTasks"), false);

PROPERTY_MINMAX(PP_FDWIDTH, int, _T("CHConfig.GUI.FolderDialog.Width"), -1, -1, 32767);
PROPERTY_MINMAX(PP_FDHEIGHT, int, _T("CHConfig.GUI.FolderDialog.Height"), -1, -1, 32767);
PROPERTY_MINMAX(PP_FDSHORTCUTLISTSTYLE, int, _T("CHConfig.GUI.FolderDialog.ShortcutListStyle"), 1, 0, 3);
PROPERTY(PP_FDEXTENDEDVIEW, bool, _T("CHConfig.GUI.FolderDialog.ExtendedView"), true);
PROPERTY(PP_FDIGNORESHELLDIALOGS, bool, _T("CHConfig.GUI.FolderDialog.IgnoreShellDialogs"), false);

PROPERTY(PP_MVSHOWFILENAMES, bool, _T("CHConfig.GUI.MiniView.ShowFilenames"), true);
PROPERTY(PP_MVSHOWSINGLETASKS, bool, _T("CHConfig.GUI.MiniView.ShowSingleTasks"), true);
PROPERTY_MINMAX(PP_MVREFRESHINTERVAL, unsigned int, _T("CHConfig.GUI.MiniView.RefreshInterval"), 200, 0, 24*Hour);
PROPERTY(PP_MVAUTOSHOWWHENRUN, bool, _T("CHConfig.GUI.MiniView.AutoShowOnStartup"), true);
PROPERTY(PP_MVAUTOHIDEWHENEMPTY, bool, _T("CHConfig.GUI.MiniView.AutohideWhenEmpty"), true);
PROPERTY(PP_MVUSESMOOTHPROGRESS, bool, _T("CHConfig.GUI.MiniView.UseSmoothProgress"), true);

// Core engine
PROPERTY_MINMAX(PP_PAUTOSAVEINTERVAL, unsigned int, _T("CHConfig.Core.AutosaveInterval"), 30*Second, 0, 24*Hour);

PROPERTY(PP_PSHUTDOWNAFTREFINISHED, bool, _T("CHConfig.Core.Shutdown.ShutdownSystemAfterFinished"), false);
PROPERTY_MINMAX(PP_PTIMEBEFORESHUTDOWN, int, _T("CHConfig.Core.Shutdown.TimeBeforeShutdown"), 10000, 0, 24*Hour);
PROPERTY(PP_PFORCESHUTDOWN, bool, _T("CHConfig.Core.Shutdown.ForceShutdown"), false);

PROPERTY(PP_SNDPLAYSOUNDS, bool, _T("CHConfig.Core.Notifications.Sounds.Enable"), true);
PROPERTY(PP_SNDERRORSOUNDPATH, CString, _T("CHConfig.Core.Notifications.Sounds.ErrorSoundPath"), _T("<WINDOWS>\\media\\chord.wav"));
PROPERTY(PP_SNDFINISHEDSOUNDPATH, CString, _T("CHConfig.Core.Notifications.Sounds.FinishedSoundPath"), _T("<WINDOWS>\\media\\ding.wav"));

PROPERTY(PP_CMLIMITMAXOPERATIONS, unsigned int, _T("CHConfig.Core.Operation.LimitMaxOperations"), 1);

PROPERTY(PP_CMSETDESTATTRIBUTES, bool, _T("CHConfig.Core.Operation.SetDestinationAttributes"), true);
PROPERTY(PP_CMSETDESTDATE, bool, _T("CHConfig.Core.Operation.SetDestinationTime"), true);
PROPERTY(PP_CMPROTECTROFILES, bool, _T("CHConfig.Core.Operation.ProtectReadOnlyFiles"), true);
PROPERTY(PP_CMREADSIZEBEFOREBLOCKING, bool, _T("CHConfig.Core.Operation.ScanForFilesBeforeBlocking"), true);
PROPERTY(PP_CMDEFAULTPRIORITY, int, _T("CHConfig.Core.Operation.Thread.Priority"), THREAD_PRIORITY_NORMAL);
PROPERTY(PP_CMDISABLEPRIORITYBOOST, bool, _T("CHConfig.Core.Operation.Thread.DisablePriorityBoost"), false);
PROPERTY(PP_CMDELETEAFTERFINISHED, bool, _T("CHConfig.Core.Operation.DeleteDilesInSeparateOperation"), true);

PROPERTY(PP_BFUSEONLYDEFAULT, bool, _T("CHConfig.Core.Operation.Buffer.UseOnlyDefaultBuffer"), false);
PROPERTY_MINMAX(PP_BFDEFAULT, unsigned int, _T("CHConfig.Core.Operation.Buffer.DefaultBufferSize"), 2097152, 1, 0xffffffff);
PROPERTY_MINMAX(PP_BFONEDISK, unsigned int, _T("CHConfig.Core.Operation.Buffer.OnePhysicalDiskSize"), 4194304, 1, 0xffffffff);
PROPERTY_MINMAX(PP_BFTWODISKS, unsigned int, _T("CHConfig.Core.Operation.Buffer.TwoPhysicalDisksSize"), 524288, 1, 0xffffffff);
PROPERTY_MINMAX(PP_BFCD, unsigned int, _T("CHConfig.Core.Operation.Buffer.CDSize"), 262144, 1, 0xffffffff);
PROPERTY_MINMAX(PP_BFLAN, unsigned int, _T("CHConfig.Core.Operation.Buffer.LANSize"), 131072, 1, 0xffffffff);
PROPERTY(PP_BFUSENOBUFFERING, bool, _T("CHConfig.Core.Operation.Buffering.DisableBufferingForLargeFiles"), true);
PROPERTY_MINMAX(PP_BFBOUNDARYLIMIT, int, _T("CHConfig.Core.Operation.Buffering.MinSizeOfFileToDisableBuffering"), 2097152, 1, 0xffffffff);

// Shell extension
PROPERTY(PP_SHSHOWCOPY, bool, _T("CHConfig.ShellExtension.ShowCommands.Copy"), true);
PROPERTY(PP_SHSHOWMOVE, bool, _T("CHConfig.ShellExtension.ShowCommands.Move"), true);
PROPERTY(PP_SHSHOWCOPYMOVE, bool, _T("CHConfig.ShellExtension.ShowCommands.CopyMoveSpecial"), true);
PROPERTY(PP_SHSHOWPASTE, bool, _T("CHConfig.ShellExtension.ShowCommands.Paste"), true);
PROPERTY(PP_SHSHOWPASTESPECIAL, bool, _T("CHConfig.ShellExtension.ShowCommands.PasteSpecial"), true);
PROPERTY(PP_SHSHOWCOPYTO, bool, _T("CHConfig.ShellExtension.ShowCommands.CopyTo"), true);
PROPERTY(PP_SHSHOWMOVETO, bool, _T("CHConfig.ShellExtension.ShowCommands.MoveTo"), true);
PROPERTY(PP_SHSHOWCOPYMOVETO, bool, _T("CHConfig.ShellExtension.ShowCommands.CopyToMoveToSpecial"), true);
PROPERTY(PP_SHSHOWFREESPACE, bool, _T("CHConfig.ShellExtension.ShowFreeSpaceAlongShortcuts"), true);
PROPERTY(PP_SHSHOWSHELLICONS, bool, _T("CHConfig.ShellExtension.ShowShortcutsShellIcons"), false);
PROPERTY(PP_SHINTERCEPTDRAGDROP, bool, _T("CHConfig.ShellExtension.InterceptDragDrop"), true);
PROPERTY(PP_SHINTERCEPTKEYACTIONS, bool, _T("CHConfig.ShellExtension.ShowCommands.InterceptKeyboardActions"), true);
PROPERTY(PP_SHINTERCEPTCTXMENUACTIONS, bool, _T("CHConfig.ShellExtension.ShowCommands.InterceptDefaultContextMenuActions"), false);

// Invisible options
PROPERTY_MINMAX(PP_LAST_UPDATE_TIMESTAMP, long long, _T("CHConfig.RuntimeState.LastCheckedForUpdates"), 0, 0, LLONG_MAX);

/////////////////////////////////////////////////////////////////////////////////////////////
// Properties retrieval

template<ECHProperties PropID>
typename PropData<PropID>::value_type GetPropValue(const TConfig& rConfig)
{
	typename PropData<PropID>::value_type tValue;
	rConfig.GetValue(PropData<PropID>::GetPropertyName(), tValue, PropData<PropID>::GetDefaultValue());
	return tValue;
}

template<ECHProperties PropID>
bool GetPropValue(const TConfig& rConfig, typename PropData<PropID>::value_type& rValue)
{
	return rConfig.GetValue(PropData<PropID>::GetPropertyName(), rValue, PropData<PropID>::GetDefaultValue());
}

template<ECHProperties PropID>
void SetPropValue(TConfig& rConfig, const typename PropData<PropID>::value_type& rValue)
{
	rConfig.SetValue(PropData<PropID>::GetPropertyName(), rValue);
}

#endif
