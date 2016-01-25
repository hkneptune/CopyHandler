// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../common/targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <boost/assert.hpp>
#include <boost/bind.hpp>

#pragma warning(push)
#pragma warning(disable: 4985)

#include <boost/thread/shared_mutex.hpp>

#pragma warning(pop)

#include <boost/thread/locks.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <list>
#include <set>
#include <vector>
#include <tchar.h>
