// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#ifndef __TIPCMUTEX_H__
#define __TIPCMUTEX_H__

#include "libchcore.h"

namespace chcore
{
	class LIBCHCORE_API TIpcMutex
	{
	public:
		TIpcMutex();
		TIpcMutex(const TIpcMutex&) = delete;
		explicit TIpcMutex(const wchar_t* pszName);
		~TIpcMutex();

		TIpcMutex& operator=(const TIpcMutex&) = delete;

		void CreateMutex(const wchar_t* pszName);

		void Lock(DWORD dwTimeout = INFINITE);
		void Unlock();

		bool IsLocked() const;
	private:
		void Close();

	private:
		HANDLE m_hMutex = nullptr;
		bool m_bLocked = false;
	};
}

#endif
