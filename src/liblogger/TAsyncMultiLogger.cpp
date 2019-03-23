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
#include <boost/format.hpp>

namespace logger
{
	TAsyncMultiLoggerPtr TAsyncMultiLogger::GetInstance()
	{
		static TAsyncMultiLoggerPtr Logger(new TAsyncMultiLogger);
		return Logger;
	}

	TAsyncMultiLogger::TAsyncMultiLogger() :
		m_spStopEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr), CloseHandle),
		m_spStoppedEvent(CreateEvent(nullptr, TRUE, TRUE, nullptr), CloseHandle),
		m_spGlobalRotationInfo(std::make_shared<TLoggerRotationInfo>())
	{
		if (!m_spStopEvent)
			throw std::runtime_error("Cannot create stop event");
	}

	TAsyncMultiLogger::~TAsyncMultiLogger()
	{
		FinishLogging();
	}

	void TAsyncMultiLogger::FinishLogging()
	{
		SetEvent(m_spStopEvent.get());
		if(m_hThread)
		{
			WaitForSingleObject(m_spStoppedEvent.get(), INFINITE);

			// NOTE: for some unknown reason we can't wait for the thread event in shell extension even when the thread
			// exited successfully. For that reason we're synchronizing thread exit with m_spStoppedEvent.
			//WaitForSingleObject(m_hThread, INFINITE);
			CloseHandle(m_hThread);
			m_hThread = nullptr;
		}

		m_setLoggerData.clear();
	}

	TLogFileDataPtr TAsyncMultiLogger::CreateLoggerData(PCTSTR pszLogPath, const TMultiLoggerConfigPtr& spLoggerConfig)
	{
		TLogFileDataPtr spLogFileData = std::make_shared<TLogFileData>(pszLogPath, spLoggerConfig, m_spGlobalRotationInfo);
		if(!m_bLoggingEnabled)
			spLogFileData->DisableLogging();

		boost::unique_lock<boost::shared_mutex> lock(m_mutex);
		m_setLoggerData.insert(spLogFileData);

		if(!m_hThread)
			m_hThread = CreateThread(nullptr, 0, &LoggingThread, this, 0, nullptr);

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

	DWORD TAsyncMultiLogger::LoggingThread(void* pParam)
	{
		TAsyncMultiLogger* pAsyncLogger = (TAsyncMultiLogger*)pParam;

		ResetEvent(pAsyncLogger->m_spStoppedEvent.get());

		try
		{
			bool bStopProcessing = false;
			do
			{
				std::vector<HANDLE> vHandles;
				std::wstring strError;

				{
					boost::unique_lock<boost::shared_mutex> lock(pAsyncLogger->m_mutex);
					vHandles.push_back(pAsyncLogger->m_spStopEvent.get());

					std::transform(pAsyncLogger->m_setLoggerData.begin(), pAsyncLogger->m_setLoggerData.end(), std::back_inserter(vHandles), [](const TLogFileDataPtr& rData) { return rData->GetEntriesEvent().get(); });
				}

				DWORD dwWaitResult = WaitForMultipleObjectsEx(boost::numeric_cast<DWORD>(vHandles.size()), &vHandles[ 0 ], FALSE, 500, FALSE);
				if(dwWaitResult == WAIT_OBJECT_0)
				{
					bStopProcessing = true;
					break;
				}
				if(dwWaitResult == WAIT_FAILED)
				{
					DWORD dwLastError = GetLastError();
					_ASSERTE(dwLastError == ERROR_SUCCESS);

					strError = boost::str(boost::wformat(L"Asynchronous logger critical failure: waiting failed with error %1%. Logging will be disabled until the application will be restarted.") % dwLastError);
					pAsyncLogger->m_bLoggingEnabled = false;

					bStopProcessing = true;
				}

				std::vector<TLogFileDataPtr> vLogs;
				{
					boost::shared_lock<boost::shared_mutex> lock(pAsyncLogger->m_mutex);
					vLogs.insert(vLogs.begin(), pAsyncLogger->m_setLoggerData.begin(), pAsyncLogger->m_setLoggerData.end());
				}

				for(const TLogFileDataPtr& spLogData : vLogs)
				{
					try
					{
						// append emergency message
						if(!strError.empty())
						{
							spLogData->PushLogEntry(strError.c_str());
							spLogData->DisableLogging();
						}

						spLogData->StoreLogEntries();
						spLogData->CloseUnusedFile();
					}
					catch(const std::exception& e)
					{
						e;
						ATLTRACE(e.what());
					}
				}
			}
			while(!bStopProcessing);
		}
		catch(const std::exception&)
		{
		}

		SetEvent(pAsyncLogger->m_spStoppedEvent.get());
		return 0;
	}
}
