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
#ifndef __TEXTENSIONDETECTOR_H__
#define __TEXTENSIONDETECTOR_H__

#include <string>

class TExtensionDetector
{
public:
	TExtensionDetector();

	const std::wstring& GetNativeExtension() const { return m_strNativeExtension; }
	const std::wstring& GetNativeBasePath() const { return m_strNativeBasePath; }
#ifdef _WIN64
	const std::wstring& Get32bitExtension() const { return m_str32bitExtension; }
	const std::wstring& Get32bitBasePath() const { return m_str32bitBasePath; }
#endif

private:
	void DetectPaths();

private:
	std::wstring m_strNativeExtension;
	std::wstring m_strNativeBasePath;
#ifdef _WIN64
	std::wstring m_str32bitExtension;
	std::wstring m_str32bitBasePath;
#endif
};

#endif
