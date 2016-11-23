// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#include "ConfigNode.h"

namespace chcore
{
	namespace details
	{
		ConfigNode::ConfigNode(object_id_t oidObjectID, const TString& strNodeName, int iOrder, const TString& strValue) :
			m_oidObjectID(oidObjectID),
			m_iOrder(m_setModifications, iOrder),
			m_strNodeName(m_setModifications, strNodeName),
			m_strValue(m_setModifications, strValue)
		{
			m_setModifications[eMod_Added] = true;
		}

		ConfigNode::ConfigNode(const ConfigNode& rSrc) :
			m_oidObjectID(rSrc.m_oidObjectID),
			m_iOrder(m_setModifications, rSrc.m_iOrder),
			m_strNodeName(m_setModifications, rSrc.m_strNodeName),
			m_strValue(m_setModifications, rSrc.m_strValue),
			m_setModifications(rSrc.m_setModifications)
		{
		}

		ConfigNode& ConfigNode::operator=(const ConfigNode& rSrc)
		{
			if (this != &rSrc)
			{
				m_oidObjectID = rSrc.m_oidObjectID;
				m_iOrder = rSrc.m_iOrder;
				m_strNodeName = rSrc.m_strNodeName;
				m_strValue = rSrc.m_strValue;
				m_setModifications = rSrc.m_setModifications;
			}

			return *this;
		}

		TString ConfigNode::GetNodeName() const
		{
			return m_strNodeName;
		}

		int ConfigNode::GetOrder() const
		{
			return m_iOrder;
		}
	}
}
