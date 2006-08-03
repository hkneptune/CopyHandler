#include "vector.h"
#include "err_codes.h"
#include <vector>
#include "exception.h"

BEGIN_ICPF_NAMESPACE

#define STORAGE32 ((std::vector<ulong_t>*)m_pStorage)
#define STORAGE64 ((std::vector<ull_t>*)m_pStorage)

vector64::vector64()
{
	m_pStorage=(void*)new std::vector<ull_t>;
}

vector64::~vector64()
{
	delete STORAGE64;
}

void vector64::push(ull_t ullValue)
{
	STORAGE64->push_back(ullValue);
}

void vector64::remove(ulong_t ulIndex)
{
	if (ulIndex >= STORAGE64->size())
		THROW(exception::format("[vector64::remove()] Index (" ULFMT ") out of range", ulIndex), CO_OUTOFRANGE, 0, 0);
	STORAGE64->erase(STORAGE64->begin()+ulIndex);
}

ull_t vector64::at(ulong_t ulIndex)
{
	if (ulIndex >= STORAGE64->size())
		THROW(exception::format("[vector64::at()] Index (" ULFMT ") out of range", ulIndex), CO_OUTOFRANGE, 0, 0);
	return STORAGE64->at(ulIndex);
}

ulong_t vector64::size()
{
	return (ulong_t)STORAGE64->size();
}


vector32::vector32()
{
	m_pStorage=(void*)new std::vector<ulong_t>;
}

vector32::~vector32()
{
	delete STORAGE32;
}

void vector32::push(ulong_t ulValue)
{
	STORAGE32->push_back(ulValue);
}

void vector32::remove(ulong_t ulIndex)
{
	if (ulIndex >= STORAGE32->size())
		THROW(exception::format("[vector32::remove()] Index (" ULFMT ") out of range", ulIndex), CO_OUTOFRANGE, 0, 0);
	STORAGE32->erase(STORAGE32->begin()+ulIndex);
}

ulong_t vector32::at(ulong_t ulIndex)
{
	if (ulIndex >= STORAGE32->size())
		THROW(exception::format("[vector32::at()] Index (" ULFMT ") out of range", ulIndex), CO_OUTOFRANGE, 0, 0);
	return STORAGE32->at(ulIndex);
}

ulong_t vector32::size()
{
	return (ulong_t)STORAGE32->size();
}

END_ICPF_NAMESPACE
