#pragma once
#include "../libchcore/TCoreException.h"
#include "../libchcore/ErrorCodes.h"

namespace serializer
{
	template<class T>
	class SerializableContainer
	{
	public:
		SerializableContainer() = default;

		SerializableContainer(const SerializableContainer<T>& rSrc)
		{
			for(const T& item : rSrc.m_vEntries)
			{
				// ensures proper assignment of new oids
				Add(item);
			}
		}

		SerializableContainer<T>& operator=(const SerializableContainer<T>& rSrc)
		{
			if(this != &rSrc)
			{
				Clear();
				for(const T& item : rSrc.m_vEntries)
				{
					// ensures proper assignment of new oids
					Add(item);
				}
			}

			return *this;
		}

		virtual ~SerializableContainer() = default;

		void Store(const serializer::ISerializerContainerPtr& spContainer) const
		{
			if(!spContainer)
				throw chcore::TCoreException(chcore::eErr_InvalidPointer, L"spContainer", LOCATION);

			InitColumns(spContainer);

			spContainer->DeleteRows(m_setRemovedObjects);
			m_setRemovedObjects.Clear();

			for(const T& rEntry : m_vEntries)
			{
				rEntry.Store(spContainer);
			}
		}

		void Load(const serializer::ISerializerContainerPtr& spContainer)
		{
			if(!spContainer)
				throw chcore::TCoreException(chcore::eErr_InvalidPointer, L"spContainer", LOCATION);

			m_setRemovedObjects.Clear();
			m_vEntries.clear();
			m_oidLastObjectID = 0;

			InitColumns(spContainer);

			ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
			while(spRowReader->Next())
			{
				T tEntry;
				tEntry.Load(spRowReader);

				m_oidLastObjectID = std::max(m_oidLastObjectID, tEntry.GetObjectID());
				m_vEntries.push_back(tEntry);
			}

			// ensure all objects have modification flag stripped to avoid unnecessary writing the same data to db again
			// NOTE: Load() method above should reset modification flag, but storing it in vector will set it again - hence the separate reset
			ResetModifications();
		}

		virtual void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const = 0;

		bool IsEmpty() const
		{
			return m_vEntries.empty();
		}

		void Add(const T& rEntry)
		{
			auto iterResult = m_vEntries.insert(m_vEntries.end(), rEntry);
			iterResult->SetObjectID(++m_oidLastObjectID);
			iterResult->MarkAsAdded();
		}

		bool SetAt(size_t stIndex, const T& rNewEntry)
		{
			BOOST_ASSERT(stIndex < m_vEntries.size());
			if(stIndex < m_vEntries.size())
			{
				T& rEntry = m_vEntries.at(stIndex);

				// set only data, without changing oid
				rEntry.SetData(rNewEntry);
				return true;
			}

			return false;
		}

		bool InsertAt(size_t stIndex, const T& rNewEntry)
		{
			BOOST_ASSERT(stIndex <= m_vEntries.size());
			if(stIndex <= m_vEntries.size())
			{
				auto iterResult = m_vEntries.insert(m_vEntries.begin() + stIndex, rNewEntry);
				iterResult->SetObjectID(++m_oidLastObjectID);
				iterResult->MarkAsAdded();
				return true;
			}

			return false;
		}

		const T& GetAt(size_t stIndex) const
		{
			if(stIndex >= m_vEntries.size())
				throw std::out_of_range("stIndex is out of range");

			return m_vEntries.at(stIndex);
		}

		T& GetAt(size_t stIndex)
		{
			if(stIndex >= m_vEntries.size())
				throw std::out_of_range("stIndex is out of range");

			return m_vEntries.at(stIndex);
		}

		bool RemoveAt(size_t stIndex)
		{
			BOOST_ASSERT(stIndex < m_vEntries.size());
			if(stIndex < m_vEntries.size())
			{
				m_setRemovedObjects.Add(m_vEntries[stIndex].GetObjectID());

				m_vEntries.erase(m_vEntries.begin() + stIndex);
				return true;
			}

			return false;
		}

		size_t GetCount() const
		{
			return m_vEntries.size();
		}

		void Clear()
		{
			for(const T& rEntry : m_vEntries)
			{
				m_setRemovedObjects.Add(rEntry.GetObjectID());
			}
			m_vEntries.clear();
		}

		void ResetModifications()
		{
			for(T& rEntry : m_vEntries)
			{
				rEntry.ResetModifications();
			}
			m_setRemovedObjects.Clear();
		}

		bool IsModified() const
		{
			if(!m_setRemovedObjects.IsEmpty())
				return true;

			for(const T& rEntry : m_vEntries)
			{
				if(rEntry.IsModified())
					return true;
			}
			return false;
		}

	protected:
		serializer::object_id_t m_oidLastObjectID = 0;
		std::vector<T> m_vEntries;
		mutable serializer::TRemovedObjects m_setRemovedObjects;
	};
}
