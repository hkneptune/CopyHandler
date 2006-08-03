#ifndef __ICPFVECTOR_H__
#define __ICPFVECTOR_H__

#include "libicpf.h"
#include "gen_types.h"

BEGIN_ICPF_NAMESPACE

class vector64
{
public:
	vector64();
	virtual ~vector64();

	void push(ull_t ullValue);
	void remove(ulong_t ulIndex);
	ull_t at(ulong_t ulIndex);
	ulong_t size();

private:
	void* m_pStorage;
};

class vector32
{
public:
	vector32();
	virtual ~vector32();

	void push(ulong_t ulValue);
	void remove(ulong_t ulIndex);
	ulong_t at(ulong_t ulIndex);
	ulong_t size();

private:
	void* m_pStorage;
};

// needs to be fixed for linux
#if defined _WIN64
	typedef vector64 vectorptr;
	typedef ull_t vectoritem;
#else
	typedef vector32 vectorptr;
	typedef ulong_t vectoritem;
#endif

template<class T, class BASE=vectorptr, class BASEITEM=vectoritem>
class vector : public BASE
{
public:
	vector() : BASE() { };
	virtual ~vector() { };

	void push(T tValue)
	{
		((BASE*)this)->push((BASEITEM)tValue);
	}

	T at(ulong_t ulIndex)
	{
		return (T)(((BASE*)this)->at(ulIndex));
	}
};

END_ICPF_NAMESPACE

#endif
