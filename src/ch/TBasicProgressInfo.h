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
/// @file  TBasicProgressInfo.h
/// @date  2011/03/28
/// @brief Contains class with progress tracking.
// ============================================================================
#ifndef __TBASICPROGRESSINFO_H__
#define __TBASICPROGRESSINFO_H__


///////////////////////////////////////////////////////////////////////////
// TTaskBasicProgressInfo

class TTaskBasicProgressInfo
{
public:
	TTaskBasicProgressInfo();
	~TTaskBasicProgressInfo();

	void SetCurrentIndex(size_t stIndex);	// might be unneeded when serialization is implemented
	void IncreaseCurrentIndex();
	size_t GetCurrentIndex() const;

	void SetCurrentFileProcessedSize(unsigned long long ullSize);
	unsigned long long GetCurrentFileProcessedSize() const;
	void IncreaseCurrentFileProcessedSize(unsigned long long ullSizeToAdd);

	void SetSubOperationIndex(size_t stSubOperationIndex);
	size_t GetSubOperationIndex() const;
	void IncreaseSubOperationIndex();

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		size_t stCurrentIndex = 0;
		ar >> stCurrentIndex;

		unsigned long long ullCurrentFileProcessedSize = 0;
		ar >> ullCurrentFileProcessedSize;

		size_t stSubOperationIndex = 0;
		ar >> stSubOperationIndex;

		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_stCurrentIndex = stCurrentIndex;
		m_ullCurrentFileProcessedSize = ullCurrentFileProcessedSize;
		m_stSubOperationIndex = stSubOperationIndex;
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		m_lock.lock_shared();

		size_t stCurrentIndex = m_stCurrentIndex;
		unsigned long long ullCurrentFileProcessedSize = m_ullCurrentFileProcessedSize;
		size_t stSubOperationIndex = m_stSubOperationIndex;

		m_lock.unlock_shared();

		ar << stCurrentIndex;
		ar << ullCurrentFileProcessedSize;
		ar << stSubOperationIndex;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	volatile size_t m_stSubOperationIndex;		 // index of sub-operation from TOperationDescription
	volatile size_t m_stCurrentIndex;   // index to the m_files array stating currently processed item
	volatile unsigned long long m_ullCurrentFileProcessedSize;	// count of bytes processed for current file

	mutable boost::shared_mutex m_lock;
};

#endif
