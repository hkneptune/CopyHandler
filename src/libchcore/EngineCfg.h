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
#ifndef __ENGINE_CFG_H__
#define __ENGINE_CFG_H__

#include "libchcore.h"
#include "../libicpf/cfg.h"

BEGIN_CHCORE_NAMESPACE

// contains everything that could be configured inside the engine.
// supports both the informations contained in the ini file and 
// ones related to current instance of CH core
class LIBCHCORE_API engine_config : public icpf::config
{
protected:
	engine_config(icpf::config::config_base_types eType);
	virtual ~engine_config();

public:
	static engine_config& Acquire();

	// paths handling
	void set_base_path(const tchar_t* pszPath);
	const tchar_t* get_base_path() { return m_strBasePath.c_str(); }
	const tchar_t* get_tasks_path() { return m_strTasksPath.c_str(); }

private:
	tstring_t m_strBasePath;
	tstring_t m_strTasksPath;

	static engine_config S_Config;
};

END_CHCORE_NAMESPACE
#endif
