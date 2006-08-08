#include "dmutex.h"
#include <assert.h>
#include <stdio.h>

BEGIN_ICPF_NAMESPACE

///////////////////////////////////////////////////////////////
// debuggable mutex

d_mutex::d_mutex(dumpctx* pctx)
	: mutex()
{
	const char_t* psz="Unnamed";
	m_pszName=new char_t[strlen(psz)+1];
	strcpy(m_pszName, psz);

	m_pContext=pctx;

	m_ulLockCount=0;
}

d_mutex::d_mutex(const char_t* pszStr, dumpctx* pctx) :
	mutex(pszStr)
{
	m_pszName=new char_t[strlen(pszStr)+1];
	strcpy(m_pszName, pszStr);

	m_pContext=pctx;
}

d_mutex::~d_mutex()
{
	delete [] m_pszName;
}

bool d_mutex::lock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction)
{
	assert(m_pContext);

	((mutex*)this)->lock();

	m_ulLockCount++;

	// log the attempt and lock it
	char_t sz[512];
	sprintf(sz, "%s: Lock (lock count after operation: %lu) in (%s-%lu: %s)", m_pszName, m_ulLockCount, pszFile, ulLine, pszFunction);

	m_pContext->open(sz);
	m_pContext->close();

	return true;
}

bool d_mutex::unlock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction)
{
	assert(m_pContext);

	// log the attempt and lock it
	m_ulLockCount--;

	// log the attempt and lock it
	char_t sz[512];
	sprintf(sz, "%s: Unlock (lock count after operation: %lu) in (%s-%lu: %s)", m_pszName, m_ulLockCount, pszFile, ulLine, pszFunction);

	m_pContext->open(sz);
	m_pContext->close();

	((mutex*)this)->unlock();

	return true;
}

END_ICPF_NAMESPACE
