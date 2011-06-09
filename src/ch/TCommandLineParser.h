// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TCommandLineParser.h
/// @date  2011/04/08
/// @brief Contains class for handling command line options.
// ============================================================================
#ifndef __TCOMMANDLINEPARSER_H__
#define __TCOMMANDLINEPARSER_H__

#include <boost/program_options.hpp>

namespace chcore { class TPathContainer; }

class TCommandLineParser
{
public:
	TCommandLineParser();
	~TCommandLineParser();

	void ParseCommandLine(const wchar_t* pszCommandLine);

	bool HasCommandLineParams() const;

	bool HasTaskDefinitionPath() const;
	void GetTaskDefinitionPaths(chcore::TPathContainer& vPaths) const;

private:
	boost::program_options::variables_map m_mapVariables;
};
#endif
