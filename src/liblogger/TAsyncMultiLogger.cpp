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
#include "stdafx.h"
#include "TAsyncMultiLogger.h"
#include <thread>
#include <boost/numeric/conversion/cast.hpp>
#include <atltrace.h>

namespace logger
{
	TAsyncMultiLoggerPtr TAsyncMultiLogger::GetInstance()
	{
		static TAsyncMultiLoggerPtr Logger(new TAsyncMultiLogger);
		return Logger;
	}

	TAsyncMultiLogger::TAsyncMultiLogger() :
		m_spStopEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr), CloseHandle),
		m_spGlobalRotationInfo(std::make_shared<TLoggerRotationInfo>())
	{
		if (!m_spStopEvent)
			throw std::runtime_error("Cannot create stop event");
	}

	void TAsyncMultiLogger::FinishLogging()
	{
		std::unique_ptr<std::thread> spThread;

		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);
			SetEvent(m_spStopEvent.get());
			std::swap(m_spThread, spThread);
		}

		if(spThread)
		{
			if(spThread->joinable())
				spThread->join();
		}

		{
			boost::unique_lock<boost::shared_mutex> lock(m_mutex);
			m_setLoggerData.clear();
		}
	}

	TLogFileDataPtr TAsyncMultiLogger::CreateLoggerData(PCTSTR pszLogPath, const TMultiLoggerConfigPtr& spLoggerConfig)
	{
		TLogFileDataPtr spLogFileData = std::make_shared<TLogFileData>(pszLogPath, spLoggerConfig, m_spGlobalRotationInfo);

		boost::unique_lock<boost::shared_mutex> lock(m_mutex);
		m_setLoggerData.insert(spLogFileData);

		if(!m_spThread)
			m_spThread.reset(new std::thread(&TAsyncMultiLogger::LoggingThread, this));

		return spLogFileData;
	}

	TLoggerRotationInfoPtr TAsyncMultiLogger::GetRotationInfo() const
	{
		return m_spGlobalRotationInfo;
	}

	void TAsyncMultiLogger::SetMaxLogSize(unsigned int uiMaxLogSize)
	{
		m_spGlobalRotationInfo->SetMaxLogSize(uiMaxLogSize);
	}

	void TAsyncMultiLogger::SetMaxRotatedCount(unsigned int uiMaxRotatedCount)
	{
		m_spGlobalRotationInfo->SetRotatedCount(uiMaxRotatedCount);
	}

	void TAsyncMultiLogger::LoggingThread()
	{
		std::vector<HANDLE> vHandles;

		bool bStopProcessing = false;
		do
		{
			{
				boost::unique_lock<boost::shared_mutex> lock(m_mutex);
				vHandles.clear();
				vHandles.push_back(m_spStopEvent.get());

				std::transform(m_setLoggerData.begin(), m_setLoggerData.end(), std::back_inserter(vHandles), [](const TLogFileDataPtr& rData) { return rData->GetEntriesEvent().get(); });
			}

			DWORD dwWaitResult = WaitForMultipleObjectsEx(boost::numeric_cast<DWORD>(vHandles.size()), &vHandles[ 0 ], FALSE, 500, FALSE);
			if(dwWaitResult == WAIT_OBJECT_0)
			{
				bStopProcessing = true;
				break;
			}

			std::vector<TLogFileDataPtr> vLogs;
			{
				boost::shared_lock<boost::shared_mutex> lock(m_mutex);
				vLogs.insert(vLogs.begin(), m_setLoggerData.begin(), m_setLoggerData.end());
			}

			for (const TLogFileDataPtr& spLogData : vLogs)
			{
				try
				{
					spLogData->StoreLogEntries();
					spLogData->CloseUnusedFile();
				}
				catch (const std::exception& e)
				{
					e;
					ATLTRACE(e.what());
				}
			}
		}
		while(!bStopProcessing);
	}
}
