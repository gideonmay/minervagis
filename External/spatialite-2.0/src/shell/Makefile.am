bin_PROGRAMS = spatialite

INCLUDES = @CFLAGS@
INCLUDES += -I$(top_srcdir)/src/headers

spatialite_SOURCES = shell.c shapefiles.c

if MINGW
LDADD = ../spatialite/.libs/libspatialite.a \
	../gaiageo/.libs/libgaiageo.a \
	/usr/local/lib/libgeos_c.a \
	/usr/local/lib/libgeos.a \
	/usr/local/lib/libproj.a \
	-lm -lstdc++
else
LDADD = ../spatialite/.libs/libspatialite.a \
	../gaiageo/.libs/libgaiageo.a \
	/usr/local/lib/libgeos_c.a \
	/usr/local/lib/libgeos.a \
	/usr/local/lib/libproj.a \
	-lm -lstdc++ -lpthread -ldl
endif
