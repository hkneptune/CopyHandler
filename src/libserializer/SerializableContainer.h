#pragma once

namespace serializer
{
	template<class T>
	class SerializableContainer
	{
	public:

		void Store(const serializer::ISerializerContainerPtr& spContainer) const
		{
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
			InitColumns(spContainer);

			ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
			while(spRowReader->Next())
			{
				T tEntry;
				tEntry.Load(spRowReader);

				tEntry.ResetModifications();

				m_vEntries.push_back(tEntry);
			}
		}

		virtual void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const = 0;

		bool IsEmpty() const
		{
			return m_vEntries.empty();
		}

		void Add(const T& rEntry)
		{
			m_vEntries.push_back(rEntry);
		}

		bool SetAt(size_t stIndex, const T& rNewEntry)
		{
			BOOST_ASSERT(stIndex < m_vEntries.size());
			if(stIndex < m_vEntries.size())
			{
				T& rEntry = m_vEntries.at(stIndex);

				rEntry.SetData(rNewEntry);
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

	protected:
		std::vector<T> m_vEntries;
		mutable serializer::TRemovedObjects m_setRemovedObjects;
	};
}
