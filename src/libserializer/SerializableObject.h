#pragma once

#include "ISerializerRowData.h"
#include "ISerializerRowReader.h"
#include "SerializerDataTypes.h"
#include <bitset>

namespace serializer
{
	template<size_t BitsetSize>
	class SerializableObject
	{
	public:
		SerializableObject();
		SerializableObject(const SerializableObject& rSrc);
		virtual ~SerializableObject();

		SerializableObject& operator=(const SerializableObject& rSrc);

		// serialization interface
		virtual void Store(const serializer::ISerializerContainerPtr& spContainer) const = 0;
		virtual void Load(const serializer::ISerializerRowReaderPtr& spRowReader) = 0;

		serializer::object_id_t GetObjectID() const;
		void SetObjectID(serializer::object_id_t oidObjectID);

		void ResetModifications();

	protected:
		serializer::object_id_t m_oidObjectID = 0;

		using Bitset = std::bitset<BitsetSize>;
		mutable Bitset m_setModifications;
	};

	template<size_t BitsetSize>
	serializer::SerializableObject<BitsetSize>::SerializableObject()
	{
	}

	template<size_t BitsetSize>
	serializer::SerializableObject<BitsetSize>::SerializableObject(const SerializableObject& rSrc) :
		m_oidObjectID(rSrc.m_oidObjectID),
		m_setModifications(rSrc.m_setModifications)
	{
	}

	template<size_t BitsetSize>
	serializer::SerializableObject<BitsetSize>::~SerializableObject()
	{
	}

	template<size_t BitsetSize>
	SerializableObject<BitsetSize>& serializer::SerializableObject<BitsetSize>::operator=(const SerializableObject& rSrc)
	{
		m_oidObjectID = rSrc.m_oidObjectID;
		m_setModifications = rSrc.m_setModifications;

		return *this;
	}

	template<size_t BitsetSize>
	void serializer::SerializableObject<BitsetSize>::ResetModifications()
	{
		m_setModifications.reset();
	}

	template<size_t BitsetSize>
	void serializer::SerializableObject<BitsetSize>::SetObjectID(serializer::object_id_t oidObjectID)
	{
		m_oidObjectID = oidObjectID;
	}

	template<size_t BitsetSize>
	serializer::object_id_t serializer::SerializableObject<BitsetSize>::GetObjectID() const
	{
		return m_oidObjectID;
	}
}
