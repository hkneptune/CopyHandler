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
/// @file  TCommandLineParser.cpp
/// @date  2011/04/08
/// @brief Contains an implementation of command line parser class.
// ============================================================================
#include "stdafx.h"
#include "TCommandLineParser.h"
#include <boost/bind.hpp>
#include "../libchcore/TPath.h"
#include "../libchcore/TPathContainer.h"

TCommandLineParser::TCommandLineParser()
{
}

TCommandLineParser::~TCommandLineParser()
{
}

void TCommandLineParser::ParseCommandLine(const wchar_t* pszCommandLine)
{
	m_mapVariables.clear();

	namespace po = boost::program_options;

	std::vector<std::wstring> args = po::split_winmain(pszCommandLine);

	po::options_description desc("");
	desc.add_options()
		("ImportTaskDefinition", po::wvalue< std::vector<std::wstring> >(), "");

	po::variables_map vm;
	po::store(po::wcommand_line_parser(args).options(desc).run(), m_mapVariables);
	po::notify(vm);
}

bool TCommandLineParser::HasCommandLineParams() const
{
	return !m_mapVariables.empty();
}

bool TCommandLineParser::HasTaskDefinitionPath() const
{
	return m_mapVariables.count("ImportTaskDefinition") > 0;
}

void TCommandLineParser::GetTaskDefinitionPaths(chcore::TPathContainer& vPaths) const
{
	vPaths.Clear();

	if(HasTaskDefinitionPath())
	{
		const std::vector<std::wstring>& rvValues = m_mapVariables["ImportTaskDefinition"].as<std::vector<std::wstring> >();
		BOOST_FOREACH(const std::wstring& strPath, rvValues)
		{
			vPaths.Add(chcore::PathFromString(strPath.c_str()));
		}
	}
}
