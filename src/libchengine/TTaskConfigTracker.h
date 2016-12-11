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
#ifndef __TCONFIGTRACKER_H__
#define __TCONFIGTRACKER_H__

#include "TTaskConfiguration.h"

namespace chengine
{
	class LIBCHENGINE_API TOptionsSet
	{
	public:
		TOptionsSet& operator%(ETaskOptions eOption);

		std::set<ETaskOptions>& Get();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::set<ETaskOptions> m_setOptions;
#pragma warning(pop)
	};

	class LIBCHENGINE_API TTaskConfigTracker
	{
	public:
		TTaskConfigTracker();
		TTaskConfigTracker(const TTaskConfigTracker&) = delete;
		~TTaskConfigTracker();

		TTaskConfigTracker& operator=(const TTaskConfigTracker&) = delete;

		bool IsModified() const;
		bool IsModified(ETaskOptions eOption) const;
		bool IsModified(TOptionsSet setOptions) const;
		bool IsModified(ETaskOptions eOption, bool bResetModificationState);
		bool IsModified(TOptionsSet setOptions, bool bResetModificationState);

		void AddModified(const string::TString& strModified);
		void AddModified(ETaskOptions eModified);
		void AddModified(TOptionsSet setOptions);
		void AddModified(const string::TStringSet& setModified);
		void AddModified(const std::set<ETaskOptions>& setModified);

		void RemoveModification(ETaskOptions eModified);
		void RemoveModificationSet(TOptionsSet setOptions);
		void RemoveModification(const string::TString& strModified);
		void Clear();

		static void NotificationProc(const string::TStringSet& setModifications, void* pParam);

	protected:
		static ETaskOptions GetOptionFromString(const string::TString& strOption);

	protected:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::set<ETaskOptions> m_setModified;
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)
	};
}

#endif // __TCONFIGTRACKER_H__
