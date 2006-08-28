#include "mutex.h"

BEGIN_ICPF_NAMESPACE

mutex::mutex()
{
#ifdef _WIN32
	::InitializeCriticalSection(&m_cs);
#else
	pthread_mutexattr_t mta;
	pthread_mutexattr_init(&mta);
//#warning Recursive mutexes are disabled; Make sure you use them the right way.
	pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&m_mutex, &mta);

	pthread_mutexattr_destroy(&mta);
#endif
}

mutex::mutex(const char_t* /*pszStr*/)
{
#ifdef _WIN32
	::InitializeCriticalSection(&m_cs);
#else
	pthread_mutexattr_t mta;
	pthread_mutexattr_init(&mta);
//#warning Recursive mutexes are disabled; Make sure you use them the right way.
	pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&m_mutex, &mta);

	pthread_mutexattr_destroy(&mta);
#endif
}

mutex::~mutex()
{
#ifdef _WIN32
	::DeleteCriticalSection(&m_cs);
#else
	pthread_mutex_destroy(&m_mutex);
#endif
}
	
// standard locking
void mutex::lock()
{
#ifdef _WIN32
	::EnterCriticalSection(&m_cs);
#else
	pthread_mutex_lock(&m_mutex) == 0;
#endif
}

void mutex::unlock()
{
#ifdef _WIN32
	::LeaveCriticalSection(&m_cs);
#else
	pthread_mutex_unlock(&m_mutex) == 0;		// return 0 on success
#endif
}

#ifdef ENABLE_MUTEX_DEBUGGING
void mutex::lock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction)
{
	lock();
}

void mutex::unlock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction)
{
	unlock();
}
#endif

END_ICPF_NAMESPACE
