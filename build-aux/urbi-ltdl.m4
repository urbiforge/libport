#
# urbi-tdl.m4: This file is part of build-aux.
# Copyright (C) 2008, 2009, Gostai S.A.S.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
# See the LICENSE file for more information.
# For comments, bug reports and feedback: http://www.urbiforge.com
#

m4_pattern_forbid([^URBI_])

AC_PREREQ([2.60])

# Help aclocal finding this macro.
AC_DEFUN([URBI_LTDL], [m4_fatal([aie])])

# URBI_LTDL
# ---------
# Use libltdl as shipped by libport, as a convenience library.
#
# To complete the use, one needs to add the following lines in the
# top-level Makefile:
#
# libtool: $(LIBTOOL_DEPS)
#	$(SHELL) ./config.status $@
#
# and use LTDL_CPPFLAGS and LTDL_LIBS where appropriate.
#
# You may also use LTDL_DEPS, but beware that specifying
# PROG_DEPENDENCIES stops Automake from generating dependencies from
# LDADD, which is a nuisance.
#
# Note that using "PROG_LDADD = $(LTDL_LIBS)" using the following
# AC_SUBST'ed LTDL_LIBS is *not* a good idea: because this variable is
# opaque to Automake (could actually be -lFOO etc. which are not valid
# dependencies), it cannot add it as is as a dependency.  As a result,
# if libltdl changes, no relinking will be done.  It is better to
# define LTDL_LIBS in the Makefile.am.
m4_define([URBI_LTDL],
[# Name of the subdirectory that contains libltdl sources.
LT_CONFIG_LTDL_DIR([libltdl])
LTDL_INIT([nonrecursive])

AC_SUBST([LTDL_CPPFLAGS], ['$(LTDLINCL)'])
AC_SUBST([LTDL_LIBS],     ['$(LIBLTDL)'])
AC_SUBST([LTDL_DEPS],     ['$(LTDLDEPS)'])
])

## Local Variables:
## mode: autoconf
## End:
