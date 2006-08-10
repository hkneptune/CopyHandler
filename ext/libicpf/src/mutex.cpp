#include "mutex.h"
#ifdef _WIN32
	#include <windows.h>
#else
	#include <pthread.h>
#endif

BEGIN_ICPF_NAMESPACE

#define m_pcsLock ((CRITICAL_SECTION*)m_pLock)
#define m_pmLock ((pthread_mutex_t*)m_pLock)

/** Standard constructor.
 */
mutex::mutex()
{
	construct();
}

/** Compatibility layer constructor (with d_mutex). Can take a fake dumpctx pointer, although
 *  does nothing with it. Effectively it is almost the same as standard constructor.
 */
mutex::mutex(void* /*pUnused*/)
{
	construct();
}

/** Compatibility layer constructor (with d_mutex). Can take a fake dumpctx pointer and a fake mutex name,
 *  although does nothing with it. Effectively it is almost the same as standard constructor.
 */
mutex::mutex(const char_t* /*pszStr*/, void* /*pUnused*/)
{
	construct();
}

/** Destructs the mutex.
 */
mutex::~mutex()
{
#ifdef _WIN32
	::DeleteCriticalSection(m_pcsLock);
	delete m_pcsLock;
#else
	pthread_mutex_destroy(m_pmLock);
	delete m_pmLock;
#endif
}

/** Performs a construction of this mutex. Used by every constructor to alloc the internal members
 *  and initialize it.
 */
void mutex::construct()
{
#ifdef _WIN32
	m_pLock=(void*)new CRITICAL_SECTION;
	::InitializeCriticalSection(m_pcsLock);
#else
	m_pLock=(void*)new pthread_mutex_t;

	pthread_mutexattr_t mta;
	pthread_mutexattr_init(&mta);
	pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(m_pmLock, &mta);

	pthread_mutexattr_destroy(&mta);
#endif
}

/** Locks this mutex using an underlying, system-dependent locking mechanism.
 */
void mutex::lock()
{
#ifdef _WIN32
	::EnterCriticalSection(m_pcsLock);
#else
	pthread_mutex_lock(m_pmLock);
#endif
}

/** Unlocks this mutex using an underlying, system-dependent locking mechanism.
 */
void mutex::unlock()
{
#ifdef _WIN32
	::LeaveCriticalSection(m_pcsLock);
#else
	pthread_mutex_unlock(m_pmLock);
#endif
}

/** Locks this mutex using an underlying, system-dependent locking mechanism.
 *  This is a compatibility layer over d_mutex. This class does not use any of the
 *  parameters given - they are only to allow seamless migration to/from the d_mutex.
 * \see MLOCK and MUNLOCK macros.
 */
void mutex::lock(const char_t* /*pszFile*/, ulong_t /*ulLine*/, const char_t* /*pszFunction*/)
{
	lock();
}

/** Unlocks this mutex using an underlying, system-dependent locking mechanism.
 *  This is a compatibility layer over d_mutex. This class does not use any of the
 *  parameters given - they are only to allow seamless migration to/from the d_mutex.
 * \see MLOCK and MUNLOCK macros.
 */
void mutex::unlock(const char_t* /*pszFile*/, ulong_t /*ulLine*/, const char_t* /*pszFunction*/)
{
	unlock();
}

END_ICPF_NAMESPACE
