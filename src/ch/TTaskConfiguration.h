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

	eTO_SetDestinationAttributes,
	eTO_SetDestinationDateTime,
	eTO_ProtectReadOnlyFiles,
	eTO_ScanDirectoriesBeforeBlocking,
	eTO_ThreadPriority,
	eTO_DisablePriorityBoost,
	eTO_DeleteInSeparateSubTask,

	eTO_CreateEmptyFiles,
	eTO_CreateDirectoriesRelativeToRoot,
	eTO_IgnoreDirectories,

	eTO_AlternateFilenameFormatString_First,
	eTO_AlternateFilenameFormatString_AfterFirst,

	// add new elements before this one
	eTO_Last
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Properties definitions

template<ETaskOptions PropID> struct TaskPropData;

#define TASK_PROPERTY(enum_id, val_type, val_name, def_value)\
	template<> struct TaskPropData<enum_id> : public PropDataBase<val_type>\
{\
	static value_type GetDefaultValue() { return def_value; }\
	static const wchar_t* GetPropertyName() { return val_name; }\
}

#define TASK_PROPERTY_MINMAX(enum_id, val_type, val_name, def_value, min_val, max_val)\
	template<> struct TaskPropData<enum_id> : public PropDataMinMaxBase<val_type>\
{\
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

TASK_PROPERTY(eTO_UseOnlyDefaultBuffer, bool, _T("Buffer.UseOnlyDefaultBuffer"), false);
TASK_PROPERTY_MINMAX(eTO_DefaultBufferSize, unsigned int, _T("Buffer.DefaultBufferSize"), 2097152, 1, 0xffffffff);
TASK_PROPERTY_MINMAX(eTO_OneDiskBufferSize, unsigned int, _T("Buffer.OnePhysicalDiskSize"), 4194304, 1, 0xffffffff);
TASK_PROPERTY_MINMAX(eTO_TwoDisksBufferSize, unsigned int, _T("Buffer.TwoPhysicalDisksSize"), 524288, 1, 0xffffffff);
TASK_PROPERTY_MINMAX(eTO_CDBufferSize, unsigned int, _T("Buffer.CDSize"), 262144, 1, 0xffffffff);
TASK_PROPERTY_MINMAX(eTO_LANBufferSize, unsigned int, _T("Buffer.LANSize"), 131072, 1, 0xffffffff);

TASK_PROPERTY(eTO_DisableBuffering, bool, _T("Operation.Buffering.DisableBufferingForLargeFiles"), true);
TASK_PROPERTY_MINMAX(eTO_DisableBufferingMinSize, int, _T("Operation.Buffering.MinSizeOfFileToDisableBuffering"), 2097152, 1, 0xffffffff);

TASK_PROPERTY(eTO_SetDestinationAttributes, bool, _T("Operation.SetDestinationAttributes"), true);
TASK_PROPERTY(eTO_SetDestinationDateTime, bool, _T("Operation.SetDestinationTime"), true);
TASK_PROPERTY(eTO_ProtectReadOnlyFiles, bool, _T("Operation.ProtectReadOnlyFiles"), true);
TASK_PROPERTY(eTO_ScanDirectoriesBeforeBlocking, bool, _T("Operation.ScanForFilesBeforeBlocking"), true);

TASK_PROPERTY(eTO_ThreadPriority, int, _T("Operation.Thread.Priority"), THREAD_PRIORITY_NORMAL);
TASK_PROPERTY(eTO_DisablePriorityBoost, bool, _T("Operation.Thread.DisablePriorityBoost"), false);

TASK_PROPERTY(eTO_DeleteInSeparateSubTask, bool, _T("Operation.DeleteFilesInSeparateOperation"), true);

TASK_PROPERTY(eTO_CreateEmptyFiles, bool, _T("Operation.CreateEmptyFiles"), false);
TASK_PROPERTY(eTO_CreateDirectoriesRelativeToRoot, bool, _T("Operation.CreateDirectoriesRelativeToRoot"), false);
TASK_PROPERTY(eTO_IgnoreDirectories, bool, _T("Operation.IgnoreDirectories"), false);

TASK_PROPERTY(eTO_AlternateFilenameFormatString_First, CString, _T("Naming.AlternateFilenameFormatFirst"), _T("Copy of %name"));
TASK_PROPERTY(eTO_AlternateFilenameFormatString_AfterFirst, CString, _T("Naming.AlternateFilenameFormatAfterFirst"), _T("Copy (%count) of %name"));

/////////////////////////////////////////////////////////////////////////////////////////////
// Properties retrieval
template<ETaskOptions PropID>
typename TaskPropData<PropID>::value_type GetTaskPropValue(const TConfig& rConfig)
{
	return rConfig.GetPropValue<TaskPropData<PropID> >();
}

template<ETaskOptions PropID>
bool GetTaskPropValue(const TConfig& rConfig, typename TaskPropData<PropID>::value_type& rValue)
{
	return rConfig.GetPropValue<TaskPropData<PropID> >(rValue);
}

template<ETaskOptions PropID>
void SetTaskPropValue(TConfig& rConfig, const typename TaskPropData<PropID>::value_type& rValue)
{
	rConfig.SetPropValue<TaskPropData<PropID> >(rValue);
}

#endif
