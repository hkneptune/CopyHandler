// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TFakeVolumeInfo.h"

BEGIN_CHCORE_NAMESPACE

TFakeVolumeInfo::TFakeVolumeInfo(file_size_t fsTotalSize, UINT uiDriveType, DWORD dwPhysicalDriveNumber) :
	m_fsTotalSize(fsTotalSize),
	m_uiDriveType(uiDriveType),
	m_dwPhysicalDriveNumber(dwPhysicalDriveNumber)
{
}

TFakeVolumeInfo::~TFakeVolumeInfo()
{
}

void TFakeVolumeInfo::SetTotalSize(file_size_t fsTotalSize)
{
	m_fsTotalSize = fsTotalSize;
}

file_size_t TFakeVolumeInfo::GetTotalSize() const
{
	return m_fsTotalSize;
}

void TFakeVolumeInfo::SetDriveType(UINT uiDriveType)
{
	m_uiDriveType = uiDriveType;
}

UINT TFakeVolumeInfo::GetDriveType() const
{
	return m_uiDriveType;
}

void TFakeVolumeInfo::SetPhysicalDriveNumber(DWORD dwDriveNumber)
{
	m_dwPhysicalDriveNumber = dwDriveNumber;
}

DWORD TFakeVolumeInfo::GetPhysicalDriveNumber() const
{
	return m_dwPhysicalDriveNumber;
}

END_CHCORE_NAMESPACE
