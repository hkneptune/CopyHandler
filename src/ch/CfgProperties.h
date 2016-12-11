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

#include "UpdateVersionInfo.h"
#include "../libchengine/TTaskConfiguration.h"

// properties definitions
enum ECHProperties
{
	PP_PCLIPBOARDMONITORING = 0,
	PP_PMONITORSCANINTERVAL,
	PP_PRELOADAFTERRESTART,

	PP_PCHECK_FOR_UPDATES_FREQUENCY,
	PP_PUPDATECHANNEL,
	PP_PUPDATE_USE_SECURE_CONNECTION,

	PP_PSHUTDOWNAFTREFINISHED,
	PP_PTIMEBEFORESHUTDOWN,
	PP_PFORCESHUTDOWN,
	PP_PAUTOSAVEINTERVAL,
	PP_PPROCESSPRIORITYCLASS,
	PP_PLANGUAGE,

	PP_STATUSREFRESHINTERVAL,
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
	PP_CMPROTECTROFILES,

	PP_USECUSTOMNAMING,
	PP_CUSTOMNAME_FIRST,
	PP_CUSTOMNAME_SUBSEQUENT,

	PP_CMLIMITMAXOPERATIONS,
	PP_CMREADSIZEBEFOREBLOCKING,
	PP_CMFASTMOVEBEFOREBLOCKING,

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
	PP_BFQUEUEDEPTH,
	PP_MAXREADAHEAD,
	PP_MAXCONCURRENTREADS,
	PP_MAXCONCURRENTWRITES,

	PP_LOGMAXSIZE,
	PP_LOGROTATECOUNT,
	PP_LOGLEVEL_APP,
	PP_LOGLEVEL_ENGINEDEFAULT,
	PP_LOGLEVEL_SERIALIZER,
	PP_LOGLEVEL_TASK,
	PP_LOGLEVEL_SUBTASK_SCANDIR,
	PP_LOGLEVEL_SUBTASK_COPYMOVE,
	PP_LOGLEVEL_SUBTASK_FASTMOVE,
	PP_LOGLEVEL_SUBTASK_DELETE,
	PP_LOGLEVEL_FILESYSTEM,

	PP_SNDPLAYSOUNDS,
	PP_SNDERRORSOUNDPATH,
	PP_SNDFINISHEDSOUNDPATH,

	PP_SHORTCUTS,
	PP_RECENTPATHS,

	// dialog box "do not show" info
	PP_HIDE_SHELLEXT_UNREGISTERED,
	PP_HIDE_SHELLEXT_VERSIONMISMATCH,

	// invisible options
	PP_LAST_UPDATE_TIMESTAMP,
};

enum EUpdatesFrequency : unsigned int
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

enum EDoNotShowDialog_ShellExtension
{
	eDNS_AlwaysShow = 0,
	eDNS_HideAndRegister = 1,
	eDNS_HideAndDontRegister = 2
};

enum EUseSecureConnection
{
	eSecure_No = 0,
	eSecure_Yes = 1,
	eSecure_Auto = 2
};

///////////////////////////////////////////////////////////////////////////////////////////////
// specific branches in configuration

#define BRANCH_TASK_SETTINGS _T("CHConfig.TaskSettings")

/////////////////////////////////////////////////////////////////////////////////////////////
// Properties definitions

template<ECHProperties PropID> struct PropData;

#define PROPERTY(enum_id, val_type, val_name, def_value)\
	template<> struct PropData<enum_id>\
	{\
		typedef val_type value_type;\
		static value_type GetDefaultValue() { return def_value; }\
		static const wchar_t* GetPropertyName() { return val_name; }\
		static const wchar_t* GetPropertyNamePrefix() { return _T(""); }\
	}

#define PROPERTY_MINMAX(enum_id, val_type, val_name, def_value, min_val, max_val)\
	template<> struct PropData<enum_id>\
	{\
		typedef val_type value_type;\
		static value_type GetDefaultValue() { return def_value; }\
		static const wchar_t* GetPropertyName() { return val_name; }\
		static const wchar_t* GetPropertyNamePrefix() { return _T(""); }\
	}

#define ADAPT_TASK_PROPERTY(enum_id, task_enum_id)\
	template<> struct PropData<enum_id>\
{\
	typedef chengine::TaskPropData<task_enum_id>::value_type value_type;\
	static value_type GetDefaultValue() { return chengine::TaskPropData<task_enum_id>::GetDefaultValue(); }\
	static const wchar_t* GetPropertyName() { return chengine::TaskPropData<task_enum_id>::GetPropertyName(); }\
	static const wchar_t* GetPropertyNamePrefix() { return BRANCH_TASK_SETTINGS _T("."); }\
}

const long long Hour = 3600UL*1000UL;
const long long Minute = 60UL*1000UL;
const long long Second = 1000UL;

typedef std::vector<CString> CStringVector;

///////////////////////////////////////////////////////////////////////////////////////////////
// General settings
PROPERTY(PP_PCLIPBOARDMONITORING, bool, _T("CHConfig.General.Program.EnableClipboardMonitoring"), false);
PROPERTY_MINMAX(PP_PMONITORSCANINTERVAL, unsigned int, _T("CHConfig.General.Program.ClipboardMonitorScanInterval"), 1000, 0, 3600UL*1000UL);
PROPERTY(PP_PRELOADAFTERRESTART, bool, _T("CHConfig.General.Program.RunWithSystem"), false);

PROPERTY_MINMAX(PP_PCHECK_FOR_UPDATES_FREQUENCY, unsigned int, _T("CHConfig.General.Program.Updates.Frequency"), eFreq_Weekly, eFreq_Never, eFreq_Max - 1);
PROPERTY_MINMAX(PP_PUPDATECHANNEL, int, _T("CHConfig.General.Program.Updates.UpdateChannel"), UpdateVersionInfo::eReleaseCandidate, UpdateVersionInfo::eStable, UpdateVersionInfo::eMax - 1);
PROPERTY_MINMAX(PP_PUPDATE_USE_SECURE_CONNECTION, unsigned int, _T("CHConfig.General.Program.Updates.UseSecureConnection"), eSecure_Auto, eSecure_No, eSecure_Auto);

PROPERTY(PP_PPROCESSPRIORITYCLASS, int, _T("CHConfig.General.Program.ProcessPriority"), NORMAL_PRIORITY_CLASS);
PROPERTY(PP_PLANGUAGE, CString, _T("CHConfig.General.Program.Language"), _T("<PROGRAM>\\Langs\\English.lng"));

PROPERTY(PP_SHORTCUTS, CStringVector, _T("CHConfig.General.Program.Shortcuts.Shortcut"), (CStringVector()));
PROPERTY(PP_RECENTPATHS, CStringVector, _T("CHConfig.General.Program.RecentPaths.Path"), (CStringVector()));

PROPERTY_MINMAX(PP_LOGMAXSIZE, unsigned int, _T("CHConfig.General.Logging.SizeLimit"), 1024 * 1024, 1024, 0xffffffff);
PROPERTY_MINMAX(PP_LOGROTATECOUNT, unsigned int, _T("CHConfig.General.Logging.RotateCount"), 5, 1, 0xffffffff);
PROPERTY_MINMAX(PP_LOGLEVEL_APP, int, _T("CHConfig.General.Logging.Level.App"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_ENGINEDEFAULT, int, _T("CHConfig.General.Logging.Level.EngineDefault"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_SERIALIZER, int, _T("CHConfig.General.Logging.Level.Serializer"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_TASK, int, _T("CHConfig.General.Logging.Level.Task"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_SUBTASK_SCANDIR, int, _T("CHConfig.General.Logging.Level.SubtaskScanDir"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_SUBTASK_COPYMOVE, int, _T("CHConfig.General.Logging.Level.SubtaskCopyMove"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_SUBTASK_FASTMOVE, int, _T("CHConfig.General.Logging.Level.SubtaskFastMove"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_SUBTASK_DELETE, int, _T("CHConfig.General.Logging.Level.SubtaskDelete"), logger::warning, logger::trace, logger::fatal);
PROPERTY_MINMAX(PP_LOGLEVEL_FILESYSTEM, int, _T("CHConfig.General.Logging.Level.Filesystem"), logger::warning, logger::trace, logger::fatal);

// GUI
PROPERTY_MINMAX(PP_STATUSREFRESHINTERVAL, unsigned int, _T("CHConfig.GUI.StatusDialog.RefreshInterval"), 1000, 0, 24*Hour);
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

// Task default settings (see TTaskConfiguration.h)
ADAPT_TASK_PROPERTY(PP_BFUSEONLYDEFAULT, chengine::eTO_UseOnlyDefaultBuffer);
ADAPT_TASK_PROPERTY(PP_BFDEFAULT, chengine::eTO_DefaultBufferSize);
ADAPT_TASK_PROPERTY(PP_BFONEDISK, chengine::eTO_OneDiskBufferSize);
ADAPT_TASK_PROPERTY(PP_BFTWODISKS, chengine::eTO_TwoDisksBufferSize);
ADAPT_TASK_PROPERTY(PP_BFCD, chengine::eTO_CDBufferSize);
ADAPT_TASK_PROPERTY(PP_BFLAN, chengine::eTO_LANBufferSize);
ADAPT_TASK_PROPERTY(PP_BFUSENOBUFFERING, chengine::eTO_DisableBuffering);
ADAPT_TASK_PROPERTY(PP_BFBOUNDARYLIMIT, chengine::eTO_DisableBufferingMinSize);
ADAPT_TASK_PROPERTY(PP_BFQUEUEDEPTH, chengine::eTO_BufferQueueDepth);
ADAPT_TASK_PROPERTY(PP_MAXREADAHEAD, chengine::eTO_MaxReadAheadBuffers);
ADAPT_TASK_PROPERTY(PP_MAXCONCURRENTREADS, chengine::eTO_MaxConcurrentReads);
ADAPT_TASK_PROPERTY(PP_MAXCONCURRENTWRITES, chengine::eTO_MaxConcurrentWrites);

ADAPT_TASK_PROPERTY(PP_CMSETDESTATTRIBUTES, chengine::eTO_SetDestinationAttributes);
ADAPT_TASK_PROPERTY(PP_CMPROTECTROFILES, chengine::eTO_ProtectReadOnlyFiles);
ADAPT_TASK_PROPERTY(PP_CMREADSIZEBEFOREBLOCKING, chengine::eTO_ScanDirectoriesBeforeBlocking);
ADAPT_TASK_PROPERTY(PP_CMFASTMOVEBEFOREBLOCKING, chengine::eTO_FastMoveBeforeBlocking);
ADAPT_TASK_PROPERTY(PP_CMDEFAULTPRIORITY, chengine::eTO_ThreadPriority);
ADAPT_TASK_PROPERTY(PP_CMDISABLEPRIORITYBOOST, chengine::eTO_DisablePriorityBoost);
ADAPT_TASK_PROPERTY(PP_CMDELETEAFTERFINISHED, chengine::eTO_DeleteInSeparateSubTask);

PROPERTY(PP_USECUSTOMNAMING, bool, _T("CHConfig.Core.Naming.UseCustomNaming"), false);
PROPERTY(PP_CUSTOMNAME_FIRST, CString, _T("CHConfig.Core.Naming.FirstCustomName"), _T(""));
PROPERTY(PP_CUSTOMNAME_SUBSEQUENT, CString, _T("CHConfig.Core.Naming.SubsequentCustomName"), _T(""));

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
PROPERTY(PP_SHSHOWSHELLICONS, bool, _T("CHConfig.ShellExtension.ShowShortcutsShellIcons"), true);
PROPERTY(PP_SHINTERCEPTDRAGDROP, bool, _T("CHConfig.ShellExtension.InterceptDragDrop"), true);
PROPERTY(PP_SHINTERCEPTKEYACTIONS, bool, _T("CHConfig.ShellExtension.InterceptKeyboardActions"), true);
PROPERTY(PP_SHINTERCEPTCTXMENUACTIONS, bool, _T("CHConfig.ShellExtension.InterceptDefaultContextMenuActions"), false);

// "do not show" dialog boxes
PROPERTY(PP_HIDE_SHELLEXT_UNREGISTERED, int, _T("CHConfig.GUI.ShowHideDialogs.ShellExtensionUnregistered"), eDNS_AlwaysShow);
PROPERTY(PP_HIDE_SHELLEXT_VERSIONMISMATCH, int, _T("CHConfig.GUI.ShowHideDialogs.ShellExtensionVersionMismatch"), eDNS_AlwaysShow);

// Invisible options
PROPERTY_MINMAX(PP_LAST_UPDATE_TIMESTAMP, long long, _T("CHConfig.RuntimeState.LastCheckedForUpdates"), 0, 0, LLONG_MAX);

/////////////////////////////////////////////////////////////////////////////////////////////
// Properties retrieval

template<ECHProperties PropID>
typename PropData<PropID>::value_type GetPropValue(const chengine::TConfig& rConfig)
{
	typename PropData<PropID>::value_type tValue;
	if(!GetConfigValue(rConfig, CString(PropData<PropID>::GetPropertyNamePrefix()) + PropData<PropID>::GetPropertyName(), tValue))
		tValue = PropData<PropID>::GetDefaultValue();
	return tValue;
}

template<ECHProperties PropID>
bool GetPropValue(const chengine::TConfig& rConfig, typename PropData<PropID>::value_type& rValue)
{
	bool bResult = GetConfigValue(rConfig, CString(PropData<PropID>::GetPropertyNamePrefix()) + PropData<PropID>::GetPropertyName(), rValue);
	if(!bResult)
		rValue = PropData<PropID>::GetDefaultValue();

	return bResult;
}

template<ECHProperties PropID>
void SetPropValue(chengine::TConfig& rConfig, const typename PropData<PropID>::value_type& rValue)
{
	SetConfigValue(rConfig, CString(PropData<PropID>::GetPropertyNamePrefix()) + PropData<PropID>::GetPropertyName(), rValue);
}

#endif
