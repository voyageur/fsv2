if DEBUG
DEBUG_CFLAGS = -I$(top_srcdir)/debug
DEBUG_LIBS = $(top_builddir)/debug/libdebug.a
else
DEBUG_CFLAGS =
DEBUG_LIBS =
endif

INCLUDES = \
	-DDOCDIR=\"$(docdir)\" \
	$(DEBUG_CFLAGS) \
	@GTK_CFLAGS@ \
	@GTKGL_CFLAGS@ \
	@GL_CFLAGS@ \
	-DLOCALE_DIR=\"@localedir@\"

bin_PROGRAMS = fsv2
noinst_HEADERS = \
	about.h \
	animation.h \
	camera.h \
	colexp.h \
	color.h \
	common.h \
	dirtree.h \
	filelist.h \
	fsv.h \
	fsvwindow.h \
	fsv3d.h \
	geometry.h \
	ogl.h \
	scanfs.h \
	tmaptext.h \
	viewport.h \
	property_dlg.h \
	options_dlg.h

fsv2_SOURCES = \
	fsv.cxx \
	fsvwindow.cxx \
	dirtree.cxx \
	filelist.cxx \
	property_dlg.cxx \
	options_dlg.cxx \
	about.c \
	animation.c \
	camera.c \
	colexp.c \
	color.c \
	common.c \
	geometry.c \
	ogl.c \
	scanfs.c \
	tmaptext.c \
	viewport.c

fsv2_LDADD = \
	$(DEBUG_LIBS) \
	@GTKGL_LIBS@ \
	@GL_LIBS@ \
	@GTK_LIBS@ \
	@FTGL_LIBS@

EXTRA_DIST = \
	xmaps/birdseye_view.xpm \
	xmaps/blockdev.xpm \
	xmaps/chardev.xpm \
	xmaps/charset.xbm \
	xmaps/fifo.xpm \
	xmaps/fsv-icon.xpm \
	xmaps/mini-blockdev.xpm \
	xmaps/mini-chardev.xpm \
	xmaps/mini-fifo.xpm \
	xmaps/mini-socket.xpm \
	xmaps/mini-symlink.xpm \
	xmaps/mini-unknown.xpm \
	xmaps/socket.xpm \
	xmaps/symlink.xpm \
	xmaps/unknown.xpm
