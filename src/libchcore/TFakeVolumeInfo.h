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
#ifndef __TFAKEVOLUMEINFO_H__
#define __TFAKEVOLUMEINFO_H__

#include "libchcore.h"
#include "CommonDataTypes.h"

namespace chcore
{
	class LIBCHCORE_API TFakeVolumeInfo
	{
	public:
		TFakeVolumeInfo(file_size_t fsTotalSize, UINT uiDriveType, DWORD dwPhysicalDriveNumber);
		~TFakeVolumeInfo();

		void SetTotalSize(file_size_t fsTotalSize);
		file_size_t GetTotalSize() const;

		void SetDriveType(UINT uiDriveType);
		UINT GetDriveType() const;

		void SetPhysicalDriveNumber(DWORD dwDriveNumber);
		DWORD GetPhysicalDriveNumber() const;

	private:
		file_size_t m_fsTotalSize;
		UINT m_uiDriveType;
		DWORD m_dwPhysicalDriveNumber;
	};
}

#endif
