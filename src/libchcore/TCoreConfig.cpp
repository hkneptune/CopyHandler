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
#include "stdafx.h"
#include "../libicpf/cfg.h"
#include "TCoreConfig.h"
#include "../libicpf/exception.h"

BEGIN_CHCORE_NAMESPACE

TCoreConfig TCoreConfig::S_Config = icpf::config::eIni;

TCoreConfig::TCoreConfig(icpf::config::config_base_types eType) :
	icpf::config(eType)
{
}

TCoreConfig::~TCoreConfig()
{
}

void TCoreConfig::SetBasePath(const tchar_t* pszPath)
{
	if(!pszPath)
		THROW(_T("Invalid argument"), 0, 0, 0);
	m_strBasePath = pszPath;
	m_strTasksPath = m_strBasePath + _T("\\tasks\\");
}

TCoreConfig& TCoreConfig::Acquire()
{
	return S_Config;
}

END_CHCORE_NAMESPACE
