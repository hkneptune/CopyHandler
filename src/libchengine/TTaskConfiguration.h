// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
/// @file  TTaskConfiguration.h
/// @date  2010/09/18
/// @brief Contains class responsible for keeping task configuration.
// ============================================================================
#ifndef __TTASKCONFIGURATION_H__
#define __TTASKCONFIGURATION_H__

#include "TConfig.h"

namespace chengine
{
	enum ETaskOptions
	{
		eTO_UseOnlyDefaultBuffer,
		eTO_DefaultBufferSize,
		eTO_OneDiskBufferSize,
		eTO_TwoDisksBufferSize,
		eTO_CDBufferSize,
		eTO_LANBufferSize,
		eTO_DisableBuffering,
		eTO_DisableBufferingMinSize,
		eTO_BufferQueueDepth,
		eTO_MaxReadAheadBuffers,
		eTO_MaxConcurrentReads,
		eTO_MaxConcurrentWrites,

		eTO_FeedbackGeneralFileError,
		eTO_FeedbackFileAlreadyExists,
		eTO_FeedbackNotEnoughSpace,

		eTO_SetDestinationAttributes,
		eTO_ProtectReadOnlyFiles,
		eTO_ScanDirectoriesBeforeBlocking,
		eTO_FastMoveBeforeBlocking,
		eTO_ThreadPriority,
		eTO_DisablePriorityBoost,
		eTO_DeleteInSeparateSubTask,

		eTO_CreateEmptyFiles,
		eTO_CreateDirectoriesRelativeToRoot,
		eTO_IgnoreDirectories,
		eTO_ExcludeEmptyDirectories,

		eTO_AlternateFilenameFormatString_First,
		eTO_AlternateFilenameFormatString_AfterFirst,

		// add new elements before this one
		eTO_Last
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Properties definitions

	template<ETaskOptions PropID> struct TaskPropData;

#define TASK_PROPERTY(enum_id, val_type, val_name, def_value)\
	template<> struct TaskPropData<enum_id>\
{\
	typedef val_type value_type;\
\
	static value_type GetDefaultValue() { return def_value; }\
	static const wchar_t* GetPropertyName() { return val_name; }\
	static void ValidateRange(value_type&) {}\
}

#define TASK_PROPERTY_MINMAX(enum_id, val_type, val_name, def_value, min_val, max_val)\
	template<> struct TaskPropData<enum_id>\
{\
	typedef val_type value_type;\
\
	static value_type GetDefaultValue() { return def_value; }\
	static const wchar_t* GetPropertyName() { return val_name; }\
	static void ValidateRange(value_type& rValue)\
	{\
		if(rValue < (min_val))\
			rValue = (min_val);\
		else if(rValue > (max_val))\
			rValue = (max_val);\
	}\
}

#define TASK_PROPERTY_MIN(enum_id, val_type, val_name, def_value, min_val)\
	template<> struct TaskPropData<enum_id>\
{\
	typedef val_type value_type;\
\
	static value_type GetDefaultValue() { return def_value; }\
	static const wchar_t* GetPropertyName() { return val_name; }\
	static void ValidateRange(value_type& rValue)\
	{\
		if(rValue < (min_val))\
			rValue = (min_val);\
	}\
}

	// Buffer settings
	TASK_PROPERTY(eTO_UseOnlyDefaultBuffer, bool, _T("Buffer.UseOnlyDefaultBuffer"), false);
	TASK_PROPERTY_MIN(eTO_DefaultBufferSize, unsigned int, _T("Buffer.DefaultBufferSize"), 1048576, 1);
	TASK_PROPERTY_MIN(eTO_OneDiskBufferSize, unsigned int, _T("Buffer.OnePhysicalDiskSize"), 2097152, 1);
	TASK_PROPERTY_MIN(eTO_TwoDisksBufferSize, unsigned int, _T("Buffer.TwoPhysicalDisksSize"), 1048576, 1);
	TASK_PROPERTY_MIN(eTO_CDBufferSize, unsigned int, _T("Buffer.CDSize"), 1048576, 1);
	TASK_PROPERTY_MIN(eTO_LANBufferSize, unsigned int, _T("Buffer.LANSize"), 1048576, 1);
	TASK_PROPERTY_MINMAX(eTO_BufferQueueDepth, unsigned int, _T("Buffer.QueueDepth"), 10, 1, 1000);
	TASK_PROPERTY_MINMAX(eTO_MaxReadAheadBuffers, unsigned int, _T("Buffer.MaxReadAhead"), 10, 1, 1000);
	TASK_PROPERTY_MINMAX(eTO_MaxConcurrentReads, unsigned int, _T("Buffer.MaxConcurrentReads"), 3, 1, 1000);
	TASK_PROPERTY_MINMAX(eTO_MaxConcurrentWrites, unsigned int, _T("Buffer.MaxConcurrentWrites"), 2, 1, 1000);

	TASK_PROPERTY(eTO_DisableBuffering, bool, _T("Operation.Buffering.DisableBufferingForLargeFiles"), true);
	TASK_PROPERTY_MIN(eTO_DisableBufferingMinSize, int, _T("Operation.Buffering.MinSizeOfFileToDisableBuffering"), 0, 1);

	TASK_PROPERTY(eTO_SetDestinationAttributes, bool, _T("Operation.SetDestinationAttributes"), true);
	TASK_PROPERTY(eTO_ProtectReadOnlyFiles, bool, _T("Operation.ProtectReadOnlyFiles"), false);
	TASK_PROPERTY(eTO_ScanDirectoriesBeforeBlocking, bool, _T("Operation.ScanForFilesBeforeBlocking"), false);
	TASK_PROPERTY(eTO_FastMoveBeforeBlocking, bool, _T("Operation.FastMoveBeforeBlocking"), false);

	// Thread settings
	TASK_PROPERTY(eTO_ThreadPriority, int, _T("Operation.Thread.Priority"), THREAD_PRIORITY_NORMAL);
	TASK_PROPERTY(eTO_DisablePriorityBoost, bool, _T("Operation.Thread.DisablePriorityBoost"), false);

	// Operation settings
	TASK_PROPERTY(eTO_DeleteInSeparateSubTask, bool, _T("Operation.DeleteFilesInSeparateOperation"), true);

	TASK_PROPERTY(eTO_CreateEmptyFiles, bool, _T("Operation.CreateEmptyFiles"), false);
	TASK_PROPERTY(eTO_CreateDirectoriesRelativeToRoot, bool, _T("Operation.CreateDirectoriesRelativeToRoot"), false);
	TASK_PROPERTY(eTO_IgnoreDirectories, bool, _T("Operation.IgnoreDirectories"), false);
	TASK_PROPERTY(eTO_ExcludeEmptyDirectories, bool, _T("Operation.ExcludeEmptyDirectories"), false);
	
	// Naming settings
	TASK_PROPERTY(eTO_AlternateFilenameFormatString_First, string::TString, _T("Naming.AlternateFilenameFormatFirst"), _T("%name - copy%ext"));
	TASK_PROPERTY(eTO_AlternateFilenameFormatString_AfterFirst, string::TString, _T("Naming.AlternateFilenameFormatAfterFirst"), _T("%name - copy (%count)%ext"));

	/////////////////////////////////////////////////////////////////////////////////////////////
	// other properties names
	//#define TASK_PROP_NAME_FILTERING		_T("Filtering")

	/////////////////////////////////////////////////////////////////////////////////////////////
	// Properties retrieval
	template<ETaskOptions PropID>
	typename TaskPropData<PropID>::value_type GetTaskPropValue(const TConfig& rConfig)
	{
		typename TaskPropData<PropID>::value_type tValue;
		bool bResult = GetConfigValue(rConfig, TaskPropData<PropID>::GetPropertyName(), tValue);
		if (!bResult)
			tValue = TaskPropData<PropID>::GetDefaultValue();

		TaskPropData<PropID>::ValidateRange(tValue);
		return tValue;
	}

	template<ETaskOptions PropID>
	bool GetTaskPropValue(const TConfig& rConfig, typename TaskPropData<PropID>::value_type& rValue)
	{
		bool bResult = GetConfigValue(rConfig, TaskPropData<PropID>::GetPropertyName(), rValue);
		if (bResult)
			TaskPropData<PropID>::ValidateRange(rValue);
		return bResult;
	}

	template<ETaskOptions PropID>
	void SetTaskPropValue(TConfig& rConfig, const typename TaskPropData<PropID>::value_type& rValue)
	{
		SetConfigValue(rConfig, TaskPropData<PropID>::GetPropertyName(), rValue);
	}
}

#endif
