/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "Stdafx.h"
#include "task.h"
#include "StringHelpers.h"
#include "../common/FileSupport.h"
#include "ch.h"
#include "FeedbackHandler.h"
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>

// assume max sectors of 4kB (for rounding)
#define MAXSECTORSIZE			4096

///////////////////////////////////////////////////////////////////////
// CProcessingException

CProcessingException::CProcessingException(int iType, CTask* pTask, UINT uiFmtID, DWORD dwError, ...)
{
	// std values
	m_iType=iType;
	m_pTask=pTask;
	m_dwError=dwError;

	// format some text
	CString strFormat=GetResManager().LoadString(uiFmtID);
	ExpandFormatString(&strFormat, dwError);

	// get param list
	va_list marker;
	va_start(marker, dwError);
	m_strErrorDesc.FormatV(strFormat, marker);
	va_end(marker);
}

CProcessingException::CProcessingException(int iType, CTask* pTask, DWORD dwError, const tchar_t* pszDesc)
{
	// std values
	m_iType=iType;
	m_pTask=pTask;
	m_dwError=dwError;

	// format some text
	m_strErrorDesc = pszDesc;
}

////////////////////////////////////////////////////////////////////////////
// CTask members
CTask::CTask(chcore::IFeedbackHandler* piFeedbackHandler, const TASK_CREATE_DATA *pCreateData) :
	m_log(),
	m_piFeedbackHandler(piFeedbackHandler),
	m_files(m_clipboard),
	m_stCurrentIndex(0),
	m_stLastProcessedIndex(0),
	m_nStatus(ST_NULL_STATUS),
	m_pThread(NULL),
	m_nPriority(THREAD_PRIORITY_NORMAL),
	m_nProcessed(0),
	m_nAll(0),
	m_pnTasksProcessed(pCreateData->pTasksProcessed),
	m_pnTasksAll(pCreateData->pTasksAll),
	m_bKill(false),
	m_bKilled(true),
	m_pcs(pCreateData->pcs),
	m_lTimeElapsed(0),
	m_lLastTime(-1),
	m_puiOperationsPending(pCreateData->puiOperationsPending),
	m_bQueued(false),
	m_ucCopies(1),
	m_ucCurrentCopy(0),
	m_uiResumeInterval(0),
	m_plFinished(pCreateData->plFinished),
	m_bForce(false),
	m_bContinue(false),
	m_bSaved(false),
	m_lOsError(0)
{
	BOOST_ASSERT(piFeedbackHandler);

	m_bsSizes.m_uiDefaultSize=65536;
	m_bsSizes.m_uiOneDiskSize=4194304;
	m_bsSizes.m_uiTwoDisksSize=262144;
	m_bsSizes.m_uiCDSize=262144;
	m_bsSizes.m_uiLANSize=65536;

	_itot((int)time(NULL), m_strUniqueName.GetBufferSetLength(16), 10);
	m_strUniqueName.ReleaseBuffer();
}

CTask::~CTask()
{
	KillThread();
	if(m_piFeedbackHandler)
		m_piFeedbackHandler->Delete();
}

// m_clipboard
void CTask::AddClipboardData(const CClipboardEntryPtr& spEntry)
{
	m_cs.Lock();
	m_clipboard.Add(spEntry);
	m_cs.Unlock();
}

CClipboardEntryPtr CTask::GetClipboardData(size_t stIndex)
{
	m_cs.Lock();
	CClipboardEntryPtr spEntry = m_clipboard.GetAt(stIndex);
	m_cs.Unlock();

	return spEntry;
}

size_t CTask::GetClipboardDataSize()
{
	m_cs.Lock();
	size_t rv=m_clipboard.GetSize();
	m_cs.Unlock();

	return rv;
}

int CTask::ReplaceClipboardStrings(CString strOld, CString strNew)
{
	// small chars to make comparing case insensitive
	strOld.MakeLower();

	CString strText;
	int iOffset;
	int iCount=0;
	m_cs.Lock();
	for (size_t stIndex = 0; stIndex < m_clipboard.GetSize(); ++stIndex)
	{
		CClipboardEntryPtr spEntry = m_clipboard.GetAt(stIndex);
		strText = spEntry->GetPath();
		strText.MakeLower();
		iOffset=strText.Find(strOld, 0);
		if (iOffset != -1)
		{
			// found
			strText = spEntry->GetPath();
			strText = strText.Left(iOffset)+strNew+strText.Mid(iOffset+strOld.GetLength());
			spEntry->SetPath(strText);
			iCount++;
		}
	}
	m_cs.Unlock();

	return iCount;
}

// m_files
int CTask::FilesAddDir(const CString strDirName, const CFiltersArray* pFilters, size_t stSrcIndex,
						const bool bRecurse, const bool bIncludeDirs)
{
	// this uses much of memory, but resolves problem critical section hungs and m_bKill
	CFileInfoArray fa(m_clipboard);

	fa.AddDir(strDirName, pFilters, stSrcIndex, bRecurse, bIncludeDirs, &m_bKill);

	m_cs.Lock();

	m_files.AppendArray(fa);

	m_cs.Unlock();

	return 0;
}

void CTask::FilesAdd(CFileInfo fi)
{
	m_cs.Lock();
	if (fi.IsDirectory() || m_afFilters.Match(fi))
		m_files.AddFileInfo(fi);
	m_cs.Unlock();
}	

CFileInfo CTask::FilesGetAt(size_t stIndex)
{
	m_cs.Lock();
	CFileInfo info = m_files.GetAt(stIndex);
	m_cs.Unlock();

	return info;
}

CFileInfo& CTask::FilesGetAtCurrentIndex()
{
	m_cs.Lock();
	CFileInfo& info = m_files.GetAt(m_stCurrentIndex);
	m_cs.Unlock();
	return info;
}

void CTask::FilesRemoveAll()
{
	m_cs.Lock();
	m_files.Clear();
	m_cs.Unlock();
}

size_t CTask::FilesGetSize()
{
	m_cs.Lock();
	size_t nSize=m_files.GetSize();
	m_cs.Unlock();

	return nSize;
}

// m_stCurrentIndex
void CTask::IncreaseCurrentIndex()
{
	m_cs.Lock();
	++m_stCurrentIndex;
	m_cs.Unlock();
}

size_t CTask::GetCurrentIndex()
{
	m_cs.Lock();
	size_t stIndex = m_stCurrentIndex;
	m_cs.Unlock();

	return stIndex;
}

void CTask::SetCurrentIndex(size_t stIndex)
{
	m_cs.Lock();
	m_stCurrentIndex = stIndex;
	m_cs.Unlock();
}

// m_strDestPath - adds '\\'
void CTask::SetDestPath(LPCTSTR lpszPath)
{
	m_dpDestPath.SetPath(lpszPath);
}

// guaranteed '\\'
const CDestPath& CTask::GetDestPath()
{
	return m_dpDestPath;
}

int CTask::GetDestDriveNumber()
{
	return m_dpDestPath.GetDriveNumber();
}

// m_nStatus
void CTask::SetStatus(UINT nStatus, UINT nMask)
{
	m_cs.Lock();
	m_nStatus &= ~nMask;
	m_nStatus |= nStatus;
	m_cs.Unlock();
}

UINT CTask::GetStatus(UINT nMask)
{
	m_cs.Lock();
	UINT nStatus=m_nStatus;
	m_cs.Unlock();

	return (nStatus & nMask);
}

// m_nBufferSize
void CTask::SetBufferSizes(const BUFFERSIZES* bsSizes)
{
	m_cs.Lock();
	m_bsSizes=*bsSizes;
	m_bSaved=false;
	m_cs.Unlock();
}

const BUFFERSIZES* CTask::GetBufferSizes()
{
	m_cs.Lock();
	const BUFFERSIZES* pbsSizes=&m_bsSizes;
	m_cs.Unlock();

	return pbsSizes;
}

int CTask::GetCurrentBufferIndex()
{
	int rv=0;
	m_cs.Lock();
	size_t stSize = m_files.GetSize();
	if (stSize > 0 && m_stCurrentIndex != -1)
		rv=m_bsSizes.m_bOnlyDefault ? 0 : m_files.GetAt((m_stCurrentIndex < stSize) ? m_stCurrentIndex : 0).GetBufferIndex();
	m_cs.Unlock();

	return rv;
}

// m_pThread
// m_nPriority
int CTask::GetPriority()
{
	m_cs.Lock();
	int nPriority=m_nPriority;
	m_cs.Unlock();
	return nPriority;
}

void CTask::SetPriority(int nPriority)
{
	m_cs.Lock();
	m_nPriority=nPriority;
	m_bSaved=false;
	if (m_pThread != NULL)
	{
		TRACE("Changing thread priority");
		m_pThread->SuspendThread();
		m_pThread->SetThreadPriority(nPriority);
		m_pThread->ResumeThread();
	}
	m_cs.Unlock();
}

// m_nProcessed
void CTask::IncreaseProcessedSize(__int64 nSize)
{
	m_cs.Lock();
	m_nProcessed+=nSize;
	m_cs.Unlock();
}

void CTask::SetProcessedSize(__int64 nSize)
{
	m_cs.Lock();
	m_nProcessed=nSize;
	m_cs.Unlock();
}

__int64 CTask::GetProcessedSize()
{
	m_cs.Lock();
	__int64 nSize=m_nProcessed;
	m_cs.Unlock();

	return nSize;
}

// m_nAll
void CTask::SetAllSize(__int64 nSize)
{
	m_cs.Lock();
	m_nAll=nSize;
	m_cs.Unlock();
}

__int64 CTask::GetAllSize()
{
	m_cs.Lock();
	__int64 nAll=m_nAll;
	m_cs.Unlock();

	return nAll;
}

void CTask::CalcAllSize()
{
	m_cs.Lock();
	m_nAll=0;

	size_t nSize = m_files.GetSize();
	for (size_t i = 0; i < nSize; i++)
	{
		m_nAll += m_files.GetAt(i).GetLength64();
	}

	m_nAll*=m_ucCopies;

	m_cs.Unlock();
}

void CTask::CalcProcessedSize()
{
	m_cs.Lock();
	m_nProcessed=0;

	// count all from previous passes
	if(m_ucCopies)
		m_nProcessed+=m_ucCurrentCopy*(m_nAll/m_ucCopies);
	else
		m_nProcessed+=m_ucCurrentCopy*m_nAll;

	for(size_t stIndex = 0; stIndex < m_stCurrentIndex; ++stIndex)
	{
		m_nProcessed += m_files.GetAt(stIndex).GetLength64();
	}
	IncreaseProcessedTasksSize(m_nProcessed);

	m_cs.Unlock();
}

// m_pnTasksProcessed
void CTask::IncreaseProcessedTasksSize(__int64 nSize)
{
	//	m_cs.Lock();
	m_pcs->Lock();
	(*m_pnTasksProcessed)+=nSize;
	m_pcs->Unlock();
	//	m_cs.Unlock();
}

void CTask::DecreaseProcessedTasksSize(__int64 nSize)
{
	//	m_cs.Lock();
	m_pcs->Lock();
	(*m_pnTasksProcessed)-=nSize;
	m_pcs->Unlock();
	//	m_cs.Unlock();
}

// m_pnTasksAll
void CTask::IncreaseAllTasksSize(__int64 nSize)
{
	//	m_cs.Lock();
	m_pcs->Lock();
	(*m_pnTasksAll)+=nSize;
	m_pcs->Unlock();
	//	m_cs.Unlock();
}

void CTask::DecreaseAllTasksSize(__int64 nSize)
{
	//	m_cs.Lock();
	m_pcs->Lock();
	(*m_pnTasksAll)-=nSize;
	m_pcs->Unlock();
	//	m_cs.Unlock();
}

// m_bKill
/*inline*/ void CTask::SetKillFlag(bool bKill)
{
	m_cs.Lock();
	m_bKill=bKill;
	m_cs.Unlock();
}

bool CTask::GetKillFlag()
{
	m_cs.Lock();
	bool bKill=m_bKill;
	m_cs.Unlock();

	return bKill;
}

// m_bKilled
/*inline*/ void CTask::SetKilledFlag(bool bKilled)
{
	m_cs.Lock();
	m_bKilled=bKilled;
	m_cs.Unlock();
}

/*inline*/ bool CTask::GetKilledFlag()
{
	m_cs.Lock();
	bool bKilled=m_bKilled;
	m_cs.Unlock();

	return bKilled;
}

// m_strUniqueName

CString CTask::GetUniqueName()
{
	m_cs.Lock();
	CString name=m_strUniqueName;
	m_cs.Unlock();

	return name;
}

void CTask::Load(const CString& strPath, bool bData)
{
	std::ifstream ifs(strPath);
	boost::archive::binary_iarchive ar(ifs);

	m_cs.Lock();
	try
	{
		if(bData)
		{
			m_clipboard.Load(ar, 0, bData);

			m_files.Load(ar, 0, false);

			ar >> m_dpDestPath;

			ar >> m_strUniqueName;
			ar >> m_afFilters;
			ar >> m_ucCopies;
		}
		else
		{
			size_t stData = 0;
			UINT uiData = 0;

			ar >> stData;
			m_stCurrentIndex = stData;
			ar >> uiData;
			m_nStatus = uiData;
			ar >> m_lOsError;

			ar >> m_strErrorDesc;

			ar >> m_bsSizes;
			ar >> m_nPriority;
			ar >> m_nAll;
			ar >> m_lTimeElapsed;

			ar >> m_nProcessed;
			ar >> m_ucCurrentCopy;

			m_clipboard.Load(ar, 0, bData);
			m_files.Load(ar, 0, true);

			ar >> m_bSaved;
		}
	}
	catch(...)
	{
		m_cs.Unlock();
		throw;
	}
	m_cs.Unlock();
}

void CTask::Store(bool bData)
{
	m_cs.Lock();
	BOOST_ASSERT(!m_strTaskBasePath.empty());
	if(m_strTaskBasePath.empty())
	{
		m_cs.Unlock();
		THROW(_t("Missing task path."), 0, 0, 0);
	}
	if (!bData && m_bSaved)
	{
		m_cs.Unlock();
		TRACE("Saving locked - file not saved\n");
		return;
	}

	if(!bData && !m_bSaved && ( (m_nStatus & ST_STEP_MASK) == ST_FINISHED || (m_nStatus & ST_STEP_MASK) == ST_CANCELLED
		|| (m_nStatus & ST_WORKING_MASK) == ST_PAUSED ))
	{
		TRACE("Last save - saving blocked\n");
		m_bSaved = true;
	}

	try
	{
		CString strPath = m_strTaskBasePath.c_str() + GetUniqueName() + (bData ? _T(".atd") : _T(".atp"));

		std::ofstream ofs(strPath);
		boost::archive::binary_oarchive ar(ofs);

		if(bData)
		{
			m_clipboard.Store(ar, 0, bData);

			if(GetStatus(ST_STEP_MASK) > ST_SEARCHING)
				m_files.Store(ar, 0, false);
			else
			{
				size_t st(0);
				ar << st;
			}

			ar << m_dpDestPath;
			ar << m_strUniqueName;
			ar << m_afFilters;
			ar << m_ucCopies;
		}
		else
		{
			size_t stCurrentIndex = m_stCurrentIndex;
			ar << stCurrentIndex;
			UINT uiStatus = (m_nStatus & ST_WRITE_MASK);
			ar << uiStatus;
			ar << m_lOsError;

			ar << m_strErrorDesc;

			ar << m_bsSizes;
			ar << m_nPriority;
			ar << m_nAll;
			ar << m_lTimeElapsed;

			ar << m_nProcessed;
			ar << m_ucCurrentCopy;

			m_clipboard.Store(ar, 0, bData);
			if(GetStatus(ST_STEP_MASK) > ST_SEARCHING)
				m_files.Store(ar, 0, true);
			else
			{
				size_t st(0);
				ar << st;
			}
			ar << m_bSaved;
		}
	}
	catch(...)
	{
		m_cs.Unlock();
		return;
	}
	m_cs.Unlock();
}

/*inline*/ void CTask::KillThread()
{
	if (!GetKilledFlag())	// protection from recalling Cleanup
	{
		SetKillFlag();
		while (!GetKilledFlag())
			Sleep(10);

		// cleanup
		CleanupAfterKill();
	}
}

void CTask::BeginProcessing()
{
	m_cs.Lock();
	if (m_pThread != NULL)
	{
		m_cs.Unlock();
		return;
	}
	m_cs.Unlock();

	// create new thread
	m_uiResumeInterval=0;	// just in case
	m_bSaved=false;			// save
	SetKillFlag(false);
	SetKilledFlag(false);
	CWinThread* pThread=AfxBeginThread(ThrdProc, this, GetPriority());

	m_cs.Lock();
	m_pThread=pThread;
	m_cs.Unlock();
}

void CTask::ResumeProcessing()
{
	// the same as retry but less demanding
	if ( (GetStatus(ST_WORKING_MASK) & ST_PAUSED) && GetStatus(ST_STEP_MASK) != ST_FINISHED
		&& GetStatus(ST_STEP_MASK) != ST_CANCELLED)
	{
		SetStatus(0, ST_ERROR);
		BeginProcessing();
	}
}

bool CTask::RetryProcessing(bool bOnlyErrors/*=false*/, UINT uiInterval)
{
	// retry used to auto-resume, after loading
	if ( (GetStatus(ST_WORKING_MASK) == ST_ERROR || (!bOnlyErrors && GetStatus(ST_WORKING_MASK) != ST_PAUSED))
		&& GetStatus(ST_STEP_MASK) != ST_FINISHED && GetStatus(ST_STEP_MASK) != ST_CANCELLED)
	{
		if (uiInterval != 0)
		{
			m_uiResumeInterval+=uiInterval;
			if (m_uiResumeInterval < (UINT)GetConfig().get_signed_num(PP_CMAUTORETRYINTERVAL))
				return false;
			else
				m_uiResumeInterval=0;
		}

		SetStatus(0, ST_ERROR);
		BeginProcessing();
		return true;
	}
	return false;
}

void CTask::RestartProcessing()
{
	KillThread();
	SetStatus(0, ST_ERROR);
	SetStatus(ST_NULL_STATUS, ST_STEP_MASK);
	m_lTimeElapsed=0;
	SetCurrentIndex(0);
	SetCurrentCopy(0);
	BeginProcessing();
}

void CTask::PauseProcessing()
{
	if (GetStatus(ST_STEP_MASK) != ST_FINISHED && GetStatus(ST_STEP_MASK) != ST_CANCELLED)
	{
		KillThread();
		SetStatus(ST_PAUSED, ST_WORKING_MASK);
		SetLastProcessedIndex(GetCurrentIndex());
		m_bSaved=false;
	}
}

void CTask::CancelProcessing()
{
	// change to ST_CANCELLED
	if (GetStatus(ST_STEP_MASK) != ST_FINISHED)
	{
		KillThread();
		SetStatus(ST_CANCELLED, ST_STEP_MASK);
		SetStatus(0, ST_ERROR);
		m_bSaved=false;
	}
}

void CTask::GetMiniSnapshot(TASK_MINI_DISPLAY_DATA *pData)
{
	m_cs.Lock();
	if (m_stCurrentIndex >= 0 && m_stCurrentIndex < m_files.GetSize())
		pData->m_fi=m_files.GetAt(m_stCurrentIndex);
	else
	{
		if (m_files.GetSize() > 0)
		{
			pData->m_fi=m_files.GetAt(0);
			pData->m_fi.SetFilePath(pData->m_fi.GetFullFilePath());
			pData->m_fi.SetSrcIndex(-1);
		}
		else
		{
			if (m_clipboard.GetSize() > 0)
			{
				pData->m_fi.SetFilePath(m_clipboard.GetAt(0)->GetPath());
				pData->m_fi.SetSrcIndex(-1);
			}
			else
			{
				pData->m_fi.SetFilePath(GetResManager().LoadString(IDS_NONEINPUTFILE_STRING));
				pData->m_fi.SetSrcIndex(-1);
			}
		}
	}

	pData->m_uiStatus=m_nStatus;

	// percents
	size_t stSize = m_files.GetSize() * m_ucCopies;
	size_t stIndex = m_stCurrentIndex + m_ucCurrentCopy * m_files.GetSize();
	if (m_nAll != 0 && !((m_nStatus & ST_SPECIAL_MASK) & ST_IGNORE_CONTENT))
		pData->m_nPercent=static_cast<int>( (static_cast<double>(m_nProcessed)*100.0)/static_cast<double>(m_nAll) );
	else
		if (stSize != 0)
			pData->m_nPercent=static_cast<int>( static_cast<double>(stIndex)*100.0/static_cast<double>(stSize) );
		else
			pData->m_nPercent=0;

	m_cs.Unlock();
}

void CTask::GetSnapshot(TASK_DISPLAY_DATA *pData)
{
	m_cs.Lock();
	if (m_stCurrentIndex >= 0 && m_stCurrentIndex < m_files.GetSize())
		pData->m_fi=m_files.GetAt(m_stCurrentIndex);
	else
	{
		if (m_files.GetSize() > 0)
		{
			pData->m_fi=m_files.GetAt(0);
			pData->m_fi.SetFilePath(pData->m_fi.GetFullFilePath());
			pData->m_fi.SetSrcIndex(-1);
		}
		else
		{
			if (m_clipboard.GetSize() > 0)
			{
				pData->m_fi.SetFilePath(m_clipboard.GetAt(0)->GetPath());
				pData->m_fi.SetSrcIndex(-1);
			}
			else
			{
				pData->m_fi.SetFilePath(GetResManager().LoadString(IDS_NONEINPUTFILE_STRING));
				pData->m_fi.SetSrcIndex(-1);
			}
		}
	}

	pData->m_pbsSizes=&m_bsSizes;
	pData->m_nPriority=m_nPriority;
	pData->m_pdpDestPath=&m_dpDestPath;
	pData->m_pafFilters=&m_afFilters;
	pData->m_dwOsErrorCode=m_lOsError;
	pData->m_strErrorDesc=m_strErrorDesc;
	pData->m_uiStatus=m_nStatus;
	pData->m_stIndex=m_stCurrentIndex+m_ucCurrentCopy*m_files.GetSize();
	pData->m_ullProcessedSize=m_nProcessed;
	pData->m_stSize=m_files.GetSize()*m_ucCopies;
	pData->m_ullSizeAll=m_nAll;
	pData->m_ucCurrentCopy=static_cast<unsigned char>(m_ucCurrentCopy+1);	// visual aspect
	pData->m_ucCopies=m_ucCopies;
	pData->m_pstrUniqueName=&m_strUniqueName;

	if (m_files.GetSize() > 0 && m_stCurrentIndex != -1)
		pData->m_iCurrentBufferIndex=m_bsSizes.m_bOnlyDefault ? 0 : m_files.GetAt((m_stCurrentIndex < m_files.GetSize()) ? m_stCurrentIndex : 0).GetBufferIndex();
	else
		pData->m_iCurrentBufferIndex=0;

	// percents
	if (m_nAll != 0 && !((m_nStatus & ST_SPECIAL_MASK) & ST_IGNORE_CONTENT))
		pData->m_nPercent=static_cast<int>( (static_cast<double>(m_nProcessed)*100.0)/static_cast<double>(m_nAll) );
	else
		if (pData->m_stSize != 0)
			pData->m_nPercent=static_cast<int>( static_cast<double>(pData->m_stIndex)*100.0/static_cast<double>(pData->m_stSize) );
		else
			pData->m_nPercent=0;

	// status string
	// first
	if ( (m_nStatus & ST_WORKING_MASK) == ST_ERROR )
	{
		GetResManager().LoadStringCopy(IDS_STATUS0_STRING+4, pData->m_szStatusText, _MAX_PATH);
		_tcscat(pData->m_szStatusText, _T("/"));
	}
	else if ( (m_nStatus & ST_WORKING_MASK) == ST_PAUSED )
	{
		GetResManager().LoadStringCopy(IDS_STATUS0_STRING+5, pData->m_szStatusText, _MAX_PATH);
		_tcscat(pData->m_szStatusText, _T("/"));
	}
	else if ( (m_nStatus & ST_STEP_MASK) == ST_FINISHED )
	{
		GetResManager().LoadStringCopy(IDS_STATUS0_STRING+3, pData->m_szStatusText, _MAX_PATH);
		_tcscat(pData->m_szStatusText, _T("/"));
	}
	else if ( (m_nStatus & ST_WAITING_MASK) == ST_WAITING )
	{
		GetResManager().LoadStringCopy(IDS_STATUS0_STRING+9, pData->m_szStatusText, _MAX_PATH);
		_tcscat(pData->m_szStatusText, _T("/"));
	}
	else if ( (m_nStatus & ST_STEP_MASK) == ST_CANCELLED )
	{
		GetResManager().LoadStringCopy(IDS_STATUS0_STRING+8, pData->m_szStatusText, _MAX_PATH);
		_tcscat(pData->m_szStatusText, _T("/"));
	}
	else
		_tcscpy(pData->m_szStatusText, _T(""));

	// second part
	if ( (m_nStatus & ST_STEP_MASK) == ST_DELETING )
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+6));
	else if ( (m_nStatus & ST_STEP_MASK) == ST_SEARCHING )
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+0));
	else if ((m_nStatus & ST_OPERATION_MASK) == ST_COPY )
	{
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+1));
		if(!m_afFilters.IsEmpty())
			_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_FILTERING_STRING));
	}
	else if ( (m_nStatus & ST_OPERATION_MASK) == ST_MOVE )
	{
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+2));
		if(!m_afFilters.IsEmpty())
			_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_FILTERING_STRING));
	}
	else
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+7));

	// third part
	if ( (m_nStatus & ST_SPECIAL_MASK) & ST_IGNORE_DIRS )
	{
		_tcscat(pData->m_szStatusText, _T("/"));
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+10));
	}
	if ( (m_nStatus & ST_SPECIAL_MASK) & ST_IGNORE_CONTENT )
	{
		_tcscat(pData->m_szStatusText, _T("/"));
		_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_STATUS0_STRING+11));
	}

	// count of copies
	if (m_ucCopies > 1)
	{
		_tcscat(pData->m_szStatusText, _T("/"));
		TCHAR xx[4];
		_tcscat(pData->m_szStatusText, _itot(m_ucCopies, xx, 10));
		if (m_ucCopies < 5)
			_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_COPYWORDLESSFIVE_STRING));
		else
			_tcscat(pData->m_szStatusText, GetResManager().LoadString(IDS_COPYWORDMOREFOUR_STRING));
	}

	// time
	UpdateTime();
	pData->m_lTimeElapsed=m_lTimeElapsed;

	m_cs.Unlock();
}

/*inline*/ void CTask::CleanupAfterKill()
{
	m_cs.Lock();
	m_pThread=NULL;
	UpdateTime();
	m_lLastTime=-1;
	m_cs.Unlock();
}

void CTask::DeleteProgress(LPCTSTR lpszDirectory)
{
	m_cs.Lock();
	DeleteFile(lpszDirectory+m_strUniqueName+_T(".atd"));
	DeleteFile(lpszDirectory+m_strUniqueName+_T(".atp"));
	DeleteFile(lpszDirectory+m_strUniqueName+_T(".log"));
	m_cs.Unlock();
}

void CTask::SetOsErrorCode(DWORD dwError, LPCTSTR lpszErrDesc)
{
	m_cs.Lock();
	m_lOsError=dwError;
	m_strErrorDesc=lpszErrDesc;
	m_cs.Unlock();
}

void CTask::UpdateTime()
{
	m_cs.Lock();
	if (m_lLastTime != -1)
	{
		long lVal=(long)time(NULL);
		m_lTimeElapsed+=lVal-m_lLastTime;
		m_lLastTime=lVal;
	}
	m_cs.Unlock();
}

void CTask::DecreaseOperationsPending(UINT uiBy)
{
	m_pcs->Lock();
	if (m_bQueued)
	{
		TRACE("Decreasing operations pending by %lu\n", uiBy);
		(*m_puiOperationsPending)-=uiBy;
		m_bQueued=false;
	}
	m_pcs->Unlock();
}

void CTask::IncreaseOperationsPending(UINT uiBy)
{
	TRACE("Trying to increase operations pending...\n");
	if (!m_bQueued)
	{
		TRACE("Increasing operations pending by %lu\n", uiBy);
		m_pcs->Lock();
		(*m_puiOperationsPending)+=uiBy;
		m_pcs->Unlock();
		m_bQueued=true;
	}
}

const CFiltersArray* CTask::GetFilters()
{
	return &m_afFilters;
}

void CTask::SetFilters(const CFiltersArray* pFilters)
{
	BOOST_ASSERT(pFilters);
	if(!pFilters)
		return;

	m_cs.Lock();
	m_afFilters = *pFilters;
	m_cs.Unlock();
}

bool CTask::CanBegin()
{
	bool bRet=true;
	m_cs.Lock();
	if (GetContinueFlag() || GetForceFlag())
	{
		TRACE("New operation Begins... continue: %d, force: %d\n", GetContinueFlag(), GetForceFlag());
		IncreaseOperationsPending();
		SetForceFlag(false);
		SetContinueFlag(false);
	}
	else
		bRet=false;
	m_cs.Unlock();

	return bRet;
}

void CTask::SetCopies(unsigned char ucCopies)
{
	m_cs.Lock();
	m_ucCopies=ucCopies;
	m_cs.Unlock();
}

unsigned char CTask::GetCopies()
{
	unsigned char ucCopies;
	m_cs.Lock();
	ucCopies=m_ucCopies;
	m_cs.Unlock();

	return ucCopies;
}

void CTask::SetCurrentCopy(unsigned char ucCopy)
{
	m_cs.Lock();
	m_ucCurrentCopy=ucCopy;
	m_cs.Unlock();
}

unsigned char CTask::GetCurrentCopy()
{
	m_cs.Lock();
	unsigned char ucCopy=m_ucCurrentCopy;
	m_cs.Unlock();

	return ucCopy;
}

void CTask::SetLastProcessedIndex(size_t stIndex)
{
	m_cs.Lock();
	m_stLastProcessedIndex = stIndex;
	m_cs.Unlock();
}

size_t CTask::GetLastProcessedIndex()
{
	size_t stIndex = 0;
	m_cs.Lock();
	stIndex = m_stLastProcessedIndex;
	m_cs.Unlock();

	return stIndex;
}

bool CTask::GetRequiredFreeSpace(ull_t *pullNeeded, ull_t *pullAvailable)
{
	*pullNeeded=GetAllSize()-GetProcessedSize(); // it'd be nice to round up to take cluster size into consideration,
	// but GetDiskFreeSpace returns flase values

	// get free space
	if (!GetDynamicFreeSpace(GetDestPath().GetPath(), pullAvailable, NULL))
		return true;

	return (*pullNeeded <= *pullAvailable);
}

void CTask::SetTaskPath(const tchar_t* pszDir)
{
	m_cs.Lock();
	m_strTaskBasePath = pszDir;
	m_cs.Unlock();
}

const tchar_t* CTask::GetTaskPath() const
{
	const tchar_t* pszText = NULL;
	m_cs.Lock();
	pszText = m_strTaskBasePath.c_str();
	m_cs.Unlock();

	return pszText;
}

void CTask::SetForceFlag(bool bFlag)
{
	m_cs.Lock();
	m_bForce=bFlag;
	m_cs.Unlock();
}

bool CTask::GetForceFlag()
{
	return m_bForce;
}

void CTask::SetContinueFlag(bool bFlag)
{
	m_cs.Lock();
	m_bContinue=bFlag;
	m_cs.Unlock();
}

bool CTask::GetContinueFlag()
{
	return m_bContinue;
}

////////////////////////////////////////////////////////////////////////////////
// CTaskArray members
CTaskArray::CTaskArray() :
CArray<CTask*, CTask*>(),
m_uhRange(0),
m_uhPosition(0),
m_uiOperationsPending(0),
m_lFinished(0),
m_piFeedbackFactory(NULL)
{
}

CTaskArray::~CTaskArray()
{
	// NOTE: do not delete the feedback factory, since we are not responsible for releasing it
}

void CTaskArray::Create(chcore::IFeedbackHandlerFactory* piFeedbackHandlerFactory)
{
	BOOST_ASSERT(piFeedbackHandlerFactory);

	m_tcd.pcs=&m_cs;
	m_tcd.pTasksAll=&m_uhRange;
	m_tcd.pTasksProcessed=&m_uhPosition;
	m_tcd.puiOperationsPending=&m_uiOperationsPending;
	m_tcd.plFinished=&m_lFinished;
	m_piFeedbackFactory = piFeedbackHandlerFactory;
}

CTask* CTaskArray::CreateTask()
{
	BOOST_ASSERT(m_piFeedbackFactory);
	if(!m_piFeedbackFactory)
		return NULL;

	chcore::IFeedbackHandler* piHandler = m_piFeedbackFactory->Create();
	if(!piHandler)
		return NULL;

	CTask* pTask = NULL;
	try
	{
		pTask = new CTask(piHandler, &m_tcd);
	}
	catch(...)
	{
		//		piHandler->Delete();
		throw;
	}

	return pTask;
}

size_t CTaskArray::GetSize()
{
	m_cs.Lock();
	size_t stSize = m_nSize;
	m_cs.Unlock();

	return stSize;
}

size_t CTaskArray::GetUpperBound( )
{
	m_cs.Lock();
	size_t stUpper = m_nSize;
	m_cs.Unlock();

	return stUpper - 1;
}

void CTaskArray::SetSize(size_t nNewSize, int nGrowBy)
{
	m_cs.Lock();
	(static_cast<CArray<CTask*, CTask*>*>(this))->SetSize(nNewSize, nGrowBy);
	m_cs.Unlock();
}

CTask* CTaskArray::GetAt(size_t nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < (size_t)m_nSize);
	m_cs.Lock();
	CTask* pTask = m_pData[nIndex];
	m_cs.Unlock();

	return pTask;
}

size_t CTaskArray::Add(CTask* newElement)
{
	if(!newElement)
		THROW(_t("Invalid argument"), 0, 0, 0);
	m_cs.Lock();
	// here we know load succeeded
	newElement->SetTaskPath(m_strTasksDir.c_str());

	m_uhRange+=newElement->GetAllSize();
	m_uhPosition+=newElement->GetProcessedSize();
	size_t pos = (static_cast<CArray<CTask*, CTask*>*>(this))->Add(newElement);
	m_cs.Unlock();

	return pos;
}

void CTaskArray::RemoveAt(size_t stIndex, size_t stCount)
{
	m_cs.Lock();
	for(size_t i = stIndex; i < stIndex + stCount; i++)
	{
		CTask* pTask = GetAt(i);

		// kill task if needed
		pTask->KillThread();

		m_uhRange -= pTask->GetAllSize();
		m_uhPosition -= pTask->GetProcessedSize();

		delete pTask;
	}

	// remove elements from array
	(static_cast<CArray<CTask*, CTask*>*>(this))->RemoveAt(stIndex, stCount);
	m_cs.Unlock();
}

void CTaskArray::RemoveAll()
{
	m_cs.Lock();
	CTask* pTask;

	for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
   {
      GetAt(stIndex)->SetKillFlag();		// send an info about finishing
   }

	// wait for finishing and get rid of it
	for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
	{
		pTask=GetAt(stIndex);

		// wait
		while(!pTask->GetKilledFlag())
			Sleep(10);

		pTask->CleanupAfterKill();

		m_uhRange-=pTask->GetAllSize();
		m_uhPosition-=pTask->GetProcessedSize();

		// delete data
		delete pTask;
	}

	(static_cast<CArray<CTask*, CTask*>*>(this))->RemoveAll();
	m_cs.Unlock();
}

void CTaskArray::RemoveAllFinished()
{
	m_cs.Lock();
	size_t i = GetSize();

	while(i--)
	{
		CTask* pTask = GetAt(i);

		// delete only when the thread is finished
		if ( (pTask->GetStatus(ST_STEP_MASK) == ST_FINISHED || pTask->GetStatus(ST_STEP_MASK) == ST_CANCELLED)
			&& pTask->GetKilledFlag())
		{
			m_uhRange-=pTask->GetAllSize();
			m_uhPosition-=pTask->GetProcessedSize();

			// delete associated files
			pTask->DeleteProgress(m_strTasksDir.c_str());

			delete pTask;

			static_cast<CArray<CTask*, CTask*>*>(this)->RemoveAt(i);
		}
	}

	m_cs.Unlock();
}

void CTaskArray::RemoveFinished(CTask** pSelTask)
{
	m_cs.Lock();
	for(size_t stIndex=0; stIndex < GetSize(); ++stIndex)
	{
		CTask* pTask = GetAt(stIndex);

		if (pTask == *pSelTask && (pTask->GetStatus(ST_STEP_MASK) == ST_FINISHED || pTask->GetStatus(ST_STEP_MASK) == ST_CANCELLED))
		{
			// kill task if needed
			pTask->KillThread();

			m_uhRange-=pTask->GetAllSize();
			m_uhPosition-=pTask->GetProcessedSize();

			// delete associated files
			pTask->DeleteProgress(m_strTasksDir.c_str());

			// delete data
			delete pTask;

			static_cast<CArray<CTask*, CTask*>*>(this)->RemoveAt(stIndex);

			m_cs.Unlock();
			return;
		}
	}
	m_cs.Unlock();
}

void CTaskArray::SaveData()
{
	m_cs.Lock();
	for (int i=0;i<m_nSize;i++)
		m_pData[i]->Store(true);
	m_cs.Unlock();
}

void CTaskArray::SaveProgress()
{
	m_cs.Lock();
	for (int i=0;i<m_nSize;i++)
		m_pData[i]->Store(false);
	m_cs.Unlock();
}

void CTaskArray::LoadDataProgress()
{
	m_cs.Lock();
	CFileFind finder;
	CTask* pTask;
	CString strPath;

	BOOL bWorking=finder.FindFile(CString(m_strTasksDir.c_str())+_T("*.atd"));
	while(bWorking)
	{
		bWorking = finder.FindNextFile();

		// load data
		pTask = CreateTask();

		try
		{
			strPath = finder.GetFilePath();

			pTask->Load(strPath, true);

			strPath = strPath.Left(strPath.GetLength() - 4);
			strPath += _T(".atp");

			pTask->Load(strPath, false);

			// add read task to array
			Add(pTask);
		}
		catch(std::exception& e)
		{
			CString strFmt;
			strFmt.Format(_T("Cannot load task data: %s (reason: %S)"), strPath, e.what());
			LOG_ERROR(strFmt);
			delete pTask;
		}
	}
	finder.Close();

	m_cs.Unlock();
}

void CTaskArray::TasksBeginProcessing()
{
	for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
   {
      GetAt(stIndex)->BeginProcessing();
   }
}

void CTaskArray::TasksPauseProcessing()
{
   for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
		GetAt(stIndex)->PauseProcessing();
}

void CTaskArray::TasksResumeProcessing()
{
   for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
		GetAt(stIndex)->ResumeProcessing();
}

void CTaskArray::TasksRestartProcessing()
{
   for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
		GetAt(stIndex)->RestartProcessing();
}

bool CTaskArray::TasksRetryProcessing(bool bOnlyErrors/*=false*/, UINT uiInterval)
{
	bool bChanged=false;
   for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
	{
		if (GetAt(stIndex)->RetryProcessing(bOnlyErrors, uiInterval))
			bChanged=true;
	}

	return bChanged;
}

void CTaskArray::TasksCancelProcessing()
{
   for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
		GetAt(stIndex)->CancelProcessing();
}

ull_t CTaskArray::GetPosition()
{
	m_cs.Lock();
	ull_t rv=m_uhPosition;
	m_cs.Unlock();

	return rv;
}

ull_t CTaskArray::GetRange()
{
	m_cs.Lock();
	ull_t rv=m_uhRange;
	m_cs.Unlock();

	return rv;
}

int CTaskArray::GetPercent()
{
	int pos;
	m_cs.Lock();
	if (m_uhRange != 0)
		pos=static_cast<int>((static_cast<double>(m_uhPosition)*100.0)/static_cast<double>(m_uhRange));
	else
		if (GetSize() != 0)		// if anything is in an array, but size of it is 0
			pos=100;
		else
			pos=0;
	m_cs.Unlock();

	return pos;
}

UINT CTaskArray::GetOperationsPending()
{
	m_cs.Lock();
	UINT uiOP=m_uiOperationsPending;
	m_cs.Unlock();
	return uiOP;
}

bool CTaskArray::IsFinished()
{
	bool bFlag=true;
	UINT uiStatus;

	m_cs.Lock();
	if (m_uiOperationsPending != 0)
		bFlag=false;
	else
	{
      for(size_t stIndex = 0; stIndex < GetSize(); ++stIndex)
		{
			uiStatus=GetAt(stIndex)->GetStatus();
			bFlag=((uiStatus & ST_STEP_MASK) == ST_FINISHED || (uiStatus & ST_STEP_MASK) == ST_CANCELLED
				|| (uiStatus & ST_WORKING_MASK) == ST_PAUSED
				|| ((uiStatus & ST_WORKING_MASK) == ST_ERROR && !GetConfig().get_bool(PP_CMAUTORETRYONERROR)));
		}
	}

	m_cs.Unlock();
	return bFlag;
}

void CTaskArray::SetTasksDir(const tchar_t* pszPath)
{
	m_strTasksDir = pszPath;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTask::SetFileDirectoryTime(LPCTSTR lpszName, CFileInfo* pSrcInfo)
{
	HANDLE hFile = CreateFile(lpszName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | (pSrcInfo->IsDirectory() ? FILE_FLAG_BACKUP_SEMANTICS : 0), NULL);
	if(hFile == INVALID_HANDLE_VALUE)
		return false;

	if(!SetFileTime(hFile, &pSrcInfo->GetCreationTime(), &pSrcInfo->GetLastAccessTime(), &pSrcInfo->GetLastWriteTime()))
	{
		CloseHandle(hFile);
		return false;
	}

	if(!CloseHandle(hFile))
		return false;

	return true;
}

// searching for files
void CTask::RecurseDirectories(CTask* pTask)
{
	TRACE("Searching for files...\n");

	// log
	pTask->m_log.logi(_T("Searching for files..."));

	// update status
	pTask->SetStatus(ST_SEARCHING, ST_STEP_MASK);

	// delete the content of m_files
	pTask->FilesRemoveAll();

	// enter some data to m_files
	size_t stSize = pTask->GetClipboardDataSize();	// size of m_clipboard
	const CFiltersArray* pFilters=pTask->GetFilters();
	int iDestDrvNumber=pTask->GetDestDriveNumber();
	bool bIgnoreDirs=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_DIRS) != 0;
	bool bForceDirectories=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_FORCE_DIRS) != 0;
	bool bMove=pTask->GetStatus(ST_OPERATION_MASK) == ST_MOVE;
	CFileInfo fi;
	fi.SetClipboard(pTask->GetClipboard());

	// add everything
	ictranslate::CFormat fmt;
	bool bRetry = true;
	bool bSkipInputPath = false;

	for(size_t i = 0; i < stSize ; i++)
	{
		bSkipInputPath = false;
		bRetry = false;

		// try to get some info about the input path; let user know if the path does not exist.
		do
		{
			// read attributes of src file/folder
			bool bExists = fi.Create(pTask->GetClipboardData(i)->GetPath(), i);
			if(!bExists)
			{
				chcore::IFeedbackHandler* piFeedbackHandler = pTask->GetFeedbackHandler();
				BOOST_ASSERT(piFeedbackHandler);

				CString strSrcFile = pTask->GetClipboardData(i)->GetPath();
				FEEDBACK_FILEERROR ferr = { (PCTSTR)strSrcFile, NULL, eFastMoveError, ERROR_FILE_NOT_FOUND };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
				switch(frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					throw new CProcessingException(E_CANCEL, pTask);
					break;
				case CFeedbackHandler::eResult_Retry:
					bRetry = true;
					continue;
					break;
				case CFeedbackHandler::eResult_Pause:
					throw new CProcessingException(E_PAUSE, pTask);
					break;
				case CFeedbackHandler::eResult_Skip:
					bSkipInputPath = true;
					bRetry = false;
					break;		// just do nothing
				default:
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, pTask, 0, _t("Unknown feedback result type"));
				}
			}
		}
		while(bRetry);

		// if we have chosen to skip the input path then there's nothing to do
		if(bSkipInputPath)
			continue;

		// log
		fmt.SetFormat(_T("Adding file/folder (clipboard) : %path ..."));
		fmt.SetParam(_t("%path"), pTask->GetClipboardData(i)->GetPath());
		pTask->m_log.logi(fmt);

		// found file/folder - check if the dest name has been generated
		if (pTask->GetClipboardData(i)->GetDestinationPathsCount() == 0)
		{
			// generate something - if dest folder == src folder - search for copy
			if (pTask->GetDestPath().GetPath() == fi.GetFileRoot())
			{
				CString strSubst;
				FindFreeSubstituteName(fi.GetFullFilePath(), pTask->GetDestPath().GetPath(), &strSubst);
				pTask->GetClipboardData(i)->AddDestinationPath(strSubst);
			}
			else
				pTask->GetClipboardData(i)->AddDestinationPath(fi.GetFileName());
		}

		// add if needed
		if (fi.IsDirectory())
		{
			// add if folder's aren't ignored
			if (!bIgnoreDirs && !bForceDirectories)
			{
				pTask->FilesAdd(fi);

				// log
				fmt.SetFormat(_T("Added folder %path"));
				fmt.SetParam(_t("%path"), fi.GetFullFilePath());
				pTask->m_log.logi(fmt);
			}

			// don't add folder contents when moving inside one disk boundary
			if (bIgnoreDirs || !bMove || pTask->GetCopies() > 1 || iDestDrvNumber == -1
				|| iDestDrvNumber != fi.GetDriveNumber() || CFileInfo::Exist(fi.GetDestinationPath(pTask->GetDestPath().GetPath(), 0, ((int)bForceDirectories) << 1)) )
			{
				// log
				fmt.SetFormat(_T("Recursing folder %path"));
				fmt.SetParam(_t("%path"), fi.GetFullFilePath());
				pTask->m_log.logi(fmt);

				// no movefile possibility - use CustomCopyFile
				pTask->GetClipboardData(i)->SetMove(false);

				pTask->FilesAddDir(fi.GetFullFilePath(), pFilters, i, true, !bIgnoreDirs || bForceDirectories);
			}

			// check for kill need
			if (pTask->GetKillFlag())
			{
				// log
				pTask->m_log.logi(_T("Kill request while adding data to files array (RecurseDirectories)"));
				throw new CProcessingException(E_KILL_REQUEST, pTask);
			}
		}
		else
		{
			if (bMove && pTask->GetCopies() == 1 && iDestDrvNumber != -1 && iDestDrvNumber == fi.GetDriveNumber() &&
				!CFileInfo::Exist(fi.GetDestinationPath(pTask->GetDestPath().GetPath(), 0, ((int)bForceDirectories) << 1)) )
			{
				// if moving within one partition boundary set the file size to 0 so the overall size will
				// be ok
				fi.SetLength64(0);
			}
			else
				pTask->GetClipboardData(i)->SetMove(false);	// no MoveFile

			pTask->FilesAdd(fi);		// file - add

			// log
			fmt.SetFormat(_T("Added file %path"));
			fmt.SetParam(_t("%path"), fi.GetFullFilePath());
			pTask->m_log.logi(fmt);
		}
	}

	// calc size of all files
	pTask->CalcAllSize();

	// update *m_pnTasksAll;
	pTask->IncreaseAllTasksSize(pTask->GetAllSize());

	// change state to ST_COPYING - finished searching for files
	pTask->SetStatus(ST_COPYING, ST_STEP_MASK);

	// save task status
	pTask->Store(true);
	pTask->Store(false);

	// log
	pTask->m_log.logi(_T("Searching for files finished"));
}

// delete files - after copying
void CTask::DeleteFiles(CTask* pTask)
{
	// log
	pTask->m_log.logi(_T("Deleting files (DeleteFiles)..."));

	chcore::IFeedbackHandler* piFeedbackHandler = pTask->GetFeedbackHandler();
	BOOST_ASSERT(piFeedbackHandler);

	// current processed path
	BOOL bSuccess;
	CFileInfo fi;
	ictranslate::CFormat fmt;

	// index points to 0 or next item to process
	size_t stIndex = pTask->GetCurrentIndex();
	while(stIndex < pTask->FilesGetSize())
	{
		// set index in pTask to currently deleted element
		pTask->SetCurrentIndex(stIndex);

		// check for kill flag
		if (pTask->GetKillFlag())
		{
			// log
			pTask->m_log.logi(_T("Kill request while deleting files (Delete Files)"));
			throw new CProcessingException(E_KILL_REQUEST, pTask);
		}

		// current processed element
		fi=pTask->FilesGetAt(pTask->FilesGetSize()-stIndex-1);
		if(!(fi.GetFlags() & FIF_PROCESSED))
		{
			++stIndex;
			continue;
		}

		// delete data
		if (fi.IsDirectory())
		{
			if (!GetConfig().get_bool(PP_CMPROTECTROFILES))
				SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			bSuccess=RemoveDirectory(fi.GetFullFilePath());
		}
		else
		{
			// set files attributes to normal - it'd slow processing a bit, but it's better.
			if (!GetConfig().get_bool(PP_CMPROTECTROFILES))
				SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
			bSuccess=DeleteFile(fi.GetFullFilePath());
		}

		// operation failed
		DWORD dwLastError=GetLastError();
		if (!bSuccess && dwLastError != ERROR_PATH_NOT_FOUND && dwLastError != ERROR_FILE_NOT_FOUND)
		{
			// log
			fmt.SetFormat(_T("Error #%errno while deleting file/folder %path"));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), fi.GetFullFilePath());
			pTask->m_log.loge(fmt);

			CString strFile = fi.GetFullFilePath();
			FEEDBACK_FILEERROR ferr = { (PCTSTR)strFile, NULL, eDeleteError, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Cancel:
				pTask->m_log.logi(_T("Cancel request while deleting file."));
				throw new CProcessingException(E_CANCEL, pTask);
				break;
			case CFeedbackHandler::eResult_Retry:
				continue;	// no stIndex bump, since we are trying again
				break;
			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE, pTask);
				break;
			case CFeedbackHandler::eResult_Skip:
				break;		// just do nothing
			default:
				BOOST_ASSERT(FALSE);		// unknown result
				throw new CProcessingException(E_ERROR, pTask, 0, _t("Unknown feedback result type"));
			}
		}

		++stIndex;
	}//while

	// change status to finished
	pTask->SetStatus(ST_FINISHED, ST_STEP_MASK);

	// add 1 to current index - looks better
	pTask->IncreaseCurrentIndex();

	// log
	pTask->m_log.logi(_T("Deleting files finished"));
}

void CTask::CustomCopyFile(CUSTOM_COPY_PARAMS* pData)
{
	HANDLE hSrc = INVALID_HANDLE_VALUE, hDst = INVALID_HANDLE_VALUE;
	ictranslate::CFormat fmt;
	bool bRetry = false;

	try
	{
		// do we copy rest or recopy ?
		bool bCopyRest = false;

		// Data regarding dest file
		CFileInfo fiDest;
		bool bExist=fiDest.Create(pData->strDstFile, std::numeric_limits<size_t>::max());

		chcore::IFeedbackHandler* piFeedbackHandler = pData->pTask->GetFeedbackHandler();
		BOOST_ASSERT(piFeedbackHandler);

		pData->pTask->SetLastProcessedIndex(std::numeric_limits<size_t>::max());

		// if dest file size >0 - we can do something more than usual
		if(bExist)
		{
			// src and dst files are the same
			FEEDBACK_ALREADYEXISTS feedStruct = { pData->pfiSrcFile, &fiDest };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileAlreadyExists, &feedStruct);
			// check for dialog result
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Overwrite:
				{
					bCopyRest = false;
					break;
				}
			case CFeedbackHandler::eResult_CopyRest:
				{
					bCopyRest = true;
					break;
				}
			case CFeedbackHandler::eResult_Skip:
				{
					pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
					pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
					pData->bProcessed = false;
					return;
				}
			case CFeedbackHandler::eResult_Cancel:
				{
					// log
					if (GetConfig().get_bool(PP_CMCREATELOG))
					{
						fmt.SetFormat(_T("Cancel request while checking result of dialog before opening source file %path (CustomCopyFile)"));
						fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
						pData->pTask->m_log.logi(fmt);
					}
					throw new CProcessingException(E_CANCEL, pData->pTask);
					break;
				}
			case CFeedbackHandler::eResult_Pause:
				{
					throw new CProcessingException(E_PAUSE, pData->pTask);
					break;
				}
			default:
				{
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
					break;
				}
			}
		}// bExist

		// change attributes of a dest file
		if(!GetConfig().get_bool(PP_CMPROTECTROFILES))
			SetFileAttributes(pData->strDstFile, FILE_ATTRIBUTE_NORMAL);

		// first or second pass ? only for FFNB
		bool bFirstPass=true;

		// check size of src file to know whether use flag FILE_FLAG_NOBUFFERING
l_start:
		bool bNoBuffer=(bFirstPass && GetConfig().get_bool(PP_BFUSENOBUFFERING) && pData->pfiSrcFile->GetLength64() >= (unsigned long long)GetConfig().get_signed_num(PP_BFBOUNDARYLIMIT));

		// refresh data about file
		if (!bFirstPass)
			bExist=fiDest.Create(pData->strDstFile, std::numeric_limits<size_t>::max());

		// open src
		do
		{
			bRetry = false;

			hSrc = CreateFile(pData->pfiSrcFile->GetFullFilePath(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffer ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
			if(hSrc == INVALID_HANDLE_VALUE)
			{
				DWORD dwLastError=GetLastError();
				CString strFile = pData->pfiSrcFile->GetFullFilePath();
				FEEDBACK_FILEERROR feedStruct = { (PCTSTR)strFile, NULL, eCreateError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);

				switch (frResult)
				{
				case CFeedbackHandler::eResult_Skip:
					pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
					pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
					pData->bProcessed = false;
					return;
					break;
				case CFeedbackHandler::eResult_Cancel:
					// log
					fmt.SetFormat(_T("Cancel request [error %errno] while opening source file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
					pData->pTask->m_log.loge(fmt);
					throw new CProcessingException(E_CANCEL, pData->pTask);
					break;
				case CFeedbackHandler::eResult_Pause:
					throw new CProcessingException(E_PAUSE, pData->pTask);
					break;
				case CFeedbackHandler::eResult_Retry:
					// log
					fmt.SetFormat(_T("Retrying [error %errno] to open source file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
					pData->pTask->m_log.loge(fmt);
					bRetry = true;
					break;
				default:
					{
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
						break;
					}
				}
			}
		}
		while(bRetry);

		// open dest
		do 
		{
			bRetry = false;

			hDst = CreateFile(pData->strDstFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffer ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
			if (hDst == INVALID_HANDLE_VALUE)
			{
				DWORD dwLastError=GetLastError();
				CString strFile = pData->strDstFile;

				FEEDBACK_FILEERROR feedStruct = { (PCTSTR)strFile, NULL, eCreateError, dwLastError };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
				switch (frResult)
				{
				case CFeedbackHandler::eResult_Retry:
					// change attributes
					if (!GetConfig().get_bool(PP_CMPROTECTROFILES))
						SetFileAttributes(pData->strDstFile, FILE_ATTRIBUTE_NORMAL);

					// log
					fmt.SetFormat(_T("Retrying [error %errno] to open destination file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pData->strDstFile);
					pData->pTask->m_log.loge(fmt);
					bRetry = true;
					break;
				case CFeedbackHandler::eResult_Cancel:
					// log
					fmt.SetFormat(_T("Cancel request [error %errno] while opening destination file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pData->strDstFile);
					pData->pTask->m_log.loge(fmt);
					throw new CProcessingException(E_CANCEL, pData->pTask);
					break;
				case CFeedbackHandler::eResult_Skip:
					pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
					pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
					pData->bProcessed = false;
					return;
					break;
				case CFeedbackHandler::eResult_Pause:
					throw new CProcessingException(E_PAUSE, pData->pTask);
					break;
				default:
					{
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
						break;
					}
				}
			}
		}
		while(bRetry);

		// seeking
		DWORD dwLastError = 0;
		if(!pData->bOnlyCreate)
		{
			if(bCopyRest)	// if copy rest
			{
				if(!bFirstPass || (bExist && fiDest.GetLength64() > 0))
				{
					// try to move file pointers to the end
					ULONGLONG ullMove = (bNoBuffer ? ROUNDDOWN(fiDest.GetLength64(), MAXSECTORSIZE) : fiDest.GetLength64());
					bool bRetry = true;
					while(bRetry)
					{
						if(SetFilePointer64(hSrc, ullMove, FILE_BEGIN) == -1 || SetFilePointer64(hDst, ullMove, FILE_BEGIN) == -1)
						{
							dwLastError = GetLastError();
							// log
							fmt.SetFormat(_T("Error %errno while moving file pointers of %srcpath and %dstpath to %pos"));
							fmt.SetParam(_t("%errno"), dwLastError);
							fmt.SetParam(_t("%srcpath"), pData->pfiSrcFile->GetFullFilePath());
							fmt.SetParam(_t("%dstpath"), pData->strDstFile);
							fmt.SetParam(_t("%pos"), ullMove);
							pData->pTask->m_log.loge(fmt);

							CString strSrcFile = pData->pfiSrcFile->GetFullFilePath();
							CString strDstFile = pData->strDstFile;
							FEEDBACK_FILEERROR ferr = { (PCTSTR)strSrcFile, (PCTSTR)strDstFile, eSeekError, dwLastError };
							CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
							switch(frResult)
							{
							case CFeedbackHandler::eResult_Cancel:
								throw new CProcessingException(E_CANCEL, pData->pTask);
								break;
							case CFeedbackHandler::eResult_Retry:
								continue;
								break;
							case CFeedbackHandler::eResult_Pause:
								throw new CProcessingException(E_PAUSE, pData->pTask);
								break;
							case CFeedbackHandler::eResult_Skip:
								bRetry = false;
								pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
								pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
								pData->bProcessed = false;
								return;
							default:
								BOOST_ASSERT(FALSE);		// unknown result
								throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
							}
						}
						else
						{
							bRetry = false;
							// file pointers moved - so we have skipped some work - update positions
							if (bFirstPass)	// przy drugim obiegu jest ju¿ uwzglêdnione
							{
								pData->pTask->IncreaseProcessedSize(ullMove);
								pData->pTask->IncreaseProcessedTasksSize(ullMove);
							}
						}
					}
				}
			}
			else
			{
				bool bRetry = true;
				while(bRetry && !SetEndOfFile(hDst))
				{
					// log
					dwLastError=GetLastError();
					fmt.SetFormat(_T("Error %errno while setting size of file %path to 0"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pData->strDstFile);
					pData->pTask->m_log.loge(fmt);

					FEEDBACK_FILEERROR ferr = { (PCTSTR)pData->strDstFile, NULL, eResizeError, dwLastError };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
					switch(frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						throw new CProcessingException(E_CANCEL, pData->pTask);
						break;
					case CFeedbackHandler::eResult_Retry:
						continue;
						break;
					case CFeedbackHandler::eResult_Pause:
						throw new CProcessingException(E_PAUSE, pData->pTask);
						break;
					case CFeedbackHandler::eResult_Skip:
						bRetry = false;
						break;		// just do nothing
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
					}
				}
			}

			// copying
			unsigned long ulToRead = 0, ulRead = 0, ulWritten = 0;
			int iBufferIndex = 0;
			do
			{
				// kill flag checks
				if(pData->pTask->GetKillFlag())
				{
					// log
					fmt.SetFormat(_T("Kill request while main copying file %srcpath -> %dstpath"));
					fmt.SetParam(_t("%srcpath"), pData->pfiSrcFile->GetFullFilePath());
					fmt.SetParam(_t("%dstpath"), pData->strDstFile);
					pData->pTask->m_log.logi(fmt);
					throw new CProcessingException(E_KILL_REQUEST, pData->pTask);
				}

				// recreate buffer if needed
				if (!(*pData->dbBuffer.GetSizes() == *pData->pTask->GetBufferSizes()))
				{
					// log
					const BUFFERSIZES *pbs1=pData->dbBuffer.GetSizes(), *pbs2=pData->pTask->GetBufferSizes();

					fmt.SetFormat(_T("Changing buffer size from [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize] to [Def:%defsize2, One:%onesize2, Two:%twosize2, CD:%cdsize2, LAN:%lansize2] wile copying %srcfile -> %dstfile (CustomCopyFile)"));

					fmt.SetParam(_t("%defsize"), pbs1->m_uiDefaultSize);
					fmt.SetParam(_t("%onesize"), pbs1->m_uiOneDiskSize);
					fmt.SetParam(_t("%twosize"), pbs1->m_uiTwoDisksSize);
					fmt.SetParam(_t("%cdsize"), pbs1->m_uiCDSize);
					fmt.SetParam(_t("%lansize"), pbs1->m_uiLANSize);
					fmt.SetParam(_t("%defsize2"), pbs2->m_uiDefaultSize);
					fmt.SetParam(_t("%onesize2"), pbs2->m_uiOneDiskSize);
					fmt.SetParam(_t("%twosize2"), pbs2->m_uiTwoDisksSize);
					fmt.SetParam(_t("%cdsize2"), pbs2->m_uiCDSize);
					fmt.SetParam(_t("%lansize2"), pbs2->m_uiLANSize);
					fmt.SetParam(_t("%srcfile"), pData->pfiSrcFile->GetFullFilePath());
					fmt.SetParam(_t("%dstfile"), pData->strDstFile);

					pData->pTask->m_log.logi(fmt);
					pData->pTask->SetBufferSizes(pData->dbBuffer.Create(pData->pTask->GetBufferSizes()));
				}

				// establish count of data to read
				iBufferIndex=pData->pTask->GetBufferSizes()->m_bOnlyDefault ? 0 : pData->pfiSrcFile->GetBufferIndex();
				ulToRead=bNoBuffer ? ROUNDUP(pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex], MAXSECTORSIZE) : pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex];

				// read
				bool bRetry = true;
				while(bRetry && !ReadFile(hSrc, pData->dbBuffer, ulToRead, &ulRead, NULL))
				{
					// log
					dwLastError=GetLastError();
					fmt.SetFormat(_T("Error %errno while trying to read %count bytes from source file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%count"), ulToRead);
					fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
					pData->pTask->m_log.loge(fmt);

					CString strFile = pData->pfiSrcFile->GetFullFilePath();
					FEEDBACK_FILEERROR ferr = { (PCTSTR)strFile, NULL, eReadError, dwLastError };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
					switch(frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						throw new CProcessingException(E_CANCEL, pData->pTask);
						break;
					case CFeedbackHandler::eResult_Retry:
						continue;
						break;
					case CFeedbackHandler::eResult_Pause:
						throw new CProcessingException(E_PAUSE, pData->pTask);
						break;
					case CFeedbackHandler::eResult_Skip:
						bRetry = false;
						// TODO: correct the skip length handling
						pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
						pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
						pData->bProcessed = false;
						return;
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
					}
				}

				// change count of stored data
				if (bNoBuffer && (ROUNDUP(ulRead, MAXSECTORSIZE)) != ulRead)
				{
					// we need to copy rest so do the second pass
					// close files
					CloseHandle(hSrc);
					CloseHandle(hDst);

					// second pass
					bFirstPass=false;
					bCopyRest=true;		// nedd to copy rest

					goto l_start;
				}

				// write
				bRetry = true;
				while(bRetry && !WriteFile(hDst, pData->dbBuffer, ulRead, &ulWritten, NULL) || ulWritten != ulRead)
				{
					// log
					dwLastError=GetLastError();
					fmt.SetFormat(_T("Error %errno while trying to write %count bytes to destination file %path (CustomCopyFile)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%count"), ulRead);
					fmt.SetParam(_t("%path"), pData->strDstFile);
					pData->pTask->m_log.loge(fmt);

					CString strFile = pData->strDstFile;
					FEEDBACK_FILEERROR ferr = { (PCTSTR)strFile, NULL, eWriteError, dwLastError };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
					switch(frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						throw new CProcessingException(E_CANCEL, pData->pTask);
						break;
					case CFeedbackHandler::eResult_Retry:
						continue;
						break;
					case CFeedbackHandler::eResult_Pause:
						throw new CProcessingException(E_PAUSE, pData->pTask);
						break;
					case CFeedbackHandler::eResult_Skip:
						bRetry = false;
						// TODO: correct the skip length handling
						pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
						pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
						pData->bProcessed = false;
						return;
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
					}
				}

				// increase count of processed data
				pData->pTask->IncreaseProcessedSize(ulRead);
				pData->pTask->IncreaseProcessedTasksSize(ulRead);
				//				TRACE("Read: %d, Written: %d\n", rd, ulWritten);
			}
			while(ulRead != 0);
		}
		else
		{
			// we don't copy contents, but need to increase processed size
			pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
			pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
		}

		// close files
		CloseHandle(hSrc);
		CloseHandle(hDst);

		pData->bProcessed = true;
	}
	catch(...)
	{
		// close handles
		if(hSrc != INVALID_HANDLE_VALUE)
			CloseHandle(hSrc);
		if(hDst != INVALID_HANDLE_VALUE)
			CloseHandle(hDst);

		throw;
	}
}

// function processes files/folders
void CTask::ProcessFiles(CTask* pTask)
{
	chcore::IFeedbackHandler* piFeedbackHandler = pTask->GetFeedbackHandler();
	BOOST_ASSERT(piFeedbackHandler);

	// log
	pTask->m_log.logi(_T("Processing files/folders (ProcessFiles)"));

	// count how much has been done (updates also a member in CTaskArray)
	pTask->CalcProcessedSize();

	// create a buffer of size pTask->m_nBufferSize
	CUSTOM_COPY_PARAMS ccp;
	ccp.bProcessed = false;
	ccp.pTask=pTask;
	ccp.bOnlyCreate=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_CONTENT) != 0;
	ccp.dbBuffer.Create(pTask->GetBufferSizes());

	// helpers
	//CFileInfo fi;	// for currently processed element
	DWORD dwLastError;

	// begin at index which wasn't processed previously
	size_t stSize = pTask->FilesGetSize();
	int iCopiesCount=pTask->GetCopies();
	bool bIgnoreFolders=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_DIRS) != 0;
	bool bForceDirectories=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_FORCE_DIRS) != 0;
	const CDestPath& dpDestPath=pTask->GetDestPath();

	// log
	const BUFFERSIZES* pbs=ccp.dbBuffer.GetSizes();

	ictranslate::CFormat fmt;
	fmt.SetFormat(_T("Processing files/folders (ProcessFiles):\r\n\tOnlyCreate: %create\r\n\tBufferSize: [Def:%defsize, One:%onesize, Two:%twosize, CD:%cdsize, LAN:%lansize]\r\n\tFiles/folders count: %filecount\r\n\tCopies count: %copycount\r\n\tIgnore Folders: %ignorefolders\r\n\tDest path: %dstpath\r\n\tCurrent pass (0-based): %currpass\r\n\tCurrent index (0-based): %currindex"));
	fmt.SetParam(_t("%create"), ccp.bOnlyCreate);
	fmt.SetParam(_t("%defsize"), pbs->m_uiDefaultSize);
	fmt.SetParam(_t("%onesize"), pbs->m_uiOneDiskSize);
	fmt.SetParam(_t("%twosize"), pbs->m_uiTwoDisksSize);
	fmt.SetParam(_t("%cdsize"), pbs->m_uiCDSize);
	fmt.SetParam(_t("%lansize"), pbs->m_uiLANSize);
	fmt.SetParam(_t("%filecount"), stSize);
	fmt.SetParam(_t("%copycount"), iCopiesCount);
	fmt.SetParam(_t("%ignorefolders"), bIgnoreFolders);
	fmt.SetParam(_t("%dstpath"), dpDestPath.GetPath());
	fmt.SetParam(_t("%currpass"), pTask->GetCurrentCopy());
	fmt.SetParam(_t("%currindex"), pTask->GetCurrentIndex());

	pTask->m_log.logi(fmt);

	for (unsigned char j=pTask->GetCurrentCopy();j<iCopiesCount;j++)
	{
		pTask->SetCurrentCopy(j);
		for (size_t i = pTask->GetCurrentIndex(); i < stSize; i++)
		{
			// should we kill ?
			if (pTask->GetKillFlag())
			{
				// log
				pTask->m_log.logi(_T("Kill request while processing file in ProcessFiles"));
				throw new CProcessingException(E_KILL_REQUEST, pTask);
			}

			// update m_stCurrentIndex, getting current CFileInfo
			pTask->SetCurrentIndex(i);
			CFileInfo& fi=pTask->FilesGetAtCurrentIndex();

			// set dest path with filename
			ccp.strDstFile=fi.GetDestinationPath(dpDestPath.GetPath(), j, ((int)bForceDirectories) << 1 | (int)bIgnoreFolders);

			// are the files/folders lie on the same partition ?
			bool bMove=pTask->GetStatus(ST_OPERATION_MASK) == ST_MOVE;
			if (bMove && dpDestPath.GetDriveNumber() != -1 && dpDestPath.GetDriveNumber() == fi.GetDriveNumber() && iCopiesCount == 1 && fi.GetMove())
			{
				bool bRetry = true;
				if(bRetry && !MoveFile(fi.GetFullFilePath(), ccp.strDstFile))
				{
					dwLastError=GetLastError();
					//log
					fmt.SetFormat(_T("Error %errno while calling MoveFile %srcpath -> %dstpath (ProcessFiles)"));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%srcpath"), fi.GetFullFilePath());
					fmt.SetParam(_t("%dstpath"), ccp.strDstFile);
					pTask->m_log.loge(fmt);

					CString strSrcFile = fi.GetFullFilePath();
					CString strDstFile = ccp.strDstFile;
					FEEDBACK_FILEERROR ferr = { (PCTSTR)strSrcFile, (PCTSTR)strDstFile, eFastMoveError, dwLastError };
					CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
					switch(frResult)
					{
					case CFeedbackHandler::eResult_Cancel:
						throw new CProcessingException(E_CANCEL, pTask);
						break;
					case CFeedbackHandler::eResult_Retry:
						continue;
						break;
					case CFeedbackHandler::eResult_Pause:
						throw new CProcessingException(E_PAUSE, pTask);
						break;
					case CFeedbackHandler::eResult_Skip:
						bRetry = false;
						break;		// just do nothing
					default:
						BOOST_ASSERT(FALSE);		// unknown result
						throw new CProcessingException(E_ERROR, pTask, 0, _t("Unknown feedback result type"));
					}
				}
				else
					fi.SetFlags(FIF_PROCESSED, FIF_PROCESSED);
			}
			else
			{
				// if folder - create it
				if ( fi.IsDirectory() )
				{
					bool bRetry = true;
					if(bRetry && !CreateDirectory(ccp.strDstFile, NULL) && (dwLastError=GetLastError()) != ERROR_ALREADY_EXISTS )
					{
						// log
						fmt.SetFormat(_T("Error %errno while calling CreateDirectory %path (ProcessFiles)"));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), ccp.strDstFile);
						pTask->m_log.loge(fmt);

						CString strFile = ccp.strDstFile;
						FEEDBACK_FILEERROR ferr = { (PCTSTR)strFile, NULL, eCreateError, dwLastError };
						CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &ferr);
						switch(frResult)
						{
						case CFeedbackHandler::eResult_Cancel:
							throw new CProcessingException(E_CANCEL, pTask);
							break;
						case CFeedbackHandler::eResult_Retry:
							continue;
							break;
						case CFeedbackHandler::eResult_Pause:
							throw new CProcessingException(E_PAUSE, pTask);
							break;
						case CFeedbackHandler::eResult_Skip:
							bRetry = false;
							break;		// just do nothing
						default:
							BOOST_ASSERT(FALSE);		// unknown result
							throw new CProcessingException(E_ERROR, pTask, 0, _t("Unknown feedback result type"));
						}
					}

					pTask->IncreaseProcessedSize(fi.GetLength64());
					pTask->IncreaseProcessedTasksSize(fi.GetLength64());
					fi.SetFlags(FIF_PROCESSED, FIF_PROCESSED);
				}
				else
				{
					// start copying/moving file
					ccp.pfiSrcFile=&fi;
					ccp.bProcessed = false;

					// kopiuj dane
					CustomCopyFile(&ccp);
					fi.SetFlags(ccp.bProcessed ? FIF_PROCESSED : 0, FIF_PROCESSED);

					// if moving - delete file (only if config flag is set)
					if (bMove && fi.GetFlags() & FIF_PROCESSED && !GetConfig().get_bool(PP_CMDELETEAFTERFINISHED) && j == iCopiesCount-1)
					{
						if (!GetConfig().get_bool(PP_CMPROTECTROFILES))
							SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
						DeleteFile(fi.GetFullFilePath());	// there will be another try later, so I don't check
						// if succeeded
					}
				}

				// set a time
				if (GetConfig().get_bool(PP_CMSETDESTDATE))
					SetFileDirectoryTime(ccp.strDstFile, &fi); // no error check - ma³o istotne

				// attributes
				if (GetConfig().get_bool(PP_CMSETDESTATTRIBUTES))
					SetFileAttributes(ccp.strDstFile, fi.GetAttributes());	// j.w.
			}
		}

		// current copy finished - change what's needed
		pTask->SetCurrentIndex(0);
	}

	// delete buffer - it's not needed
	ccp.dbBuffer.Delete();

	// change status
	if (pTask->GetStatus(ST_OPERATION_MASK) == ST_MOVE)
	{
		pTask->SetStatus(ST_DELETING, ST_STEP_MASK);
		// set the index to 0 before deleting
		pTask->SetCurrentIndex(0);
	}
	else
	{
		pTask->SetStatus(ST_FINISHED, ST_STEP_MASK);

		// to look better - increase current index by 1
		pTask->SetCurrentIndex(stSize);
	}
	// log
	pTask->m_log.logi(_T("Finished processing in ProcessFiles"));
}

void CTask::CheckForWaitState(CTask* pTask)
{
	// limiting operation count
	pTask->SetStatus(ST_WAITING, ST_WAITING_MASK);
	bool bContinue=false;
	while (!bContinue)
	{
		if (pTask->CanBegin())
		{
			TRACE("CAN BEGIN ALLOWED TO CONTINUE...\n");
			pTask->SetStatus(0, ST_WAITING);
			bContinue=true;

			pTask->m_log.logi(_T("Finished waiting for begin permission"));

			//			return; // skips sleep and kill flag checking
		}

		Sleep(50);	// not to make it too hard for processor

		if (pTask->GetKillFlag())
		{
			// log
			pTask->m_log.logi(_T("Kill request while waiting for begin permission (wait state)"));
			throw new CProcessingException(E_KILL_REQUEST, pTask);
		}
	}
}

int CTask::OnBeginTask()
{
   return rand();
}

int CTask::OnEndTask(int i)
{
   return i + 2;
}

UINT CTask::ThrdProc(LPVOID pParam)
{
	TRACE("\n\nENTERING ThrdProc (new task started)...\n");
	CTask* pTask=static_cast<CTask*>(pParam);
   pTask->OnBeginTask();
	chcore::IFeedbackHandler* piFeedbackHandler = pTask->GetFeedbackHandler();

	tstring_t strPath = pTask->GetTaskPath();
	strPath += pTask->GetUniqueName()+_T(".log");

	pTask->m_log.init(strPath.c_str(), 262144, icpf::log_file::level_debug, false, false);

	// set thread boost
	HANDLE hThread=GetCurrentThread();
	::SetThreadPriorityBoost(hThread, GetConfig().get_bool(PP_CMDISABLEPRIORITYBOOST));

	CTime tm=CTime::GetCurrentTime();

	ictranslate::CFormat fmt;
	fmt.SetFormat(_T("\r\n# COPYING THREAD STARTED #\r\nBegan processing data (dd:mm:yyyy) %day.%month.%year at %hour:%minute.%second"));
	fmt.SetParam(_t("%year"), tm.GetYear());
	fmt.SetParam(_t("%month"), tm.GetMonth());
	fmt.SetParam(_t("%day"), tm.GetDay());
	fmt.SetParam(_t("%hour"), tm.GetHour());
	fmt.SetParam(_t("%minute"), tm.GetMinute());
	fmt.SetParam(_t("%second"), tm.GetSecond());
	pTask->m_log.logi(fmt);

	try
	{
		// to make the value stable
		bool bReadTasksSize=GetConfig().get_bool(PP_CMREADSIZEBEFOREBLOCKING);

		if (!bReadTasksSize)
			CheckForWaitState(pTask);	// operation limiting

		// set what's needed
		pTask->m_lLastTime=(long)time(NULL);	// last time (start counting)

		// search for files if needed
		if ((pTask->GetStatus(ST_STEP_MASK) == ST_NULL_STATUS
			|| pTask->GetStatus(ST_STEP_MASK) == ST_SEARCHING))
		{
			// get rid of info about processed sizes
			pTask->DecreaseProcessedTasksSize(pTask->GetProcessedSize());
			pTask->SetProcessedSize(0);
			pTask->DecreaseAllTasksSize(pTask->GetAllSize());
			pTask->SetAllSize(0);

			// start searching
			RecurseDirectories(pTask);
		}

		// check for free space
		ull_t ullNeededSize = 0, ullAvailableSize = 0;
l_showfeedback:
		pTask->m_log.logi(_T("Checking for free space on destination disk..."));

		if (!pTask->GetRequiredFreeSpace(&ullNeededSize, &ullAvailableSize))
		{
			fmt.SetFormat(_T("Not enough free space on disk - needed %needsize bytes for data, available: %availablesize bytes."));
			fmt.SetParam(_t("%needsize"), ullNeededSize);
			fmt.SetParam(_t("%availablesize"), ullAvailableSize);
			pTask->m_log.logw(fmt);

			BOOST_ASSERT(piFeedbackHandler);

			if(pTask->GetClipboardDataSize() > 0)
			{
				CString strSrcPath = pTask->GetClipboardData(0)->GetPath();
				CString strDstPath = pTask->GetDestPath().GetPath();
				FEEDBACK_NOTENOUGHSPACE feedStruct = { ullNeededSize, (PCTSTR)strSrcPath, (PCTSTR)strDstPath };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_NotEnoughSpace, &feedStruct);

				// default
				switch (frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					{
						pTask->m_log.logi(_T("Cancel request while checking for free space on disk."));
						throw new CProcessingException(E_CANCEL, pTask);
						break;
					}
				case CFeedbackHandler::eResult_Retry:
					pTask->m_log.logi(_T("Retrying to read drive's free space..."));
					goto l_showfeedback;
					break;
				case CFeedbackHandler::eResult_Skip:
					pTask->m_log.logi(_T("Ignored warning about not enough place on disk to copy data."));
					break;
				default:
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, pTask, 0, _t("Unknown feedback result type"));
					break;
				}
			}
		}

		if (bReadTasksSize)
		{
			pTask->UpdateTime();
			pTask->m_lLastTime=-1;

			CheckForWaitState(pTask);

			pTask->m_lLastTime=(long)time(NULL);
		}

		// Phase II - copying/moving
		if (pTask->GetStatus(ST_STEP_MASK) == ST_COPYING)
		{
			// decrease processed in ctaskarray - the rest will be done in ProcessFiles
			pTask->DecreaseProcessedTasksSize(pTask->GetProcessedSize());
			ProcessFiles(pTask);
		}

		// deleting data - III phase
		if (pTask->GetStatus(ST_STEP_MASK) == ST_DELETING)
			DeleteFiles(pTask);

		// refresh time
		pTask->UpdateTime();

		// save progress before killed
		pTask->Store(false);

		// we are ending
		pTask->DecreaseOperationsPending();

		// play sound
		piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_OperationFinished, NULL);

		tm=CTime::GetCurrentTime();
		fmt.SetFormat(_T("Finished processing data (dd:mm:yyyy) %day.%month.%year at %hour:%minute.%second"));
		fmt.SetParam(_t("%year"), tm.GetYear());
		fmt.SetParam(_t("%month"), tm.GetMonth());
		fmt.SetParam(_t("%day"), tm.GetDay());
		fmt.SetParam(_t("%hour"), tm.GetHour());
		fmt.SetParam(_t("%minute"), tm.GetMinute());
		fmt.SetParam(_t("%second"), tm.GetSecond());
		pTask->m_log.logi(fmt);

		// we have been killed - the last operation
		InterlockedIncrement(pTask->m_plFinished);
		pTask->CleanupAfterKill();
		pTask->SetKilledFlag();
	}
	catch(CProcessingException* e)
	{
		// increment count of beginnings
		InterlockedIncrement(pTask->m_plFinished);

		// refresh time
		pTask->UpdateTime();

		// log
		fmt.SetFormat(_T("Caught exception in ThrdProc [last error: %errno, type: %type]"));
		fmt.SetParam(_t("%errno"), e->m_dwError);
		fmt.SetParam(_t("%type"), e->m_iType);
		pTask->m_log.loge(fmt);

		if (e->m_iType == E_ERROR)
			piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_OperationError, NULL);

		// perform some adjustments depending on exception type
		switch(e->m_iType)
		{
		case E_ERROR:
			pTask->SetStatus(ST_ERROR, ST_WORKING_MASK);
			pTask->SetOsErrorCode(e->m_dwError, e->m_strErrorDesc);
			break;
		case E_CANCEL:
			pTask->SetStatus(ST_CANCELLED, ST_STEP_MASK);
			break;
		case E_PAUSE:
			pTask->SetStatus(ST_PAUSED, ST_PAUSED);
			break;
		}

		// change flags and calls cleanup for a task
		switch(pTask->GetStatus(ST_STEP_MASK))
		{
		case ST_NULL_STATUS:
		case ST_SEARCHING:
			// get rid of m_files contents
			pTask->FilesRemoveAll();

			// save state of a task
			pTask->Store(true);
			pTask->Store(false);

			break;
		case ST_COPYING:
		case ST_DELETING:
			pTask->Store(false);
			break;
		}

		if (pTask->GetStatus(ST_WAITING_MASK) & ST_WAITING)
			pTask->SetStatus(0, ST_WAITING);

		pTask->DecreaseOperationsPending();
		pTask->SetContinueFlag(false);
		pTask->SetForceFlag(false);
		pTask->SetKilledFlag();
		pTask->CleanupAfterKill();

		delete e;

		return 0xffffffff;	// almost like -1
	}

   pTask->OnEndTask(7);

	TRACE("TASK FINISHED - exiting ThrdProc.\n");
	return 0;
}
