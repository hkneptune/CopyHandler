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
/// @file  TAutoHandles.h
/// @date  2010/09/18
/// @brief Contains implementation of auto-close handles.
// ============================================================================
#ifndef __TAUTOHANDLES_H__
#define __TAUTOHANDLES_H__

namespace chcore
{
	/// class encapsulates windows HANDLE, allowing automatic closing it in destructor.
	class TAutoFileHandle
	{
	public:
		TAutoFileHandle() :
			m_hHandle(INVALID_HANDLE_VALUE)
		{
		}

		TAutoFileHandle(const TAutoFileHandle&) = delete;

		explicit TAutoFileHandle(HANDLE hHandle) :
			m_hHandle(hHandle)
		{
		}

		~TAutoFileHandle()
		{
			Close();
		}

		TAutoFileHandle& operator=(const TAutoFileHandle&) = delete;

		TAutoFileHandle& operator=(HANDLE hHandle)
		{
			if (m_hHandle != hHandle)
			{
				Close();
				m_hHandle = hHandle;
			}
			return *this;
		}

		explicit operator HANDLE() const
		{
			return m_hHandle;
		}

		bool HasValidHandle() const
		{
			return m_hHandle != INVALID_HANDLE_VALUE;
		}

		BOOL Close()
		{
			BOOL bResult = TRUE;
			if (m_hHandle != INVALID_HANDLE_VALUE)
			{
				bResult = CloseHandle(m_hHandle);
				m_hHandle = INVALID_HANDLE_VALUE;
			}

			return bResult;
		}

		HANDLE Detach()
		{
			HANDLE hHandle = m_hHandle;
			m_hHandle = INVALID_HANDLE_VALUE;
			return hHandle;
		}

	private:
		HANDLE m_hHandle;		///< System handle
	};
}

#endif
