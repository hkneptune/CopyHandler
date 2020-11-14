#pragma once

#include "ISerializerRowData.h"
#include "ISerializerRowReader.h"
#include "SerializerDataTypes.h"
#include <bitset>

namespace serializer
{
	template<size_t BitsetSize, size_t AddedBit>
	class SerializableObject
	{
	public:
		SerializableObject() = default;

		SerializableObject(const SerializableObject& rSrc) :
			m_oidObjectID(rSrc.m_oidObjectID),
			m_setModifications(rSrc.m_setModifications)
		{
		}

		virtual ~SerializableObject() = default;

		SerializableObject& operator=(const SerializableObject& rSrc)
		{
			if(this != &rSrc)
			{
				m_oidObjectID = rSrc.m_oidObjectID;
				m_setModifications = rSrc.m_setModifications;
			}

			return *this;
		}

		serializer::object_id_t GetObjectID() const
		{
			return m_oidObjectID;
		}

		void SetObjectID(serializer::object_id_t oidObjectID)
		{
			m_oidObjectID = oidObjectID;
		}

		void MarkAsAdded()
		{
			m_setModifications[AddedBit] = true;
		}

		void ResetModifications()
		{
			m_setModifications.reset();
		}

		bool IsModified() const
		{
			return m_setModifications.any();
		}

		// serialization interface
		virtual void Store(const serializer::ISerializerContainerPtr& spContainer) const = 0;
		virtual void Load(const serializer::ISerializerRowReaderPtr& spRowReader) = 0;

	protected:
		serializer::object_id_t m_oidObjectID = 0;

		using Bitset = std::bitset<BitsetSize>;
		mutable Bitset m_setModifications;
	};
}
