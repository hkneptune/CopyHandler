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

/// class encapsulates windows HANDLE, allowing automatic closing it in destructor.
class TAutoFileHandle
{
public:
	// ============================================================================
	/// TAutoFileHandle::TAutoFileHandle
	/// @date 2010/08/26
	///
	/// @brief     Constructs the TAutoFileHandle object.
	// ============================================================================
	TAutoFileHandle() :
		m_hHandle(INVALID_HANDLE_VALUE)
	{
	}

	// ============================================================================
	/// TAutoFileHandle::TAutoFileHandle
	/// @date 2010/08/26
	///
	/// @brief     Constructs the TAutoFileHandle object with specified handle.
	/// @param[in] hHandle - System handle to be managed by this class.
	// ============================================================================
	TAutoFileHandle(HANDLE hHandle) :
		m_hHandle(hHandle)
	{
	}

	// ============================================================================
	/// TAutoFileHandle::~TAutoFileHandle
	/// @date 2010/08/26
	///
	/// @brief     Destructs the TAutoFileHandle object and closes handle if not closed already.
	// ============================================================================
	~TAutoFileHandle()
	{
		VERIFY(Close());
	}

	// ============================================================================
	/// TAutoFileHandle::operator=
	/// @date 2010/08/26
	///
	/// @brief     Assignment operator.
	/// @param[in] hHandle - Handle to be assigned.
	/// @return    Reference to this object,
	// ============================================================================
	TAutoFileHandle& operator=(HANDLE hHandle)
	{
		if(m_hHandle != hHandle)
		{
			VERIFY(Close());
			m_hHandle = hHandle;
		}
		return *this;
	}

	// ============================================================================
	/// TAutoFileHandle::operator HANDLE
	/// @date 2010/08/26
	///
	/// @brief     Retrieves the system handle.
	/// @return    HANDLE value.
	// ============================================================================
	operator HANDLE()
	{
		return m_hHandle;
	}

	// ============================================================================
	/// TAutoFileHandle::Close
	/// @date 2010/08/26
	///
	/// @brief     Closes the internal handle if needed.
	/// @return    Result of the CloseHandle() function.
	// ============================================================================
	BOOL Close()
	{
		BOOL bResult = TRUE;
		if(m_hHandle != INVALID_HANDLE_VALUE)
		{
			bResult = CloseHandle(m_hHandle);
			m_hHandle = INVALID_HANDLE_VALUE;
		}

		return bResult;
	}

	// ============================================================================
	/// TAutoFileHandle::Detach
	/// @date 2010/09/12
	///
	/// @brief     Detaches the handle, so it won't be closed in destructor.
	/// @return	   Returns current handle.
	// ============================================================================
	HANDLE Detach()
	{
		HANDLE hHandle = m_hHandle;
		m_hHandle = INVALID_HANDLE_VALUE;
		return hHandle;
	}

private:
	HANDLE m_hHandle;		///< System handle
};

#endif
