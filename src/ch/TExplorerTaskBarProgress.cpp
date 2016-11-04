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
#include "TExplorerTaskBarProgress.h"

TExplorerTaskBarProgress::TExplorerTaskBarProgress()
{
	HRESULT hResult = CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_INPROC_SERVER, IID_ITaskbarList3, (void**)&m_piTaskBarList3);
	if(FAILED(hResult) && m_piTaskBarList3)
		m_piTaskBarList3 = nullptr;
}

TExplorerTaskBarProgress::~TExplorerTaskBarProgress()
{
	if(m_piTaskBarList3)
	{
		m_piTaskBarList3->Release();
		m_piTaskBarList3 = nullptr;
	}
}

void TExplorerTaskBarProgress::SetState(HWND hwnd, TBPFLAG flag) const
{
	if(m_piTaskBarList3)
		m_piTaskBarList3->SetProgressState(hwnd, flag);
}

void TExplorerTaskBarProgress::SetPosition(HWND hwnd, unsigned long long ullCompleted, unsigned long long ullTotal) const
{
	if(m_piTaskBarList3)
		m_piTaskBarList3->SetProgressValue(hwnd, ullCompleted, ullTotal);
}
