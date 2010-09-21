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

#include "FileInfo.h"
#include "FileFilter.h"
#include "DataBuffer.h"

///////////////////////////////////////////////////////////////////////////
// TSubTaskCommonConfig
class TSubTaskCommonConfig
{
public:
	TSubTaskCommonConfig();
	TSubTaskCommonConfig(const TSubTaskCommonConfig& rSrc);
	~TSubTaskCommonConfig();

	TSubTaskCommonConfig& operator=(const TSubTaskCommonConfig& rSrc);

	void SetPriority(int iPriority);
	int GetPriority() const;

	void SetDeleteAllFilesAfterAllCopyings(bool bSeparateDelete);
	bool GetDeleteAllFilesAfterAllCopyings() const;

	void SetIgnoreReadOnlyAttributes(bool bIgnoreReadOnlyAttributes);
	bool GetIgnoreReadOnlyAttributes() const;

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		ar >> m_nPriority;
		ar >> m_bDeleteAllFilesAfterAllCopyings;
		ar >> m_bIgnoreReadOnlyAttributes;
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		ar << m_nPriority;
		ar << m_bDeleteAllFilesAfterAllCopyings;
		ar << m_bIgnoreReadOnlyAttributes;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	int m_nPriority;                    // task priority (really processing thread priority)
	bool m_bDeleteAllFilesAfterAllCopyings;		///< Delete mode; true means that deleting files is a separate sub-operation launched after copying, false states that file is deleted immediately after being copied
	bool m_bIgnoreReadOnlyAttributes; // ignore read-only attributes on files (delete/overwrite) -> this should be handled by feedback requests probably

	mutable boost::shared_mutex m_lock;
};

///////////////////////////////////////////////////////////////////////////
// TSubTaskScanDirectoriesConfig

class TSubTaskScanDirectoriesConfig
{
public:
	TSubTaskScanDirectoriesConfig();
	TSubTaskScanDirectoriesConfig(const TSubTaskScanDirectoriesConfig& rSrc);
	~TSubTaskScanDirectoriesConfig();

	TSubTaskScanDirectoriesConfig& operator=(const TSubTaskScanDirectoriesConfig& rSrc);

	// filtering rules
	void SetFilters(const CFiltersArray& rFilters);
	const CFiltersArray& GetFilters() const { return m_afFilters; }

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		ar >> m_afFilters;
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		ar << m_afFilters;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	CFiltersArray m_afFilters;          // filtering settings for files (will be filtered according to the rules inside when searching for files)

	mutable boost::shared_mutex m_lock;
};

///////////////////////////////////////////////////////////////////////////
// TSubTaskCopyMoveConfig

class TSubTaskCopyMoveConfig
{
public:
	TSubTaskCopyMoveConfig();
	TSubTaskCopyMoveConfig(const TSubTaskCopyMoveConfig& rSrc);
	~TSubTaskCopyMoveConfig();

	TSubTaskCopyMoveConfig& operator=(const TSubTaskCopyMoveConfig& rSrc);

	void SetDisableSystemBuffering(bool bDisableBuffering);
	bool GetDisableSystemBuffering() const;

	void SetMinSizeToDisableBuffering(unsigned long long ullMinSize);
	unsigned long long GetMinSizeToDisableBuffering() const;

	void SetPreserveFileDateTime(bool bPreserve);
	bool GetPreserveFileDateTime() const;

	void SetPreserveFileAttributes(bool bPreserve);
	bool GetPreserveFileAttributes() const;

	void SetBufferSizes(const BUFFERSIZES& bsSizes);
	BUFFERSIZES GetBufferSizes() const;

	void SetIgnoreDirectories(bool bIgnore);
	bool GetIgnoreDirectories() const;

	void SetCreateEmptyFiles(bool bCreateEmpty);
	bool GetCreateEmptyFiles() const;

	void SetCreateOnlyDirectories(bool bCreateOnlyDirs);
	bool GetCreateOnlyDirectories() const;

	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		ar & m_bDisableSystemBuffering;
		ar & m_ullMinSizeToDisableBuffering;

		ar & m_bPreserveFileDateTime;
		ar & m_bPreserveFileAttributes;

		ar & m_bsSizes;

		ar & m_bIgnoreDirectories;
		ar & m_bCreateEmptyFiles;
		ar & m_bCreateOnlyDirectories;
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		ar & m_bDisableSystemBuffering;
		ar & m_ullMinSizeToDisableBuffering;

		ar & m_bPreserveFileDateTime;
		ar & m_bPreserveFileAttributes;

		ar & m_bsSizes;

		ar & m_bIgnoreDirectories;
		ar & m_bCreateEmptyFiles;
		ar & m_bCreateOnlyDirectories;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
	bool m_bDisableSystemBuffering;						///< Disables system buffering of files
	unsigned long long m_ullMinSizeToDisableBuffering;	///< Minimal file size to disable system buffering

	bool m_bPreserveFileDateTime;
	bool m_bPreserveFileAttributes;

	BUFFERSIZES m_bsSizes;              // sizes of buffers used to copy

	bool m_bIgnoreDirectories;
	bool m_bCreateEmptyFiles;
	bool m_bCreateOnlyDirectories;

	mutable boost::shared_mutex m_lock;
};

///////////////////////////////////////////////////////////////////////////
// TSubTaskDeleteConfig

/*
class TSubTaskDeleteConfig
{
private:
	mutable boost::shared_mutex m_lock;
};
*/

///////////////////////////////////////////////////////////////////////////
// TTaskConfiguration

class TTaskConfiguration
{
public:
	TTaskConfiguration();
	~TTaskConfiguration();

	const TSubTaskCommonConfig& GetCommonConfig() const { return m_tCommonConfig; }
	const TSubTaskScanDirectoriesConfig& GetScanDirectoriesConfig() const { return m_tScanDirectoriesConfig; }
	const TSubTaskCopyMoveConfig& GetCopyMoveConfig() const { return m_tCopyMoveConfig; }
//	const TSubTaskDeleteConfig& GetDeleteConfig() const { return m_tDeleteConfig; }

	template<class Archive>
	void serialize(Archive& ar, unsigned int /*uiVersion*/)
	{
		ar & m_tCommonConfig;
		ar & m_tScanDirectoriesConfig;
		ar & m_tCopyMoveConfig;
//		ar & m_tDeleteConfig;
	}

private:
	TSubTaskCommonConfig m_tCommonConfig;
	TSubTaskScanDirectoriesConfig m_tScanDirectoriesConfig;
	TSubTaskCopyMoveConfig m_tCopyMoveConfig;
//	TSubTaskDeleteConfig m_tDeleteConfig;
};

#endif
