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
#ifndef __CONFIGNODECONTAINER_H__
#define __CONFIGNODECONTAINER_H__

#include "TSharedModificationTracker.h"
#include "ConfigNode.h"
#include <bitset>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

#pragma warning(push)
#pragma warning(disable: 4100 4702 4512)
	#include <boost/signals2.hpp>
#pragma warning(pop)
#include "TStringSet.h"
#include "TStringArray.h"
#include "TRemovedObjects.h"
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/lexical_cast.hpp>

namespace chcore
{
	namespace details
	{
		struct ChangeValue
		{
			explicit ChangeValue(const TString& strNewValue);

			void operator()(ConfigNode& rNode);

			bool WasModified() const;

		private:
			TString m_strNewValue;
			bool m_bWasModified = false;
		};

		struct ChangeOrderAndValue
		{
			ChangeOrderAndValue(const TString& tNewValue, int iOrder);

			void operator()(ConfigNode& rNode);

			bool WasModified() const;

		private:
			TString m_strNewValue;
			int m_iOrder = 0;
			bool m_bWasModified = false;
		};

		struct ConfigNodeContainer
		{
		public:
			ConfigNodeContainer();
			ConfigNodeContainer(const ConfigNodeContainer& rSrc);

			ConfigNodeContainer& operator=(const ConfigNodeContainer& rSrc);

			void AddEntry(PCTSTR pszPropName, int iIndex, const TString& strValue);

			// get/set single values
			template<class T>
			T GetValue(PCTSTR pszPropName, const T& rDefaultValue) const
			{
				boost::shared_lock<boost::shared_mutex> lock(m_lock);
				T tResult = rDefaultValue;

				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
					tResult = boost::lexical_cast<T>((*iterFnd).m_strValue.Get().c_str());

				return tResult;
			}

			template<>
			TString GetValue<TString>(PCTSTR pszPropName, const TString& rDefaultValue) const
			{
				boost::shared_lock<boost::shared_mutex> lock(m_lock);
				TString tResult = rDefaultValue;

				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
					tResult = (*iterFnd).m_strValue;

				return tResult;
			}

			template<>
			bool GetValue<bool>(PCTSTR pszPropName, const bool& bDefaultValue) const
			{
				boost::shared_lock<boost::shared_mutex> lock(m_lock);
				bool bResult = bDefaultValue;

				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
				{
					if ((*iterFnd).m_strValue.Get().CompareNoCase(_T("false")) == 0)
						bResult = false;
					else if ((*iterFnd).m_strValue.Get().CompareNoCase(_T("true")) == 0)
						bResult = true;
					else
						bResult = boost::lexical_cast<bool>((*iterFnd).m_strValue.Get().c_str());
				}

				return bResult;
			}

			template<class T>
			bool GetValueNoDefault(PCTSTR pszPropName, T& rValue) const
			{
				boost::shared_lock<boost::shared_mutex> lock(m_lock);
				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
				{
					rValue = boost::lexical_cast<T>((*iterFnd).m_strValue.Get().c_str());
					return true;
				}

				return false;
			}

			template<>
			bool GetValueNoDefault<TString>(PCTSTR pszPropName, TString& rValue) const
			{
				boost::shared_lock<boost::shared_mutex> lock(m_lock);
				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
				{
					rValue = (*iterFnd).m_strValue;
					return true;
				}

				return false;
			}

			template<>
			bool GetValueNoDefault<bool>(PCTSTR pszPropName, bool& rValue) const
			{
				boost::shared_lock<boost::shared_mutex> lock(m_lock);
				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
				{
					const TString& strValue = (*iterFnd).m_strValue.Get();
					if (strValue.CompareNoCase(_T("false")) == 0)
						rValue = false;
					else if (strValue.CompareNoCase(_T("true")) == 0)
						rValue = true;
					else
						rValue = boost::lexical_cast<bool>(strValue.c_str());
					return true;
				}

				return false;
			}

			template<class T>
			bool SetValue(PCTSTR pszPropName, const T& rValue)
			{
				boost::unique_lock<boost::shared_mutex> lock(m_lock);

				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
				{
					ChangeValue tChange(boost::lexical_cast<std::wstring>(rValue).c_str());
					m_mic.modify(iterFnd, tChange);
					return tChange.WasModified();
				}
				else
				{
					m_mic.insert(ConfigNode(++m_oidLastObjectID, pszPropName, 0, boost::lexical_cast<std::wstring>(rValue).c_str()));
					return true;
				}
			}

			template<>
			bool SetValue<bool>(PCTSTR pszPropName, const bool& bValue)
			{
				boost::unique_lock<boost::shared_mutex> lock(m_lock);

				ConfigNodeContainer::NodeContainer::const_iterator iterFnd = m_mic.find(boost::make_tuple(pszPropName, 0));
				if (iterFnd != m_mic.end())
				{
					ChangeValue tChange(boost::lexical_cast<std::wstring>(bValue ? _T("true") : _T("false")).c_str());
					m_mic.modify(iterFnd, tChange);
					return tChange.WasModified();
				}
				else
				{
					m_mic.insert(ConfigNode(++m_oidLastObjectID, pszPropName, 0, bValue ? _T("true") : _T("false")));
					return true;
				}
			}

			// vector-based values
			TStringArray GetArrayValue(PCTSTR pszPropName, const TStringArray& rDefaultValue) const;
			bool GetArrayValueNoDefault(PCTSTR pszPropName, TStringArray& rValue) const;
			bool SetArrayValue(PCTSTR pszPropName, const TStringArray& rValue);

			// deletion
			void DeleteNode(PCTSTR pszPropName);

			// extracting nodes
			bool ExtractNodes(PCTSTR pszNode, ConfigNodeContainer& tNewContainer) const;
			bool ExtractMultipleNodes(PCTSTR pszNode, std::vector<ConfigNodeContainer>& tNewContainers) const;

			void ImportNodes(PCTSTR pszNode, const ConfigNodeContainer& tContainer);		// replaces specified node with data from tContainer
			void AddNodes(PCTSTR pszNode, const ConfigNodeContainer& tContainer);		// adds specified config as a newly numbered node in this container

			void ImportFromPropertyTree(const boost::property_tree::wiptree& rTree, boost::unique_lock<boost::shared_mutex>&);
			void ExportToPropertyTree(boost::property_tree::wiptree& rTree) const;

#ifdef _DEBUG
			// debugging
			void Dump();
#endif

		private:
			void ImportNode(TString strCurrentPath, const boost::property_tree::wiptree& rTree);

		public:
			typedef boost::multi_index_container<ConfigNode,
				boost::multi_index::indexed_by<
				boost::multi_index::ordered_unique<
				boost::multi_index::composite_key<ConfigNode,
				boost::multi_index::const_mem_fun<ConfigNode, TString, &ConfigNode::GetNodeName>,
				boost::multi_index::const_mem_fun<ConfigNode, int, &ConfigNode::GetOrder>
				>
				>
				>
			> NodeContainer;

			NodeContainer m_mic;

			TString m_strFilePath;

			boost::signals2::signal<void(const TStringSet&)> m_notifier;
			TStringSet m_setDelayedNotifications;
			bool m_bDelayedEnabled;
			object_id_t m_oidLastObjectID;

			TRemovedObjects m_setRemovedObjects;

			mutable boost::shared_mutex m_lock;
		};
	}
}

#endif
