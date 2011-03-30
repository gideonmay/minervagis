SUBDIRS = gaiageo \
	spatialite \
	shell \
	headers

INCLUDES = @CFLAGS@
INCLUDES += -I$(top_srcdir)/src/headers

lib_LTLIBRARIES = libspatialite.la

libspatialite_la_LDFLAGS = -version-info 2:0:0 -no-undefined

libspatialite_la_SOURCES = version.c

libspatialite_la_LIBADD = \
	./spatialite/libspatialite.la \
	./gaiageo/libgaiageo.la \
	/usr/local/lib/libgeos_c.la \
	/usr/local/lib/libgeos.la \
	/usr/local/lib/libproj.la 

if MINGW
LDADD = -lm -lstdc++
else
LDADD = -lm -lstdc++ -lpthread -ldl
endif

