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
#ifndef __TSHELLEXTENSIONCONFIG_H__
#define __TSHELLEXTENSIONCONFIG_H__

#include "../common/TShellExtIpcConfigDataProvider.h"
#include "../liblogger/TLogFileData.h"
#include "../liblogger/TLogger.h"

class TShellExtMenuConfig;

class TShellExtensionConfig
{
public:
	TShellExtensionConfig(const logger::TLogFileDataPtr& spLogData);

	void PrepareConfig();

	void PrepareDragAndDropMenuItems(TShellExtMenuConfig &cfgShellExt) const;
	void PrepareNormalMenuItems(TShellExtMenuConfig &cfgShellExt) const;

private:
	TShellExtIpcConfigDataProvider m_shellExtProvider;
	logger::TLoggerPtr m_spLog;
};

using TShellExtensionConfigPtr = std::shared_ptr<TShellExtensionConfig>;

#endif
