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
/// @file  TTaskConfiguration.cpp
/// @date  2010/09/18
/// @brief Contains implementation of class TTaskConfiguration.
// ============================================================================
#include "stdafx.h"
#include "TTaskConfiguration.h"


///////////////////////////////////////////////////////////////////////////
// TSubTaskCommonConfig

TSubTaskCommonConfig::TSubTaskCommonConfig() :
	m_nPriority(THREAD_PRIORITY_NORMAL),
	m_bDeleteAllFilesAfterAllCopyings(true),
	m_bIgnoreReadOnlyAttributes(false)
{
}

TSubTaskCommonConfig::~TSubTaskCommonConfig()
{
}

void TSubTaskCommonConfig::SetPriority(int iPriority)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_nPriority = iPriority;
}

int TSubTaskCommonConfig::GetPriority() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_nPriority;
}

void TSubTaskCommonConfig::SetDeleteAllFilesAfterAllCopyings(bool bSeparateDelete)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bDeleteAllFilesAfterAllCopyings = bSeparateDelete;
}

bool TSubTaskCommonConfig::GetDeleteAllFilesAfterAllCopyings() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bDeleteAllFilesAfterAllCopyings;
}

void TSubTaskCommonConfig::SetIgnoreReadOnlyAttributes(bool bIgnoreReadOnlyAttributes)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bIgnoreReadOnlyAttributes = bIgnoreReadOnlyAttributes;
}

bool TSubTaskCommonConfig::GetIgnoreReadOnlyAttributes() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bIgnoreReadOnlyAttributes;
}

////////////////////////////////////////////////////////////////////////////
// class TSubTaskScanDirectoriesConfig

TSubTaskScanDirectoriesConfig::TSubTaskScanDirectoriesConfig()
{
}

TSubTaskScanDirectoriesConfig::~TSubTaskScanDirectoriesConfig()
{
}

void TSubTaskScanDirectoriesConfig::SetFilters(const CFiltersArray& rFilters)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_afFilters = rFilters;
}

////////////////////////////////////////////////////////////////////////////
// class TSubTaskCopyMoveConfig

TSubTaskCopyMoveConfig::TSubTaskCopyMoveConfig() :
	m_bDisableSystemBuffering(false),
	m_ullMinSizeToDisableBuffering(0),
	m_bPreserveFileDateTime(false),
	m_bPreserveFileAttributes(false),
	m_bIgnoreDirectories(false),
	m_bCreateEmptyFiles(false),
	m_bCreateOnlyDirectories(false)
{
	m_bsSizes.m_uiDefaultSize = 0;
	m_bsSizes.m_uiOneDiskSize = 0;
	m_bsSizes.m_uiTwoDisksSize = 0;
	m_bsSizes.m_uiCDSize = 0;
	m_bsSizes.m_uiLANSize = 0;
	m_bsSizes.m_bOnlyDefault = false;
}

TSubTaskCopyMoveConfig::~TSubTaskCopyMoveConfig()
{
}

void TSubTaskCopyMoveConfig::SetDisableSystemBuffering(bool bDisableBuffering)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bDisableSystemBuffering = bDisableBuffering;
}

bool TSubTaskCopyMoveConfig::GetDisableSystemBuffering() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bDisableSystemBuffering;
}

void TSubTaskCopyMoveConfig::SetMinSizeToDisableBuffering(unsigned long long ullMinSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullMinSizeToDisableBuffering = ullMinSize;
}

unsigned long long TSubTaskCopyMoveConfig::GetMinSizeToDisableBuffering() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_ullMinSizeToDisableBuffering;
}

void TSubTaskCopyMoveConfig::SetPreserveFileDateTime(bool bPreserve)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bPreserveFileDateTime = bPreserve;
}

bool TSubTaskCopyMoveConfig::GetPreserveFileDateTime() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bPreserveFileDateTime;
}

void TSubTaskCopyMoveConfig::SetPreserveFileAttributes(bool bPreserve)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bPreserveFileAttributes = bPreserve;
}

bool TSubTaskCopyMoveConfig::GetPreserveFileAttributes() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bPreserveFileAttributes;
}

void TSubTaskCopyMoveConfig::SetBufferSizes(const BUFFERSIZES& bsSizes)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bsSizes = bsSizes;
}

BUFFERSIZES TSubTaskCopyMoveConfig::GetBufferSizes() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bsSizes;
}

void TSubTaskCopyMoveConfig::SetIgnoreDirectories(bool bIgnore)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bIgnoreDirectories = bIgnore;
}

bool TSubTaskCopyMoveConfig::GetIgnoreDirectories() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bIgnoreDirectories;
}

void TSubTaskCopyMoveConfig::SetCreateEmptyFiles(bool bCreateEmpty)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bCreateEmptyFiles = bCreateEmpty;
}

bool TSubTaskCopyMoveConfig::GetCreateEmptyFiles() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bCreateEmptyFiles;
}

void TSubTaskCopyMoveConfig::SetCreateOnlyDirectories(bool bCreateOnlyDirs)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bCreateOnlyDirectories = bCreateOnlyDirs;
}

bool TSubTaskCopyMoveConfig::GetCreateOnlyDirectories() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bCreateOnlyDirectories;
}

////////////////////////////////////////////////////////////////////////////
// class TTaskConfiguration

TTaskConfiguration::TTaskConfiguration()
{
}

TTaskConfiguration::~TTaskConfiguration()
{
}
