// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TSharedMemory.h
/// @date  2011/05/03
/// @brief Contains shared memory support classes.
// ============================================================================
#ifndef __TSHAREDMEMORY_H__
#define __TSHAREDMEMORY_H__

#include "TString.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TSharedMemory
{
public:
	TSharedMemory();
	~TSharedMemory();

	void Create(const wchar_t* pszName, size_t stSize);
	void Create(const wchar_t* pszName, const TString& wstrData);
	void Create(const wchar_t* pszName, const BYTE* pbyData, size_t stSize);

	void Open(const wchar_t* pszName);
	void Close() throw();

	void Read(TString& wstrData) const;
	void Write(const TString& wstrData);
	void Write(const BYTE* pbyData, size_t stSize);

	// below are the unsafe functions (i.e. not protected with mutex)
	const BYTE* GetData() const;
	BYTE* GetData();

	size_t GetSharedMemorySize() const;
	size_t GetDataSize() const;

private:
	HANDLE m_hFileMapping;
	BYTE* m_pMappedMemory;
	size_t m_stSize;     // contains full size of the allocated shared memory (in case we created the memory), size of occupied memory in case we opened the memory.

	HANDLE m_hMutex;
};

END_CHCORE_NAMESPACE

#endif
