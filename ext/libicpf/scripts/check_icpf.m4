dnl @synopsis CT_CHECK_ICPF
dnl
dnl This macro tries to find the headers and libraries for the
dnl icpf library.
dnl
dnl If includes are found, the variable ICPFINCPATH will be set. If
dnl librarys are found, the variable ICPFLIBPATH will be set. if no check
dnl was successful, the script exits with a error message.
dnl
dnl @category InstalledPackages
dnl @author Ixen Gerthannes, heavily based on CT_CHECK_POSTGRES_DB by Christian Toepp <c.toepp@gmail.com>
dnl @version 2005-07-25
dnl @license AllPermissive

AC_DEFUN([CT_CHECK_ICPF], [

AC_ARG_WITH(libicpf,
	[  --with-libicpf=PREFIX		Prefix of your libicpf installation],
	[icpf_prefix=$withval], [icpf_prefix=])
AC_ARG_WITH(libicpf-inc,
	[  --with-libicpf-inc=PATH		Path to the include directory of libicpf],
	[icpf_inc=$withval], [icpf_inc=])
AC_ARG_WITH(libicpf-lib,
	[  --with-libicpf-lib=PATH		Path to the library of libicpf],
	[icpf_lib=$withval], [icpf_lib=])


AC_SUBST(ICPFINCPATH)
AC_SUBST(ICPFLIBPATH)

if test "$icpf_prefix" != ""; then
   AC_MSG_CHECKING([for libicpf includes in $icpf_prefix/include])
   if test -f "$icpf_prefix/include/libicpf.h" ; then
      ICPFINCPATH="-I$icpf_prefix/include"
      AC_MSG_RESULT([yes])
   else
      AC_MSG_ERROR(libicpf.h not found)
   fi
   AC_MSG_CHECKING([for libicpf library in $icpf_prefix/lib])
   if test -f "$icpf_prefix/lib/libicpf.a" ; then
      ICPFLIBPATH="-L$icpf_prefix/lib"
      AC_MSG_RESULT([yes])
   else
      AC_MSG_ERROR(libicpf.a not found)
   fi
else
  if test "$icpf_inc" != ""; then
    AC_MSG_CHECKING([for libicpf includes in $icpf_inc])
    if test -f "$icpf_inc/libicpf.h" ; then
      ICPFINCPATH="-I$icpf_inc"
      AC_MSG_RESULT([yes])
    else
      AC_MSG_ERROR(libicpf.h not found)
    fi
  fi
  if test "$icpf_lib" != ""; then
    AC_MSG_CHECKING([for libicpf library in $icpf_lib])
    if test -f "$icpf_lib/libicpf.a" ; then
      ICPFLIBPATH="-L$icpf_lib"
      AC_MSG_RESULT([yes])
    else
      AC_MSG_ERROR(libicpf.a not found)
    fi
  fi
fi

if test "$ICPFINCPATH" = "" ; then
  AC_CHECK_HEADER([libicpf.h], [], AC_MSG_ERROR(libicpf.h not found))
fi
if test "$ICPFLIBPATH" = "" ; then
  AC_MSG_ERROR(libicpf.a not found)
fi

])
