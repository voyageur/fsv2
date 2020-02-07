if DEBUG
DEBUG_CFLAGS = -I$(top_srcdir)/debug
DEBUG_LIBS = $(top_builddir)/debug/libdebug.a
else
DEBUG_CFLAGS =
DEBUG_LIBS =
endif

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
	about.c \
	animation.cxx \
	ogl.cxx \
	fsv.cxx \
	fsvwindow.cxx \
	dirtree.cxx \
	filelist.cxx \
	property_dlg.cxx \
	options_dlg.cxx \
	color.cxx \
	camera.cxx \
	colexp.cxx \
	common.cxx \
	geometry.cxx \
	scanfs.cxx \
	viewport.cxx
#	tmaptext.c

fsv2_DEPENDENCIES =

fsv2_LDADD = \
	../uilib/libfsvui.la \
	$(DEBUG_LIBS) \
	$(GTK_LIBS) \
	$(GTKGL_LIBS) \
	$(FTGL_LIBS) \
	$(GNOMEUI_LIBS)

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

AM_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(GTKGL_CFLAGS) \
	$(FTGL_CFLAGS) \
	$(GNOMEUI_CFLAGS) \
	-DDOCDIR=\"$(docdir)\" \
	$(DEBUG_CFLAGS) \
	@GTK_CFLAGS@ \
	@GTKGL_CFLAGS@ \
	@GL_CFLAGS@ \
	-DLOCALE_DIR=\"@localedir@\"
