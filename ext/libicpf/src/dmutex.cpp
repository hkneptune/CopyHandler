#include "dmutex.h"
#include <assert.h>
#include <stdio.h>

#ifdef ENABLE_MUTEX_DEBUGGING

BEGIN_ICPF_NAMESPACE

///////////////////////////////////////////////////////////////
// debuggable mutex

/// Static dump context
dumpctx* d_mutex::m_pContext=NULL;

/** Constructs an unnamed mutex with a given dump context which will receive
 *  notifications about locking and unlocking of this mutex.
 */
d_mutex::d_mutex() :
	mutex(),
	m_ulLockCount(0)
{
	const char_t* psz="Unnamed";
	m_pszName=new char_t[strlen(psz)+1];
	strcpy(m_pszName, psz);

	m_ulLockCount=0;
}

/** Constructs a named mutex with a given dump context which will receive
 *  notifications about locking and unlocking of this mutex.
 *
 * \param[in] pszStr - name of this mutex (will be used for logging purposes)
 */
d_mutex::d_mutex(const char_t* pszStr) :
	mutex(pszStr),
	m_ulLockCount(0)
{
	m_pszName=new char_t[strlen(pszStr)+1];
	strcpy(m_pszName, pszStr);
}

/** Destructs the object
 */
d_mutex::~d_mutex()
{
	delete [] m_pszName;
}

/** Locks this mutex. Takes some parameters that should identify the place in code which
 *  at which the locking occurs.
 *
 * \param[in] pszFile - name of the source file in which the locking was requested
 * \param[in] ulLine - line of code in the file at which the locking was requested
 * \param[in] pszFunction - name of the function in which the locking was requested
 */
void d_mutex::lock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction)
{
	assert(m_pContext);

	((mutex*)this)->lock();

	m_ulLockCount++;

	// log the attempt and lock it
	char_t sz[512];
	sprintf(sz, "%s: Lock (lock count after operation: %lu) in (%s-%lu: %s)", m_pszName, m_ulLockCount, pszFile, ulLine, pszFunction);

	m_pContext->open(sz);
	m_pContext->close();
}

/** Unlocks this mutex. Takes some parameters that should identify the place in code which
 *  at which the unlocking occurs.
 *
 * \param[in] pszFile - name of the source file in which the unlocking was requested
 * \param[in] ulLine - line of code in the file at which the unlocking was requested
 * \param[in] pszFunction - name of the function in which the unlocking was requested
 */
void d_mutex::unlock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction)
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
}

END_ICPF_NAMESPACE

#endif
