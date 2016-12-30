/***************************************************************************
 *   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
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
#ifndef __TFILEFILTERSARRAY_H__
#define __TFILEFILTERSARRAY_H__

#include "TFileFilter.h"
#include "../libserializer/TRemovedObjects.h"
#include "../common/GenericTemplates/RandomAccessIterators.h"
#include "../common/GenericTemplates/RandomAccessContainerWrapper.h"

namespace chengine
{
	class TConfig;
	class TFileInfo;
	typedef std::shared_ptr<TFileInfo> TFileInfoPtr;

	template class LIBCHENGINE_API RandomAccessIteratorWrapper<TFileFilter>;
	class LIBCHENGINE_API TFileFiltersArrayIterator : public RandomAccessIteratorWrapper<TFileFilter>
	{
	};

	template class LIBCHENGINE_API RandomAccessConstIteratorWrapper<TFileFilter>;
	class LIBCHENGINE_API TFileFiltersArrayConstIterator : public RandomAccessConstIteratorWrapper<TFileFilter>
	{
	};

	template class LIBCHENGINE_API RandomAccessContainerWrapper<TFileFilter>;

	class LIBCHENGINE_API TFileFiltersArray : public RandomAccessContainerWrapper<TFileFilter>
	{
	public:
		bool Match(const TFileInfoPtr& spInfo) const;

		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerContainerPtr& spContainer);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

	private:
		mutable serializer::TRemovedObjects m_setRemovedObjects;
	};
}

CONFIG_MEMBER_SERIALIZATION(TFileFiltersArray)

#endif
