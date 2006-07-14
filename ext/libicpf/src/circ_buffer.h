/***************************************************************************
 *   Copyright (C) 2004 by Józef Starosczyk                                *
 *   ixen2@o2.pl                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __CIRCULARBUFFER_H__
#define __CIRCULARBUFFER_H__

#include "libicpf.h"
#include "gen_types.h"

BEGIN_ICPF_NAMESPACE

// forward_seek() results
#define FS_NOTFOUND		-1
#define FS_PARTIAL		0
#define FS_FOUND		1

typedef void(*PFNBITSCALLBACK)(unsigned char uc, void* pParam);

class LIBICPF_API circular_buffer
{
public:
	// construction/destruction
	circular_buffer();
	circular_buffer(const circular_buffer& rSrc);
	~circular_buffer();
	
	circular_buffer& operator=(const circular_buffer& rSrc);
	
	void destroy();		// destroys the contents of this class
	
	// operations
	void push_data(const byte_t* pbyBuffer, size_t tCount);
	void push_data(circular_buffer& rcb);
	
	void push_string(const char_t* pszString);		// pushes length and a string
	void push_ulonglong(ull_t ull);
	void push_ulong(ulong_t ulData);		// pushes an unsigned long value
	void push_ushort(ushort_t wData);
	void push_uchar(uchar_t byData);
	
	size_t pop_data(byte_t* pbyBuffer, size_t tCount);
	bool pop_ulonglong(ull_t* pull);
	bool pop_ulong(ulong_t* pul);
	long pop_string(char_t** pszString);		// returns the length of alloc string (-1 for error)
	bool pop_ushort(ushort_t* pw);
	bool pop_uchar(uchar_t* pby);
	
	// operation on single bits
	void push_bits(ulong_t ulBits, byte_t byCount);
//	void PushBitsFinish();		// finishes the operation of pushing bits, so we could use normal Push/PopData
	void enum_bit_packets(unsigned long ulBitsCount, PFNBITSCALLBACK pfn, void* pParam);
	
	// searching
	int forward_seek(ulong_t ulFnd);			// seeks for the value and skips the bytes previous to it
	size_t find(size_t tStartAt, ulong_t ulFnd);		// searches for the specified value in the buffer, returns an index
													// (size_t)-1 if not found
	
	void skip_bytes(size_t tCount);	// skips some bytes from the beginning of a buffer
	void flush(size_t ulToLeave);	// removes (almost) all the data from a buffer
	void clear() { m_tDataSize=0; };
	
	size_t get_datasize() const { return m_tDataSize; };
	bool is_empty() const { return m_tDataSize == 0; };
	
	operator byte_t*() const { return m_pbyBuffer; };
	byte_t* get_buffer() const { return m_pbyBuffer; };
	
//	void dump();
	
protected:
	void copy_from(const circular_buffer& rSrc);
	void resize_buffer(size_t tNewSize);		// enlarges buffer
	void shrink_buffer();
	
public:
	byte_t *m_pbyBuffer;			// internal buffer
	size_t m_tSize;					// size of the buffer
	size_t m_tDataSize;				// data size inside the buffer (the last byte could be partially filled with data
									// so when using PopData instead of PopBitsX make sure you understand it).
	size_t m_tBitsAtEndCount;		// count of bits in the last byte of the buffer. 0 if the last byte is full of data
};

END_ICPF_NAMESPACE

#endif
