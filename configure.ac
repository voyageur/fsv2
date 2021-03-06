# Process this file with autoconf to produce a configure script.
AC_INIT([fsv2], [2.0])
AC_CONFIG_SRCDIR([src/fsv.cxx])
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIRS([m4])

AM_INIT_AUTOMAKE()
AC_SUBST(RELEASE)

#AC_DEFINE(_GNU_SOURCE)
#AC_DEFINE(GTK_DISABLE_COMPAT_H)

LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# Programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL

AC_ISC_POSIX

# Header files
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS(strings.h sys/time.h unistd.h)

# Typedefs, structures
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
#AC_CHECK_TYPE(comparison_fn_t, int (*)( const void *, const void * ))
#AC_STRUCT_ST_BLOCKS
AC_STRUCT_TM

#AC_GNU_SOURCE
#AC_USE_SYSTEM_EXTENSIONS

# Library functions
AC_FUNC_ALLOCA
AC_FUNC_FNMATCH
AC_FUNC_STRFTIME
AC_CHECK_FUNCS(getcwd gettimeofday mktime strcspn strdup strspn strtod strtoul)
AC_REPLACE_FUNCS(scandir)

# Debugging
AC_ARG_ENABLE(debug, [  --enable-debug          turn on debugging])
if test "$enable_debug" = "yes" ; then
	AC_MSG_WARN([debugging enabled, CFLAGS=\"$CFLAGS\"])
	AM_CONDITIONAL(DEBUG, true)
#	AC_DEFINE(DEBUG)
#	AC_DEFINE(G_ENABLE_DEBUG)
else
	AM_CONDITIONAL(DEBUG, false)
#	AC_DEFINE(G_DISABLE_ASSERT)
#	AC_DEFINE(G_DISABLE_CHECKS)
#	AC_DEFINE(GTK_NO_CHECK_CASTS)
fi

# Internationalization
#ALL_LINGUAS=""
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.20])

# Locale directory
AC_ARG_WITH(locale-dir, [  --with-locale-dir=DIR   install locale files in DIR [DATADIR/locale]])
if test "$USE_NLS" = "yes" ; then
	if test -n "$with_locale_dir" ; then
		localedir=$with_locale_dir
	else
		localedir=${datadir}/locale
	fi
else
	localedir="none"
fi
AC_SUBST(localedir)

# Documentation directory
AC_ARG_WITH(doc-dir, [  --with-doc-dir=DIR      install documentation files in DIR [DATADIR/doc/fsv2]])
if test -n "$with_doc_dir" ; then
	docdir=$with_doc_dir
else
	docdir=${datadir}/doc/fsv2
fi
AC_SUBST(docdir)

# GTK+ libraries

PKG_CHECK_MODULES(GTK, gtkmm-2.4)

#### OpenGL libraries ####
AC_ARG_WITH(GL-prefix,  [  --with-GL-prefix=PFX    Prefix where OpenGL is installed (optional)])

if test -n "$with_GL_prefix" ; then
	GL_CFLAGS=-I$with_GL_prefix/include
	GL_LDOPTS=-L$with_GL_prefix/lib
else
	GL_CFLAGS=""
	GL_LDOPTS=""
fi

AC_CHECK_LIB(GL,     glBegin, have_GL=yes,     , $GTK_LIBS $GL_LDOPTS)

if test "$have_GL" = "yes" ; then
	GL_LIBS="$GL_LDOPTS -lGL -lGLU"
else
	AC_MSG_ERROR([Missing GL library])
fi
AC_SUBST(GL_CFLAGS)
AC_SUBST(GL_LIBS)
#### end of OpenGL configuration ####

#### GtkGLArea library ####
PKG_CHECK_MODULES(GTKGL, gtkgl-2.0)

AC_ARG_ENABLE(glc, [  --enable-glc          turn on text render thru GLC])
if test "$enable_glc" == "yes" ; then
    AC_CHECK_HEADERS([GL/glc.h],
                 [],
                 [AC_MSG_WARN([GL/glc.h not found])])
    AC_CHECK_LIB([GLC], [main])
else
    PKG_CHECK_MODULES(FTGL, ftgl)
    AC_DEFINE([HAVE_FTGL],1,[ using ftgl ])
    AC_PATH_PROG(FC_CACHE,fc-match)
    if test -n "$FC_CACHE" ; then
	FONT_FILE="`$FC_CACHE -sv sans |grep ttf | sed -e 's/.*\"\\//\\//' | sed -e 's/\".*//' | sed q`"
	if test -n "$FONT_FILE"; then 
	  AC_DEFINE_UNQUOTED([DEFAULT_FONT_FILE], ["$FONT_FILE"]," default font file ")
	else
	  AC_MSG_ERROR([Default font cannot be found])
	fi
    fi
    #"$FTGL_LIBS"
fi

#### end of GtkGLArea configuration ####

# Check for 'file' command
AC_PATH_PROG(FILE_CMD, file)
if test -n "$FILE_CMD" ; then
	AC_DEFINE([HAVE_FILE_COMMAND],1,[ Defined when file command exest ])
	AC_DEFINE_UNQUOTED([FILE_COMMAND], ["$FILE_CMD "]," file command ")
fi

#CFLAGS="-Wall $CFLAGS -DGDK_DISABLE_DEPRECATED=1 -DGTK_DISABLE_DEPRECATED=1 -DGDK_PIXBUF_DISABLE_DEPRECATED=1 -DG_DISABLE_DEPRECATED=1"
#CXXFLAGS="-Wall $CXXFLAGS -DGDK_DISABLE_DEPRECATED=1 -DGTK_DISABLE_DEPRECATED=1 -DGDK_PIXBUF_DISABLE_DEPRECATED=1 -DG_DISABLE_DEPRECATED=1"
CFLAGS="-Wall $CFLAGS -DGDK_DISABLE_DEPRECATED=1 -DGTK_DISABLE_DEPRECATED=1 -DGDK_PIXBUF_DISABLE_DEPRECATED=1"
CXXFLAGS="-Wall $CXXFLAGS -DGDK_DISABLE_DEPRECATED=1 -DGTK_DISABLE_DEPRECATED=1 -DGDK_PIXBUF_DISABLE_DEPRECATED=1"

# That's a wrap!
AC_OUTPUT(fsv.spec
	Makefile
	debugloc/Makefile
	doc/Makefile
	po/Makefile.in
	src/Makefile
        uilib/Makefile
)

#	lib/Makefile
