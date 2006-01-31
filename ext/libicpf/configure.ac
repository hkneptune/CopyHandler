AC_INIT(libicpf, 0.1alpha, ixen2@o2.pl)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(libicpf, 0.1alpha)

# Programs
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_LIBTOOL

# libraries
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([pthread], [pthread_mutex_init])

# header files
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h utime.h wchar.h pthread.h])

# typedefs, structs, ...
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# library functions
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([ftruncate memmove memset strchr strtoul utime])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
		 
AC_OUTPUT
