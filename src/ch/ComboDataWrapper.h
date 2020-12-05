// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
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
#pragma once

template<class T>
class ComboDataWrapper
{
public:
	ComboDataWrapper(CComboBox& rCombo, T defaultValue, T lastValue) :
		m_rCombo(rCombo),
		m_defaultValue(defaultValue),
		m_lastValue(lastValue)
	{
	}

	T GetSelectedValue() const
	{
		int iSel = m_rCombo.GetCurSel();
		if(iSel < 0)
			return m_defaultValue;

		DWORD_PTR dwData = m_rCombo.GetItemData(iSel);
		if((T)dwData < m_lastValue)
			return (T)dwData;

		return m_defaultValue;
	}

	void SelectComboResult(T value)
	{
		for(int iIndex = 0; iIndex < m_rCombo.GetCount(); ++iIndex)
		{
			DWORD_PTR dwData = m_rCombo.GetItemData(iIndex);
			if((T)dwData == value)
			{
				m_rCombo.SetCurSel(iIndex);
				return;
			}
		}
	}

private:
	CComboBox& m_rCombo;
	T m_defaultValue;
	T m_lastValue;
};
