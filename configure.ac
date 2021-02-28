# Initialize autoconf
AC_INIT([steel-monsters], [0.1.0], [lucas.engen.cc@gmail.com])

# Check version prerequisites
AC_PREREQ([2.69])

AC_CONFIG_SRCDIR([src])
AM_INIT_AUTOMAKE([1.16 gnu subdir-objects])

AC_PROG_CC

# Check for raylib
AC_CHECK_HEADER([raylib.h], [AC_DEFINE(HAVE_RAYLIB_H, 1, have raylib)],
                [AC_MSG_ERROR([we dont have raylib installed])])

AC_CHECK_LIB([raylib], [BeginDrawing])
AC_CHECK_LIB([raylib], [WindowShouldClose])

AC_CONFIG_HEADERS([config.h])

# Configure makefile
AC_CONFIG_FILES([Makefile])

# Write output
AC_OUTPUT
