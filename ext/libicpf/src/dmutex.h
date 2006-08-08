#ifndef __DMUTEX_H__
#define __DMUTEX_H__

#include "libicpf.h"
#include "gen_types.h"
#include "dumpctx.h"
#include "mutex.h"

BEGIN_ICPF_NAMESPACE

class LIBICPF_API d_mutex : public mutex
{
public:
/** \name Construction/destruction */
/**@{*/
	d_mutex(dumpctx* pctx);
	d_mutex(const char_t* pszStr, dumpctx* pctx);
	~d_mutex();
/**@}*/
	
	// standard locking
/** \name Locking/unlocking */
/**@{*/
	bool lock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction);
	bool unlock(const char_t* pszFile, ulong_t ulLine, const char_t* pszFunction);
/**@}*/

private:
	char* m_pszName;
	dumpctx* m_pContext;
	ulong_t m_ulLockCount;
};

END_ICPF_NAMESPACE

#endif
