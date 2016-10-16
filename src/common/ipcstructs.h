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
#ifndef __SHAREDDATA_H__
#define __SHAREDDATA_H__

#include <boost/lexical_cast.hpp>

// messages used
#define WM_GETCONFIG	WM_USER+20

enum ECopyDataType
{
	eCDType_TaskDefinitionContent,
	eCDType_TaskDefinitionContentSpecial,
	eCDType_CommandLineArguments,
};

enum ELocation
{
	eLocation_DragAndDropMenu,
	eLocation_ContextMenu
};

namespace IPCSupport
{
	static std::wstring GenerateSHMName(unsigned long ulRequestID)
	{
		std::wstring wstrName = _T("CHExtSHM_");
		wstrName += boost::lexical_cast<std::wstring>(ulRequestID);
		return wstrName;
	}
}

#endif
