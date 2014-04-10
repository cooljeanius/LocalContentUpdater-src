#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([AdobeFlashLocalContentUpdater],[8.0.0],[local_content_updater@macromedia.com])
if test "x${PACKAGE_TARNAME}" = "x"; then
    test -z "${PACKAGE_TARNAME}" && export PACKAGE_TARNAME=LocalContentUpdater
    AC_SUBST([PACKAGE_TARNAME])
fi
AC_CONFIG_SRCDIR([common/LocalContentUpdater.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

# Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_SUBST([ac_cv_path_install])
m4_ifdef([AM_PROG_INSTALL_STRIP],[
AM_PROG_INSTALL_STRIP
])
AC_PROG_MKDIR_P
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXXCPP
AC_PROG_CXX_C_O
AC_LANG([C++])

# Checks for libraries.
AC_CHECK_LIB([z],[uncompress])
AC_SEARCH_LIBS([compress],[z System stdc++ c++ c gnu iberty])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_HEADER_STDBOOL dnl# also "_CHECK"s it
AC_CHECK_HEADERS([errno.h stdio.h windows.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_C_CONST
AC_C_PROTOTYPES

# Checks for library functions.
AC_FUNC_STAT
AC_CHECK_FUNCS([fclose fopen fprintf free fseek ftell memcpy rewind \
                snprintf sort strcmp strdup strerror])

AC_CONFIG_FILES([Makefile unix/Makefile])
AC_OUTPUT
