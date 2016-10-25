#ifndef __GTESTMACROS_H__
#define __GTESTMACROS_H__

#define EXPECT_TIMEOUT(handle)\
	{\
		DWORD dwResult = WaitForSingleObject(handle, 0); \
		EXPECT_EQ(WAIT_TIMEOUT, dwResult); \
	}

#define EXPECT_SIGNALED(handle)\
	{\
		DWORD dwResult = WaitForSingleObject(handle, 0); \
		EXPECT_EQ(WAIT_OBJECT_0 + 0, dwResult); \
	}

#endif
